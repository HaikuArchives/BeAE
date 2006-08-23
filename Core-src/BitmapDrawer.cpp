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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "BitmapDrawer.h"

BitmapDrawer::BitmapDrawer(BBitmap *bitmap)
{
	mBitmap = bitmap;
	mBits = (uint8*)bitmap->Bits();
	mBounds = bitmap->Bounds();
	mWidth = bitmap->BytesPerRow();
	mRight = (int32)mBounds.right;
	mBottom = (int32)mBounds.bottom;

	mHeightLookup = new int32[mBottom+1];
	mHeightLookup2 = new int32[mBottom+1];
	// fill height buffer
	int32 y = 0, y2 = 0;
	for (int32 i = 0; i <= mBottom; i++)
	{
		mHeightLookup2[i] = y2;
		y2 += (mRight+1);

		mHeightLookup[i] = y;
		y += mWidth;
	}
}

BitmapDrawer::~BitmapDrawer()
{
	delete[] mHeightLookup;
	delete[] mHeightLookup2;
}

/*******************************************************
* Set pixel  
*******************************************************/
void BitmapDrawer::PlotBGR(BPoint p, rgb_color c)
{
	if (!WITHIN(0, (int)p.x, mRight))	return;		// check boundaries
	if (!WITHIN(0, (int)p.y, mBottom))	return;
   
	rgb_color *bits = (rgb_color*)mBits + (int)p.x + mHeightLookup2[ (int)p.y ];
	*bits=c;
}

void BitmapDrawer::PlotBGR(int32 x, int32 y, rgb_color c){
	if (!WITHIN(0, x, mRight))	return;		// check boundaries
	if (!WITHIN(0, y, mBottom))	return;
   
	rgb_color *bits = (rgb_color*)mBits + x + mHeightLookup2[y];
	*bits=c;
}

/*******************************************************
* Set pixel  
*******************************************************/
void BitmapDrawer::PlotRGB(BPoint p, rgb_color c){
	if (!WITHIN(0, (int)p.x, mRight))	return;		// check boundaries
	if (!WITHIN(0, (int)p.y, mBottom))	return;
   
	uint8 *bits = mBits + ((int)p.x<<2) + mHeightLookup[ (int)p.y ];
	bits[0]=c.blue;	bits[1]=c.green; bits[2]=c.red; bits[3]=c.alpha;
}

void BitmapDrawer::PlotRGB(int32 x, int32 y, rgb_color c){
	if (!WITHIN(0, x, mRight))	return;		// check boundaries
	if (!WITHIN(0, y, mBottom))	return;
   
	uint8 *bits = mBits + (x<<2) + mHeightLookup[ y ];
	bits[0]=c.blue;	bits[1]=c.green; bits[2]=c.red; bits[3]=c.alpha;
}

/*******************************************************
* Get pixel  
*******************************************************/
rgb_color BitmapDrawer::GetBGR(BPoint p){
	CLAMP(p.x, 0, mRight);
	CLAMP(p.y, 0, mBottom);
   
	rgb_color *bits = (rgb_color*)mBits + (int)p.x + mHeightLookup2[ (int)p.y ];
	return *bits;
}

rgb_color BitmapDrawer::GetBGR(int32 x, int32 y){
	CLAMP(x, 0, mRight);
	CLAMP(y, 0, mBottom);
   
	rgb_color *bits = (rgb_color*)mBits + x + mHeightLookup2[ y ];
	return *bits;
}

/*******************************************************
* Get pixel  
*******************************************************/
rgb_color BitmapDrawer::GetRGB(BPoint p){
	return GetRGB(p.x, p.y);
}

rgb_color BitmapDrawer::GetRGB(int32 x, int32 y){
	CLAMP(x, 0, mRight);
	CLAMP(y, 0, mBottom);

	uint8 *bits = mBits + (x<<2) + mHeightLookup[y];
	return ( (rgb_color){bits[2], bits[1], bits[0], bits[3]} );
}

rgb_color BitmapDrawer::GetBilinearRGB(BPoint p)
{	
	return GetBilinearRGB(p.x, p.y);
}

rgb_color BitmapDrawer::GetBilinearRGB(float x, float y)
{
	float m0, m1;
	rgb_color v0, v1, v2, v3;
	rgb_color ret = { 128,128,128,0 };	// white transparent if not ok

//	if (!WITHIN(0,x,input->Bounds().Width()))	return ret;		// check boundaries
//	if (!WITHIN(0,y,input->Bounds().Height()))	return ret;
	CLAMP(x, 0, mRight-1);
	CLAMP(y, 0, mBottom-1);

	int32 x2 = (int32)x;	int32 y2 = (int32)y;
	uint8 *bits = mBits + (x2<<2) + mHeightLookup[y2];
	
	v0.blue = *bits++;	v0.green = *bits++;	v0.red = *bits++;	v0.alpha = *bits++;
	v1.blue = *bits++;	v1.green = *bits++;	v1.red = *bits++;	v1.alpha = *bits++;
	bits += mWidth - 8;
	v2.blue = *bits++;	v2.green = *bits++;	v2.red = *bits++;	v2.alpha = *bits++;
	v3.blue = *bits++;	v3.green = *bits++;	v3.red = *bits++;	v3.alpha = *bits++;

	x -= (int)x;
	y -= (int)y;
	float xx = 1.0 - x;
	float yy = 1.0 - y;

	m0 = xx * v0.red + x * v1.red;
	m1 = xx * v2.red + x * v3.red;
	ret.red = (uint8)(yy * m0 + y * m1);

	m0 = xx * v0.green + x * v1.green;
	m1 = xx * v2.green + x * v3.green;
	ret.green = (uint8)(yy * m0 + y * m1);

	m0 = xx * v0.blue + x * v1.blue;
	m1 = xx * v2.blue + x * v3.blue;
	ret.blue = (uint8)(yy * m0 + y * m1);

	m0 = xx * v0.alpha + x * v1.alpha;
	m1 = xx * v2.alpha + x * v3.alpha;
	ret.alpha = (uint8)(yy * m0 + y * m1);

	return ret;
}

