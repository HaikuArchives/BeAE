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

#include <Window.h>
#include <View.h>
#include <InterfaceKit.h>
#include <stdlib.h>
#include <stdio.h>

#include "Globals.h"
#include "RealtimeFilter.h"
//#include "AmplifierFilter.h"
#include "LimiterFilter.h"

/*******************************************************
*   
*******************************************************/
LimiterFilter::LimiterFilter(bool b) : RealtimeFilter(Language.get("LIMITFILTER"), b)
{

}

/*******************************************************
*   
*******************************************************/
BView *LimiterFilter::ConfigView()
{
	BRect r(0,0,200,100);

	BView *view = new BView(r, NULL, B_FOLLOW_ALL, B_WILL_DRAW);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	r.InsetBy(8,8);
	r.bottom = r.top + 23;
	value = new SpinSlider(r, NULL, Language.get("LEVEL"), new BMessage(CONTROL_CHANGED), 0, 100);
	value->SetValue(Prefs.filter_limiter_value);
	view->AddChild(value);

	r.OffsetBy(0,40);
	mix = new SpinSlider(r, NULL, Language.get("MIX_LEVEL"), new BMessage(CONTROL_CHANGED), 0, 100);
	mix->SetValue(Prefs.filter_limiter_mix);
	view->AddChild(mix);

	return view;
}

void LimiterFilter::UpdateValues()
{
	Prefs.filter_limiter_value = value->Value();
	Prefs.filter_limiter_mix = mix->Value();
}

/*******************************************************
*   Init & exit
*******************************************************/
bool LimiterFilter::InitFilter(float f, int32 c)
{
	RealtimeFilter::InitFilter(f, c);

	old_left = old_right = 0;
/*
	buffer_size = (int32)m_frequency * m_channels;
	
	delay_buffer = new float[ buffer_size ];
	for (int32 i=0; i<buffer_size; i++)	
		delay_buffer[i] = 0.0;

	pBuffer = buffer_size-m_channels;
*/
	return true;
}

void LimiterFilter::DeAllocate()
{
//	delete[] delay_buffer;
}

/*******************************************************
*   
*******************************************************/
void LimiterFilter::FilterBuffer(float *buffer, size_t size)
{
	float tmp, tmp2, lim;
	float mix = Prefs.filter_limiter_mix/100.0;
	float mix2 = 1-mix;
	float value = Prefs.filter_limiter_value/1000.0;

	if (m_channels == 2){
// Stereo
		// left
		for (size_t i=0; i<size; i+=2){
			tmp = buffer[i];
			lim = tmp - old_left;
			old_left = tmp;

			if (lim > 0)
			{
				if (lim >= value)
				{
					tmp += value;
				}
			}
			if (lim < 0)
			{
				lim = -lim;
				if (lim >= value)
				{
					tmp -= value;
				}
			}

			if (tmp > 1)		tmp = 1;
			else if (tmp<-1)	tmp = -1;
			
			buffer[i] = mix2*buffer[i] + mix*tmp;
		}
		// right
		for (size_t i=1; i<size; i+=2){
			tmp = buffer[i];
			lim = tmp - old_right;
			old_right = tmp;

			if (lim > 0)
			{
				if (lim >= value)
				{
					tmp += value;
				}
			}
			if (lim < 0)
			{
				lim = -lim;
				if (lim >= value)
				{
					tmp -= value;
				}
			}

			if (tmp > 1)		tmp = 1;
			else if (tmp<-1)	tmp = -1;
			
			buffer[i] = mix2*buffer[i] + mix*tmp;
		}
	}else if (m_channels ==1 ){
// Mono	
		for (size_t i=0; i<size; i++){
			tmp = buffer[i];
			lim = old_left - tmp;
			old_left = tmp;

			if (lim > 0)
			{
				if (lim >= value)
				{
					tmp += value;
				}
			}
			if (lim < 0)
			{
				lim = -lim;
				if (lim >= value)
				{
					tmp -= value;
				}
			}

			if (tmp > 1)		tmp = 1;
			else if (tmp<-1)	tmp = -1;
			
			buffer[i] = mix2*buffer[i] + mix*tmp;
		}
	}
}
