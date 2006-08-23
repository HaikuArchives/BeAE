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
#include <String.h>
#include <stdio.h>

#include "MyClipBoard.h"
#include "CommonPool.h"

MyClipBoard ClipBoard;

#define BLOCK_SIZE	512*1024
#define BLOCK_SIZE2	64*1024

/*******************************************************
*   
*******************************************************/
MyClipBoard::MyClipBoard()
{
}

/*******************************************************
*   
*******************************************************/
void MyClipBoard::Init()
{
	app_info info;
	be_app->GetAppInfo(&info);

	// set up the temporary disk cache for the history
	// check some stuff and get it readu to go
	BString str;
	str = Prefs.temp_dir;
	str << "/BeAEClipBoard";
//	str << info.thread;						// make the file unique

	BPath path;
	path.SetTo(str.String());

	BEntry e(path.Path());
	e.GetRef(&file_ref);
	
	if (e.Exists())	m_clip = true;
	else			m_clip = false;
   
	MyClipBoardFile = new BFile(path.Path(), B_READ_WRITE|B_CREATE_FILE);//|B_ERASE_FILE);
	if(MyClipBoardFile->InitCheck() != B_OK){
		// No file ... no undo i gess
	}else{
		// good to go
	}

	if (!m_clip){		// make sure the clip is invalid
		MyClipBoardFile->Seek(0, SEEK_SET);
		MyClipBoardFile->Write(&m_clip,sizeof(&m_clip));
	}
   
	if((clipSem = create_sem(1, "MyClipBoard Sem")) < 0){
//		debugger(CREATE_SEM_FAIL_MSG);
	}
}

/*******************************************************
*   
*******************************************************/
MyClipBoard::~MyClipBoard(){
	if(MyClipBoardFile){
		delete MyClipBoardFile;
	}

//	BEntry e(&file_ref);
//	e.Remove();
	delete_sem(clipSem);
}

/*******************************************************
*   
*******************************************************/
bool MyClipBoard::Copy(){
	if (Pool.selection == NONE)		return false;		// nothing to copy
	acquire_sem(clipSem);
   
// need to add 1 channel support for stereo files
	int32 size;
	int32 sample_type;

	if (Pool.sample_type==STEREO && Pool.selection!=BOTH){
		size = (Pool.r_sel_pointer-Pool.pointer+1)*4;			// only one channel
		sample_type = MONO;
	}else{	// Mono + Both
		size = (Pool.r_sel_pointer-Pool.pointer+1)*4*Pool.sample_type;
		sample_type = Pool.sample_type;
	}

	BVolume vol(file_ref.device);
	if(vol.FreeBytes() < (size+(Prefs.keep_free*1024*1224))){
		(new BAlert(NULL,Language.get("CLIP_ERROR"),Language.get("OK")))->Go();
		release_sem(clipSem);
		return false;
	}
   
	MyClipBoardFile->Seek(0, SEEK_SET);

	m_clip = true;
	MyClipBoardFile->Write(&m_clip,sizeof(&m_clip));		// is clip valid ?

	MyClipBoardFile->Write(&size,sizeof(&size));
	MyClipBoardFile->Write(&sample_type,sizeof(&sample_type));
	MyClipBoardFile->Write(&Pool.frequency,sizeof(&Pool.frequency));
	MyClipBoardFile->Write(&Pool.sample_bits,sizeof(&Pool.sample_bits));


	if (Pool.sample_type==STEREO && Pool.selection!=BOTH){
		if (size < BLOCK_SIZE2*2){					// files smaller than 256Kb without progressBar
			float *p = Pool.sample_memory + Pool.pointer*Pool.sample_type;	// start of clip
			if (Pool.selection==RIGHT)	p++;								// go to right channel
			for (int32 i=0; i<size; i+=4){
				MyClipBoardFile->Write(p, sizeof(p));
				p+=2;
			}
		}else{
			Pool.StartProgress(Language.get("SAVE_CLIP"), size);
			
			float *buffer = new float[BLOCK_SIZE2 / 4];
			
			float *p = Pool.sample_memory + Pool.pointer*Pool.sample_type;	// start of clip
			if (Pool.selection==RIGHT)	p++;								// go to right channel
			while(size>=BLOCK_SIZE2){
				for (int32 i=0; i<BLOCK_SIZE2; i+=4){
					buffer[i/4] = *p;
					p+=2;
				}
				MyClipBoardFile->Write(buffer, BLOCK_SIZE2);
				size-=BLOCK_SIZE2;
				Pool.ProgressUpdate( BLOCK_SIZE2 );
			}
			if (size){
				for (int32 i=0; i<size; i+=4){
					MyClipBoardFile->Write(p, sizeof(p));
					p+=2;
				}
			}

			delete buffer;
			Pool.HideProgress();
		}
	}else{							//============================ Mono + Both
		if (size < BLOCK_SIZE*4){					// files smaller than 2Mb without progressBar
			MyClipBoardFile->Write((void*)(Pool.sample_memory+Pool.pointer*Pool.sample_type), size);
		}else{
			Pool.StartProgress(Language.get("SAVE_CLIP"), size);
			char *p = (char*)(Pool.sample_memory+Pool.pointer*Pool.sample_type);
			while(size>=BLOCK_SIZE){
				MyClipBoardFile->Write((void*)p, BLOCK_SIZE);
				p+=BLOCK_SIZE;
				size-=BLOCK_SIZE;
				Pool.ProgressUpdate( BLOCK_SIZE );
			}
			if (size)
				MyClipBoardFile->Write((void*)p, size);

			Pool.HideProgress();
		}
	}

	release_sem(clipSem);
	Update();
	
	return true;
}

/*******************************************************
*   
*******************************************************/
void MyClipBoard::Cut(bool cut){
	if (Pool.selection == NONE)		return;			// nothing to copy
	Pool.mainWindow->PostMessage(TRANSPORT_STOP);	// can not use playing here
	if (Copy())											// copy to clipboard
	{
		if (cut){
			DoCut();
		}else{
			DoSilence();
		}

		Pool.selection=NONE;
		Pool.changed = true;
		Pool.ResetIndexView();
		Update();
		Pool.RedrawWindow();
	}
}

void MyClipBoard::DoSilence()
{
	acquire_sem(clipSem);

	Pool.SaveUndo();
	float *p = Pool.sample_memory + Pool.pointer*Pool.sample_type;
	float *end = Pool.sample_memory + Pool.r_sel_pointer*Pool.sample_type;

	int count = 10000;
	Pool.StartProgress(Language.get("WORKING"), end-p);

	if (Pool.sample_type == MONO || Pool.selection == BOTH){
		while (p<=end){
			*p++ = 0.0;
			if (count-- <0){count = 10000;	Pool.ProgressUpdate( 10000 );	}
		}
	}else{		// stereo
		switch(Pool.selection){
		case LEFT:
			while (p<=end){
				p[0] = 0.0;
				p+=2;
				if (count-- <0){count = 10000;	Pool.ProgressUpdate( 10000 );	}
			}
			break;
		case RIGHT:
			while (p<=end){
				p[1] = 0.0;
				p+=2;
				if (count-- <0){count = 10000;	Pool.ProgressUpdate( 10000 );	}
			}
			break;
		}
	}
	Pool.HideProgress();

	release_sem(clipSem);
}

void MyClipBoard::DoCut(){
	acquire_sem(clipSem);

	// Now remove the selected part
	if (Pool.sample_type==STEREO && (Pool.selection==LEFT || Pool.selection==RIGHT)){
		if (Prefs.save_undo)	Hist.Save(H_REPLACE, Pool.pointer, Pool.size);		// save the part to cut
		if (Pool.r_sel_pointer != Pool.size){		// need to copy ?
			// copy channel back
			float *src = Pool.sample_memory + (Pool.r_sel_pointer+1)*Pool.sample_type;
			float *dst = Pool.sample_memory + (Pool.pointer)*Pool.sample_type;
			if (Pool.selection==RIGHT){		// go to right channel
				src++;
				dst++;
			}
			float *end = Pool.sample_memory + Pool.size*Pool.sample_type;
			while (src<=end){
				*dst = *src;	dst+=2;		src+=2;
			}
		}
		// now wipe till end
		float *p = Pool.sample_memory + (Pool.pointer + Pool.size - Pool.r_sel_pointer)*Pool.sample_type;
		if (Pool.selection==RIGHT)	p++;		// go to right channel
		float *end = Pool.sample_memory + Pool.size*Pool.sample_type;
		while (p <= end){
			*p=0;	p+=2;
		}
	}else{
		if (Prefs.save_undo)	Hist.Save(H_DELETE, Pool.pointer, Pool.r_sel_pointer);		// save the part to cut
		if (Pool.r_sel_pointer != Pool.size){		// need to copy ?
			// copy channel back
			float *src = Pool.sample_memory + (Pool.r_sel_pointer+1)*Pool.sample_type;
			float *dst = Pool.sample_memory + (Pool.pointer)*Pool.sample_type;
			float *end = Pool.sample_memory + Pool.size*Pool.sample_type;
			while (src<=end){
				*dst = *src;	dst++;		src++;
			}
			for (int i=0; i<255; i++)		// now delete 1Kb additional memory as reserved to prevent crashes
				*src++=0;
		}
		// resize memory
		int32 w = Pool.r_pointer - Pool.l_pointer;
		Pool.size -= (Pool.r_sel_pointer - Pool.pointer +1);		// adjust size
		if (Pool.size<0)	Pool.size=0;
		if (Pool.r_pointer>Pool.size)	Pool.r_pointer = Pool.size;
		if (Pool.l_pointer>Pool.size)	Pool.l_pointer = Pool.r_pointer - w;
		if (Pool.l_pointer<0)			Pool.l_pointer  = 0;

		Pool.sample_memory = (float*)realloc(Pool.sample_memory, Pool.size*Pool.sample_type*4 +1024);
	}

	release_sem(clipSem);
}

/*******************************************************
*   
*******************************************************/
void MyClipBoard::Paste(){
// When a clip is selected, this needs to be removed.
// After the paste, the pasted piece needs to be the selected portion

	MyClipBoardFile->Seek(0, SEEK_SET);

	int32 size, cut_size = 0;
	int32 sample_type, sample_bits;
	float frequency;
	
	MyClipBoardFile->Read(&m_clip,sizeof(&m_clip));
	if (!m_clip)	return;

	Pool.mainWindow->PostMessage(TRANSPORT_STOP);		// can not use playing here
	MyClipBoardFile->Read(&size,sizeof(&size));
	MyClipBoardFile->Read(&sample_type,sizeof(&sample_type));
	MyClipBoardFile->Read(&frequency,sizeof(&frequency));
	MyClipBoardFile->Read(&sample_bits,sizeof(&Pool.sample_bits));

	// enable paste in empty instance
	if (Pool.size == 0)
	{
		Pool.sample_type = sample_type;
		Pool.frequency = frequency;
		Pool.selection = NONE;
		Pool.sample_bits = sample_bits;
		Pool.size = 0;//size/(4*sample_type);
		
		Pool.pointer = Pool.l_pointer = 0;
		Pool.r_pointer = Pool.size;

		Pool.sample_view_dirty = true;	// update the sample-view
		Pool.update_index = true;
	}
	
	int32 start = Pool.pointer;
	int32 end = Pool.pointer + size/(4*sample_type) - 1;

	if (Pool.size && Pool.selection == NONE && Prefs.save_undo)
		Hist.Save(H_PASTE, start, end);

	if (Pool.selection != NONE){		// first CUT the piece
		bool old = Prefs.save_undo;
		Prefs.save_undo = false;
		DoCut();
		Prefs.save_undo = old;
		if (Pool.selection == BOTH)
			Pool.selection = NONE;
		
		cut_size = Pool.r_sel_pointer - Pool.pointer +1;
	}

	if (Pool.selection == NONE){		// do a normal paste
	
		int32 old_size = Pool.size;
		if (Pool.r_pointer==Pool.size)			// if full view, keep full view
			Pool.r_pointer += (end - start +1);
		Pool.size += (end - start +1);		// adjust size
		// add memory
		float *p;
		if (Pool.sample_memory)
			p = (float*)realloc(Pool.sample_memory, Pool.size*Pool.sample_type*4 +1024);
		else
			p = (float*)malloc(Pool.size*Pool.sample_type*4 +1024);
			
		if (p){
			Pool.sample_memory = p;		// new block
		}else{
			(new BAlert(NULL,Language.get("MEM_ERROR"),Language.get("OK")))->Go();
			Pool.size = old_size;
			if (Pool.r_pointer>Pool.size)
				Pool.r_pointer = Pool.size;
			goto einde;
		}

		float *src = (float*)(Pool.sample_memory+old_size*Pool.sample_type);
		float *dst = (float*)(Pool.sample_memory+Pool.size*Pool.sample_type);
		float *stop = (float*)(Pool.sample_memory+end*Pool.sample_type);
		// copy forward
		while (dst>stop){
			*dst-- = *src--;
		}
		// now read the file
	
//		size = (end-start+1)*4*Pool.sample_type;
		if (sample_type == Pool.sample_type){			// 1:1
			if (size < BLOCK_SIZE*4){					// files smaller than 256Kb without progressBar
				MyClipBoardFile->Read((void*)(Pool.sample_memory+start*Pool.sample_type), size);
			}else{
				Pool.StartProgress(Language.get("PASTING"), size);
				char *p = (char*)(Pool.sample_memory+start*Pool.sample_type);
				while(size>=BLOCK_SIZE){
					MyClipBoardFile->Read((void*)p, BLOCK_SIZE);
					p+=BLOCK_SIZE;
					size-=BLOCK_SIZE;
					Pool.ProgressUpdate( BLOCK_SIZE );
				}
				if (size)
					MyClipBoardFile->Read((void*)p, size);

				Pool.HideProgress();
			}
		}else if (sample_type == STEREO && Pool.sample_type == MONO){
			Pool.StartProgress(Language.get("PASTE_MONO"), size);
			float *p = Pool.sample_memory + Pool.pointer*Pool.sample_type;	// start of clip
			float left, right;
			
			float *buffer = new float[BLOCK_SIZE2/4];
			while(size>=BLOCK_SIZE2){
				MyClipBoardFile->Read(buffer, BLOCK_SIZE2);
				for (int32 i=0; i<BLOCK_SIZE2; i+=8){
					left = buffer[i/8];
					right = buffer[i/8 +1];
					*p++ = (right+left)/2;
				}
				size-=BLOCK_SIZE2;
				Pool.ProgressUpdate( BLOCK_SIZE2 );
			}
			if (size){
				for (int32 i=0; i<size; i+=8){
					MyClipBoardFile->Read(&left, sizeof(&left));
					MyClipBoardFile->Read(&right, sizeof(&right));
					*p++ = (right+left)/2;
				}
			}
			delete buffer;
			Pool.HideProgress();
		}else if (Pool.sample_type == STEREO && sample_type == MONO){
			Pool.StartProgress(Language.get("PASTE_STEREO"), size);
			float *p = Pool.sample_memory + Pool.pointer*Pool.sample_type;	// start of clip
			float left;
			float *buffer = new float[BLOCK_SIZE2/4];
			while(size>BLOCK_SIZE2){
				MyClipBoardFile->Read(buffer, BLOCK_SIZE2);
				for (int32 i=0; i<BLOCK_SIZE2; i+=4){
					*p++ = buffer[i/4];
					*p++ = buffer[i/4];
				}
				size-=BLOCK_SIZE2;
				Pool.ProgressUpdate( BLOCK_SIZE2 );
			}
			if (size){
				for (int32 i=0; i<size; i+=4){
					MyClipBoardFile->Read(&left, sizeof(&left));
					*p++ = left;
					*p++ = left;
				}
			}
			delete buffer;
			Pool.HideProgress();
		}
		Pool.pointer = start;
		if (Prefs.select_after_paste){
			Pool.r_sel_pointer =end;
			Pool.selection = BOTH;
		}
	}
	// paste in a single channel of a stereo file
	else if (Pool.sample_type == STEREO && (Pool.selection == LEFT || Pool.selection == RIGHT))
	{
		int32 old_size = Pool.size;
		int32 clip_size = (end - start +1);
		int32 alter = clip_size - cut_size;
		if (alter < 0) alter = 0;

		if (Pool.r_pointer==Pool.size)			// if full view, keep full view
			Pool.r_pointer += alter;

		Pool.size += alter;		// adjust size
		// add memory
		float *p;
		if (Pool.sample_memory)
			p = (float*)realloc(Pool.sample_memory, Pool.size*Pool.sample_type*4 +1024);
		else
			p = (float*)malloc(Pool.size*Pool.sample_type*4 +1024);
			
		if (p){
			Pool.sample_memory = p;		// new block
		}else{
			(new BAlert(NULL,Language.get("MEM_ERROR"),Language.get("OK")))->Go();
			Pool.size = old_size;
			if (Pool.r_pointer>Pool.size)
				Pool.r_pointer = Pool.size;
			goto einde;
		}

		if (alter)	// enlargen the memory
		{
			float *src = (float*)(Pool.sample_memory+(old_size - cut_size)*Pool.sample_type) - (Pool.selection == RIGHT);
			float *dst = (float*)(Pool.sample_memory+Pool.size*Pool.sample_type) - (Pool.selection == RIGHT);
			float *stop = (float*)(Pool.sample_memory+end*Pool.sample_type);
		
			// copy forward
			while (dst>stop){
				*dst = *src;
				dst -= 2;
				src -= 2;
			}

			// wipe other channel
			src = (float*)(Pool.sample_memory+old_size*Pool.sample_type) - (Pool.selection == LEFT);
			dst = (float*)(Pool.sample_memory+Pool.size*Pool.sample_type + 200) - (Pool.selection == LEFT);
			
			while (src<dst){
				*src = 0;
				src += 2;
			}

			if (sample_type == MONO){
				Pool.StartProgress(Language.get("PASTE_MONO"), size);
				float *p = Pool.sample_memory + Pool.pointer*Pool.sample_type - (Pool.selection == RIGHT);	// start of clip
				float left;
				float *buffer = new float[BLOCK_SIZE2/4];
				while(size>=BLOCK_SIZE2){
					MyClipBoardFile->Read(buffer, BLOCK_SIZE2);
					for (int32 i=0; i<BLOCK_SIZE2; i+=4){
						*p = buffer[i/4];
						p += 2;
					}
					size-=BLOCK_SIZE2;
					Pool.ProgressUpdate( BLOCK_SIZE2 );
				}
				if (size){
					for (int32 i=0; i<size; i+=4){
						MyClipBoardFile->Read(&left, sizeof(&left));
						*p = left;
						p += 2;
					}
				}
				delete buffer;
				Pool.HideProgress();
			}else if (sample_type == STEREO){
				Pool.StartProgress(Language.get("PASTE_STEREO"), size);
				float *p = Pool.sample_memory + Pool.pointer*Pool.sample_type - (Pool.selection == RIGHT);	// start of clip
				float left, right;
				float *buffer = new float[BLOCK_SIZE2/4];
				while(size>=BLOCK_SIZE2){
					MyClipBoardFile->Read(buffer, BLOCK_SIZE2);
					for (int32 i=0; i<BLOCK_SIZE2; i+=8){
						left = buffer[i/8];
						right = buffer[i/8 +1];
						*p = (right+left)/2;
						p += 2;
					}
					size-=BLOCK_SIZE2;
					Pool.ProgressUpdate( BLOCK_SIZE2 );
				}
				if (size){
					for (int32 i=0; i<size; i+=8){
						MyClipBoardFile->Read(&left, sizeof(&left));
						MyClipBoardFile->Read(&right, sizeof(&right));
						*p = (right+left)/2;
						p += 2;
					}
				}
				delete buffer;
				Pool.HideProgress();
			}
		}
		else		// deleted part is bigger than pasted
		{		
			float *src = (float*)(Pool.sample_memory+(old_size-cut_size)*Pool.sample_type) + (Pool.selection == RIGHT);
			float *dst = (float*)(Pool.sample_memory+(Pool.size-cut_size+clip_size)*Pool.sample_type) + (Pool.selection == RIGHT);
			float *stop = (float*)(Pool.sample_memory+Pool.pointer*Pool.sample_type);
		
			// copy forward
			while (dst>=stop){
				*dst = *src;
				dst -= 2;
				src -= 2;
			}

			if (sample_type == MONO){
				Pool.StartProgress(Language.get("PASTE_MONO"), size);
				float *p = Pool.sample_memory + Pool.pointer*Pool.sample_type - (Pool.selection == RIGHT);	// start of clip
				float left;
				while(size>=BLOCK_SIZE2){
					for (int32 i=0; i<BLOCK_SIZE2; i+=4){
						MyClipBoardFile->Read(&left, sizeof(&left));
						*p = left;
						p += 2;
					}
					size-=BLOCK_SIZE2;
					Pool.ProgressUpdate( BLOCK_SIZE2 );
				}
				if (size){
					for (int32 i=0; i<size; i+=4){
						MyClipBoardFile->Read(&left, sizeof(&left));
						*p = left;
						p += 2;
					}
				}
				Pool.HideProgress();
			}else if (sample_type == STEREO){
				Pool.StartProgress(Language.get("PASTE_STEREO"), size);
				float *p = Pool.sample_memory + Pool.pointer*Pool.sample_type - (Pool.selection == RIGHT);	// start of clip
				float left, right;
				while(size>=BLOCK_SIZE2){
					for (int32 i=0; i<BLOCK_SIZE2; i+=8){
						MyClipBoardFile->Read(&left, sizeof(&left));
						MyClipBoardFile->Read(&right, sizeof(&right));
						*p = (right+left)/2;
						p += 2;
					}
					size-=BLOCK_SIZE2;
					Pool.ProgressUpdate( BLOCK_SIZE2 );
				}
				if (size){
					for (int32 i=0; i<size; i+=8){
						MyClipBoardFile->Read(&left, sizeof(&left));
						MyClipBoardFile->Read(&right, sizeof(&right));
						*p = (right+left)/2;
						p += 2;
					}
				}
				Pool.HideProgress();
			}
		}
		Pool.pointer = start;
		if (Prefs.select_after_paste)
			Pool.r_sel_pointer =end;
	}

einde:
	Pool.changed = true;
	Pool.ResetIndexView();
	Update();
	Pool.RedrawWindow();
}

/*******************************************************
*   
*******************************************************/
void MyClipBoard::PasteMix(){
	if (!m_clip)	return;
	Pool.mainWindow->PostMessage(TRANSPORT_STOP);		// can not use playing here

	Pool.changed = true;
	Pool.ResetIndexView();
	Update();
	Pool.RedrawWindow();
}

/*******************************************************
*   
*******************************************************/
bool MyClipBoard::HasClip(){
	MyClipBoardFile->Seek(0, SEEK_SET);
	MyClipBoardFile->Read(&m_clip,sizeof(&m_clip));

	return m_clip;
}

/*******************************************************
*   
*******************************************************/
void MyClipBoard::Reset(){
	m_clip = false;
	MyClipBoardFile->Seek(0, SEEK_SET);
	MyClipBoardFile->Write(&m_clip,sizeof(&m_clip));
}

/*******************************************************
*   
*******************************************************/
void MyClipBoard::Update(){
	BList *teams = new BList;
	app_info info;
	be_app->GetAppInfo(&info);	// get the signature of this app

	be_roster->GetAppList(info.signature, teams);	// get a list of all apps with the same signature
	for (int i=0; i<teams->CountItems(); i++)
	{
		team_id who = (team_id)teams->ItemAt(i);
		BMessenger app(NULL, who);
		app.SendMessage(UPDATE_MENU);
	}
	
	delete teams;
}
