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
#include <stdio.h>

#include "Globals.h"
#include "RealtimeFilter.h"
#include "NormalizeFilter.h"
#include "main.h"

#define SET_TEXT		'setT'
#define SELECT			'slct'

/*******************************************************
*   
*******************************************************/
NormalizeFilter::NormalizeFilter() : RealtimeFilter(Language.get("NORMALIZE"), false)
{
	// can do some initiation here
}

/*******************************************************
*   
*******************************************************/
BView *NormalizeFilter::ConfigView()
{
	BRect r(0,0,180,80);

	BView *view = new BView(r, NULL, B_FOLLOW_ALL, B_WILL_DRAW);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	r.InsetBy(8,8);
	r.bottom = r.top + 19;
	value = new SpinControl(r, NULL, Language.get("NORMALIZE_LEVEL"), NULL, 1, 100, Prefs.filter_normalize, 1);
	value->SetDivider(120);
	view->AddChild(value);

	return view;
}

/*******************************************************
*   
*******************************************************/
void NormalizeFilter::UpdateValues()
{
	Prefs.filter_normalize = value->Value();
}

/*******************************************************
*   Init & exit
*******************************************************/
bool NormalizeFilter::InitFilter(float f, int32 c, int32 pass, int32 size)
{
	RealtimeFilter::InitFilter(f, c, pass, size);
	if (pass == 0){
		max_left = 0.0f;
		min_left = 0.0f;
		max_right = 0.0f;
		min_right = 0.0f;
	}
	else{
		if (max_left == 0.0f && min_left == 0.0f)
			power_left = 0.0f;
		else
			power_left = Prefs.filter_normalize/(MAX(-min_left, max_left)*100.0);	// to multiply

		if (max_right == 0.0f && min_right == 0.0f)
			power_right = 0.0f;
		else
			power_right = Prefs.filter_normalize/(MAX(-min_right, max_right)*100.0);	// to multiply
	}

	return true;
}

/*******************************************************
*   
*******************************************************/
void NormalizeFilter::FilterBuffer(float *buffer, size_t size)
{
	if (m_pass == 0)	// check level
	{
		if (m_channels == 2){	// Stereo
			for (size_t i=0; i<size; i+=2){

				if (buffer[i] < min_left)	min_left = buffer[i];
				if (buffer[i] > max_left)	max_left = buffer[i];
				
				if (buffer[i+1] < min_right)	min_right = buffer[i+1];
				if (buffer[i+1] > max_right)	max_right = buffer[i+1];

			}
		}else if (m_channels ==1 ){	// Mono	
			for (size_t i=0; i<size; i++){

				if (buffer[i] < min_left)	min_left = buffer[i];
				if (buffer[i] > max_left)	max_left = buffer[i];

			}
		}
	}
	else		// normalize it
	{
		if (m_channels == 2){	// Stereo
			for (size_t i=0; i<size; i+=2){
				*buffer++ *= power_left;
				*buffer++ *= power_right;
			}
		}else if (m_channels ==1 ){	// Mono	
			for (size_t i=0; i<size; i++){
				*buffer++ *= power_left;
			}
		}
	}
}

