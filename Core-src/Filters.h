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

#ifndef _FILTER_H
#define _FILTER_H
#include "RealtimeFilter.h"

#define FILTER_MONO 	0x01
#define FILTER_STEREO	0x02
#define FILTER_BOTH		0x03
#define FILTER_REALTIME	0x04
#define FILTER_GUI		0x08

typedef struct filter_info
{
	char *name;
	int32 type;
	int32 passes;

} filter_info;

void FiltersInit();
void RunFilter(int32 filter);
void RunFilter(const char *tag);
void ExecuteFilter(RealtimeFilter *filter);
void CancelFilter(RealtimeFilter *filter);
void RunLastFilter();

void DoTrim();
void DoResample();
void ZeroLL();
void ZeroLR();
void ZeroRL();
void ZeroRR();


//===================== Realtime Filter classes without GUI

class SwapFilter : public RealtimeFilter {
  public:
	SwapFilter();
	virtual void FilterBuffer(float *, size_t);
};

class InvertFilter : public RealtimeFilter {
  public:
	InvertFilter();
	virtual void FilterBuffer(float *, size_t);
};

class SilenceFilter : public RealtimeFilter {
  public:
	SilenceFilter();
	virtual void FilterBuffer(float *, size_t);
};

class FadeInFilter : public RealtimeFilter {
  public:
	FadeInFilter();
	virtual void FilterBuffer(float *, size_t);
	virtual bool InitFilter(float f, int32 channels = 2, int32 pass = 0, int32 size = 0);
  private:
	int32 count;
};

class FadeOutFilter : public RealtimeFilter {
  public:
	FadeOutFilter();
	virtual void FilterBuffer(float *, size_t);
	virtual bool InitFilter(float f, int32 channels = 2, int32 pass = 0, int32 size = 0);
  private:
	int32 count;
};



#endif
