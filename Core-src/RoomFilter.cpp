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
#include "RoomFilter.h"
#include "main.h"

/*******************************************************
*   
*******************************************************/
RoomWindow::RoomWindow(bool b) : RealtimeFilter(Language.get("ROOM"), b)
{
	// can do some initiation here
}

/*******************************************************
*   
*******************************************************/
BView *RoomWindow::ConfigView()
{
	BRect r(0,0,200,140);

	BView *view = new BView(r, NULL, B_FOLLOW_ALL, B_WILL_DRAW);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	r.InsetBy(8,8);
	r.bottom = r.top + 23;
	delay = new SpinSlider(r, NULL, Language.get("DELAY_MS"), new BMessage(CONTROL_CHANGED), 1, 500);
	delay->SetValue(Prefs.filter_room_delay * 1000);
	view->AddChild(delay);

	r.OffsetBy(0,40);
	damping = new SpinSlider(r, NULL, Language.get("DAMPING"), new BMessage(CONTROL_CHANGED), 1, 100);
	damping->SetValue(Prefs.filter_room_damping * 200);
	view->AddChild(damping);

	r.OffsetBy(0,40);
	gain = new SpinSlider(r, NULL, Language.get("GAIN"), new BMessage(CONTROL_CHANGED), 1, 100);
	gain->SetValue(Prefs.filter_room_gain * 200);
	view->AddChild(gain);

	return view;
}

void RoomWindow::UpdateValues()
{
	Prefs.filter_room_delay = delay->Value()/1000.0;
	Prefs.filter_room_gain = gain->Value()/200.0;
	Prefs.filter_room_damping = damping->Value()/200.0;

	// wipe buffer
	for (int32 i=0; i<buffer_size; i++)
		delay_buffer[i] = 0;
}

/*******************************************************
*   Init & exit
*******************************************************/
bool RoomWindow::InitFilter(float f, int32 c, int32 pass, int32 size)
{
	RealtimeFilter::InitFilter(f, c, pass, size);

	buffer_size = (int32)m_frequency * m_channels * 4;		// max 4 seconds delay
	
	delay_buffer = new float[ buffer_size ];
	for (int32 i=0; i<buffer_size; i++)
		delay_buffer[i] = 0;

	pBuffer = buffer_size-m_channels;
	return true;
}

void RoomWindow::DeAllocate()
{
	delete[] delay_buffer;
}

/*******************************************************
*   
*******************************************************/
void RoomWindow::FilterBuffer(float *buffer, size_t size)
{
	float left = 0, right = 0;
	int32 delay = (int32)(m_frequency*Prefs.filter_room_delay);
	int32 d1 = delay*m_channels;
	int32 d2 = ((int32)(1.40 * delay))*m_channels;
	int32 d3 = ((int32)(2.64 * delay))*m_channels;
	int32 d4 = ((int32)(2.10 * delay))*m_channels;
	int32 d5 = ((int32)(3.54 * delay))*m_channels;
	int32 d6 = ((int32)(3.90 * delay))*m_channels;
	float damp = Prefs.filter_room_damping;
	float damp2 = Prefs.filter_room_damping * Prefs.filter_room_damping;


	if (m_channels == 2){
// Stereo
		for (size_t i=0; i<size; i+=2){

			left = MIN(buffer[i+0] + ( delay_buffer[ (pBuffer-d1) % buffer_size ]*damp
						 				+delay_buffer[ (pBuffer-d2) % buffer_size ]*damp
						 				+delay_buffer[ (pBuffer-d3) % buffer_size ]*damp2
						 				+delay_buffer[ (pBuffer-d5) % buffer_size ]*damp
						 				+delay_buffer[ (pBuffer-d6) % buffer_size ]*damp2
										+delay_buffer[ (pBuffer-d4) % buffer_size ]*damp2) * Prefs.filter_room_gain, 1);

			right = MIN(buffer[i+1] + ( delay_buffer[ (pBuffer-d1+1) % buffer_size]*damp
					 				 	+delay_buffer[ (pBuffer-d2+1) % buffer_size]*damp
						 			 	+delay_buffer[ (pBuffer-d3+1) % buffer_size]*damp2
						 				+delay_buffer[ (pBuffer-d5+1) % buffer_size ]*damp
						 				+delay_buffer[ (pBuffer-d6+1) % buffer_size ]*damp2
									 	+delay_buffer[ (pBuffer-d4+1) % buffer_size]*damp2) * Prefs.filter_room_gain, 1);

			delay_buffer[ pBuffer % buffer_size   ] = left;
			delay_buffer[ (pBuffer+1) % buffer_size] = right;
			buffer[i+0] = left;
			buffer[i+1] = right;

			pBuffer += 2;
		}
	}else if (m_channels ==1 ){
// Mono	
		for (size_t i=0; i<size; i++){

			left = MIN(buffer[i] + ( delay_buffer[ (pBuffer-d1) % buffer_size ]*damp
						 			 +delay_buffer[ (pBuffer-d2) % buffer_size ]*damp
						 			 +delay_buffer[ (pBuffer-d3) % buffer_size ]*damp2
						 			 +delay_buffer[ (pBuffer-d5) % buffer_size ]*damp
						 			 +delay_buffer[ (pBuffer-d6) % buffer_size ]*damp2
									 +delay_buffer[ (pBuffer-d4) % buffer_size ]*damp2) * Prefs.filter_room_gain, 1);

			delay_buffer[ pBuffer % buffer_size   ] = left;
			buffer[i] = left;

			pBuffer ++;
		}
	}
}
