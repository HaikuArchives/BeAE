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

#include <stdio.h>
#include <File.h>

#include "Globals.h"
#include "Filters.h"
#include "FilterDialogs.h"
#include "PeakFile.h"
#include "VMSystem.h"

// the filters with GUI
#include "RoomFilter.h"
#include "ReverbFilter.h"
#include "DelayFilter.h"
#include "BassBoostFilter.h"
#include "AmplifierFilter.h"
#include "CompressorFilter.h"
#include "NormalizeFilter.h"

#define FILTER_BLOCK	2048

float filmod[11][260]; // stores for 9 band EQ 
float filstor[11];     // more stores for 9 band EQ

int32 __Last_Filter = 0;
int32 __Last_FilterTmp = 0;
int32 __FilterCount = 0;		// nr of open filters

filter_info __FilterList[] = { 	
	{"---", FILTER_BOTH | FILTER_REALTIME | FILTER_GUI, 1},
	{"SILENCE", FILTER_BOTH, 1},
	{"---", FILTER_BOTH | FILTER_REALTIME | FILTER_GUI, 1},
	{"FADE_IN", FILTER_BOTH, 1},
	{"FADE_OUT", FILTER_BOTH, 1},
	{"NORMALIZE", FILTER_BOTH | FILTER_GUI, 2},
	{"---", FILTER_BOTH | FILTER_REALTIME | FILTER_GUI, 1},
	{"BASSBOOST", FILTER_BOTH | FILTER_REALTIME | FILTER_GUI, 1},
	{"COMPRESSOR", FILTER_BOTH | FILTER_REALTIME | FILTER_GUI, 1},
	{"AMPLIFIER", FILTER_BOTH | FILTER_REALTIME | FILTER_GUI, 1},
	{"---", FILTER_BOTH | FILTER_REALTIME | FILTER_GUI, 1},
	{"DELAY", FILTER_BOTH | FILTER_REALTIME | FILTER_GUI, 1},
	{"REVERB", FILTER_BOTH | FILTER_REALTIME | FILTER_GUI, 1},
	{"ROOM", FILTER_BOTH | FILTER_REALTIME | FILTER_GUI, 1},
	{"---", FILTER_BOTH | FILTER_REALTIME | FILTER_GUI, 1},
	{"INVERT", FILTER_BOTH, 1},
	{"SWAP", FILTER_STEREO, 1},
{NULL,0, 0} };

// reinstate the last filter using the tag from the prefs
void FiltersInit()
{
	int32 filter = 0;
	while(__FilterList[filter].name != NULL)
	{
		if (strcmp(__FilterList[filter].name, Prefs.repeat_tag.String()) == 0){
			__Last_Filter = filter;
			__Last_FilterTmp = filter;
			return;
		}
		filter++;
	}
}


// Create the filter object and return it's pointer
RealtimeFilter *CreateFilter(int32 filter)
{
	RealtimeFilter *pFilter = NULL;
	bool bRealtime = __FilterList[filter].type & FILTER_REALTIME;
	
	if (strcmp(__FilterList[filter].name, "ROOM") == 0)
		pFilter = new RoomWindow(bRealtime);

	else if (strcmp(__FilterList[filter].name, "DELAY") == 0)
		pFilter = new DelayWindow(bRealtime);

	else if (strcmp(__FilterList[filter].name, "REVERB") == 0)
		pFilter = new ReverbWindow(bRealtime);

	else if (strcmp(__FilterList[filter].name, "BASSBOOST") == 0)
		pFilter = new BassBoostFilter(bRealtime);

	else if (strcmp(__FilterList[filter].name, "INVERT") == 0)
		pFilter = new InvertFilter();

	else if (strcmp(__FilterList[filter].name, "SWAP") == 0)
		pFilter = new SwapFilter();

	else if (strcmp(__FilterList[filter].name, "AMPLIFIER") == 0)
		pFilter = new AmplifierFilter(bRealtime);

	else if (strcmp(__FilterList[filter].name, "COMPRESSOR") == 0)
		pFilter = new CompressorFilter(bRealtime);

	else if (strcmp(__FilterList[filter].name, "SILENCE") == 0)
		pFilter = new SilenceFilter();

	else if (strcmp(__FilterList[filter].name, "FADE_IN") == 0)
		pFilter = new FadeInFilter();

	else if (strcmp(__FilterList[filter].name, "FADE_OUT") == 0)
		pFilter = new FadeOutFilter();

	else if (strcmp(__FilterList[filter].name, "NORMALIZE") == 0)
		pFilter = new NormalizeFilter();

	// set the passes the filter needs
	if (pFilter)
		pFilter->SetPasses( __FilterList[filter].passes );

	return pFilter;
}

// here we need to run the filters. For the lite version it will just use
// the build in ones, while the other will use plug-ins
void RunFilter(const char *tag)
{
	int32 filter = 0;
	while(__FilterList[filter].name != NULL)
	{
		if (strcmp(__FilterList[filter].name, tag) == 0){
			RunFilter(filter);
			return;
		}
		filter++;
	}
}

void RunFilter(int32 filter)
{
	if (__FilterCount == PLAY_HOOKS/2)	return;

	__Last_FilterTmp = filter;
	RealtimeFilter *pFilter = CreateFilter(filter);

	if (pFilter)
	{
		__FilterCount++;		// another filter open

		// add the GUI
		BView *view = pFilter->ConfigView();
		if (view){
			// run with GUI
			
			// resize the window
			pFilter->ResizeTo( view->Bounds().Width(), view->Bounds().Height() + 40);
			// add the config view
			if (view->Bounds().Width()<FILTER_MIN_WIDTH)
				view->MoveBy( (FILTER_MIN_WIDTH - view->Bounds().Width())/2, 0);
			pFilter->ChildAt(0)->AddChild(view);
			pFilter->MoveTo( Pool.mainWindow->Frame().left +__FilterCount * 40 +40, Pool.mainWindow->Frame().top + 80 + __FilterCount * 20);
			
			pFilter->Run();			// start looper
			pFilter->InitFilter(Pool.system_frequency, 2);	// initiase
			pFilter->Start();		// start media buffers
			pFilter->Show();		// show to user
		}else{
			// run without GUI
			ExecuteFilter(pFilter);
		}
	}
}

void WriteBack(float *src, float *dest, int32 size)
{
	if (Pool.selection == BOTH || Pool.sample_type == MONO)
		memcpy( dest, src, size*4 );
	else
	if (Pool.selection == LEFT)		/* allways stereo */
	{
		for (int32 x = 0; x < size; x+=2)
			dest[x] = src[x];
	}
	else
	{
		src++;
		dest++;
		for (int32 x = 0; x < size; x+=2)
			dest[x] = src[x];
	}
}

void ExecuteFilter(RealtimeFilter *pFilter)
{
	if (!pFilter)	return;

	__FilterCount--;		// another filter closed
	
	pFilter->Lock();
	if (pFilter->IsHidden()){
		// run without GUI
	}
	else{
		pFilter->Hide();
		pFilter->Stop();
		pFilter->DeAllocate();		// remove buffers used for realtime effect
	}

	if (Pool.PrepareFilter()){
		// init with track data
		
		float *filter_buffer = new float[FILTER_BLOCK];
		
		int32 size = (Pool.r_sel_pointer - Pool.pointer +1)*Pool.sample_type;
		Pool.StartProgress(Language.get("WORKING"), size * pFilter->Passes());

#ifndef __VM_SYSTEM	// RAM
		for (int32 filter_pass = 0; filter_pass < pFilter->Passes(); filter_pass++)
		{
			int32 size = (Pool.r_sel_pointer - Pool.pointer +1)*Pool.sample_type;
			float *p = Pool.sample_memory + Pool.pointer*Pool.sample_type;
			pFilter->InitFilter(Pool.system_frequency, Pool.sample_type, filter_pass, size);

			while(size >= FILTER_BLOCK){
				memcpy( filter_buffer, p, FILTER_BLOCK*4 );	// get the data
				pFilter->FilterBuffer( filter_buffer, FILTER_BLOCK);
				WriteBack( filter_buffer, p, FILTER_BLOCK);
				p+=FILTER_BLOCK;
				size-=FILTER_BLOCK;
				Pool.ProgressUpdate( FILTER_BLOCK );
			}
			if (size)
			{
				memcpy( filter_buffer, p, size*4 );	// get the data
				pFilter->FilterBuffer( filter_buffer, size);
				WriteBack( filter_buffer, p, size);
			}
		}
#else	// VM
	// TODO:
#endif
		Pool.HideProgress();
		Pool.changed = true;
		
		delete filter_buffer;
	}

	// enable repeat action
	__Last_Filter = __Last_FilterTmp;
	Prefs.repeat_tag.SetTo(__FilterList[__Last_Filter].name);

	pFilter->DeAllocate();
	pFilter->Quit();

	Pool.ResetIndexView();
	Pool.UpdateMenu();
	Pool.RedrawWindow();
}

void CancelFilter(RealtimeFilter *pFilter)
{
	if (!pFilter)	return;

	__FilterCount--;		// another filter closed
	pFilter->Stop();
	pFilter->DeAllocate();
	pFilter->Lock();
	pFilter->Quit();
}

void RunLastFilter()
{
	RealtimeFilter *pFilter = CreateFilter(__Last_Filter);
	if (pFilter)
	{
		ExecuteFilter(pFilter);
	}
}

// ============================================================ Swap
SwapFilter::SwapFilter() : RealtimeFilter(NULL, false)
{}

void SwapFilter::FilterBuffer(float *buffer, size_t size)
{
	for (size_t i=0; i<size; i+=2){
		float tmp = buffer[i+0];
		buffer[i+0] = buffer[i+1];
		buffer[i+1] = tmp;
	}
}

// ============================================================ Invert
InvertFilter::InvertFilter() : RealtimeFilter(NULL, false)
{}

void InvertFilter::FilterBuffer(float *buffer, size_t size)
{
	for (size_t i=0; i<size; i++){
		*buffer++ = -*buffer;
	}
}

// ============================================================ Silence
SilenceFilter::SilenceFilter() : RealtimeFilter(NULL, false)
{}

void SilenceFilter::FilterBuffer(float *buffer, size_t size)
{
	for (size_t i=0; i<size; i++){
		*buffer++ = 0.0f;
	}
}

// ============================================================ FadeIn
FadeInFilter::FadeInFilter() : RealtimeFilter(NULL, false)
{}

void FadeInFilter::FilterBuffer(float *buffer, size_t size)
{
	for (size_t i=0; i<size; i++){
		*buffer++ = (*buffer)*count / m_total;
		count++;
	}
}

bool FadeInFilter::InitFilter(float f, int32 c, int32 pass, int32 size)
{
	RealtimeFilter::InitFilter(f, c, pass, size);
	count = 0;
	return true;
}

// ============================================================ FadeOut
FadeOutFilter::FadeOutFilter() : RealtimeFilter(NULL, false)
{}

void FadeOutFilter::FilterBuffer(float *buffer, size_t size)
{
	for (size_t i=0; i<size; i++){
		*buffer++ = (*buffer)*count / m_total;
		count--;
	}
}

bool FadeOutFilter::InitFilter(float f, int32 c, int32 pass, int32 size)
{
	RealtimeFilter::InitFilter(f, c, pass, size);
	count = size;
	return true;
}

// ============================================================ Trim
void DoTrim()
{
	if (Pool.sample_type == NONE || Pool.selection == NONE)	return;
	Pool.mainWindow->PostMessage(TRANSPORT_STOP);		// stop playing

	if (Prefs.save_undo)	Hist.Save(H_FULL, 0, Pool.size);	// full undo

	if (Pool.pointer != 0){			// copy to begin of memory
		float *src = Pool.sample_memory + Pool.pointer*Pool.sample_type;
		float *dst = Pool.sample_memory;
		float *end = Pool.sample_memory + Pool.r_sel_pointer*Pool.sample_type;
		while(src<=end){
			*dst++ = *src++;
		}
	}
	// resize memory
	Pool.size = (Pool.r_sel_pointer - Pool.pointer);
	Pool.r_sel_pointer = Pool.size;
	Pool.r_pointer = Pool.size;
	Pool.pointer = 0;
	Pool.l_pointer = 0;

	Pool.sample_memory = (float*)realloc(Pool.sample_memory, Pool.size*Pool.sample_type*4 +1024);

	// wipe last piece
	float *p = Pool.sample_memory + (Pool.size+1)*Pool.sample_type;
	for (int i=0; i<256; i++)		// now delete 1Kb additional memory as reserved to prevent crashes
		*p++=0;

	Pool.changed = true;
	Pool.ResetIndexView();
	Pool.UpdateMenu();
	Pool.RedrawWindow();
}

#define BUFFER_SIZE		128*1024
#define BLOCK_SIZE		BUFFER_SIZE*4
/*******************************************************
*   
*******************************************************/
void DoResample()
{
	if (Pool.sample_type == NONE)	return;

	float *buffer = (float*)malloc(BUFFER_SIZE*4+4);
	if (!buffer)	return;	// error
	
	Pool.player->Stop();
	Pool.mainWindow->PostMessage(TRANSPORT_STOP);		// stop playing

	if (Prefs.save_undo)	Hist.Save(H_FULL, 0, Pool.size);	// full undo

	app_info info;
	be_app->GetAppInfo(&info);
	BString str("/tmp/BeAETmp");
	str << info.thread;						// make the file unique
	BPath path;
	path.SetTo(str.String());
   
	BFile *file = new BFile(path.Path(), B_READ_WRITE|B_CREATE_FILE|B_ERASE_FILE);//|B_FAIL_IF_EXISTS
	if(file->InitCheck() != B_OK){
		// errror
		return;
	}
	BEntry e(path.Path());

	float left = 0.0, right = 0.0, add = 0.0;
	float fraq = Pool.frequency/Prefs.filter_resample_freq;		// the convert frequency
	float *mem = Pool.sample_memory;
	float *end = Pool.sample_memory + Pool.size*Pool.sample_type;
	
	int32 count = 1000;
	int32 bit_convert = (1 << Prefs.filter_resample_bits)-1;

	Pool.size = (int64)ceil(Pool.size * Prefs.filter_resample_freq / Pool.frequency);

	int32 buffer_pointer = 0;

	Pool.StartProgress(Language.get("RESAMPLING"), Pool.size);
	if (Pool.sample_type == MONO && Prefs.filter_resample_mono){		// mono to mono
		while(mem<= end){
			left = *mem;
			// sample conversion
			left = floor(left * bit_convert)/bit_convert;
			
			buffer[buffer_pointer++] = left;
			if (buffer_pointer == BUFFER_SIZE){
				file->Write((void*)buffer, BUFFER_SIZE*4);
				buffer_pointer = 0;
			}
	
			add += fraq;
			while (add >= 1.0){
				++mem;
				--add;
			}
			if (count-- <0){count = 1000;	Pool.ProgressUpdate( 500 );	}
		}
		if (buffer_pointer){
			file->Write((void*)buffer, buffer_pointer*4);
		}
	}else if (Pool.sample_type == MONO && !Prefs.filter_resample_mono){			// mono to stereo
		float l_mul = bit_convert*Prefs.filter_resample_sl/100;
		float r_mul = bit_convert*Prefs.filter_resample_sr/100;
		while(mem<= end){
			left = *mem;
			right = *mem;
			
			// sample conversion
			left = floor(left * l_mul)/bit_convert;
			right = floor(right * r_mul)/bit_convert;
			
			buffer[buffer_pointer++] = MIN(left,1);
			buffer[buffer_pointer++] = MIN(right,1);
			if (buffer_pointer == BUFFER_SIZE){
				file->Write((void*)buffer, BUFFER_SIZE*4);
				buffer_pointer = 0;
			}
	
			add += fraq;
			while (add >= 1.0){
				++mem;
				--add;
			}
			if (count-- <0){count = 1000;	Pool.ProgressUpdate( 500 );	}
		}
		if (buffer_pointer){
			file->Write((void*)buffer, buffer_pointer*4);
		}
	}else if (Pool.sample_type == STEREO && !Prefs.filter_resample_mono){		// stereo to stereo
		while(mem<= end){
			// sample conversion
			left = floor(mem[0] * bit_convert)/bit_convert;
			right = floor(mem[1] * bit_convert)/bit_convert;
			
			buffer[buffer_pointer++] = left;
			buffer[buffer_pointer++] = right;
			if (buffer_pointer == BUFFER_SIZE){
				file->Write((void*)buffer, BUFFER_SIZE*4);
				buffer_pointer = 0;
			}

			add += fraq;
			while (add >= 1.0){
				mem+=2;
				--add;
			}
			if (count-- <0){count = 1000;	Pool.ProgressUpdate( 500 );	}
		}
		if (buffer_pointer){
			file->Write((void*)buffer, buffer_pointer*4);
		}
	}else if (Pool.sample_type == STEREO && Prefs.filter_resample_mono){		// stereo to mono
		float l_mul = bit_convert*Prefs.filter_resample_ml/100;
		float r_mul = bit_convert*Prefs.filter_resample_mr/100;
		while(mem<= end){
			// sample conversion
			left = floor(mem[0] * l_mul)/bit_convert + floor(mem[1] * r_mul)/bit_convert;
			
			buffer[buffer_pointer++] = MIN(left,1);
			if (buffer_pointer == BUFFER_SIZE){
				file->Write((void*)buffer, BUFFER_SIZE*4);
				buffer_pointer = 0;
			}

			add += fraq;
			while (add >= 1.0){
				mem+=2;
				--add;
			}
			if (count-- <0){count = 1000;	Pool.ProgressUpdate( 500 );	}
		}
		if (buffer_pointer){
			file->Write((void*)buffer, buffer_pointer*4);
		}
	}

// now reset memory and load the file
	Pool.frequency = Prefs.filter_resample_freq;
	if (Prefs.filter_resample_mono)
		Pool.sample_type = MONO;
	else
		Pool.sample_type = STEREO;

	Pool.sample_bits = Prefs.filter_resample_bits;
	
	Pool.m_format.u.raw_audio.frame_rate = Prefs.filter_resample_freq;
	Pool.m_format.u.raw_audio.channel_count = Pool.sample_type;
	if (Pool.sample_bits <= 8)							Pool.m_format.u.raw_audio.format = 0x11;	// 8bits
	if (Pool.sample_bits <= 16 && Pool.sample_bits >8)	Pool.m_format.u.raw_audio.format = 0x2;		// 16bits
	if (Pool.sample_bits <= 32 && Pool.sample_bits >16)	Pool.m_format.u.raw_audio.format = 0x24;	// 32bits (float)


	// resize mem to frequncy
	Pool.pointer = 0;
	Pool.l_pointer = 0;
	Pool.r_pointer = Pool.size;
	Pool.r_sel_pointer = Pool.size;
	Pool.selection = BOTH;

	free(Pool.sample_memory);
	Pool.sample_memory = (float*)malloc(Pool.size*Pool.sample_type*4 +1024);

	file->Seek(0, SEEK_SET);		// reset file
	end = Pool.sample_memory + Pool.size*Pool.sample_type;
	
	uint32 size = (Pool.size+1)*4*Pool.sample_type;

	char *p = (char*)(Pool.sample_memory);
	while(size>=BLOCK_SIZE){
		file->Read((void*)p, BLOCK_SIZE);
		p+=BLOCK_SIZE;
		size-=BLOCK_SIZE;
		Pool.ProgressUpdate( BLOCK_SIZE/(4*Pool.sample_type) );
	}
	if (size)
		file->Read((void*)p, size);

	Pool.HideProgress();

	if(file)	delete file;
	e.Remove();				// remove file

	free(buffer);

	// init the BSoundPlayer to the new frequency
	Pool.InitBufferPlayer( Pool.frequency );

	Pool.changed = true;
	Peak.Init( Pool.size+1, (Pool.sample_type == MONO) );	// Init peakfile
	Pool.ResetIndexView();
	Pool.UpdateMenu();
	Pool.RedrawWindow();
}



// ========================= Zero crossings
void ZeroLL()
{
	float left = 0, right = 0, tmpL = 0, tmpR = 0;

	switch(Pool.sample_type){
	case MONO:
		left = Pool.sample_memory[Pool.pointer];
		while (Pool.pointer>0){
			tmpL = Pool.sample_memory[Pool.pointer];
			if (left>0 && tmpL<0)	break;
			if (left<0 && tmpL>0)	break;
			Pool.pointer--;
		}
		break;
	case STEREO:
		left = Pool.sample_memory[Pool.pointer*2];
		right = Pool.sample_memory[Pool.pointer*2+1];
		while (Pool.pointer>0){
			tmpL = Pool.sample_memory[Pool.pointer*2];
			tmpR = Pool.sample_memory[Pool.pointer*2+1];
			if ((left>0 && tmpL<0) && (right>0 && tmpR<0))	break;
			if ((left<0 && tmpL>0) && (right<0 && tmpR>0))	break;
			Pool.pointer--;
		}
		break;
	}
}

void ZeroLR()
{
	float left = 0, right = 0, tmpL = 0, tmpR = 0;

	switch(Pool.sample_type){
	case MONO:
		left = Pool.sample_memory[Pool.pointer];
		while (Pool.pointer<Pool.size){
			tmpL = Pool.sample_memory[Pool.pointer];
			if (left>0 && tmpL<0)	break;
			if (left<0 && tmpL>0)	break;
			Pool.pointer++;
		}
		break;
	case STEREO:
		left = Pool.sample_memory[Pool.pointer*2];
		right = Pool.sample_memory[Pool.pointer*2+1];
		while (Pool.pointer<Pool.size){
			tmpL = Pool.sample_memory[Pool.pointer*2];
			tmpR = Pool.sample_memory[Pool.pointer*2+1];
			if ((left>0 && tmpL<0) && (right>0 && tmpR<0))	break;
			if ((left<0 && tmpL>0) && (right<0 && tmpR>0))	break;
			Pool.pointer++;
		}
		break;
	}
	if (Pool.pointer > Pool.r_sel_pointer)	Pool.selection = NONE;
}

void ZeroRL()
{
	if (Pool.selection==NONE)
		return;

	float left = 0, right = 0, tmpL = 0, tmpR = 0;

	switch(Pool.sample_type){
	case MONO:
		left = Pool.sample_memory[Pool.r_sel_pointer];
		while (Pool.r_sel_pointer>Pool.pointer){
			tmpL = Pool.sample_memory[Pool.r_sel_pointer];
			if (left>0 && tmpL<0)	break;
			if (left<0 && tmpL>0)	break;
			Pool.r_sel_pointer--;
		}
		break;
	case STEREO:
		left = Pool.sample_memory[Pool.r_sel_pointer*2];
		right = Pool.sample_memory[Pool.r_sel_pointer*2+1];
		while (Pool.r_sel_pointer>Pool.pointer){
			tmpL = Pool.sample_memory[Pool.r_sel_pointer*2];
			tmpR = Pool.sample_memory[Pool.r_sel_pointer*2+1];
			if ((left>0 && tmpL<0) && (right>0 && tmpR<0))	break;
			if ((left<0 && tmpL>0) && (right<0 && tmpR>0))	break;
			Pool.r_sel_pointer--;
		}
		break;
	}
}

void ZeroRR()
{
	if (Pool.selection==NONE){
		Pool.selection = BOTH;
		Pool.r_sel_pointer = Pool.pointer;
	}
	float left = 0, right = 0, tmpL = 0, tmpR = 0;

	switch(Pool.sample_type){
	case MONO:
		left = Pool.sample_memory[Pool.r_sel_pointer];
		while (Pool.r_sel_pointer<Pool.size){
			tmpL = Pool.sample_memory[Pool.r_sel_pointer];
			if (left>0 && tmpL<0)	break;
			if (left<0 && tmpL>0)	break;
			Pool.r_sel_pointer++;
		}
		break;
	case STEREO:
		left = Pool.sample_memory[Pool.r_sel_pointer*2];
		right = Pool.sample_memory[Pool.r_sel_pointer*2+1];
		while (Pool.r_sel_pointer<Pool.size){
			tmpL = Pool.sample_memory[Pool.r_sel_pointer*2];
			tmpR = Pool.sample_memory[Pool.r_sel_pointer*2+1];
			if ((left>0 && tmpL<0) && (right>0 && tmpR<0))	break;
			if ((left<0 && tmpL>0) && (right<0 && tmpR>0))	break;
			Pool.r_sel_pointer++;
		}
		break;
	}
}
