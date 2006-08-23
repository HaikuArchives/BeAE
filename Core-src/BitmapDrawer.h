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

#ifndef _BITMAP_DRAWER_H
#define _BITMAP_DRAWER_H

#include <Bitmap.h>
#include "Globals.h"

class BitmapDrawer
{
  public:
	// construction
	BitmapDrawer(BBitmap *bitmap);
	~BitmapDrawer();
	
	rgb_color GetRGB(int32 x, int32 y);
	rgb_color GetRGB(BPoint);
	rgb_color GetBilinearRGB(float x, float y);		// get with bilinear filtering
	rgb_color GetBilinearRGB(BPoint p);
	void PlotRGB(int32 x, int32 y, rgb_color c);
	void PlotRGB(BPoint p, rgb_color c);

	rgb_color GetBGR(int32 x, int32 y);
	rgb_color GetBGR(BPoint);
	void PlotBGR(int32 x, int32 y, rgb_color c);
	void PlotBGR(BPoint p, rgb_color c);
	
  private:
  	BBitmap *mBitmap;
  	uint8 *mBits;
	int32 mWidth;
	int32 mRight;
	int32 mBottom;
	BRect mBounds;
	
	int32 *mHeightLookup;
	int32 *mHeightLookup2;

	int32 eDrawMode;		// draw mode for Draw functions
	rgb_color mHighColor;	// foreground color
	rgb_color mLowColor;	// background color
};

#endif
