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
#include "main.h"

#define UPDATE		'updt'
#define QUIT		'quit'
#define SET			'setF'

/*******************************************************
*   
*******************************************************/
RealtimeFilter::RealtimeFilter(const char *name, bool realtime)
	: BWindow(BRect(1,1,240,200),name, B_FLOATING_WINDOW_LOOK,B_FLOATING_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE| B_AVOID_FOCUS)
	, m_passes(1), m_pass(0)
{
	BRect r = Bounds();
	BView *view = new BView(r, NULL, B_FOLLOW_ALL, B_WILL_DRAW);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	view->AddChild(new BButton(BRect(r.right-78, r.bottom-30, r.right-8, r.bottom-8), NULL, Language.get("APPLY"), new BMessage(SET),
		B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM ));
	view->AddChild(new BButton(BRect(r.right-153, r.bottom-30, r.right-83, r.bottom-8), NULL, Language.get("CANCEL"), new BMessage(B_QUIT_REQUESTED),
		B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM ));

	view->AddChild(box = new BCheckBox(BRect(8, r.bottom-28, r.right-153, r.bottom-8), NULL, Language.get("BYPASS"), NULL, B_FOLLOW_BOTTOM));
	if (!realtime)
	{
		box->SetValue(B_CONTROL_ON);
		box->SetEnabled(false);
	}

	SetSizeLimits(FILTER_MIN_WIDTH,3000,40,3000);
	AddChild(view);
}

/*******************************************************
*   
*******************************************************/
void RealtimeFilter::Start()
{
	// set the playHook
	m_id = Pool.SetPlayHook( _FilterBuffer, 0, (void*)this);
	if (m_id == -1){
		// error
	}

/*	if (!Pool.IsPlaying()){
		play_self = true;
		Pool.mainWindow->PostMessage(TRANSPORT_PLAYS);
		loop = Pool.SetLoop(true);
	}else{
		play_self = false;
	}
*/
}

/*******************************************************
*   
*******************************************************/
BView *RealtimeFilter::ConfigView()
{
	return NULL;
}

/*******************************************************
*   
*******************************************************/
void RealtimeFilter::_FilterBuffer(float *buffer, size_t size, void *cookie)
{
	RealtimeFilter *win = (RealtimeFilter*)cookie;	// cast to our own clas

	// process effect
	if (win->box->Value() == B_CONTROL_OFF)
		win->FilterBuffer(buffer, size);
}

/*******************************************************
*   Aquire user memory
*******************************************************/
bool RealtimeFilter::InitFilter(float f, int32 c, int32 pass, int32 size)
{
	m_frequency = f;
	m_channels = c;
	m_pass = pass;
	m_total = size;
	return true;
}

/*******************************************************
*   To delete user memory
*******************************************************/
void RealtimeFilter::DeAllocate()
{
}

/*******************************************************
*   To enable multi-pass filters
*******************************************************/
void RealtimeFilter::SetPasses(int32 x)
{
	m_passes = x;
}

int32 RealtimeFilter::Passes()
{
	return m_passes;
}

/*******************************************************
*   
*******************************************************/
void RealtimeFilter::UpdateValues()
{
}

/*******************************************************
*   
*******************************************************/
void RealtimeFilter::Stop()
{
	Pool.RemovePlayHook( _FilterBuffer, m_id );
/*	if (play_self){
		Pool.mainWindow->PostMessage(TRANSPORT_STOP);
		Pool.SetLoop(loop);
	}
*/
}

/*******************************************************
*   Do cancel
*******************************************************/
bool RealtimeFilter::QuitRequested()
{
	BMessage message(CANCEL_FILTER);
	message.AddPointer("filter", (void*)this);
	Pool.mainWindow->PostMessage(&message);
	
	return false;
}

/*******************************************************
*   
*******************************************************/
void RealtimeFilter::MessageReceived(BMessage* msg){
	switch(msg->what){
	case SET:
{		BMessage message(EXE_FILTER);
		message.AddPointer("filter", (void*)this);
		Pool.mainWindow->PostMessage(&message);
}		break;

	case CONTROL_CHANGED:
		UpdateValues();
		break;

	default:
		BWindow::MessageReceived(msg);
	}
}
