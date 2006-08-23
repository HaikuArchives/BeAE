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

#include "TimeBarView.h"
#include "MainWindow.h"
#include "Globals.h"

TimeBarView::TimeBarView(BRect r) : 
	BView(r, "TimeBar view", B_FOLLOW_BOTTOM | B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE)
{
	SetViewColor(B_TRANSPARENT_COLOR);
}

//*****************************************************
TimeBarView::~TimeBarView()
{
}

//*****************************************************
void TimeBarView::AttachedToWindow()
{
}

//*****************************************************
void TimeBarView::Draw(BRect rect)
{
	BRect r = Bounds();
	char s[255];

	SetLowColor(Prefs.time_back_color);
	FillRect(r, B_SOLID_LOW);
	
	if (Pool.size == 0)	return;
	
	SetHighColor(64,64,64);
	StrokeLine(BPoint(r.left, r.top), BPoint(r.right, r.top));

	if (Pool.sample_type == NONE)	return;

	int32 w = r.IntegerWidth();
	int32 x = (int32)r.left;
	float b;

	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);
	
	float fw = font.StringWidth(" 00:00.000 ");
//	float t = floor( Pool.l_pointer/Pool.frequency);
	float t = ( Pool.l_pointer/Pool.frequency);
	float t_add = (Pool.r_pointer - Pool.l_pointer)/(Pool.frequency*w);
	float t_marge = t_add * fw;
	float t_small_marge = t_marge/5.0f;
	float t_small_bound = t + t_small_marge;
	float t_bound = t + t_marge;

	while (w>0){
		t += t_add;

		if (t >= t_bound){
			int time = (int)((t-(int)t)*1000);
			int sec = (int)fmod(t,60);
			int min = ((int)t)/60;
			sprintf(s, "%d:%.2d.%.3d", min, sec, time);
			t_bound += t_marge;
			b = r.top + 5;
			SetHighColor(Prefs.time_text_color);
			DrawString(s, BPoint(x - font.StringWidth(s)/2.0f, r.bottom));
			SetHighColor(Prefs.time_marks_color);
			StrokeLine( BPoint( x, r.top+1 ), BPoint( x, b) );
		}else if (t >= t_small_bound){
			t_small_bound += t_small_marge;
			SetHighColor(Prefs.time_small_marks_color);
			b = r.top + 1;
			StrokeLine( BPoint( x, r.top+1 ), BPoint( x, b) );
		}
		x ++;
		w --;
	}

}
