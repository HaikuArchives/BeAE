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
#include "Analyzers.h"
#include "main.h"

#define WIDTH		256

class SampleScopeView : public BView{
  public:
	SampleScopeView(BRect);
	~SampleScopeView();
	virtual void Draw(BRect);
	virtual void Pulse();
	float Data[2][WIDTH];
};


SampleScopeView::SampleScopeView(BRect r) : BView(r, NULL, B_FOLLOW_ALL, B_WILL_DRAW | B_PULSE_NEEDED)
{
	SetViewColor(0,0,0);
	for (int i=0; i<WIDTH; i++){
		Data[0][i] = 0;		// clear buffer
		Data[1][i] = 0;		// clear buffer
	}
}


SampleScopeView::~SampleScopeView()
{
}

void SampleScopeView::Pulse()
{
	Invalidate();
}

void SampleScopeView::Draw(BRect r)
{
	r = Bounds();
	SetHighColor(0,0,0);
	FillRect(r);

	r.bottom /= 2;		// half
	float m = (r.top + r.bottom)/2;
	float a = r.Height()/2;
	
	SetHighColor( 160,0,0 );
	StrokeLine( BPoint(0,m), BPoint(r.right,m) );
	SetHighColor( 255,80,0 );
	MovePenTo( BPoint(0, m-Data[0][0]*a) );
	for (int i=0; i<WIDTH; i++){
		StrokeLine( BPoint( i*r.Width()/WIDTH, m-Data[0][i]*a) );
		Data[0][i] = 0;
	}

	m += r.Height();
	SetHighColor( 0,0,160 );
	StrokeLine( BPoint(0,m), BPoint(r.right,m) );
	SetHighColor( 0,80,255 );
	MovePenTo( BPoint(0, m-Data[1][0]*a) );
	for (int i=0; i<WIDTH; i++){
		StrokeLine( BPoint( i*r.Width()/WIDTH, m-Data[1][i]*a) );
		Data[1][i] = 0;
	}
}

/*******************************************************
*   
*******************************************************/
SampleScopeWindow::SampleScopeWindow() : AnalyzeWindow(BRect(50, 50, 270, 150),Language.get("SAMPLE_SCOPE"))
{
	AddChild(view = new SampleScopeView(Bounds()));
}

/*******************************************************
*   
*******************************************************/
void SampleScopeWindow::PlayBuffer(float *buffer, size_t size)
{
	for (size_t i=0; i<WIDTH; i++){
		((SampleScopeView*)view)->Data[0][i] = *buffer++;
		((SampleScopeView*)view)->Data[1][i] = *buffer++;
	}
}
