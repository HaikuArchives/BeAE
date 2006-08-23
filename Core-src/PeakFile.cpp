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

#include <stdlib.h>
#include <stdio.h>

#include "Globals.h"
#include "PeakFile.h"
#include "VMSystem.h"

CPeakFile Peak;

#define BUFFER_SIZE		64*256	// 64Kb

// ============================================================
CPeakFile::CPeakFile()
{
	buffer_left = NULL;
	buffer_right = NULL;
	buffer = NULL;
}

// ============================================================
CPeakFile::~CPeakFile()
{
	if (buffer)			delete[] buffer;
	if (buffer_left)	free(buffer_left);
	if (buffer_right)	free(buffer_right);
}

// ============================================================
void CPeakFile::Init(int32 size, bool mono)
{
	if (buffer)
		delete[] buffer;
	buffer = new float[BUFFER_SIZE];

	m_size = size;
	size = (size >> 7) + 1;
	m_mono = mono;

	int64 mem = size * 4 +16;	// 2 int16's for each channel
	if (!mono)	mem *= 2;

	int16 *p = (int16*)realloc(buffer_left, mem);
	if (p){
		buffer_left = p;				// new block
		memset( buffer_left, 0, mem);	// wipe buffer
	}else{
		(new BAlert(NULL,Language.get("MEM_ERROR"),Language.get("OK")))->Go();
		be_app->Quit();
	}
	
	if (!mono){
		int16 *p = (int16*)realloc(buffer_right, mem);
		if (p){
			buffer_right = p;				// new block
			memset( buffer_right, 0, mem);	// wipe buffer
		}else{
			(new BAlert(NULL,Language.get("MEM_ERROR"),Language.get("OK")))->Go();
			be_app->Quit();
		}
	}else{
		if (buffer_right)	free(buffer_right);
		buffer_right = NULL;
	}
}

// ============================================================
void CPeakFile::CreatePeaks(int32 start, int32 end, int32 progress)
{
	float min, max, max_r, min_r;
	int32 to, ii;

	start &= 0xfffffff8;	// mask off 1st 7 bits to round on 128 bytes
	int32 p_add = 0, p_count = 0, count = 0;
	if (progress){	// init progress process
		p_count = (end-start)/(100*128);
		p_add = progress/100;
		if (!m_mono)	p_add <<=1;
	}

	if (m_mono)		// mono
	{
		#ifndef __VM_SYSTEM	// RAM
			float *p = Pool.sample_memory;
		#else
			float *p = buffer;
			int32 index = 0;
			VM.ReadBlockAt(start, p, BUFFER_SIZE);
		#endif

		for (int32 i=start; i<=end; i+=128){
			min = max = 0.0;
			to = i+127;
			if (to>Pool.size)	to = Pool.size;
			for (int32 x=i; x<=to; x++){
				#ifndef __VM_SYSTEM	// RAM
					if (p[x]>max)	max = p[x];
					if (p[x]<min)	min = p[x];
				#else
					if (p[index]>max)	max = p[index];
					if (p[index]<min)	min = p[index];
					index++;
					if (index == BUFFER_SIZE){
						index = 0;
						VM.ReadBlock(p, BUFFER_SIZE);
					}
				#endif
			}
			ii = i>>6;
			buffer_left[ii] = (int16)(min * 32767);
			buffer_left[ii+1] = (int16)(max * 32767);

			if (progress && count--<0){	count = p_count;	Pool.ProgressUpdate( p_add ); }
		}
	}
	else	// Stereo
	{
		#ifndef __VM_SYSTEM	// RAM
			float *p = Pool.sample_memory;
		#else
			float *p = buffer;
			int32 index = 0;
			VM.ReadBlockAt(start, p, BUFFER_SIZE);
		#endif

		for (int32 i=start*2; i<=end*2; i+=256){
			min = max = 0.0;
			min_r = max_r = 0.0;
			to = i+255;
			if (to>Pool.size)	to = Pool.size;
			for (int32 x=i; x<=to; x+=2){
				#ifndef __VM_SYSTEM	// RAM
					if (p[x]>max)		max = p[x];
					if (p[x]<min)		min = p[x];
					if (p[x+1]>max_r)	max_r = p[x+1];
					if (p[x+1]<min_r)	min_r = p[x+1];
				#else
					if (p[index]>max)		max = p[index];
					if (p[index]<min)		min = p[index];
					if (p[index+1]>max_r)	max_r = p[index+1];
					if (p[index+1]<min_r)	min_r = p[index+1];
					index+=2;
					if (index >= BUFFER_SIZE){
						index = 0;
						VM.ReadBlock(p, BUFFER_SIZE);
					}
				#endif
			}
			ii = i>>6;
			buffer_left[ii] = (int16)(min * 32767);
			buffer_left[ii+1] = (int16)(max * 32767);
			buffer_right[ii] = (int16)(min_r * 32767);
			buffer_right[ii+1] = (int16)(max_r * 32767);

			if (progress && count--<0){	count = p_count;	Pool.ProgressUpdate( p_add ); }
		}
	}
	Pool.update_peak = true;
}

// ============================================================
void CPeakFile::MonoBuffer(float *out, int32 start, int32 end, float w)
{
	if (!buffer_left || !m_mono)	return;
	
	float step = (end - start)/w;
	int32 iStep = (int32)step;
	int32 index, to;
	
	#ifdef __VM_SYSTEM	// VM
	int32 nBufferSize = MIN( BUFFER_SIZE, end-start);
	#endif
	
	if ( step <= 1 )
	{
		#ifndef __VM_SYSTEM	// RAM
			float *p = Pool.sample_memory;
		#else
			float *p = buffer;
			VM.ReadBlockAt(start, p, nBufferSize);
		#endif

		for (int32 x = 0; x<w; x++){
			#ifndef __VM_SYSTEM	// RAM
				index = start + (int32)(x * step);
			#else
				index = (int32)(x * step);
			#endif
			float fTemp = p[index];

			if (fTemp>1.0f)	fTemp = 1.0f;
			else if (fTemp<-1.0f)	fTemp = -1.0f;
			*out++ = fTemp;
			*out++ = 0.0f;
		}
	}else
	if ( step < 64 )
	{	float min, max;
		#ifndef __VM_SYSTEM	// RAM
			float *p = Pool.sample_memory;
		#else
			float *p = buffer;
			int32 idx = 0;
			VM.ReadBlockAt(start, p, nBufferSize);
		#endif

		for (int32 x = 0; x<w; x++){
			index = start + (int32)(x * step);
			to = index + iStep;	if (to>m_size)	to = m_size;

			min = max = 0;
			for (int32 i=index; i<=to; i++){
				#ifndef __VM_SYSTEM	// RAM
					if (p[i]>max)	max = p[i];
					if (p[i]<min)	min = p[i];
				#else
					if (p[idx]>max)	max = p[idx];
					if (p[idx]<min)	min = p[idx];
					idx++;
					if (idx == nBufferSize){
						idx = 0;
						VM.ReadBlock(p, nBufferSize);
					}
				#endif

			}
			if (max > -min)	*out++ = MIN(max, 1);
			else			*out++ = MAX(min, -1);
			*out++ = 0.0f;
		}
	}else
	if ( step < 128 )
	{	float min, max;
		#ifndef __VM_SYSTEM	// RAM
			float *p = Pool.sample_memory;
		#else
			float *p = buffer;
			int32 idx = 0;
			VM.ReadBlockAt(start, p, nBufferSize);
		#endif

		for (int32 x = 0; x<w; x++){
			index = start + (int32)(x * step);
			to = index + iStep;	if (to>m_size)	to = m_size;

			min = max = 0;
			for (int32 i=index; i<=to; i++){
				#ifndef __VM_SYSTEM	// RAM
					if (p[i]>max)	max = p[i];
					if (p[i]<min)	min = p[i];
				#else
					if (p[idx]>max)	max = p[idx];
					if (p[idx]<min)	min = p[idx];
					idx++;
					if (idx == nBufferSize){
						idx = 0;
						VM.ReadBlock(p, nBufferSize);
					}
				#endif
			}

			*out++ = min;
			*out++ = max;
		}
	}
	else
	{	int16 min, max;
		for (int32 x = 0; x<w; x++){
			index = start + (int32)(x * step);
			to = index + iStep;	if (to>m_size)	to = m_size;
			index >>= 6;	index &= 0xfffffffe;
			to >>= 6;		to &= 0xfffffffe;

			min = max = 0;
			for (int32 i=index; i<=to; i+=2){
				if (buffer_left[i]<min)		min = buffer_left[i];
				if (buffer_left[i+1]>max)	max = buffer_left[i+1];
			}

			*out++ = min/32767.0;
			*out++ = max/32767.0;
		}
	}
}

// ============================================================
void CPeakFile::StereoBuffer(float *out, float *out_r, int32 start, int32 end, float w)
{
	if (!buffer_left ||!buffer_right || m_mono)
		return;
	
	float step = (end - start)/w;
	int32 iStep = (int32)step;
	int32 index, to;
	
	#ifdef __VM_SYSTEM	// VM
	int32 nBufferSize = MIN( BUFFER_SIZE, (end-start)*2);
	#endif
	
	if ( step <= 1 )
	{
		#ifndef __VM_SYSTEM	// RAM
			float *p = Pool.sample_memory;
		#else
			float *p = buffer;
			VM.ReadBlockAt(start, p, nBufferSize);
		#endif

		for (int32 x = 0; x<w; x++){
			#ifndef __VM_SYSTEM	// RAM
				index = start + (int32)(x * step);
			#else
				index = (int32)(x * step);
			#endif
			float fTemp = p[index*2];
			float fTempR = p[index*2+1];

			if (fTemp>1.0f)			fTemp = 1.0f;
			else if (fTemp<-1.0f)	fTemp = -1.0f;
			if (fTempR>1.0f)		fTempR = 1.0f;
			else if (fTempR<-1.0f)	fTempR = -1.0f;
			*out++ = fTemp;
			*out++ = 0.0f;
			*out_r++ = fTempR;
			*out_r++ = 0.0f;
		}
	}else
	if ( step < 64 )
	{	float min, max, min_r, max_r;
		#ifndef __VM_SYSTEM	// RAM
			float *p = Pool.sample_memory;
		#else
			float *p = buffer;
			int32 idx = 0;
			VM.ReadBlockAt(start, p, nBufferSize);
		#endif

		for (int32 x = 0; x<w; x++){
			index = start + (int32)(x * step);
			to = index + iStep;	if (to>m_size)	to = m_size;

			index *= 2;
			to *= 2;

			min = max = min_r = max_r = 0;
			for (int32 i=index; i<=to; i+=2){
				#ifndef __VM_SYSTEM	// RAM
					if (p[i]>max)		max = p[i];
					if (p[i]<min)		min = p[i];
					if (p[i+1]>max_r)	max_r = p[i+1];
					if (p[i+1]<min_r)	min_r = p[i+1];
				#else
					if (p[idx]>max)	max = p[idx];
					if (p[idx]<min)	min = p[idx];
					idx++;
					if (p[idx]>max_r)	max_r = p[idx];
					if (p[idx]<min_r)	min_r = p[idx];
					idx++;
					if (idx >= nBufferSize){
						idx = 0;
						VM.ReadBlock(p, nBufferSize);
					}
				#endif

			}
			if (max > -min)	*out++ = MIN(max, 1);
			else			*out++ = MAX(min, -1);
			*out++ = 0.0f;

			if (max_r > -min_r)	*out_r++ = MIN(max_r, 1);
			else				*out_r++ = MAX(min_r, -1);
			*out_r++ = 0.0f;
		}
	}else
	if (step <128)
	{	float min, max, min_r, max_r;

		#ifndef __VM_SYSTEM	// RAM
			float *p = Pool.sample_memory;
		#else
			float *p = buffer;
			int32 idx = 0;
			VM.ReadBlockAt(start, p, nBufferSize);
		#endif

		for (int32 x = 0; x<w; x++){
			index = start + (int32)(x * step);
			to = index + iStep;	if (to>m_size)	to = m_size;

			index *= 2;
			to *= 2;

			min = max = min_r = max_r = 0;
			for (int32 i=index; i<=to; i+=2){
				#ifndef __VM_SYSTEM	// RAM
					if (p[i]>max)		max = p[i];
					if (p[i]<min)		min = p[i];
					if (p[i+1]>max_r)	max_r = p[i+1];
					if (p[i+1]<min_r)	min_r = p[i+1];
				#else
					if (p[idx]>max)	max = p[idx];
					if (p[idx]<min)	min = p[idx];
					idx++;
					if (p[idx]>max_r)	max_r = p[idx];
					if (p[idx]<min_r)	min_r = p[idx];
					idx++;
					if (idx >= nBufferSize){
						idx = 0;
						VM.ReadBlock(p, nBufferSize);
					}
				#endif
			}
			*out++ = min;
			*out++ = max;
			*out_r++ = min_r;
			*out_r++ = max_r;
		}
	}
	else
	{	int16 min, max, min_r, max_r;
		for (int32 x = 0; x<w; x++){
			index = start + (int32)(x * step);
			to = index + iStep;	if (to>m_size)	to = m_size;
			index >>= 6;	index &= 0xfffffffe;
			to >>= 6;		to &= 0xfffffffe;

			min = max = min_r = max_r = 0;
			for (int32 i=index; i<=to; i+=2){
				if (buffer_left[i]<min)		min = buffer_left[i];
				if (buffer_left[i+1]>max)	max = buffer_left[i+1];
				if (buffer_right[i]<min_r)	min_r = buffer_right[i];
				if (buffer_right[i+1]>max_r)	max_r = buffer_right[i+1];
			}
			*out++ = min/32767.0;
			*out++ = max/32767.0;
			*out_r++ = min_r/32767.0;
			*out_r++ = max_r/32767.0;
		}
	}
}

