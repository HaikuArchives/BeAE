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

#include <Application.h>
#include <Entry.h>
#include <Path.h>
#include <File.h>
#include <Volume.h>
#include <stdio.h>

#include "CommonPool.h"
#include "History.h"
#include "MyClipBoard.h"

History Hist;

#define BLOCK_SIZE	512*1024

/*******************************************************
*   
*******************************************************/
History::History()
{
}

/*******************************************************
*   
*******************************************************/
void History::Init()
{
	app_info info;
	be_app->GetAppInfo(&info);

	// set up the temporary disk cache for the history
	// check some stuff and get it readu to go
	BString str;
	str = Prefs.temp_dir;
	str << "/BeAEUndo";
	str << info.thread;						// make the file unique
   
	BPath path;
	path.SetTo(str.String());
   
	HistoryFile = new BFile(path.Path(), B_READ_WRITE|B_CREATE_FILE|B_ERASE_FILE);//|B_FAIL_IF_EXISTS
	if(HistoryFile->InitCheck() != B_OK){
		// No file ... no undo i gess
	}else{
		// good to go
	}
   
	BEntry e(path.Path());
	e.GetRef(&file_ref);
	
	m_undo = false;
	m_redo = false;
   
	if((histSem = create_sem(1, "History Sem")) < 0){
//		debugger(CREATE_SEM_FAIL_MSG);
	}
}

/*******************************************************
*   
*******************************************************/
History::~History(){
	if(HistoryFile){
		delete HistoryFile;
	}

	BEntry e(&file_ref);
	e.Remove();
	delete_sem(histSem);
}

/*******************************************************
*   
*******************************************************/
bool History::Save(uint32 action, uint32 start, uint32 end){
	if (Pool.size == 0)	return false;
	acquire_sem(histSem);

	BVolume vol(file_ref.device);
	if(vol.FreeBytes() < ((end-start)*4+(Prefs.keep_free*1024*1224))){
		(new BAlert(NULL,Language.get("UNDO_ERROR"),Language.get("OK")))->Go();
		release_sem(histSem);
		return false;
	}
   
	HistoryFile->Seek(0, SEEK_SET);
	uint32 size = (end-start+1)*4*Pool.sample_type;
	
	switch(action){
	case H_PASTE:
		HistoryFile->Write(&action,sizeof(&action));
		HistoryFile->Write(&start,sizeof(&start));
		HistoryFile->Write(&end,sizeof(&end));
		HistoryFile->Write(&Pool.l_pointer,sizeof(&Pool.l_pointer));
		HistoryFile->Write(&Pool.r_pointer,sizeof(&Pool.r_pointer));
		break;
	case H_FULL:
		HistoryFile->Write(&action,sizeof(&action));
		HistoryFile->Write(&Pool.selection,sizeof(&Pool.selection));
		HistoryFile->Write(&end,sizeof(&end));
		HistoryFile->Write(&Pool.pointer,sizeof(&Pool.pointer));
		HistoryFile->Write(&Pool.l_pointer,sizeof(&Pool.l_pointer));
		HistoryFile->Write(&Pool.r_pointer,sizeof(&Pool.r_pointer));
		HistoryFile->Write(&Pool.r_sel_pointer,sizeof(&Pool.r_sel_pointer));	// needed for cut
		HistoryFile->Write(&Pool.sample_bits,sizeof(&Pool.sample_bits));
		HistoryFile->Write(&Pool.frequency,sizeof(&Pool.frequency));
		HistoryFile->Write(&Pool.sample_type,sizeof(&Pool.sample_type));
		HistoryFile->Write(&Pool.m_format,sizeof(&Pool.m_format));

		if (size < BLOCK_SIZE*4){					// files smaller than 256Kb without progressBar
			HistoryFile->Write((void*)(Pool.sample_memory), size);
		}else{
			Pool.StartProgress(Language.get("SAVE_UNDO"), size);
			char *p = (char*)(Pool.sample_memory);
			while(size>=BLOCK_SIZE){
				HistoryFile->Write((void*)p, BLOCK_SIZE);
				p+=BLOCK_SIZE;
				size-=BLOCK_SIZE;
				Pool.ProgressUpdate( BLOCK_SIZE );
			}
			if (size)
				HistoryFile->Write((void*)p, size);
			Pool.HideProgress();
		}
		break;
	case H_REPLACE:			// just 1:1, like undo for a filter
	case H_DELETE:			// for cut, need an insert actio to restore
		HistoryFile->Write(&action,sizeof(&action));
		HistoryFile->Write(&Pool.selection,sizeof(&Pool.selection));
		HistoryFile->Write(&start,sizeof(&start));
		HistoryFile->Write(&end,sizeof(&end));
		HistoryFile->Write(&Pool.r_sel_pointer,sizeof(&Pool.r_sel_pointer));	// needed for cut
		HistoryFile->Write(&Pool.r_pointer,sizeof(&Pool.r_pointer));
		HistoryFile->Write(&Pool.l_pointer,sizeof(&Pool.l_pointer));
		HistoryFile->Write(&Pool.pointer,sizeof(&Pool.pointer));
		if (size < BLOCK_SIZE*4){					// files smaller than 256Kb without progressBar
			HistoryFile->Write((void*)(Pool.sample_memory+start*Pool.sample_type), size);
		}else{
			Pool.StartProgress(Language.get("SAVE_UNDO"), size);
			char *p = (char*)(Pool.sample_memory+start*Pool.sample_type);
			while(size>=BLOCK_SIZE){
				HistoryFile->Write((void*)p, BLOCK_SIZE);
				p+=BLOCK_SIZE;
				size-=BLOCK_SIZE;
				Pool.ProgressUpdate( BLOCK_SIZE );
			}
			if (size)
				HistoryFile->Write((void*)p, size);
			Pool.HideProgress();
		}
		break;
	}

	m_undo = true;

	release_sem(histSem);
	return true;
}

/*******************************************************
*   Restore. This also handles the ReDo data
*******************************************************/
bool History::Restore(){
	if (!m_undo)	return false;			// no undo data at all
	Pool.mainWindow->PostMessage(TRANSPORT_STOP);	// can not use playing here

/*
 *	@TODO: Add Redo Code
*/

	uint32 action;
	HistoryFile->Seek(0, SEEK_SET);
	HistoryFile->Read(&action,sizeof(&action));

	uint32 start, end, size=0;

	if (action == H_DELETE || action == H_REPLACE){
		HistoryFile->Read(&Pool.selection,sizeof(&Pool.selection));
		HistoryFile->Read(&start,sizeof(&start));
		HistoryFile->Read(&end,sizeof(&end));
		HistoryFile->Read(&Pool.r_sel_pointer,sizeof(&Pool.r_sel_pointer));
		HistoryFile->Read(&Pool.r_pointer,sizeof(&Pool.r_pointer));
		HistoryFile->Read(&Pool.l_pointer,sizeof(&Pool.l_pointer));
		HistoryFile->Read(&Pool.pointer,sizeof(&Pool.pointer));
	}

	switch(action){
	case H_PASTE:
{		HistoryFile->Read(&start,sizeof(&start));
		HistoryFile->Read(&end,sizeof(&end));
		HistoryFile->Read(&Pool.l_pointer,sizeof(&Pool.l_pointer));
		HistoryFile->Read(&Pool.r_pointer,sizeof(&Pool.r_pointer));
		Pool.pointer = start;
		Pool.r_sel_pointer = end;
		Pool.selection = BOTH;
		bool old = Prefs.save_undo;
		Prefs.save_undo = false;
		ClipBoard.DoCut();
		Prefs.save_undo = old;
		Pool.selection = NONE;
}		break;
	case H_DELETE:			// for cut, need an insert actio to restore
{		// data needs to be inserted
		int64 old_size = Pool.size;
		if (Pool.r_pointer==Pool.size)			// if full view, keep full view
			Pool.r_pointer += (end - start +1);
		Pool.size += (end - start +1);		// adjust size
		// add memory
		float *p = (float*)realloc(Pool.sample_memory, Pool.size*Pool.sample_type*4 +1024);
		if (p){
			Pool.sample_memory = p;		// new block
		}else{
			(new BAlert(NULL,Language.get("UNDO_MEM_ERROR"),Language.get("OK")))->Go();
			Pool.size = old_size;
			if (Pool.r_pointer>Pool.size)
				Pool.r_pointer = Pool.size;
			break;
		}
		float *src = (float*)(Pool.sample_memory+old_size*Pool.sample_type);
		float *dst = (float*)(Pool.sample_memory+Pool.size*Pool.sample_type);
		float *stop = (float*)(Pool.sample_memory+end*Pool.sample_type);
		// copy forward
		while (dst>stop){
			*dst-- = *src--;
		}
}		// now read the file	
	case H_REPLACE:			// just 1:1, like undo for a filter, no sample_type change needed/possible
		size = (end-start+1)*4*Pool.sample_type;
		if (size < BLOCK_SIZE*4){					// files smaller than 256Kb without progressBar
			HistoryFile->Read((void*)(Pool.sample_memory+start*Pool.sample_type), size);
		}else{
			Pool.StartProgress(Language.get("RESTORE_UNDO"), size);
			char *p = (char*)(Pool.sample_memory+start*Pool.sample_type);
			while(size>=BLOCK_SIZE){
				HistoryFile->Read((void*)p, BLOCK_SIZE);
				p+=BLOCK_SIZE;
				size-=BLOCK_SIZE;
				Pool.ProgressUpdate( BLOCK_SIZE );
			}
			if (size)
				HistoryFile->Read((void*)p, size);

			Pool.HideProgress();
		}
		break;
	case H_FULL:
		HistoryFile->Read(&Pool.selection,sizeof(&Pool.selection));
		HistoryFile->Read(&end,sizeof(&end));
		HistoryFile->Read(&Pool.pointer,sizeof(&Pool.pointer));
		HistoryFile->Read(&Pool.l_pointer,sizeof(&Pool.l_pointer));
		HistoryFile->Read(&Pool.r_pointer,sizeof(&Pool.r_pointer));
		HistoryFile->Read(&Pool.r_sel_pointer,sizeof(&Pool.r_sel_pointer));
		HistoryFile->Read(&Pool.sample_bits,sizeof(&Pool.sample_bits));
		HistoryFile->Read(&Pool.frequency,sizeof(&Pool.frequency));
		HistoryFile->Read(&Pool.sample_type,sizeof(&Pool.sample_type));
		HistoryFile->Read(&Pool.m_format,sizeof(&Pool.m_format));

		size = (end+1)*4*Pool.sample_type;
		Pool.size = end;

		free(Pool.sample_memory);
		Pool.sample_memory = (float*)malloc(Pool.size*Pool.sample_type*4 +1024);

		if (size < BLOCK_SIZE*4){					// files smaller than 256Kb without progressBar
			HistoryFile->Read((void*)(Pool.sample_memory), size);
		}else{
			Pool.StartProgress(Language.get("RESTORE_UNDO"), size);
			char *p = (char*)(Pool.sample_memory);
			while(size>=BLOCK_SIZE){
				HistoryFile->Read((void*)p, BLOCK_SIZE);
				p+=BLOCK_SIZE;
				size-=BLOCK_SIZE;
				Pool.ProgressUpdate( BLOCK_SIZE );
			}
			if (size)
				HistoryFile->Read((void*)p, size);
			
			Pool.HideProgress();
		}
		break;
	}
	
	m_undo = false;
	m_redo = true;

	return true;
}

/*******************************************************
*   ReDo
*******************************************************/
bool History::ReDo(){
	if (!m_undo)	return false;			// no undo data at all
	Pool.mainWindow->PostMessage(TRANSPORT_STOP);	// can not use playing here

/*
 *	@TODO: Add Redo Code
*/

	m_redo = false;
	return true;
}

/*******************************************************
*   
*******************************************************/
bool History::HasUndo(){
	return m_undo;
}

/*******************************************************
*   
*******************************************************/
bool History::HasRedo(){
	return m_redo;
}

/*******************************************************
*   
*******************************************************/
void History::Reset(){
	m_undo = false;
}
