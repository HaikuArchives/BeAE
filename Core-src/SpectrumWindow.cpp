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

#define NUM_BANDS		32

class SpectrumView : public BView{
  public:
	SpectrumView(BRect);
	~SpectrumView();
	virtual void Draw(BRect);
	virtual void Pulse();
	float Data[3][NUM_BANDS];
  private:
};


SpectrumView::SpectrumView(BRect r) : BView(r, NULL, B_FOLLOW_ALL, B_WILL_DRAW | B_PULSE_NEEDED)
{
	SetViewColor(0,0,0);
	for (int i=0; i<32; i++){
		Data[0][i] = 0;		// clear buffer
		Data[1][i] = 0;		// clear buffer
		Data[2][i] = 0;		// clear buffer
	}
}


SpectrumView::~SpectrumView()
{
}

void SpectrumView::Draw(BRect r)
{
	rgb_color col[] = { (rgb_color){ 255,0,0 },
 						(rgb_color){ 255,31,0 },
 						(rgb_color){ 255,63,0 },
 						(rgb_color){ 255,95,0 },
 						(rgb_color){ 255,127,0 },
 						(rgb_color){ 255,159,0 },
 						(rgb_color){ 255,191,0 },
 						(rgb_color){ 255,255,0 },

 						(rgb_color){ 191,255,0 },
 						(rgb_color){ 159,255,0 },
 						(rgb_color){ 127,255,0 },
 						(rgb_color){ 95,255,0 },
 						(rgb_color){ 63,255,0 },
 						(rgb_color){ 31,255,0 },
 						(rgb_color){ 0,255,0 },
 						(rgb_color){ 0,255,31 },

 						(rgb_color){ 0,255,63 },
 						(rgb_color){ 0,255,95 },
 						(rgb_color){ 0,255,127 },
 						(rgb_color){ 0,255,159 },
 						(rgb_color){ 0,255,191 },
 						(rgb_color){ 0,255,255 },
 						(rgb_color){ 0,191,255 },
 						(rgb_color){ 0,159,255 },

 						(rgb_color){ 0,127,255 },
 						(rgb_color){ 0,95,255 },
 						(rgb_color){ 0,63,255 },
 						(rgb_color){ 0,31,255 },
 						(rgb_color){ 0,0,255 },
 						(rgb_color){ 31,0,255 },
 						(rgb_color){ 63,0,255 },
 						(rgb_color){ 95,0,255 },
 						(rgb_color){ 127,0,255 },
 						(rgb_color){ 159,0,255 },
 						(rgb_color){ 191,0,255 },
 						(rgb_color){ 255,0,255 },
 						};
	float y1, y2;
	r = Bounds();
//	r.bottom -= 12;
	SetHighColor(0,0,0);
	FillRect(r);

	float mul = r.Width() / NUM_BANDS;
	for (int i=0; i<NUM_BANDS; i++){
		y1 = r.Height() * Data[0][i];
		y2 = r.Height() * Data[1][i];
		SetHighColor(col[i]);
		FillRect( BRect(i*mul+1, r.bottom - y1, i*mul+mul-1, r.bottom) );
		SetHighColor(200, 200, 200);
		if (Data[0][i] > Data[1][i])
		{
			Data[2][i] = 1;		// delay
			Data[1][i] = Data[0][i];
		}

		StrokeLine( BPoint(i*mul+1, r.bottom - y2)
				  , BPoint(i*mul+mul-1, r.bottom - y2) );

		Data[0][i] -= 0.05;
		if (Data[0][i] <0)
			Data[0][i] = 0;
	}
}

void SpectrumView::Pulse()
{
	for (int32 i=0; i<NUM_BANDS; i++){
		if ( Data[2][i] >0 )
			Data[2][i] -= 0.1;
		else
		{
			Data[1][i] -= 0.01;
			if (Data[1][i] <0)
				Data[1][i] = 0;
		}
	}
	Draw(Bounds());
}

/*******************************************************
*   
*******************************************************/
SpectrumWindow::SpectrumWindow() : AnalyzeWindow(BRect(50,200,270, 300),Language.get("SPECTRUM_ANALYZER"))
{
	AddChild(view = new SpectrumView(Bounds()));
}

/*******************************************************
*   
*******************************************************/
void SpectrumWindow::PlayBuffer(float *buffer, size_t size)
{
	int c, i;
	float y;
	int xscale[] = 	{ 2,   3,   4,   5,   6,   7,   8,   9,
				     10,  12,  14,  18,  22,  26,  32,  38,
				     44,  48,  52,  56,  60,  67,  74,  81,
				     88, 110, 132, 154, 176, 202, 228, 254 };
	
	for (size_t j=0; j<size; j+=NUM_BANDS*512){
		for(i = 0; i < NUM_BANDS; i++)
		{
			for(c = 0, y = 0; c < 256; c+=xscale[i])
			{
				y += buffer[j*512+c*2];
			}
			y /= ceil(256/(float)xscale[i]);
			if(y > ((SpectrumView*)view)->Data[0][NUM_BANDS-i-1])
				((SpectrumView*)view)->Data[0][NUM_BANDS-i-1] = y;
		}
	}
}
