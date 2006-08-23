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

#ifndef _VM_FILE_H
#define _VM_FILE_H
#include <stdlib.h>
#include <stdio.h>

#include "Globals.h"
#define CACHE_SIZE	1024*64	// 256Kb
#define CACHE_HALF	CACHE_SIZE/2
#define CACHE_THRES	CACHE_SIZE/4

class VMSystem {
  public:
	VMSystem();
	~VMSystem();
	
	// chache size in bytes
	void Init();

	// cursor management
	void SetPointer(int64 p);		// Set the current pointer
	int64 Pointer();				// Get pointer

	// manage play cache
	void	SetPlayPointer(int64 p);// Set the current play pointer and initiase cache
	int64	PlayPointer();				// Get play pointer
	void	ReadCache(float*, size_t size);
	void	StopCache();

	// read/write data
	float	Read();					// read a sample
	void	Write(float);			// write sample
	float	ReadAt(int64);			// read a sample
	void	WriteAt(int64,float);	// write sample
	
	// read/write memory blocks, size in samples
	void	ReadBlock(float*, size_t size);
	void	WriteBlock(float*, size_t size);
	void	ReadBlockAt(int64, float*, size_t size);
	void	WriteBlockAt(int64, float*, size_t size);

	void	Reset();				// delete file and create a new one, flush

  private:
	static	int32 Thread2_(void *data){ return ((VMSystem*)(data))->FillCache();  }
	int32	FillCache();
	thread_id myThread;
	bool	m_lock_read;
	int32	m_nReadCachePos;

	int64	m_lPlayPointer;		// current playback cache
	int64	m_lCachePointer;		// current cache position
	float	*fCache;			// cache buffer

	BFile *CacheFile;				//  the file to contain the cache
	entry_ref file_ref;

  	int64	m_lCurrentPointer;		// pointer in samples
	sem_id	m_semVM;
};

extern VMSystem VM; // Included so you don't have too 
#endif
