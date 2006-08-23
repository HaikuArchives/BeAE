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
#include "AmplifierFilter.h"

/*******************************************************
*   
*******************************************************/
AmplifierFilter::AmplifierFilter(bool b) : RealtimeFilter(Language.get("AMPLIFIER"), b)
{

}

/*******************************************************
*   
*******************************************************/
BView *AmplifierFilter::ConfigView()
{
	BRect r(0,0,200,60);

	BView *view = new BView(r, NULL, B_FOLLOW_ALL, B_WILL_DRAW);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	r.InsetBy(8,8);
	r.bottom = r.top + 23;
	value = new SpinSlider(r, NULL, Language.get("LEVEL"), new BMessage(CONTROL_CHANGED), 1, 300);
	value->SetValue(Prefs.filter_amplifier_value);
	view->AddChild(value);

	return view;
}

void AmplifierFilter::UpdateValues()
{
	Prefs.filter_amplifier_value = value->Value();
}

/*******************************************************
*   
*******************************************************/
void AmplifierFilter::FilterBuffer(float *buffer, size_t size)
{
	float amp = Prefs.filter_amplifier_value/100.0;
	float tmp;

	for (size_t i=0; i<size; i++){
		tmp = *buffer * amp;
		if (tmp>1.0)	tmp = 1.0;
		if (tmp<-1.0)	tmp = -1.0;
		*buffer++ = tmp;
	}
}
