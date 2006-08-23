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
#include <StopWatch.h>

#include "IndexView.h"
#include "MainWindow.h"
#include "Globals.h"
#include "PeakFile.h"
#include "VMSystem.h"
#include "BitmapDrawer.h"

#define UPDATE	'updt'
#define iHEIGHT	INDEXVIEW_HEIGHT/2

IndexView::IndexView(BRect r) : 
	BView(r, "Index view", B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE)
{
	SetViewColor(B_TRANSPARENT_COLOR);

	drag = false;
	select = false;
	m_resized = true;		// alocated offscreen bitmap
	cache.Set(0,0,-1,-1);
	old.Set(-1,-1);
	old_x = -1;
	
	OffScreen = NULL;
	
	// allocate memory to hold peak-cache
	index_memory_left = new float [MAX_W*2];
	index_memory_right = new float [MAX_W*2];

	if((indexSem = create_sem(1, "IndexView Sem")) < 0){
//		debugger(CREATE_SEM_FAIL_MSG);
	}
}

//*****************************************************
IndexView::~IndexView()
{
	delete[] index_memory_left;
	delete[] index_memory_right;
	if (OffScreen)
		delete OffScreen;
}

//*****************************************************
void IndexView::AttachedToWindow()
{
}

//*****************************************************
void IndexView::Draw(BRect rect)
{
	LockLooper();

	BRect r = Bounds();
	if (Pool.update_index && !m_resized)
		CalculateCache(Bounds());

	if (m_resized)
	{
		acquire_sem(indexSem);
		// allocate offscreen bitmap here
		if (OffScreen)	{ delete OffScreen; OffScreen = NULL; }
		OffScreen = new BBitmap(r, B_RGB32);
		release_sem(indexSem);

		// recalculate caches for resized screen
		if (CalculateCache(r))
			m_resized = false;
	}

	DrawBitmapAsync( OffScreen, rect, rect);

	if (Pool.sample_type != NONE)
	{
		float x = Pool.last_pointer * Bounds().Width() / Pool.size;
		SetDrawingMode(B_OP_INVERT);
		StrokeLine( BPoint( x, 0), BPoint( x, r.bottom-1));//, B_MIXED_COLORS);
		SetDrawingMode(B_OP_COPY);
	}
	UnlockLooper();
}


//*****************************************************
void IndexView::MouseDown(BPoint p)
{
	SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY);

	cache = Bounds();
	cache.left = Pool.l_pointer * Bounds().Width() / Pool.size;
	cache.right = Pool.r_pointer * Bounds().Width() / Pool.size;
	
	uint32 buttons;
	GetMouse(&p, &buttons);

	if (buttons == B_PRIMARY_MOUSE_BUTTON && (modifiers() & B_SHIFT_KEY)){		// do new selection
		Pool.selection = NONE;
		Pool.r_sel_pointer = 0;
		select = true;
		t = (int32)(p.x * Pool.size/Bounds().Width());
		Pool.pointer = t;
		
		old = p;
	}else if (buttons == B_PRIMARY_MOUSE_BUTTON){
		int snooze_time = 300000;
		if (cache.Contains(p))
			drag = true;
		else if (p.x<cache.left){
			while(buttons && (p.x<cache.left)){
				int32 s = Pool.r_pointer - Pool.l_pointer;
				Pool.l_pointer -= s/2;
				if (Pool.l_pointer<0)	Pool.l_pointer = 0;
				Pool.r_pointer = Pool.l_pointer + s;
				Window()->FindView("Pointers view")->Invalidate();
				Window()->FindView("Sample view")->Invalidate();
				cache = Bounds();
				cache.left = Pool.l_pointer * Bounds().Width() / Pool.size;
				cache.right = Pool.r_pointer * Bounds().Width() / Pool.size;
				Pool.update_index = true;
				Invalidate();
			
				while(buttons && snooze_time){
					GetMouse(&p, &buttons);
					snooze(10000);
					snooze_time -= 10000;
				}
				snooze_time = 50000;
				
			}
			if (buttons && cache.Contains(p))
				drag = true;
		}
		else if (p.x>cache.right){
			while(buttons && (p.x>cache.right)){
				int32 s = Pool.r_pointer - Pool.l_pointer;
				Pool.l_pointer += s/2;
				if (Pool.l_pointer>(Pool.size-s))	Pool.l_pointer = Pool.size-s;
				Pool.r_pointer = Pool.l_pointer + s;
				Window()->FindView("Pointers view")->Invalidate();
				Window()->FindView("Sample view")->Invalidate();
				cache = Bounds();
				cache.left = Pool.l_pointer * Bounds().Width() / Pool.size;
				cache.right = Pool.r_pointer * Bounds().Width() / Pool.size;
				Pool.update_index = true;
				Invalidate();

				while(buttons && snooze_time){
					GetMouse(&p, &buttons);
					snooze(10000);
					snooze_time -= 10000;
				}
				snooze_time = 50000;
			}
			if (buttons && cache.Contains(p))
				drag = true;
		}
	}else if (buttons == B_SECONDARY_MOUSE_BUTTON){
		// secondary
		t = (int32)(p.x * Pool.size/Bounds().Width());
		if (t > (Pool.pointer + Pool.r_sel_pointer)/2){					// drag the right part
			t = (int32)Pool.pointer;			// use original begin of selection
			select= true;
			MouseMoved( p, 0, NULL );
		}else{
			t = (int32)(Pool.r_sel_pointer);
			select = true;
			MouseMoved( p, 0, NULL );
		}
	}
}

//*****************************************************
void IndexView::MouseMoved(BPoint p, uint32 button, const BMessage *msg)
{
	if (drag && p!=old){
		old = p;
		p.x -= (cache.Width()/2.0f);
		if (p.x<0.0f)	p.x = 0.0f;
		if (p.x>(Bounds().Width() - cache.Width())) p.x = (Bounds().Width() - cache.Width());
		
		int32 s = Pool.r_pointer - Pool.l_pointer;
		Pool.l_pointer = ((int32)p.x) * (Pool.size)/Bounds().IntegerWidth();
		Pool.r_pointer = Pool.l_pointer + s;

		Window()->FindView("Pointers view")->Invalidate();
		Window()->FindView("TimeBar view")->Invalidate();
		Window()->FindView("Sample view")->Invalidate();
		Pool.update_index = true;
		Invalidate();
	}else if (select && p!=old){		// right moved		Make selection with right-mouse
		Pool.selection = BOTH;
		t2 = (int32)(p.x * Pool.size/Bounds().Width());
		
		if (t > t2){
			Pool.pointer = t2;
			Pool.r_sel_pointer = t;
		}else{
			Pool.pointer = t;
			Pool.r_sel_pointer = t2;
		}

		if (Pool.pointer < 0)
			Pool.pointer = 0;

		if (Pool.r_sel_pointer > Pool.size)
			Pool.r_sel_pointer = Pool.size;
		
		Window()->FindView("Pointers view")->Pulse();
		Window()->FindView("Index view")->Invalidate();
		Window()->FindView("Sample view")->Invalidate();
		BRect update;
		if (p.x>old.x){
			update.Set(old.x-POINTER_BAR_HEIGHT-4, 0, p.x+POINTER_BAR_HEIGHT+4, Bounds().bottom);
		}else{
			update.Set(p.x-POINTER_BAR_HEIGHT-4, 0, old.x+POINTER_BAR_HEIGHT+4, Bounds().bottom);
		}
		CalculateCache(update);
		Invalidate(update);
		old = p;
	}
}


//*****************************************************
void IndexView::MouseUp(BPoint p)
{
	drag = false;
	select = false;
	old.Set(-1,-1);
}

/*******************************************************
*
*******************************************************/
void IndexView::MessageReceived(BMessage *msg){
	switch(msg->what){
	default:
		BView::MessageReceived(msg);
		break;   
	}
}

/*****************************************************
*	Indicate that the frame is resized
*****************************************************/
void IndexView::FrameResized(float width, float height)
{
	m_resized = true;				// re-allocate offscreen bitmap
	Pool.update_index = true;		// update the draw cache
}

/*****************************************************
* Create the cache bitmaps for drawing
* This is needed on resize, start and recoloring
*****************************************************/
bool IndexView::CalculateCache(BRect rect)
{
	if (!OffScreen){
		return false;
	}

	acquire_sem(indexSem);
	BRect r = Bounds();

	int32 size = INDEXVIEW_HEIGHT+1, temp;
	rgb_color a[size], b[size], c[size], d[size];

	rgb_color back2s = Prefs.index_back_color;
	rgb_color back1s = Prefs.index_back_color2;
	rgb_color back2 = Prefs.index_back_selected_color;
	rgb_color back1 = Prefs.index_back_selected_color2;
	rgb_color fore2s = Prefs.index_left_selected_color;
	rgb_color fore1s = Prefs.index_left_selected_color2;
	rgb_color fore2 = Prefs.index_left_color;
	rgb_color fore1 = Prefs.index_left_color2;

	for(int32 y=0; y<size; y++)
	{
		uint8 alpha = (uint8)(y*255/size);

		a[y].red = INT_BLEND(back1.red, back2.red, alpha, temp);
		a[y].green = INT_BLEND(back1.green, back2.green, alpha, temp);
		a[y].blue = INT_BLEND(back1.blue, back2.blue, alpha, temp);
		
		b[y].red = INT_BLEND(back1s.red, back2s.red, alpha, temp);
		b[y].green = INT_BLEND(back1s.green, back2s.green, alpha, temp);
		b[y].blue = INT_BLEND(back1s.blue, back2s.blue, alpha, temp);

		c[y].red = INT_BLEND(fore1.red, fore2.red, alpha, temp);
		c[y].green = INT_BLEND(fore1.green, fore2.green, alpha, temp);
		c[y].blue = INT_BLEND(fore1.blue, fore2.blue, alpha, temp);
			
		d[y].red = INT_BLEND(fore1s.red, fore2s.red, alpha, temp);
		d[y].green = INT_BLEND(fore1s.green, fore2s.green, alpha, temp);
		d[y].blue = INT_BLEND(fore1s.blue, fore2s.blue, alpha, temp);
	}


	int32 left = (int32)r.right+1;
	int32 right = -1;
		
	BitmapDrawer draw(OffScreen);

	// the background
	float w = r.Width();
	if ((Pool.r_pointer - Pool.l_pointer) == Pool.size || Pool.sample_type == NONE){
		for (int32 y = (int32)r.top; y < (int32)r.bottom; y++)
			for (int32 x = (int32)r.left; x <= (int32)r.right; x++)
				draw.PlotRGB( x, y, b[y] );

		left = (int32)r.left;
		right = (int32)r.right;
	}else{
		if (Pool.l_pointer>0){
			r.right = Pool.l_pointer * w / Pool.size;
			for (int32 y = (int32)r.top; y < (int32)r.bottom; y++)
				for (int32 x = (int32)r.left; x <= (int32)r.right; x++)
					draw.PlotRGB( x, y, a[y] );

			r.left = r.right;
		}
		r.right = Pool.r_pointer * w / Pool.size;
		for (int32 y = (int32)r.top; y < (int32)r.bottom; y++)
			for (int32 x = (int32)r.left; x <= (int32)r.right; x++)
				draw.PlotRGB( x, y, b[y] );

		left = (int32)r.left;
		right = (int32)r.right;

		if (Pool.r_pointer != Pool.size){
			r.left = r.right+1.0f;
			r.right = Bounds().right;
			for (int32 y = (int32)r.top; y < (int32)r.bottom; y++)
				for (int32 x = (int32)r.left; x <= (int32)r.right; x++)
					draw.PlotRGB( x, y, a[y] );

		}
	}

	int32 mid = int32((r.bottom+r.top)/2);
//	for (int32 x = (int32)r.left; x <= (int32)r.right; x++)
//		draw.PlotRGB( x, mid, Prefs.index_mid_color );

	if (Pool.sample_type != NONE){
		// get the new index view data
		if (Pool.sample_type == MONO)
			Peak.MonoBuffer(index_memory_left, 0, Pool.size, Bounds().IntegerWidth()+1);
		else if (Pool.sample_type == STEREO){
			Peak.StereoBuffer(index_memory_left, index_memory_right, 0, Pool.size, Bounds().IntegerWidth()+1);
			for (int32 i=0; i<r.right*2; i++){
				index_memory_left[i] = (index_memory_left[i] + index_memory_right[i])/2.0;
			}
		}

		// Draw the data
		int32 amp = int32(r.Height()/2.0);
		int32 index = 0, from, to;
		for (int32 x=0; x<r.right; x++){
			to =  mid -  int32(index_memory_left[ index ]*amp);
			from = mid -  int32(index_memory_left[ index+1 ]*amp);

			if ( from == to && from != mid)
				draw.PlotRGB( x, mid, Prefs.index_mid_color );
			else if (x>=left && x<=right)
			{
				for (int32 y = from; y <= to; y++)
					draw.PlotRGB( x, y, c[y] );
			}
			else
			{
				for (int32 y = from; y <= to; y++)
					draw.PlotRGB( x, y, d[y] );
			}

			index += 2;
		}


		// draw pointer
		if (Pool.selection == NONE){
			int32 x = (int32)(Pool.pointer * w / Pool.size);
			for (int32 y = (int32)r.top; y < (int32)r.bottom; y+=2)
				draw.PlotRGB( x, y, Prefs.index_pointer_color );
		}else{
			r.top ++;	r.bottom --;
			r.left = Pool.pointer * w / Pool.size;
			r.right = Pool.r_sel_pointer * w / Pool.size;
			for (int32 y = (int32)r.top; y < (int32)r.bottom; y+=2)
			{
				draw.PlotRGB( (int32)r.left, y, Prefs.index_pointer_color );
				draw.PlotRGB( (int32)r.right, y, Prefs.index_pointer_color );
			}
			int32 top = (int32)r.top;
			int32 bottom = (int32)r.bottom;
			for (int32 x = (int32)r.left; x < (int32)r.right; x+=2)
			{
				draw.PlotRGB( x, top, Prefs.index_pointer_color );
				draw.PlotRGB( x, bottom, Prefs.index_pointer_color );
			}

		}
	}
	Pool.update_index = false;
	release_sem(indexSem);
	return true;
}
