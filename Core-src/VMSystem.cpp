/*
   	Copyright (c) 2003, Xentronix
	Author: Frans van Nispen (frans@xentronix.com)
	All rights reserved.
	
	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:
	
	Redistributions of source code must retain the above copyright notice, this list
	of conditions and the following disclaimer. Redistributions in binary form must
	reproduce the above copyright notice, this list of conditions and the following
	disclaimer in the documentation and/or other materials provided with the distribution. 
	
	Neither the name of Xentronix nor the names of its contributors may be used
	to endorse or promote products derived from this software without specific prior
	written permission. 
	
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
	SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// This is experimental code !

#include "Globals.h"
#ifdef __VM_SYSTEM

//////////////////////////////////////////////////////////////
// Virtual Memory system with cache
// by Frans van Nispen

#include <stdlib.h>
#include <stdio.h>

#include "VMSystem.h"

VMSystem VM;

// ============================================================
VMSystem::VMSystem()
{
	m_lCurrentPointer = 0;
	m_lPlayPointer = 0;
	m_lCachePointer = 0;
	
	fCache = NULL;

	myThread = spawn_thread(Thread2_,"FillCache", B_NORMAL_PRIORITY, (void *)this);
	if(myThread < 0){
		debugger("FillCache thread failed!");
	}
	resume_thread(myThread);
}

// ============================================================
VMSystem::~VMSystem()
{
	kill_thread(myThread);

	BEntry e(&file_ref);
	e.Remove();
	delete_sem(m_semVM);
	
	if (fCache)
		delete[] fCache;
}

// ============================================================
void VMSystem::Init()
{
	m_lock_read = true;		// do not run the loader

	// allocate cache
	if (fCache)	delete[] fCache;
	fCache = new float[CACHE_SIZE + CACHE_HALF];

	// create a temp file
	app_info info;
	be_app->GetAppInfo(&info);

	// set up the temporary disk cache for the history
	// check some stuff and get it readu to go
	BString str;
	str = Prefs.temp_dir;
	str << "/BeAECache";
	str << info.thread;						// make the file unique

	BPath path;
	path.SetTo(str.String());
   
	CacheFile = new BFile(path.Path(), B_READ_WRITE|B_CREATE_FILE|B_ERASE_FILE);//|B_FAIL_IF_EXISTS
	if(CacheFile->InitCheck() != B_OK){
		// No file ... no undo i gess
	}else{
		// good to go
	}
   
	BEntry e(path.Path());
	e.GetRef(&file_ref);
	
	if((m_semVM = create_sem(1, "VM Sem")) < 0){
//		debugger(CREATE_SEM_FAIL_MSG);
	}

	// reset parameters
	m_lCurrentPointer = 0;
}


// ============================================================
// Fill the cache
int32 VMSystem::FillCache()
{
	while(1){
	if (!m_lock_read){
		if (m_nReadCachePos == 0)		// read first half
		{
			if (m_lPlayPointer >= m_lCachePointer + CACHE_THRES)
			{
				CacheFile->Seek( (m_lCachePointer+CACHE_HALF)*4, SEEK_SET);
				CacheFile->Read((void*)(fCache + CACHE_HALF), CACHE_HALF*4);
				m_nReadCachePos = 1;
			}
		}
		else							// second half
		{
			if (m_lPlayPointer >= m_lCachePointer + CACHE_SIZE-CACHE_THRES)
			{
				CacheFile->Seek( m_lPlayPointer*4, SEEK_SET);
				CacheFile->Read((void*)fCache, CACHE_HALF*4);

//				memcpy( (void*)(fCache + CACHE_SIZE), (void*)fCache, 8192);

				m_lCachePointer = m_lPlayPointer;
				m_nReadCachePos = 0;
			}
		}
	}
	snooze(100000);
	}
}

// ============================================================
void VMSystem::ReadCache(float *output, size_t size)
{
	int32 index = m_lPlayPointer - m_lCachePointer;
	for (size_t i = 0; i < size; i++)
	{
		*output++ = fCache[ index++ ];
	}

//	memcpy( (void*)output, (void*)(fCache+index), size*4);
 
	m_lPlayPointer += size;
}

// Set the playpointer and init cache
void VMSystem::SetPlayPointer(int64 p)
{
	m_lock_read = true;	// stop caching

	m_lPlayPointer = p;
	CacheFile->Seek( m_lPlayPointer*4, SEEK_SET);
	CacheFile->Read((void*)fCache, CACHE_SIZE*4);
	m_lCachePointer = p;
	m_nReadCachePos = 0;

	m_lock_read = false;// start caching
}

int64 VMSystem::PlayPointer()
{
	return m_lPlayPointer;
}

void VMSystem::StopCache()
{
	m_lock_read = true;	// stop caching
}

// ============================================================
// Set the cursor
void VMSystem::SetPointer(int64 p)
{
	m_lCurrentPointer = p;
	CacheFile->Seek( m_lCurrentPointer*4, SEEK_SET);
}

// ============================================================
int64 VMSystem::Pointer()
{
	return m_lCurrentPointer;
}

// ============================================================
// Return sample at current cursor
float VMSystem::Read()
{
	float fTemp;
	CacheFile->Read((void*)&fTemp, sizeof(fTemp));
	m_lCurrentPointer++;
	return fTemp;
}

// ============================================================
void VMSystem::Write(float fTemp)
{
	CacheFile->Write((void*)&fTemp, sizeof(fTemp));
	m_lCurrentPointer++;
}

// ============================================================
float VMSystem::ReadAt(int64 p)
{
	float fTemp;
	SetPointer(p);
	CacheFile->Read((void*)&fTemp, sizeof(fTemp));
	m_lCurrentPointer++;
	return fTemp;
}

// ============================================================
void VMSystem::WriteAt(int64 p, float fTemp)
{
	SetPointer(p);
	CacheFile->Write((void*)&fTemp, sizeof(fTemp));
	m_lCurrentPointer++;
}

// ============================================================
void VMSystem::ReadBlock(float *output, size_t size)
{
	CacheFile->Read((void*)output, size*4);
	m_lCurrentPointer += size;
}

// ============================================================
void VMSystem::WriteBlock(float *input, size_t size)
{
	CacheFile->Write((void*)input, size*4);
	m_lCurrentPointer += size;
}

// ============================================================
void VMSystem::ReadBlockAt(int64 p, float *output, size_t size)
{
	SetPointer(p);
	CacheFile->Read((void*)output, size*4);
	m_lCurrentPointer += size;
}

// ============================================================
void VMSystem::WriteBlockAt(int64 p, float *input, size_t size)
{
	SetPointer(p);
	CacheFile->Write((void*)input, size*4);
	m_lCurrentPointer += size;
}

// ============================================================
// close file and create new one
void VMSystem::Reset()
{
	CacheFile->SetSize(0);
	SetPointer(0);
}

#endif
