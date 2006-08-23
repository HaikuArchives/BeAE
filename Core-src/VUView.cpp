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

#include "VUView.h"
#include "Globals.h"

extern cookie_record play_cookie;

VUView::VUView(BRect r) : 
	BView(r, "VU view", B_FOLLOW_BOTTOM, B_WILL_DRAW | B_PULSE_NEEDED)
{
	SetViewColor(B_TRANSPARENT_COLOR);
	LedsActive = BTranslationUtils::GetBitmapFile("./Bitmaps/LedsActive.png");
	LedsInactive = BTranslationUtils::GetBitmapFile("./Bitmaps/LedsInactive.png");
}

//*****************************************************
VUView::~VUView()
{
	delete LedsActive;
	delete LedsInactive;
}

//*****************************************************
void VUView::AttachedToWindow()
{
	SetViewBitmap(BTranslationUtils::GetBitmapFile("./Bitmaps/VU.png"), B_FOLLOW_ALL);
}

//*****************************************************
void VUView::Pulse()
{
	left_max -= 0.04;	if (left_max<0.0)	left_max = 0.0;
	right_max -= 0.04;	if (right_max<0.0)	right_max = 0.0;

	if (!Pool.IsPlaying()){
		play_cookie.left = play_cookie.right = 0.0;
	}else{
		left_max = MAX(left_max, play_cookie.left);
		right_max = MAX(right_max, play_cookie.right);
	}
	Invalidate(BRect(9,8,23,56));
}

//*****************************************************
void VUView::Draw(BRect rect)
{
	float left = 56.0f - play_cookie.left * 48.0f;
	float right = 56.0f - play_cookie.right * 48.0f;
	DrawBitmapAsync(LedsActive, BRect(0, left-8, 6, 48), BRect(9, left, 15, 56));
	DrawBitmapAsync(LedsActive, BRect(0, right-8, 6, 48), BRect(17, right, 23, 56));

	left = 56.0f - left_max * 48.0f;
	right = 56.0f - right_max * 48.0f;
	DrawBitmapAsync(LedsActive, BRect(0, left-8, 6, left-8), BRect(9, left, 15, left));
	DrawBitmapAsync(LedsActive, BRect(0, right-8, 6, right-8), BRect(17, right, 23, right));
}
