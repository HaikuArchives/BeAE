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

#ifndef _PEAK_FILE_H
#define _PEAK_FILE_H
#include <stdlib.h>
#include <stdio.h>

#include "Globals.h"

class CPeakFile {
  public:
	CPeakFile();
	~CPeakFile();
	void Init(int32 size, bool mono);	// size in samples

	// create peaks from memeory
	// start, end are the sample-pointers
	// when progress: do an update progressbar
	void CreatePeaks(int32 start, int32 end, int32 progress = 0);
	
	// Fill a buffer with the peak-values ready to draw
	// start, end are the sample-pointers
	// w: width
	void MonoBuffer(float *out, int32 start, int32 end, float w);
	void StereoBuffer(float *out_l,float *out_r, int32 start, int32 end, float w);

  private:
  	int32 m_size;		// memory size in samples
	bool m_mono;
	int16 *buffer_left;
	int16 *buffer_right;
	float *buffer;
};

extern CPeakFile Peak; // Included so you don't have too 
#endif
