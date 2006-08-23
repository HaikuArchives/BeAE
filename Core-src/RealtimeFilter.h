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

#ifndef REALTIME_FILTER_WINDOW_H
#define REALTIME_FILTER_WINDOW_H

#include <Application.h>
#include <AppKit.h>
#include <InterfaceKit.h>
#include <String.h>
#include "SpinControl.h"
#include "SpinSlider.h"

#define FILTER_MIN_WIDTH	240

// if a contro has this message, it will be patched to UpdateValues()
#define CONTROL_CHANGED	'chgd'

class RealtimeFilter : public BWindow {
  public:
	RealtimeFilter(const char *name, bool realtime = true);
	virtual void MessageReceived(BMessage*);
	virtual bool QuitRequested();

	// init the filters, allocate the required data, ....	
	// total_samples is only needed in filters like fade
	// The pass is needed for multi-pass filters that also need analyzing
	// like the normalize filter
	virtual bool InitFilter(float frequency, int32 channels = 2, int32 pass = 0, int32 total_samples = 0);
	
	// start the realtime buffers and stop them again
	// no need to overload, just for internal use
	void Start();
	void Stop();

	// need to release memory here as the play-routines need
	// to be stopped before this is possible
	virtual void DeAllocate();
	
	// The actual data to analyze !
	virtual void FilterBuffer(float *, size_t) = 0;
	
	// Update values, called on a CONTROL_CHANGED message
	virtual void UpdateValues();

	// the configView
	virtual BView* ConfigView();
	
	// multipass stuff
	int32 Passes();
	void SetPasses(int32 x);

  private:
	static void _FilterBuffer(float *, size_t, void *);
	bool play_self, loop;
	int32 m_id;
	int32 m_passes;
  protected:
	float m_frequency;
	int32 m_channels, m_pass, m_total;
	BView *parent;
	BCheckBox *box;
};

#endif