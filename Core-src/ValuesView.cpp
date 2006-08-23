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

#include <TranslationKit.h>
#include <Bitmap.h>
#include <stdio.h>

#include "ValuesView.h"
#include "MainWindow.h"
#include "Globals.h"

ValuesView::ValuesView(BRect r) : 
	BView(r, "Values view", B_FOLLOW_TOP_BOTTOM, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE)
{
	SetViewColor(B_TRANSPARENT_COLOR);
}

//*****************************************************
ValuesView::~ValuesView()
{
}

//*****************************************************
void ValuesView::AttachedToWindow()
{
}

//*****************************************************
void ValuesView::Draw(BRect rect)
{
	int conv[] = {0, 2, 4, 8, 8, 10, 10, 10, 20, 20, 20, 20};
	int height_div;
	BRect r = Bounds();
	char s[255];

	SetLowColor(Prefs.time_back_color);
	FillRect(r, B_SOLID_LOW);
	SetHighColor(64,64,64);
	StrokeLine(r.RightTop(), BPoint(r.right, r.bottom - TIMEBAR_HEIGHT));
	
	if (Pool.sample_type == NONE)	return;

	r.bottom -= TIMEBAR_HEIGHT;
	r.top += INDEXVIEW_HEIGHT;
	r.top += POINTER_BAR_HEIGHT;

	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);
	
	SetHighColor(Prefs.time_text_color);
	DrawString("\%", BPoint(r.right - font.StringWidth("%"), INDEXVIEW_HEIGHT+POINTER_BAR_HEIGHT/2));

	switch(Pool.sample_type){
	case MONO:
		height_div = (int)ceil(r.Height()/(font.Size()*2)) & 0xfffffe;
		if (height_div>20)	height_div = 20;
		height_div = conv[height_div/2];

		for (int i=0; i<=height_div; i++){
			float y = i*r.Height()/height_div + r.top;
			if (i!=height_div){
				SetHighColor(Prefs.time_small_marks_color);
				for (int ii=1; ii<5; ii++){
					float yy = (ii*r.Height())/(height_div*5) + y;
					StrokeLine(BPoint(r.right-2, yy), BPoint(r.right-2, yy) );
				}
			}
			SetHighColor(Prefs.time_marks_color);
			StrokeLine(BPoint(r.right-5, y), BPoint(r.right-2, y) );
			sprintf(s, "%d", -(int)(ROUND((y-r.top)*200/r.Height()) - 100));
			SetHighColor(Prefs.time_text_color);
			DrawString(s, BPoint(r.right - font.StringWidth(s) - 8, y+font.Size()/2));
		}
		break;
	case STEREO:
		r.bottom = r.Height()/2.0f -0.0f +r.top;					// calc left area
		height_div = (int)ceil(r.Height()/(font.Size()*2)) & 0xfffffe;
		if (height_div>20)	height_div = 20;
		height_div = conv[height_div/2];

	for (int k=0; k<2; k++){
		for (int i=0; i<=height_div; i++){
			float y = i*r.Height()/height_div + r.top;
			if (i!=height_div){
				SetHighColor(Prefs.time_small_marks_color);
				for (int ii=1; ii<5; ii++){
					float yy = (ii*r.Height())/(height_div*5) + y;
					StrokeLine(BPoint(r.right-2, yy), BPoint(r.right-2, yy) );
				}
			}
			SetHighColor(Prefs.time_marks_color);
			StrokeLine(BPoint(r.right-5, y), BPoint(r.right-2, y) );
			if (i==height_div && k==0){}else{
				sprintf(s, "%d", -(int)(ROUND((y-r.top)*200/r.Height()) - 100));
				SetHighColor(Prefs.time_text_color);
				DrawString(s, BPoint(r.right - font.StringWidth(s) - 8, y+font.Size()/2));
			}
		}

		r.OffsetTo(0, r.bottom+0.0f);					// calc right area
		}	
	}
}
