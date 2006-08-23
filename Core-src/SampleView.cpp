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

#include "SampleView.h"
#include "MainWindow.h"
#include "Globals.h"
#include "PeakFile.h"
#include "BitmapDrawer.h"
#include "Shortcut.h"
#include "MyClipBoard.h"
#include "VMSystem.h"

extern cookie_record play_cookie;

/*****************************************************
*	Init View
*****************************************************/
SampleView::SampleView(BRect r) : 
	BView(r, "Sample view", B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE | B_PULSE_NEEDED)
{
	SetViewColor(B_TRANSPARENT_COLOR);
	drag = false;
	edit = false;
	Pool.update_peak = true;
	stop_following = false;
	drag_selection = false;
	drag_border = false;
	cache_left_valid = false;
	cache_right_valid = false;
	old.Set(-1,-1);
	old_x = -1;
	
	peak_buffer_l = NULL;
	peak_buffer_r = NULL;
	m_width = -1;

	OffScreen = NULL;
	leftCache = NULL;
	rightCache = NULL;
	leftSelected = NULL;
	rightSelected = NULL;

	m_old_l_pointer = -1;
	m_old_r_pointer = -1;

	m_resized = true;		// need to create background buffers

	if((viewSem = create_sem(1, "SampleView Sem")) < 0){
//		debugger(CREATE_SEM_FAIL_MSG);
	}
}

/*****************************************************
*
*****************************************************/
SampleView::~SampleView()
{
	if (peak_buffer_l)	delete peak_buffer_l;
	if (peak_buffer_r)	delete peak_buffer_r;
	if (leftCache)		delete leftCache;
	if (rightCache)		delete rightCache;
	if (leftSelected)	delete leftSelected;
	if (rightSelected)	delete rightSelected;
	if (OffScreen)		delete OffScreen;
}

/*****************************************************
*
*****************************************************/
void SampleView::AttachedToWindow()
{
}

/*****************************************************
*
*****************************************************/
void SampleView::Init()
{
	peak_buffer_l = new float[ MAX_W * 2 ];
	peak_buffer_r = new float[ MAX_W * 2 ];
	memset( peak_buffer_l, 0, MAX_W * 2 * sizeof(float));	// wipe buffer
	memset( peak_buffer_r, 0, MAX_W * 2 * sizeof(float));	// wipe buffer
}

/*****************************************************
*
*****************************************************/
void SampleView::Pulse()
{
	if (Pool.sample_type == NONE)	return;

	BRect r = Bounds();
	r.top += POINTER_BAR_HEIGHT;

	if (!Pool.IsPlaying() && !play_cookie.pause){
		if (old_x != -1){
			SetDrawingMode(B_OP_INVERT);
			StrokeLine( BPoint( old_x, r.top), BPoint( old_x, r.bottom));
			SetDrawingMode(B_OP_COPY);
			old_x = -1;
		}
		return;
	}
	int64 p = 0;

	if (Pool.sample_type != NONE)
		if (Pool.IsPlaying()){
			p = Pool.last_pointer;
		}else
			p = Pool.pointer;

	if (p!=pointer){
		pointer = p;
		
		int32 xx = Pool.r_pointer - Pool.l_pointer;
		if (Prefs.follow_playing && !stop_following && pointer < Pool.size - xx/2)
		{
			int64 ptr;
			ptr = p - xx/2;
			if ( ptr > (Pool.size-xx))	ptr = Pool.size-xx;
			if ( ptr < 0 )				ptr = 0;

			if ( Pool.l_pointer != ptr )
			{
				Pool.l_pointer = ptr;
				Pool.r_pointer = Pool.l_pointer + xx;

				Window()->FindView("Pointers view")->Pulse();//Invalidate();
				Pool.update_index = true;	// @TODO: need to be pulse for fast update
				Window()->FindView("Big view")->Pulse();//Draw()Invalidate();
				Window()->FindView("Sample view")->Invalidate();
				Window()->FindView("TimeBar view")->Invalidate();
			}
		}	

		// play pointer
		float x = (pointer-Pool.l_pointer) * Bounds().Width() /(Pool.r_pointer - Pool.l_pointer);
		SetDrawingMode(B_OP_INVERT);

		StrokeLine( BPoint( old_x, r.top), BPoint( old_x, r.bottom));
		old_x = x;
		StrokeLine( BPoint( x, r.top), BPoint( x, r.bottom));

		SetDrawingMode(B_OP_COPY);
	}
}

/*****************************************************
*
*****************************************************/
void SampleView::Draw(BRect rect)
{
	Looper()->Lock();

	if (Pool.update_draw_cache && !m_resized)
		CalculateCache();

	if (m_resized)
	{
		acquire_sem(viewSem);

		// allocate offscreen bitmap here
		if (OffScreen)	delete OffScreen;
		BRect r = Bounds();
		r.InsetBy(0,-2);
		OffScreen = new BBitmap(r, B_RGB32);

		screenBits = (rgb_color*)OffScreen->Bits();
		screenWidth = OffScreen->Bounds().IntegerWidth()+1;
		screenBits += POINTER_BAR_HEIGHT*screenWidth;		// adjust for the top bar

		// fill upper with gray bar
		int32 width = Bounds().IntegerWidth();
		BitmapDrawer draw(OffScreen);
		rgb_color a;
		for(int32 y=0; y<POINTER_BAR_HEIGHT; y++)
		{
			if (y==0)							a = (rgb_color){255,255,255};
			else if (y==POINTER_BAR_HEIGHT-1)	a = (rgb_color){128,128,128};
			else								a = (rgb_color){192,192,192};
			for(int32 x=0; x<=width; x++)
				draw.PlotBGR(x,y,a);
		}
		
		release_sem(viewSem);

		// recalculate caches for resized screen
		CalculateCache();
	}

	rect.top = Bounds().top;
	rect.bottom = Bounds().bottom;
	if (Pool.sample_view_dirty)
	{
		switch(Pool.sample_type)
		{
			case MONO:
				if (!cache_left_valid)	break;
				rect.bottom = Bounds().bottom-POINTER_BAR_HEIGHT;
				DrawMono(rect, true, (Pool.selection == BOTH));
				rect.bottom = Bounds().bottom;
				break;

			case STEREO:
				DrawStereo(rect);
				break;

			default:
			{
				BitmapDrawer draw(OffScreen);

				rgb_color a = Prefs.back_color;
				uint8 t = a.red;
				a.red = a.blue;
				a.blue = t;
				
				int32 bottom = Bounds().IntegerHeight();
				int32 right = Bounds().IntegerWidth();
				// need to fix this !

				for(int32 y=POINTER_BAR_HEIGHT; y<=bottom; y++)
				{
					for(int32 x=0; x<=right; x++)
					{
						draw.PlotBGR(x,y,a);
					}
				}
			}break;
		}
	}	

	DrawBitmapAsync( OffScreen, rect, rect);

	if (Pool.sample_type == NONE){
		Looper()->Unlock();
		return;
	}

	// Draw the pointer
	BRect r = Bounds();
	int32 xx = POINTER_BAR_HEIGHT/2 -2;
	if (Pool.selection == NONE){
		float x = (Pool.pointer-Pool.l_pointer) * Bounds().Width() /(Pool.r_pointer - Pool.l_pointer);
		SetHighColor(Prefs.pointer_color);
		SetLowColor(Prefs.back_color);

		for (float y = POINTER_BAR_HEIGHT; y<r.bottom; y+=4)
			StrokeLine( BPoint( x, y), BPoint( x, y));
		FillTriangle( BPoint(x, POINTER_BAR_HEIGHT-3), BPoint(x - xx, 5), BPoint(x + xx, 5));
	}else{
		float x = (Pool.pointer-Pool.l_pointer) * Bounds().Width() /(Pool.r_pointer - Pool.l_pointer);
		SetHighColor(Prefs.pointer_color);
		FillTriangle( BPoint(x, POINTER_BAR_HEIGHT-3), BPoint(x, 5), BPoint(x + xx, 5));
//		StrokeLine( BPoint( x, POINTER_BAR_HEIGHT), BPoint( x, r.bottom));

		x = (Pool.r_sel_pointer-Pool.l_pointer +1) * Bounds().Width() /(Pool.r_pointer - Pool.l_pointer);//+1;
		FillTriangle( BPoint(x, POINTER_BAR_HEIGHT-3), BPoint(x, 5), BPoint(x - xx, 5));
//		StrokeLine( BPoint( x, POINTER_BAR_HEIGHT), BPoint( x, r.bottom));
	}
	
	if (Pool.IsPlaying()){				// remove line
		SetDrawingMode(B_OP_INVERT);
		StrokeLine( BPoint( old_x, Bounds().top+POINTER_BAR_HEIGHT), BPoint( old_x, Bounds().bottom));
		SetDrawingMode(B_OP_COPY);
	}

	// these are to notify screen changes to update peak-caches or scroll fast
	m_old_l_pointer = Pool.l_pointer;
	m_old_r_pointer = Pool.r_pointer;
	m_resized = false;					//  put here, so all sub-routines can benefit
	Pool.update_draw_cache = false;
	Pool.update_peak = false;				// needed for pencil edit

	Looper()->Unlock();
}

/*****************************************************
*
*****************************************************/
void SampleView::MouseDown(BPoint p)
{
	if (Pool.size == 0)	return;

	BMessage *currentMsg = Window()->CurrentMessage();
	uint32 button = 0, clicks = 0;
	currentMsg->FindInt32("buttons", (int32 *)&button);
	currentMsg->FindInt32("clicks", (int32 *)&clicks);

	SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY);

	if ( p.y < POINTER_BAR_HEIGHT)
	{
		/* Handle the triangles here */
		return;
	}

	//
	// The selection tool handles all select modes
	//
	switch(Pool.tool_mode){
	case SELECT_TOOL:			/* Select Tool */
	{	if (clicks == 1){
			float middle = (Bounds().Height()-POINTER_BAR_HEIGHT)*0.50+POINTER_BAR_HEIGHT;	// middle
			// calculate position of cursors on screen
			float pointer_x = (Pool.pointer-Pool.l_pointer) * Bounds().Width() /(Pool.r_pointer - Pool.l_pointer);
			float sel_pointer_x = (Pool.r_sel_pointer-Pool.l_pointer+1) * Bounds().Width() /(Pool.r_pointer - Pool.l_pointer);
	
			bool left_select = false, right_select = false;
			bool left_pointer = (p.x < pointer_x+3 && p.x > pointer_x-3) && Pool.selection != NONE;
			bool right_pointer = (p.x < sel_pointer_x+3 && p.x > sel_pointer_x-3) && Pool.selection != NONE;

			if (Pool.selection == BOTH || (Pool.selection == LEFT && p.y <= middle))	left_select = true;
			if (Pool.selection == BOTH || (Pool.selection == RIGHT && p.y >= middle))	right_select = true;

			bool drag_area = (p.x > pointer_x && p.x < sel_pointer_x) && Pool.selection != NONE
							&& !left_pointer && !right_pointer && (left_select || right_select);

			/* Do the selecting */
			if (button == B_SECONDARY_MOUSE_BUTTON)
			{																/* show a pop-up menu here */
				BMenuItem *menuItem;
				BPopUpMenu *menu = new BPopUpMenu("");

				menu->AddItem(new BMenuItem(Language.get("TRANSPORT_PLAYS"), new BMessage(TRANSPORT_PLAYS), KeyBind.GetKey("TRANSPORT_PLAYS"), KeyBind.GetMod("TRANSPORT_PLAYS")));
				menu->AddItem(menuItem = new BMenuItem(Language.get("TRANSPORT_PAUSE"), new BMessage(TRANSPORT_PAUSE_MAN), KeyBind.GetKey("TRANSPORT_PAUSE"), KeyBind.GetMod("TRANSPORT_PAUSE")));
				if (((MainWindow*)Pool.mainWindow)->transport_view->pause->Value() == B_CONTROL_ON)
					menuItem->SetMarked(true);
				menu->AddItem(new BMenuItem(Language.get("TRANSPORT_STOP"), new BMessage(TRANSPORT_STOP), KeyBind.GetKey("TRANSPORT_STOP"), KeyBind.GetMod("TRANSPORT_STOP")));
				menu->AddSeparatorItem();
				menu->AddItem(menuItem = new BMenuItem(Language.get("COPY"), new BMessage(B_COPY), KeyBind.GetKey("COPY"), KeyBind.GetMod("COPY")));
				menuItem->SetEnabled(Pool.selection != NONE);
				menu->AddItem(menuItem = new BMenuItem(Language.get("COPY_SILENCE"), new BMessage(COPY_SILENCE), KeyBind.GetKey("COPY_SILENCE"), KeyBind.GetMod("COPY_SILENCE")));
				menuItem->SetEnabled(Pool.selection != NONE);
				menu->AddItem(menuItem = new BMenuItem(Language.get("CUT"), new BMessage(B_CUT), KeyBind.GetKey("CUT"), KeyBind.GetMod("CUT")));
				menuItem->SetEnabled(Pool.selection != NONE);
				menu->AddItem(menuItem = new BMenuItem(Language.get("PASTE"), new BMessage(B_PASTE), KeyBind.GetKey("PASTE"), KeyBind.GetMod("PASTE")));
				menuItem->SetEnabled(ClipBoard.HasClip());
				menu->AddItem(menuItem = new BMenuItem(Language.get("PASTE_NEW"), new BMessage(PASTE_NEW), KeyBind.GetKey("PASTE_NEW"), KeyBind.GetMod("PASTE_NEW")));
				menuItem->SetEnabled(ClipBoard.HasClip());
				#ifdef __SAMPLE_STUDIO_LE
					menu->AddItem(menuItem = new BMenuItem(Language.get("EDIT_PASTE_MIX"), new BMessage(PASTE_MIXED), KeyBind.GetKey("EDIT_PASTE_MIX"), KeyBind.GetMod("EDIT_PASTE_MIX")));
					menuItem->SetEnabled(ClipBoard.HasClip());
					menu->AddItem(menuItem = new BMenuItem(Language.get("COPY_TO_STACK"), new BMessage(TO_STACK), KeyBind.GetKey("COPY_TO_STACK"), KeyBind.GetMod("COPY_TO_STACK")));
					menuItem->SetEnabled(Pool.selection != NONE);
				#endif
				menu->AddSeparatorItem();
				menu->AddItem(menuItem = new BMenuItem(Language.get("SELECT_ALL"), new BMessage(B_SELECT_ALL), KeyBind.GetKey("SELECT_ALL"), KeyBind.GetMod("SELECT_ALL")));
				menu->AddItem(menuItem = new BMenuItem(Language.get("UNSELECT_ALL"), new BMessage(UNSELECT_ALL), KeyBind.GetKey("UNSELECT_ALL"), KeyBind.GetMod("UNSELECT_ALL")));
				menuItem->SetEnabled(Pool.selection != NONE);
				menu->AddSeparatorItem();
				BMenu *sub = new BMenu(Language.get("ZERO_CROSS"));
				sub->AddItem(new BMenuItem(Language.get("ZERO_IN"), new BMessage(ZERO_IN), KeyBind.GetKey("ZERO_IN"), KeyBind.GetMod("ZERO_IN")));
				sub->AddItem(new BMenuItem(Language.get("ZERO_OUT"), new BMessage(ZERO_OUT), KeyBind.GetKey("ZERO_OUT"), KeyBind.GetMod("ZERO_OUT")));
				sub->AddItem(new BMenuItem(Language.get("ZERO_LL"), new BMessage(ZERO_LL), KeyBind.GetKey("ZERO_LL"), KeyBind.GetMod("ZERO_LL")));
				sub->AddItem(new BMenuItem(Language.get("ZERO_LR"), new BMessage(ZERO_LR), KeyBind.GetKey("ZERO_LR"), KeyBind.GetMod("ZERO_LR")));
				sub->AddItem(new BMenuItem(Language.get("ZERO_RL"), new BMessage(ZERO_RL), KeyBind.GetKey("ZERO_RL"), KeyBind.GetMod("ZERO_RL")));
				sub->AddItem(new BMenuItem(Language.get("ZERO_RR"), new BMessage(ZERO_RR), KeyBind.GetKey("ZERO_RR"), KeyBind.GetMod("ZERO_RR")));
				sub->SetTargetForItems(Window());
				menu->AddItem(sub);

				menu->AddSeparatorItem();
				sub = new BMenu(Language.get("ZOOM_FUNCTIONS"));
				sub->AddItem(new BMenuItem(Language.get("ZOOM_IN"), new BMessage(ZOOM_IN), KeyBind.GetKey("ZOOM_IN"), KeyBind.GetMod("ZOOM_IN")));
				sub->AddItem(new BMenuItem(Language.get("ZOOM_OUT"), new BMessage(ZOOM_OUT), KeyBind.GetKey("ZOOM_OUT"), KeyBind.GetMod("ZOOM_OUT")));
				sub->AddItem(new BMenuItem(Language.get("ZOOM_FULL"), new BMessage(ZOOM_FULL), KeyBind.GetKey("ZOOM_FULL"), KeyBind.GetMod("ZOOM_FULL")));
				sub->AddItem(menuItem = new BMenuItem(Language.get("ZOOM_SELECTION"), new BMessage(ZOOM_SELECTION), KeyBind.GetKey("ZOOM_SELECTION"), KeyBind.GetMod("ZOOM_SELECTION")));
				menuItem->SetEnabled(Pool.selection != NONE);
				sub->AddItem(menuItem = new BMenuItem(Language.get("ZOOM_LEFT"), new BMessage(ZOOM_LEFT), KeyBind.GetKey("ZOOM_LEFT"), KeyBind.GetMod("ZOOM_LEFT")));
				menuItem->SetEnabled(Pool.selection != NONE);
				sub->AddItem(menuItem = new BMenuItem(Language.get("ZOOM_RIGHT"), new BMessage(ZOOM_RIGHT), KeyBind.GetKey("ZOOM_RIGHT"), KeyBind.GetMod("ZOOM_RIGHT")));
				menuItem->SetEnabled(Pool.selection != NONE);
				sub->SetTargetForItems(Window());
				menu->AddItem(sub);


				ConvertToScreen(&p);
				menu->SetTargetForItems(Window());
				menuItem = menu->Go(p, true, false, true); 
//				if ( menuItem && menuItem->Message())
//					Window()->PostMessage(menuItem->Message());

			}
			// Check for extending the selection
			else if ((left_pointer || right_pointer) && (left_select || right_select) ||
					(button == B_PRIMARY_MOUSE_BUTTON && (modifiers() & B_SHIFT_KEY))){
				/* Drag the end-pointers */
				drag_border = true;
				t = (int32)(Pool.l_pointer + p.x * (Pool.r_pointer - Pool.l_pointer)/Bounds().Width());
				if (t > (Pool.pointer + Pool.r_sel_pointer)/2){					// drag the right part
					drag = true;
					t = (int32)Pool.pointer;			// use original begin of selection
					MouseMoved( p, 0, NULL );
				}else{
					drag = true;
					t = (int32)(Pool.r_sel_pointer);
					MouseMoved( p, 0, NULL );
				}
			}else if (drag_area)			/* start a dragging session */
			{
				drag_selection = true;
				start_selection = p;
			}
			// Here we start a new selection
			else if (button == B_PRIMARY_MOUSE_BUTTON && !(modifiers() & B_SHIFT_KEY)){					// do new selection
				old_x = -1;
				drag = true;
				Pool.selection = NONE;
				Pool.r_sel_pointer = 0;
				t = (int32)(Pool.l_pointer + p.x * (Pool.r_pointer - Pool.l_pointer)/Bounds().Width());
				Pool.pointer = t;
			
				old = p;
				start_selection = p;
			}else{
				// interactive zooming
			
				float factor;
				float y = p.y;
				int32 m = (Pool.r_pointer+Pool.l_pointer)/2;				// center
				int32 x = Pool.r_pointer - Pool.l_pointer;
				float old_y = p.y;

				int32 in = (int32)(x - Bounds().Width()/16);		// possible to zoom in
				int32 out = Pool.size -x;							// possible to zoom out
				if (out > in*6)			out = in*6;
				int32 z;
			
				float old_x = p.x;

				while(button){
					GetMouse(&p, &button);
					
					if (p.y != old_y || p.x != old_x){
						factor = (y-p.y) *0.005;		// scaled zoom
						if (factor<0){
							if (factor<-1)	factor =-1;
							z = (int32)((factor+1) * in/2 +Bounds().Width()/32);
						}else{
							if (factor>1)	factor =1;
							z = (int32)(factor * out/2 +x/2);
						}

						int32 add = (int32)((old_x-p.x)*x/Bounds().Width());
		
						Pool.l_pointer = m - z + add;
						Pool.r_pointer = m + z + add;
					
						if (Pool.l_pointer < 0)			Pool.l_pointer = 0;
						if (Pool.r_pointer > Pool.size)	Pool.r_pointer = Pool.size;

						Invalidate();
						Pool.update_index = true;
						Window()->FindView("Index view")->Invalidate();
						Window()->FindView("Pointers view")->Invalidate();
						Window()->FindView("TimeBar view")->Invalidate();
						old_y = p.y;
						old_x = p.x;
					}
					snooze(100000);
				}
			}
		}else if (clicks == 2 && Prefs.select_all_on_double){		/* The double-click selection */
			Pool.SelectAll();
		}
		break;
	}
	case DRAW_TOOL:												/* Drawing with the Pencil */
	{
		if (Pool.tool_mode == DRAW_TOOL && clicks == 1){
			// save undo data
			if (Prefs.save_undo){
				Hist.Save(H_REPLACE, Pool.l_pointer, Pool.r_pointer);
				Pool.UpdateMenu();
			}

			edit_channel = NONE;	// needed to track stereo editing
			old = p;
			EditPoint( p );
			edit = true;
		}	
		break;
	}
	case PLAY_TOOL:												/* Playing tool */
	{
		stop_following = true;
		Pool.selection = NONE;									// Set the play-pointer
		Pool.r_sel_pointer = 0;
		Pool.pointer = (int32)(Pool.l_pointer + p.x * (Pool.r_pointer - Pool.l_pointer)/Bounds().Width());
		if (button){
			bool bRight = (button & B_SECONDARY_MOUSE_BUTTON);
			Pool.StartPlaying(Pool.pointer*Pool.sample_type, true);	// play till end
			Draw(Bounds());
			snooze(10000);
			while(button){										// Keep playing as long as the mouse is pressed
				GetMouse(&p, &button);
				Pulse();
				Window()->FindView("Big view")->Pulse();
				snooze(10000);
			}
			Pool.StopPlaying();
			if (bRight)
				Pool.pointer = Pool.last_pointer;
		stop_following = false;
		}
		break;
	}
	case SCRUB_TOOL:											/* Scrubbing Tool */
	{
		stop_following = true;
		Pool.selection = NONE;									// Set the play-pointer
		Pool.r_sel_pointer = 0;
		Pool.pointer = (int32)(Pool.l_pointer + p.x * (Pool.r_pointer - Pool.l_pointer)/Bounds().Width());
		
		bool bLoop = Pool.SetLoop(true);
		
		if (button){
			Pool.StartPlaying(Pool.pointer*Pool.sample_type, true);	// play till end
			Draw(Bounds());
			snooze(10000);
			Pool.last_pointer = Pool.pointer;
			while(button){										// Keep playing as long as the mouse is pressed
				GetMouse(&p, &button);

				float x = (Pool.last_pointer-Pool.l_pointer) * Bounds().Width() /(Pool.r_pointer - Pool.l_pointer);
				Pool.frequency = Pool.system_frequency * (p.x-x)/50.0;

				Pulse();
				Window()->FindView("Big view")->Pulse();
				snooze(10000);
			}
			Pool.SetLoop(bLoop);
			Pool.StopPlaying();
			Pool.pointer = Pool.last_pointer;
			Pool.frequency = Pool.system_frequency;
			stop_following = false;
		}
		break;
	}
	default:
		break;
	}

	Invalidate();
	Window()->FindView("Pointers view")->Invalidate();
	Pool.update_index = true;
	Window()->FindView("Index view")->Invalidate();
	Window()->FindView("Big view")->Invalidate();
	Pool.UpdateMenu();
}


/*****************************************************
*	Mouse Moved
*****************************************************/
void SampleView::MouseMoved(BPoint p, uint32 button, const BMessage *msg)
{
	// area where the cursors work for left/right selection
	float top = (Bounds().Height()-POINTER_BAR_HEIGHT)*0.20 +POINTER_BAR_HEIGHT;	// left
	float bottom = (Bounds().Height()-POINTER_BAR_HEIGHT)*0.80+POINTER_BAR_HEIGHT;	// right
	float middle = (Bounds().Height()-POINTER_BAR_HEIGHT)*0.50+POINTER_BAR_HEIGHT;	// middle

	// calculate position of cursors on screen
	float pointer_x = (Pool.pointer-Pool.l_pointer) * Bounds().Width() /(Pool.r_pointer - Pool.l_pointer);
	float sel_pointer_x = (Pool.r_sel_pointer-Pool.l_pointer+1) * Bounds().Width() /(Pool.r_pointer - Pool.l_pointer);
	
	bool left_pointer = (p.x < pointer_x+3 && p.x > pointer_x-3) && Pool.selection != NONE;
	bool right_pointer = (p.x < sel_pointer_x+3 && p.x > sel_pointer_x-3) && Pool.selection != NONE;

	bool left_select = false, right_select = false;
	if (Pool.selection == BOTH || (Pool.selection == LEFT && p.y <= middle))	left_select = true;
	if (Pool.selection == BOTH || (Pool.selection == RIGHT && p.y >= middle))	right_select = true;
		
	bool drag_area = (p.x > pointer_x && p.x < sel_pointer_x) && Pool.selection != NONE
					&& !left_pointer && !right_pointer && (left_select || right_select);

	if (Pool.tool_mode == DRAW_TOOL)			/* pencil mouse cursor */
		SetViewCursor( Pool.mousePencil );
	else if (Pool.tool_mode == SELECT_TOOL)		/* mousecursors for selections */
	{
		if (drag_selection)	SetViewCursor( Pool.mouseArrow );	// drag&drop cursor goes above all
		else if (drag_border || (!drag && (left_pointer || right_pointer) && (left_select || right_select)))
			SetViewCursor( Pool.mouseLeftRight );
		else if (Pool.sample_type != STEREO )
			SetViewCursor( Pool.mouseArrow );
		else
		{
			if (drag_area && !drag)
				SetViewCursor( Pool.mouseMove );
			else if (p.y < top && p.y >= POINTER_BAR_HEIGHT)
				SetViewCursor( Pool.mouseArrowLeft );
			else if (p.y > bottom)
				SetViewCursor( Pool.mouseArrowRight );
			else
				SetViewCursor( Pool.mouseArrow );
		}
	}
	else	SetViewCursor( B_CURSOR_SYSTEM_DEFAULT );

	// 
	// Handle the mouse-move events
	//
	/* decide which part would be selected */
	bool full_update = false;
	
	if (Prefs.drag_drop && drag_selection)				/* drag & drop */
	{
		if (abs(p.x - start_selection.x) >3)
		{
			BRect r = Bounds();
			r.top += POINTER_BAR_HEIGHT;
			r.left = p.x;
			r.right = p.x+1;
				
			if (modifiers() & B_COMMAND_KEY)
				ClipBoard.Cut(true);
			else
				ClipBoard.Copy();

			BMessage msg(DROP_PASTE);
			DragMessage(&msg, r);
			
			drag_selection = false;
		}
		old = p;
	}
	else if (drag && p!=old){		/* do the selecting */
		if (Pool.sample_type == MONO)
		{
			Pool.selection = BOTH;
			SetViewCursor( Pool.mouseArrow );
		}
		else	/* Check to see which channels are selected */
		{
			if (p.y < top)
			{
				if (Pool.selection == BOTH)	full_update = true;
				Pool.selection = LEFT;
			}
			else if (p.y > bottom)
			{
				if (Pool.selection == BOTH)	full_update = true;
				Pool.selection = RIGHT;
			}
			else
			{
				if (Pool.selection == LEFT || Pool.selection == RIGHT)	full_update = true;
				Pool.selection = BOTH;
			}
		}
			
		t2 = (int32)(Pool.l_pointer + p.x * (Pool.r_pointer - Pool.l_pointer)/Bounds().Width());
		
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
		
		Pool.update_index = true;
		Window()->FindView("Pointers view")->Draw(Window()->FindView("Pointers view")->Bounds());
		Window()->FindView("Index view")->Invalidate();
		int32 step = (int32)((Pool.r_pointer-Pool.l_pointer)/Bounds().Width());
		int32 zoom_x = 4;		// normal case just extend the width of the selection triangles
		if (step==0)
			zoom_x =  (int32)MAX(ceil(Bounds().Width()/(Pool.r_pointer - Pool.l_pointer)),POINTER_BAR_HEIGHT) ;

		BRect update;
		if (full_update)
			update = Bounds();
		else if (p.x>old.x)
			update.Set(old.x - zoom_x, 0, p.x + zoom_x, Bounds().bottom);
		else
			update.Set(p.x - zoom_x, 0, old.x + zoom_x, Bounds().bottom);

		if (Pool.IsPlaying()){				// remove line
			SetDrawingMode(B_OP_INVERT);
			StrokeLine( BPoint( old_x, Bounds().top+POINTER_BAR_HEIGHT), BPoint( old_x, Bounds().bottom));
			SetDrawingMode(B_OP_COPY);
		}

		Draw(update);
		old = p;
	}else if (edit){
		EditLine( old, p );
		old = p;
	}
}


/*****************************************************
*	Mouse Up
*****************************************************/
void SampleView::MouseUp(BPoint p)
{
	if (drag_selection && start_selection == p)		/* single clicked in selection without drag -> deselect */
	{
		old_x = -1;
		Pool.selection = NONE;
		Pool.r_sel_pointer = 0;
		Pool.pointer = (int32)(Pool.l_pointer + p.x * (Pool.r_pointer - Pool.l_pointer)/Bounds().Width());
		
		Invalidate();
		Window()->FindView("Pointers view")->Invalidate();
		Pool.update_index = true;
		Window()->FindView("Index view")->Invalidate();
		Window()->FindView("Big view")->Invalidate();
	}

	drag = false;
	drag_selection = false;
	edit = false;
	drag_border = false;
	old.Set(-1,-1);
	Pool.UpdateMenu();
}

//*****************************************************
//
//	Set a sample to a new value indicated by p
//
//*****************************************************
void SampleView::EditPoint(BPoint p)
{
	// the sampleView area
	BRect r = Bounds();
	r.top += POINTER_BAR_HEIGHT;

	if ( p.x < 0 )
		p.x = 0;
	if ( p.x > r.right )
		p.x = r.right;

	// get the offset sample
	int64 ptr = (int64)(Pool.l_pointer + p.x * (Pool.r_pointer - Pool.l_pointer)/Bounds().Width());

	// step between samples
//	float step = (Pool.r_pointer-Pool.l_pointer)/Bounds().Width();
	float v;
//
// Mono
//
	if (Pool.sample_type == MONO){

		float amp = r.Height() /2.0;
		float mid = amp  +POINTER_BAR_HEIGHT;

		v = (mid - p.y)/amp;
		if (v < -1.0)	v = -1.0;
		if (v > 1.0)	v = 1.0;

		DoDraw(ptr, 1, v);
//
// Stereo
//
	}else if (Pool.sample_type == STEREO){

		r.bottom = r.Height()/2.0f -2.0f +r.top;					// calc left area
		float amp = r.Height()/2.0;
		float mid = amp  +r.top;
		if (r.Contains(p) && edit_channel == NONE)	edit_channel = LEFT;
		
		if (edit_channel == LEFT){
			v = (mid - p.y)/amp;
			if (v < -1.0)	v = -1.0;
			if (v > 1.0)	v = 1.0;

			DoDraw(ptr*2, 2, v);
		}else{
			// right
			r.OffsetTo(0, r.bottom+4.0f);					// calc right area
			mid = amp  +r.top;
			if (r.Contains(p) && edit_channel == NONE)	edit_channel = RIGHT;

			if (edit_channel == RIGHT){
				v = (mid - p.y)/amp;
				if (v < -1.0)	v = -1.0;
				if (v > 1.0)	v = 1.0;

				DoDraw(ptr*2+1, 2, v);
			}
		}
	}

	// Do some screen updating
	Pool.update_peak = true;
	Pool.update_index = true;
	Window()->FindView("Index view")->Invalidate();
	int32 zoom_x =  (int32)MAX(ceil(Bounds().Width()/(Pool.r_pointer - Pool.l_pointer)),POINTER_BAR_HEIGHT) ;
	if (p.x>old.x){
		Invalidate(BRect(BPoint(old.x-zoom_x*2, 0), BPoint(p.x+zoom_x, Bounds().bottom)));
	}else{
		Invalidate(BRect(BPoint(p.x-zoom_x*2, 0), BPoint(old.x+zoom_x, Bounds().bottom)));
	}
	old = p;
}

/*****************************************************
*
*****************************************************/
void SampleView::DoDraw(int64 ptr, int32 add, float v)
{
	int32 step = (int32)ceil((Pool.r_pointer-Pool.l_pointer)/Bounds().Width());

	if (step<=1){
		if (ptr > Pool.size*add) ptr = Pool.size*add;
		if (ptr < 0 ) ptr = 0;
#ifndef __VM_SYSTEM //RAM
		Pool.sample_memory[ptr] = v;
#else
		VM.WriteAt( ptr, v );
#endif
	}else{

		int32 start = ptr - step*add;	// left part
		if (start < 0) start = 0;
		
		for (int32 i=start; i<=ptr; i+=add){
#ifndef __VM_SYSTEM //RAM
			Pool.sample_memory[i] = v;
#else
			VM.WriteAt( i, v );
#endif
		}

		int32 end = ptr + step*add;	// left part
		if (end > Pool.size*add) end = Pool.size*add;
		
		for (int32 i=ptr; i<=end; i+=add){
#ifndef __VM_SYSTEM //RAM
			Pool.sample_memory[i] = v;
#else
			VM.WriteAt( i, v );
#endif
		}
	}
}

/*****************************************************
*	Indicate that the frame is resized
*****************************************************/
void SampleView::FrameResized(float width, float height)
{
	m_resized = true;				// re-allocate offscreen bitmap
	Pool.sample_view_dirty = true;	// update the sample-view
	Pool.update_draw_cache = true;	// update the draw cache
}

/*****************************************************
* Create the cache bitmaps for drawing
* This is needed on resize, start and recoloring
*****************************************************/
void SampleView::CalculateCache()
{
	if (Pool.sample_type == NONE)	return;

	acquire_sem(viewSem);

	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);

	int conv[] = {0, 2, 4, 8, 8, 10, 10, 10, 20, 20, 20, 20};

	// delete used bitmaps for cache
	if (leftCache)		{ delete leftCache;	leftCache = NULL; cache_left_valid = false;}
	if (rightCache)		{ delete rightCache; rightCache = NULL; cache_right_valid = false;}
	if (leftSelected)	{ delete leftSelected; leftSelected = NULL; }
	if (rightSelected)	{ delete rightSelected; rightSelected = NULL; }

//printf("allocated\n");

	BRect r = Bounds();
	r.bottom -= POINTER_BAR_HEIGHT;
	float amp = (r.IntegerHeight()+1)/2.0;
	int32 size;
	if (Pool.sample_type == MONO)
		size = (int32)amp;
	else
		size = (int32)(amp+1)/2;

	int32 height_div = (int)ceil(amp/font.Size()) & 0xfffffe;
	if (height_div>20)	height_div = 20;
	height_div = conv[height_div/2];

	int32 peak = (int32)(r.top + amp*(1.0f - Prefs.peak));
	// size of cache
	BRect rect(0,0,size, size);
		
	leftCache = new BBitmap(rect, B_RGB32);
	leftSelected = new BBitmap(rect, B_RGB32);
		
	leftBits = (rgb_color*)leftCache->Bits();
	leftSelectedBits = (rgb_color*)leftSelected->Bits();
	leftWidth = leftCache->Bounds().IntegerWidth()+1;
	
	BitmapDrawer draw_left(leftCache);
	BitmapDrawer draw_leftSelected(leftSelected);

	rgb_color back2 = Prefs.back_color;				// background top
	rgb_color back1 = Prefs.back_color2;			// background middle
	rgb_color back2s = Prefs.back_selected_color;	// background top selected
	rgb_color back1s = Prefs.back_selected_color2;	// background middle selected
	rgb_color fore2s = Prefs.left_selected_color;	// foreground top
	rgb_color fore1s = Prefs.left_selected_color2;	// foreground middle
	rgb_color fore2 = Prefs.left_color;				// foreground top selected
	rgb_color fore1 = Prefs.left_color2;			// foreground middle selected

	float grid_y = 0;
	// draw the background
	rgb_color a, b, c, d;
	int32 temp;
	for(int32 y=0; y<=size; y++)
	{
		uint8 alpha = y*255/size;
		if (y==size)
		{
			// middle
			a = Prefs.mid_left_color;
			b = Prefs.mid_left_selected_color;
		}else
		if (Prefs.show_grid && y == (int)grid_y && y != size-1)
		{
			// horizontal grid
			a = Prefs.grid_color;
			b = Prefs.grid_selected_color;
			grid_y += r.Height()/height_div;
		}else
		if (y == peak && Prefs.show_peak)
		{
			// peak lines
			a = Prefs.peak_color;
			b = Prefs.peak_selected_color;
		}
		else
		{
			a.red = INT_BLEND(back1.red, back2.red, alpha, temp);
			a.green = INT_BLEND(back1.green, back2.green, alpha, temp);
			a.blue = INT_BLEND(back1.blue, back2.blue, alpha, temp);
			
			b.red = INT_BLEND(back1s.red, back2s.red, alpha, temp);
			b.green = INT_BLEND(back1s.green, back2s.green, alpha, temp);
			b.blue = INT_BLEND(back1s.blue, back2s.blue, alpha, temp);
		}

		c.red = INT_BLEND(fore1.red, fore2.red, alpha, temp);
		c.green = INT_BLEND(fore1.green, fore2.green, alpha, temp);
		c.blue = INT_BLEND(fore1.blue, fore2.blue, alpha, temp);
			
		d.red = INT_BLEND(fore1s.red, fore2s.red, alpha, temp);
		d.green = INT_BLEND(fore1s.green, fore2s.green, alpha, temp);
		d.blue = INT_BLEND(fore1s.blue, fore2s.blue, alpha, temp);
			
		// background
		for(int32 x=0; x<y; x++)
		{
			draw_left.PlotRGB( size - x, y, c );
			draw_leftSelected.PlotRGB( size - x, y, d );
		}
		// foreground
		for(int32 x=y; x<=size; x++)
		{
			draw_left.PlotRGB( size - x, y, a );
			draw_leftSelected.PlotRGB( size - x, y, b );
		}
	}
	
	cache_left_valid = true;

	if (Pool.sample_type == STEREO)
	{
		rightCache = new BBitmap(rect, B_RGB32);
		rightSelected = new BBitmap(rect, B_RGB32);
		
		rightBits = (rgb_color*)rightCache->Bits();
		rightSelectedBits = (rgb_color*)rightSelected->Bits();
	
		BitmapDrawer draw_right(rightCache);
		BitmapDrawer draw_rightSelected(rightSelected);

		rgb_color fore2s = Prefs.right_selected_color;	// foreground top
		rgb_color fore1s = Prefs.right_selected_color2;	// foreground middle
		rgb_color fore2 = Prefs.right_color;			// foreground top selected
		rgb_color fore1 = Prefs.right_color2;			// foreground middle selected

		float grid_y = 0;
		// draw the background
		rgb_color a, b, c, d;
		int32 temp;
		for(int32 y=0; y<=size; y++)
		{
			uint8 alpha = y*255/size;
			if (y==size)
			{
				// middle
				a = Prefs.mid_right_color;
				b = Prefs.mid_right_selected_color;
			}else
			if (Prefs.show_grid && y == (int)grid_y && y != size-1)
			{
				// horizontal grid
				a = Prefs.grid_color;
				b = Prefs.grid_selected_color;
				grid_y += r.Height()/height_div;
			}else
			if (y == peak && Prefs.show_peak)
			{
				// peak lines
				a = Prefs.peak_color;
				b = Prefs.peak_selected_color;
			}
			else
			{
				a.red = INT_BLEND(back1.red, back2.red, alpha, temp);
				a.green = INT_BLEND(back1.green, back2.green, alpha, temp);
				a.blue = INT_BLEND(back1.blue, back2.blue, alpha, temp);
	
				b.red = INT_BLEND(back1s.red, back2s.red, alpha, temp);
				b.green = INT_BLEND(back1s.green, back2s.green, alpha, temp);
				b.blue = INT_BLEND(back1s.blue, back2s.blue, alpha, temp);
			}

			c.red = INT_BLEND(fore1.red, fore2.red, alpha, temp);
			c.green = INT_BLEND(fore1.green, fore2.green, alpha, temp);
			c.blue = INT_BLEND(fore1.blue, fore2.blue, alpha, temp);
			
			d.red = INT_BLEND(fore1s.red, fore2s.red, alpha, temp);
			d.green = INT_BLEND(fore1s.green, fore2s.green, alpha, temp);
			d.blue = INT_BLEND(fore1s.blue, fore2s.blue, alpha, temp);
			
			// background
			for(int32 x=0; x<y; x++)
			{
				draw_right.PlotRGB( size - x, y, c );
				draw_rightSelected.PlotRGB( size - x, y, d );
			}
			// foreground
			for(int32 x=y; x<=size; x++)
			{
				draw_right.PlotRGB( size - x, y, a );
				draw_rightSelected.PlotRGB( size - x, y, b );
			}
		}
		cache_right_valid = true;
	}
	release_sem(viewSem);
}

/*****************************************************
*	Render MONO view
*****************************************************/
void SampleView::DrawMono(BRect rect, bool left, bool draw_selection)
{
	acquire_sem(viewSem);
	
	// Update the peak-cache if needed
	if (Pool.update_peak
	 || Pool.update_draw_cache
	 || m_old_l_pointer != Pool.l_pointer
	 || m_old_r_pointer != Pool.r_pointer )
	{
		if (left){
			if (Pool.sample_type == MONO)
				Peak.MonoBuffer(peak_buffer_l, Pool.l_pointer, Pool.r_pointer, Bounds().IntegerWidth()+1);
			else if (Pool.sample_type == STEREO)
				Peak.StereoBuffer(peak_buffer_l, peak_buffer_r, Pool.l_pointer, Pool.r_pointer, Bounds().IntegerWidth()+1);
		}
	}
	int32 size = rect.IntegerHeight();
	int32 size2 = size/2;
	if ((size-size2)==size2)		/* even */
	{
		size = size2;
	}
	else							/* odd */
	{
		size = size2 +1;
	}

	rgb_color *inBits, *inSelectedBits, *outBits;
	rgb_color col, colS;	// this one is used for line drawing
	float *peak_buffer;
	if (left)		/* fill pointers for left channel */
	{
		outBits = screenBits;
		inBits = leftBits;
		inSelectedBits = leftSelectedBits;
		col = Prefs.left_color2;
		colS = Prefs.left_selected_color2;
		peak_buffer = peak_buffer_l;
	}
	else			/* fill pointers for right channel */
	{
		outBits = screenBits + size*2 * screenWidth;
		inBits = rightBits;
		inSelectedBits = rightSelectedBits;
		col = Prefs.right_color2;
		colS = Prefs.right_selected_color2;
		peak_buffer = peak_buffer_r;
	}
	uint8 red = col.red; col.red = col.blue; col.blue = red;	// swap red/blue for direct draw
	red = colS.red; colS.red = colS.blue; colS.blue = red;	// swap red/blue for direct draw

	// do the left part of the back
	if (!draw_selection || Pool.l_pointer<Pool.pointer || Pool.r_sel_pointer<Pool.l_pointer)
	{
		BRect r = Bounds();
		if (draw_selection && Pool.r_pointer>Pool.r_sel_pointer && !(Pool.r_sel_pointer < Pool.l_pointer))		// clip
			r.right -= (Pool.r_pointer-Pool.pointer) * Bounds().Width() /(Pool.r_pointer - Pool.l_pointer);
		
		// only draw when in update section
		if (r.right >= rect.left)
		{
			if (r.left < rect.left)	r.left = rect.left;
			DrawPart( inBits, outBits, col, peak_buffer, r, size, size2);
		}
	}

	// do the middle part of the back
	if (draw_selection && Pool.r_sel_pointer>=Pool.l_pointer && Pool.pointer<=Pool.r_pointer)
	{
		BRect r = Bounds();
		if (Pool.pointer>Pool.l_pointer)		// clip left
			r.left += floor((Pool.pointer-Pool.l_pointer) * Bounds().Width() /(Pool.r_pointer - Pool.l_pointer)+1);

		if (Pool.r_sel_pointer<Pool.r_pointer)	// clip right
			r.right -= ceil((Pool.r_pointer-Pool.r_sel_pointer-1) * Bounds().Width() /(Pool.r_pointer - Pool.l_pointer)-1);

		if (r.right == r.left)	r.left--;	// make sure there is always a selection visible, even when 1 pixel
		if (r.right >= rect.left && r.left <= rect.right)
		{
			if (r.right > rect.right)	r.right = rect.right;
			if (r.left < rect.left)		r.left = rect.left;
			DrawPart( inSelectedBits, outBits, colS, peak_buffer, r, size, size2);
		}
	}

	// do the right part of the back
	if (draw_selection && Pool.r_sel_pointer>Pool.l_pointer && Pool.r_sel_pointer<Pool.r_pointer)
	{
		BRect r = Bounds();
		r.left += (Pool.r_sel_pointer-Pool.l_pointer+1) * Bounds().Width() /(Pool.r_pointer - Pool.l_pointer)+1;
		if (r.left <= rect.right)
		{
			if (rect.left > r.left)		r.left = rect.left;
			if (rect.right < r.right)	r.right = rect.right;
			DrawPart( inBits, outBits, col, peak_buffer, r, size, size2);
		}
	}
	release_sem(viewSem);
}

/*****************************************************
*	Render STEREO view
*****************************************************/
void SampleView::DrawPart( rgb_color *inBits, rgb_color *outBits,
						   rgb_color col, float *peak_buffer,
						   BRect r, int32 size, int32 size2)
{
//r.PrintToStream();

	rgb_color *src = NULL;
	rgb_color *dest = NULL;

	int32 draw_mode;
	int32 step = (Pool.r_pointer-Pool.l_pointer)/Bounds().IntegerWidth();
//printf("step %d\n", step);

	if (step<1){
		draw_mode = DRAW_POINTS;
	}else if ( step < 64 )
		draw_mode = DRAW_PLAIN;
	else
		draw_mode = DRAW_PEAK;

	switch (draw_mode){
	case DRAW_POINTS:
	case DRAW_PLAIN:
	{
//printf("draw plain\n");
		int middle;
		int32 index = (int32)r.left*2;		// calc startpoint in buffer
//printf("index %d\n", index);
		int32 old_index = index-2;
		if (old_index<0)
			old_index = 0;

		int32 old_x1 = (int32)(size*peak_buffer[old_index]);		// -1
//printf("old_x1 %d left %f right %f\n", old_x1, r.left, r.right);
		for (int32 x = (int32)r.left; x<=(int32)r.right; x++){

			int32 x1 = (int32)(size*peak_buffer[index]);		// -1
//printf("x1 %d\n", x1);

			dest = outBits +x;		// screen
			src = inBits;			// background
			for (int32 y=0; y<=size; y++)
			{
				*dest = *src;
				dest += screenWidth;
				src += leftWidth;
			}
			src -= leftWidth;
			for (int32 y=0; y<size2; y++)
			{
				src -= leftWidth;
				*dest = *src;
				dest += screenWidth;
			}

			if (x1 == old_x1)
			{
//printf("x1==old_x1\n");
				*(outBits + x + (size-x1)*screenWidth) = col;
			}
			else if (x1 < old_x1)
			{
//printf("x1 < old_x1\n");
				middle = ( x1 + old_x1 +1)>>1;
				for (int32 offset = x1; offset < middle; offset++)
					*(outBits + x + (size-offset)*screenWidth) = col;

				if (x != 0)
					for (int32 offset = middle; offset < old_x1; offset++)
						*(outBits + x-1 + (size-offset)*screenWidth) = col;
			}
			else
			{
//printf("x1 > old_x1\n");
				middle = ( x1 + old_x1 )>>1;
				if (x != 0)
					for (int32 offset = old_x1; offset < middle; offset++)
						*(outBits + x-1 + (size-offset)*screenWidth) = col;

				for (int32 offset = middle; offset < x1; offset++)
					*(outBits + x + (size-offset)*screenWidth) = col;
			}
			old_x1 = x1;
					
			index += 2;
		}
	}	break;
				
	default:
//printf("default\n");
		// need to do the first unselected part here
		int32 index = (int32)r.left*2;		// calc startpoint in buffer
//printf("index %d left %f right %f p %p  l:%p r:%p\n", index, r.left, r.right, peak_buffer, peak_buffer_l, peak_buffer_r);
		for (int32 x = (int32)r.left; x<=(int32)r.right; x++){
			int32 x1 = (int32)(-size*peak_buffer[index]);		// -1
			int32 x2 = (int32)(size*peak_buffer[index+1]);	// +1
	
//printf("x %d x1 %d  x2 %d \n", x, x1, x2);
			// offset in screenbuffer
			dest = outBits +x;
//printf("1\n");
			// offset in cache
			src = inBits + x2;
//printf("2 src %p  size %d  sw %d lw %d  inBits %p\n", src, size, screenWidth, leftWidth, inBits);
			for (int32 y=0; y<=size; y++)
			{
				*dest = *src;
				dest += screenWidth;
				src += leftWidth;
			}
//printf("3\n");
			// offset in cache
			src = inBits + x1 +size * leftWidth - leftWidth;
//printf("4\n");
			for (int32 y=0; y<size2; y++)
			{
				*dest = *src;
				dest += screenWidth;
				src -= leftWidth;
			}
//printf("5\n");
			index += 2;
		}
//		printf("/n");
		break;
	}
}

/*****************************************************
*	Render STEREO view
*****************************************************/
void SampleView::DrawStereo(BRect rect)
{
	BRect r = rect;
	r.bottom -= POINTER_BAR_HEIGHT;
	r.bottom /= 2;
	if (cache_left_valid)
		DrawMono(r, true, (Pool.selection == LEFT || Pool.selection == BOTH));
	
	r.OffsetBy(0, r.Height());
	if (cache_right_valid)
		DrawMono(r, false, (Pool.selection == RIGHT || Pool.selection == BOTH));
}

/*****************************************************
*	Edit a line
*****************************************************/
void SampleView::EditLine(BPoint pa, BPoint pb)
{
	if(pa==pb)
		EditPoint(pa);
	else
	{
		float delta_x = (pa.x - pb.x);
		float delta_y = (pa.y - pb.y);

		if (delta_x < 0) delta_x *= -1;
		if (delta_y < 0) delta_y *= -1;

		float delta = sqrt((delta_x*delta_x) + (delta_y*delta_y));
		float nbpts;

		nbpts = delta / 1.0;
   
		BPoint pos;
		for (float i = 0; i < nbpts-0; i++){
			pos.x = ROUND(pa.x + i*(pb.x-pa.x)/nbpts);	// was floor
			pos.y = ROUND(pa.y + i*(pb.y-pa.y)/nbpts);

			EditPoint(pos);
		}
	}
}
