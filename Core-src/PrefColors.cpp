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

#include <InterfaceKit.h>
#include <StorageKit.h>
#include <String.h>
#include <Path.h>
#include <TranslationKit.h>
#include <TranslationUtils.h>
#include <stdio.h>

#include "Globals.h"
#include "PrefColors.h"
#include "SwatchView.h"
#include "MyStringItem.h"

#define COLOR_SELECT		'colS'
#define COLOR_CHANGE		'colC'
#define SWATCH_DROP			'swtc'
#define NEW_SCHEME			'schm'

/*******************************************************
*   Setup the main view. Add in all the niffty components
*   we have made and get things rolling
*******************************************************/
PrefColors::PrefColors(BRect frame):BView(frame, "Prefs color", B_FOLLOW_ALL,0){
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	char s[255];

	colors[0] = (void*)&Prefs.back_color;
	colors[1] = (void*)&Prefs.back_color2;
	colors[2] = (void*)&Prefs.back_selected_color;
	colors[3] = (void*)&Prefs.back_selected_color2;

	colors[4] = (void*)&Prefs.index_back_color;
	colors[5] = (void*)&Prefs.index_back_color2;
	colors[6] = (void*)&Prefs.index_mid_color;
	colors[7] = (void*)&Prefs.index_left_color;
	colors[8] = (void*)&Prefs.index_left_color2;
	colors[9] = (void*)&Prefs.index_back_selected_color;
	colors[10] = (void*)&Prefs.index_back_selected_color2;
	colors[11] = (void*)&Prefs.index_left_selected_color;
	colors[12] = (void*)&Prefs.index_left_selected_color2;
	colors[13] = (void*)&Prefs.index_mid_selected_color;
	colors[14] = (void*)&Prefs.index_pointer_color;

	colors[15] = (void*)&Prefs.left_color;
	colors[16] = (void*)&Prefs.left_color2;
	colors[17] = (void*)&Prefs.left_selected_color;
	colors[18] = (void*)&Prefs.left_selected_color2;

	colors[19] = (void*)&Prefs.right_color;
	colors[20] = (void*)&Prefs.right_color2;
	colors[21] = (void*)&Prefs.right_selected_color;
	colors[22] = (void*)&Prefs.right_selected_color2;

	colors[23] = (void*)&Prefs.grid_color;
	colors[24] = (void*)&Prefs.grid_selected_color;
	colors[25] = (void*)&Prefs.peak_color;
	colors[26] = (void*)&Prefs.peak_selected_color;
	colors[27] = (void*)&Prefs.mid_left_color;
	colors[28] = (void*)&Prefs.mid_right_color;
	colors[29] = (void*)&Prefs.mid_left_selected_color;
	colors[30] = (void*)&Prefs.mid_right_selected_color;
	colors[31] = (void*)&Prefs.pointer_color;
	colors[32] = (void*)&Prefs.time_back_color;
	colors[33] = (void*)&Prefs.time_marks_color;
	colors[34] = (void*)&Prefs.time_small_marks_color;
	colors[35] = (void*)&Prefs.time_text_color;

	BRect r = Bounds();
	r.left = r.right-32;	r.right -= 8;
	r.bottom = 28;	r.top = 4;
	AddChild(color_view = new SwatchView(r, NULL, new BMessage(SWATCH_DROP)));
	color_view->SetEnabled(false);

	r.right -= 32;
	r.left = 8;
	scheme = new BPopUpMenu(Language.get("COLORSCHEME"));
	BMenuItem *menuItem;
	BMenuField *menu = new BMenuField(r,NULL,Language.get("COLORSCHEME"),scheme);
	BMessage *m;
	for (int i=0; i<=4; i++){
		m = new BMessage(NEW_SCHEME);
		m->AddInt32("scheme",i);
		sprintf(s, "SCHEME%d", i+1);
		scheme->AddItem(menuItem = new BMenuItem(Language.get(s), m));
		if (i==0)	menuItem->SetMarked(true);
	}

	menu->SetDivider(be_plain_font->StringWidth(Language.get("COLORSCHEME")) +10);
	AddChild(menu);

	// add the prefs list at the left
	r = Bounds();
	r.InsetBy(4,8);
	r.bottom /= 2;	r.bottom+=16;
	r.right -= B_V_SCROLL_BAR_WIDTH;
	r.OffsetBy(0,30);
	list = new BListView(r,"color list");
	BScrollView *sv = new BScrollView("scroll", list, B_FOLLOW_ALL_SIDES, B_WILL_DRAW, false, true, B_PLAIN_BORDER);
	sv->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	sv->MakeFocus(false);
	AddChild(sv);

	for (int i=1; i<=36; i++){
		sprintf(s, "COLOR%d", i);
		list->AddItem(new StringItem(Language.get(s)));
	}

	AddChild(control = new BColorControl(BPoint(r.left+8, r.bottom+16), B_CELLS_32x8, 1, "colorControl", new BMessage(COLOR_CHANGE)));
	control->SetEnabled(false);
}

/*******************************************************
*  
*******************************************************/
PrefColors::~PrefColors()
{
}

/*******************************************************
*  
*******************************************************/
void PrefColors::AttachedToWindow(){
	list->SetTarget(this);
	list->SetSelectionMessage(new BMessage(COLOR_SELECT));
	list->SetInvocationMessage(new BMessage(COLOR_SELECT));
	control->SetTarget(this);
	color_view->SetTarget(this);
	scheme->SetTargetForItems(this);
}

/*******************************************************
*  
*******************************************************/
void PrefColors::Draw(BRect rect){
	
}

/*******************************************************
*
*******************************************************/
void PrefColors::MessageReceived(BMessage *msg){
	int32 i;
	rgb_color c, *col;

	switch(msg->what){
	case COLOR_SELECT:
		i = list->CurrentSelection();
		if(i < 0){
			control->SetEnabled(false);
			color_view->SetEnabled(false);
			Pool.sample_view_dirty = true;	// update the sample-view
			Pool.update_draw_cache = true;	// update the draw cache
			Pool.update_index = true;		// update the index cache
			Pool.RedrawWindow();
			break; // nothign selected 
		}
		control->SetEnabled(true);
		color_view->SetEnabled(true);
		c = *((rgb_color*)colors[i]);
		c.alpha = 255;
		control->SetValue(c);
		color_view->SetColor(c);
		Pool.sample_view_dirty = true;	// update the sample-view
		Pool.update_draw_cache = true;	// update the draw cache
		Pool.update_index = true;		// update the index cache
		Pool.RedrawWindow();
		break;

	case SWATCH_DROP:
		i = list->CurrentSelection();
		if(i < 0){
			control->SetEnabled(false);
			color_view->SetEnabled(false);
			break; // nothign selected 
		}
		control->SetEnabled(true);
		color_view->SetEnabled(true);
		c = color_view->Color();
		c.alpha = 255;
		col = (rgb_color*)colors[i];
		col->red = c.red;
		col->green = c.green;
		col->blue = c.blue;
		col->alpha = c.alpha;
		control->SetValue(c);
		Pool.sample_view_dirty = true;	// update the sample-view
		Pool.update_draw_cache = true;	// update the draw cache
		Pool.update_index = true;		// update the index cache
		Pool.RedrawWindow();
		break;
		
	case COLOR_CHANGE:
		i = list->CurrentSelection();
		if(i < 0){
			control->SetEnabled(false);
			color_view->SetEnabled(false);
			break; // nothign selected 
		}
		control->SetEnabled(true);
		color_view->SetEnabled(true);
		c = control->ValueAsColor();
		c.alpha = 255;
		col = (rgb_color*)colors[i];
		col->red = c.red;
		col->green = c.green;
		col->blue = c.blue;
		col->alpha = c.alpha;
		color_view->SetColor(c);
		Pool.sample_view_dirty = true;	// update the sample-view
		Pool.update_draw_cache = true;	// update the draw cache
		Pool.update_index = true;		// update the index cache
		Pool.RedrawWindow();
		break;

	case NEW_SCHEME:
		msg->FindInt32("scheme",&i);
		Prefs.SetColorScheme(i);
		Window()->PostMessage(COLOR_SELECT, this);
		break;

	default:
		BView::MessageReceived(msg);
		break;   
	}
}
