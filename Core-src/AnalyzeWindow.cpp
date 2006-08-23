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
#include "AnalyzeWindow.h"
#include "main.h"

#define UPDATE		'updt'
#define QUIT		'quit'
#define SET			'setF'

/*******************************************************
*   
*******************************************************/
AnalyzeWindow::AnalyzeWindow(BRect r, const char *name)
	: BWindow(r,name, B_FLOATING_WINDOW_LOOK,B_FLOATING_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_AVOID_FOCUS)
{
	// 25 frames per second by default
	m_frames = 25;
	m_count = 0;
	// set the playHook
	m_index = Pool.SetPlayHook( _PlayBuffer, PLAY_HOOKS/2, (void*)this);

	SetPulseRate(50000);
	Run();
	Show();
}

/*******************************************************
*   
*******************************************************/
void AnalyzeWindow::PlayBuffer(float *buffer, size_t size)
{
}

/*******************************************************
*   
*******************************************************/
void AnalyzeWindow::_PlayBuffer(float *buffer, size_t size, void *cookie)
{
	AnalyzeWindow *win = (AnalyzeWindow*)cookie;	// cast to our own clas

	// process effect
	win->PlayBuffer(buffer, size);

	// update with frames/second
	win->m_count -= size;
	if (win->m_count <0){
		win->m_count = (int)Pool.system_frequency*2/win->m_frames;
		win->PostMessage(UPDATE);
	}
}

/*******************************************************
*   
*******************************************************/
int32 AnalyzeWindow::FramesPerSecond()
{
	return m_frames;
}

void AnalyzeWindow::SetFramesPerSecond(int32 frames)
{
	m_frames = frames;
}

/*******************************************************
*   
*******************************************************/
bool AnalyzeWindow::QuitRequested(){
	Pool.RemovePlayHook( _PlayBuffer, m_index );
	return true;
}

/*******************************************************
*   
*******************************************************/
void AnalyzeWindow::MessageReceived(BMessage* msg){
	BView *view;

	switch(msg->what){
	case UPDATE:
		view = ChildAt(0);
		if (view)	view->Invalidate();
		break;

	default:
		BWindow::MessageReceived(msg);
	}
}
