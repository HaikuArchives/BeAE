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

#include "Globals.h"
#include "BigView.h"
#include "MainWindow.h"
#include "Preferences.h"

extern cookie_record play_cookie;

BigView::BigView(BRect r) : 
	BView(r, "Big view", B_FOLLOW_ALL_SIDES | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE | B_PULSE_NEEDED)
{
	SetViewColor(B_TRANSPARENT_COLOR);
	
	pointer = 0;
	full_update = true;
	
	back = BTranslationUtils::GetBitmapFile("./Bitmaps/Box.png");
	left = BTranslationUtils::GetBitmapFile("./Bitmaps/BoxLeft.png");
	right = BTranslationUtils::GetBitmapFile("./Bitmaps/BoxRight.png");
	display = BTranslationUtils::GetBitmapFile("./Bitmaps/BigDisplay.png");
	digits = BTranslationUtils::GetBitmapFile("./Bitmaps/BigDigits.png");
}

//*****************************************************
BigView::~BigView()
{
	delete back;
	delete left;
	delete right;
	delete display;
	delete digits;
}

//*****************************************************
void BigView::AttachedToWindow()
{
}

//*****************************************************
void BigView::Pulse()
{
	int64 p = 0;
	if (Pool.sample_type){
		if (Pool.IsPlaying()){
			p = Pool.last_pointer;
		}else
			p = Pool.pointer;
	}

	if (p!=pointer){
		pointer = p;
		full_update = false;
		Draw(Bounds());
		full_update = true;
	}
}

//*****************************************************
void BigView::Draw(BRect r)
{
	if (full_update){
		BRect rect = Bounds();
		int32 w = rect.IntegerWidth();
		int32 x = 0;
		while (w>0){
			DrawBitmap(back, BPoint(x,0));
			x += back->Bounds().IntegerWidth();
			w -= back->Bounds().IntegerWidth();
		}
	
		DrawBitmap(left, left->Bounds(), left->Bounds());
		rect.left = rect.right-2;
		DrawBitmap(right, right->Bounds(), rect);
	}

	if (Bounds().IntegerWidth() > 180){
		int32 x = (int32)((Bounds().IntegerWidth() - 174)/2 + Bounds().left);
		if (full_update)
			DrawBitmap(display, BPoint(x, Bounds().top +8));
		
		if (Pool.sample_type != NONE){
			if (Prefs.display_time == DISPLAY_SAMPLES){
				int32 count = pointer;
				int32 c;
				int32 div = 10000000;
				bool first = true;
				for(int32 p=0; p<8; p++){
					c = count/div +1;
					count -= (c-1)*div;
					div /= 10;
			
					if (c!=1)	first = false;
					if (c==1 && first && p!=7)	c--;
			
					BRect num(20*c, 0, 20*c+19, 29);
					BRect dest(x + p*20 +7, Bounds().top +18, x + p*20 +26, Bounds().top +47);
					DrawBitmap(digits, num, dest);
				}
			} else
			if (Prefs.display_time == DISPLAY_TIME){
//				float time = Pool.pointer / Pool.system_frequency;
				float time = pointer / Pool.system_frequency;
				float seconds = floor(time);
				time -= seconds;	time *= 1000;
				float minutes = floor(seconds / 60);
				seconds -= minutes*60;
				
				BRect num, dest;
				
				if (floor(minutes/10))
					num.Set(20* (floor(minutes/10)+1), 0, 20*(floor(minutes/10)+1)+19, 29);
				else
					num.Set(0, 0, 19, 29);
				dest.Set(x + 7, Bounds().top +18, x +26, Bounds().top +47);
				DrawBitmap(digits, num, dest);
				minutes -= (floor(minutes/10)*10);
				x+=20;

				num.Set(20* (minutes+1), 0, 20*(minutes+1)+19, 29);
				dest.Set(x +7, Bounds().top +18, x +26, Bounds().top +47);
				DrawBitmap(digits, num, dest);
				x+=20;

				num.Set(20* 11+8, 0, 20*11+15, 29);	// :
				dest.Set(x + 8, Bounds().top +18, x +8 +7, Bounds().top +47);
				DrawBitmap(digits, num, dest);
				x += 10;

				num.Set(20* (floor(seconds/10)+1), 0, 20*(floor(seconds/10)+1)+19, 29);
				dest.Set(x + 7, Bounds().top +18, x + 26, Bounds().top +47);
				DrawBitmap(digits, num, dest);
				x+=20;

				num.Set(20* ((seconds- floor(seconds/10)*10)+1), 0, 20*((seconds - floor(seconds/10)*10)+1)+19, 29);
				dest.Set(x + 7, Bounds().top +18, x + 26, Bounds().top +47);
				DrawBitmap(digits, num, dest);
				x+=20;

				num.Set(20* 11+24, 0, 20*11+24+7, 29);	// .
				dest.Set(x + 8, Bounds().top +18, x +8 +7, Bounds().top +47);
				DrawBitmap(digits, num, dest);
				x += 10;

				num.Set(20* (floor(time/100)+1), 0, 20*(floor(time/100)+1)+19, 29);
				dest.Set(x + 7, Bounds().top +18, x +26, Bounds().top +47);
				DrawBitmap(digits, num, dest);
				time -= (floor(time/100)*100);
				x+=20;

				num.Set(20* (floor(time/10)+1), 0, 20*(floor(time/10)+1)+19, 29);
				dest.Set(x + 7, Bounds().top +18, x + 26, Bounds().top +47);
				DrawBitmap(digits, num, dest);
				time -= (floor(time/10)*10);
				x+=20;

				num.Set(20* (floor(time)+1), 0, 20*(floor(time)+1)+19, 29);
				dest.Set(x + 7, Bounds().top +18, x + 26, Bounds().top +47);
				DrawBitmap(digits, num, dest);
			}
		}
	}
//	Sync();
}
