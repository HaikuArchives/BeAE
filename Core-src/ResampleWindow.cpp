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
#include <Entry.h>
#include <Path.h>
#include <File.h>
#include <Volume.h>

#include "Globals.h"
#include "FilterDialogs.h"
#include "SpinControl.h"
#include "MyStringItem.h"

#define SET_TEXT		'setT'
#define SET_BITS		'setB'
#define SELECT			'slct'
#define CHANGE_CHANNEL	'chgC'
#define SELECT_BITS		'selB'

/*******************************************************
*   
*******************************************************/
ResampleWindow::ResampleWindow(BPoint p) : BWindow(BRect(p.x,p.y,p.x,p.y),Language.get("RESAMPLE_WINDOW"),B_FLOATING_WINDOW_LOOK,B_MODAL_APP_WINDOW_FEEL, B_NOT_RESIZABLE|B_NOT_ZOOMABLE)
{
	m_frequency = Pool.frequency;

	BRect rect(0,0,350,200);
	ResizeTo(rect.Width(), rect.Height());
	MoveBy(-rect.Width()/2, -rect.Height()/2);

// The SampleRate Box
	view = new BView(rect, NULL, B_FOLLOW_ALL, B_WILL_DRAW);
	rect.InsetBy(8,8);
	rect.right = 110;
	BBox *rate_box = new BBox(rect, NULL);
	rate_box->SetLabel(Language.get("RATE"));
	BRect r = rate_box->Bounds();
	r.InsetBy(8,8);

	r.top += 36;		// space for the textbox
	r.right -= B_V_SCROLL_BAR_WIDTH;
	list = new BListView(r,"Freq list");
	BScrollView *sv = new BScrollView("scroll", list, B_FOLLOW_ALL_SIDES, B_WILL_DRAW, false, true, B_PLAIN_BORDER);
	sv->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	sv->MakeFocus(false);
	rate_box->AddChild(sv);
	
	r.right += B_V_SCROLL_BAR_WIDTH;
	r.top = 18;	r.bottom = 38;
	r.left -= 4;
	text = new SpinControl(r, NULL, NULL, new BMessage(SET_TEXT), 4000, 48000, 44100, 500);
	rate_box->AddChild(text);
	view->AddChild(rate_box);

// The Channels
	rect.left = rect.right+8;
	rect.right = Bounds().right-8;
	rect.bottom -= 32;
	
	r = rect;
	r.bottom = 130;
	r.right = 250;
	BBox *c_box = new BBox(r, NULL);
	c_box->SetLabel(Language.get("CHANNELS"));
	r = c_box->Bounds();
	r.InsetBy(8,8);
	r.OffsetBy(0,8);
	r.bottom = r.top + 19;
	c_box->AddChild(mono = new BRadioButton(r, NULL, Language.get("MONO"), new BMessage(CHANGE_CHANNEL)));
	if (Pool.sample_type == MONO)	mono->SetValue(B_CONTROL_ON);
	r.OffsetBy(0,20);
	c_box->AddChild(stereo = new BRadioButton(r, NULL, Language.get("STEREO"), new BMessage(CHANGE_CHANNEL)));
	if (Pool.sample_type == STEREO)	stereo->SetValue(B_CONTROL_ON);

	float x = 8 + MAX( be_plain_font->StringWidth(Language.get("LEFT_MIX")), be_plain_font->StringWidth(Language.get("RIGHT_MIX")));
	r.OffsetBy(0,30);
	if (Pool.sample_type==MONO){
		c_box->AddChild(left = new SpinControl(r, NULL, Language.get("LEFT_MIX"), NULL, 0, 400, Prefs.filter_resample_sl, 1));
		r.OffsetBy(0,24);
		c_box->AddChild(right = new SpinControl(r, NULL, Language.get("RIGHT_MIX"), NULL, 0, 400, Prefs.filter_resample_sr, 1));
	}else{
		c_box->AddChild(left = new SpinControl(r, NULL, Language.get("LEFT_MIX"), NULL, 0, 400, Prefs.filter_resample_ml, 1));
		r.OffsetBy(0,24);
		c_box->AddChild(right = new SpinControl(r, NULL, Language.get("RIGHT_MIX"), NULL, 0, 400, Prefs.filter_resample_mr, 1));
	}
	left->SetEnabled(false);
	right->SetEnabled(false);
	left->SetDivider(x);
	right->SetDivider(x);
	view->AddChild(c_box);

// the resolution
	r = rect;
	r.bottom = 130;
	r.left = 258;
	BBox *r_box = new BBox(r, NULL);
	r_box->SetLabel(Language.get("RESOLUTION"));
	r = r_box->Bounds();
	r.InsetBy(8,8);
	r.top += 36;		// space for the textbox
	r.right -= B_V_SCROLL_BAR_WIDTH;
	resolution = new BListView(r,"Bits list");
	sv = new BScrollView("scroll", resolution, B_FOLLOW_ALL_SIDES, B_WILL_DRAW, false, true, B_PLAIN_BORDER);
	sv->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	sv->MakeFocus(false);
	r_box->AddChild(sv);
	
	r.right += B_V_SCROLL_BAR_WIDTH;
	r.top = 18;	r.bottom = 38;
	r.left -= 4;
	bits = new SpinControl(r, NULL, NULL, new BMessage(SET_BITS), 4, 32, Pool.sample_bits, 1);
	r_box->AddChild(bits);
	view->AddChild(r_box);

	StringItem *it;
	resolution->AddItem(it = new StringItem("8"));
	if (Pool.sample_bits <= 8)	it->Select();
	resolution->AddItem(it = new StringItem("16"));
	if (Pool.sample_bits <= 16 && Pool.sample_bits >8)	it->Select();
	resolution->AddItem(it = new StringItem("32"));
	if (Pool.sample_bits <= 32 && Pool.sample_bits >16)	it->Select();
	resolution->SetSelectionMessage(new BMessage(SELECT_BITS));
	resolution->SetInvocationMessage(new BMessage(SELECT_BITS));
	m_bits = Pool.sample_bits;

	r = Bounds();
	r.left = r.right - 80;
	r.top = r.bottom - 32;
	r.bottom -=8;
	r.right -= 8;
	view->AddChild(new BButton(r, NULL, Language.get("APPLY"), new BMessage(SET)) );
//	r.OffsetBy(-(r.Width()+8), 0);
	r.OffsetBy(0,-30);
	view->AddChild(new BButton(r, NULL, Language.get("CANCEL"), new BMessage(B_QUIT_REQUESTED)) );

	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(view);

//	list->AddItem(new StringItem("96000"));
//	list->AddItem(new StringItem("64000"));
	list->AddItem(new StringItem("48000"));
	list->AddItem(new StringItem("44100"));
	list->AddItem(new StringItem("32000"));
	list->AddItem(new StringItem("22050"));
	list->AddItem(new StringItem("16000"));
	list->AddItem(new StringItem("12500"));
	list->AddItem(new StringItem("11025"));
	list->AddItem(new StringItem("8000"));
	list->SetSelectionMessage(new BMessage(SELECT));
	list->SetInvocationMessage(new BMessage(SELECT));
	SetList(true);

	Run();
	Show();
}

/*******************************************************
*   
*******************************************************/
void ResampleWindow::SetList(bool sel){
	BStringItem *it = NULL;
	int32 low = 0;

	if (sel){
		for (int32 i=0; i<list->CountItems(); i++){
			it = (BStringItem*)list->ItemAt(i);
			it->Deselect();
			float frequency = atof( it->Text() );
			if (m_frequency == frequency){	it->Select();	text->SetValue(m_frequency);	}
		}
	}else{
		for (int32 i=0; i<resolution->CountItems(); i++){
			it = (BStringItem*)resolution->ItemAt(i);
			it->Deselect();
			int32 x = atoi( it->Text() );
			if (m_bits > low && m_bits <= x){
				it->Select();
				bits->SetValue(m_bits);
				low = x;
			}
		}
	}
}

/*******************************************************
*   
*******************************************************/
void ResampleWindow::MessageReceived(BMessage* msg)
{
	int32 i;
	BStringItem *item = NULL;

	switch(msg->what){
	case SET:
		if (Pool.sample_type==MONO){
			Prefs.filter_resample_sl = left->Value();
			Prefs.filter_resample_sr = right->Value();
		}else{
			Prefs.filter_resample_ml = left->Value();
			Prefs.filter_resample_mr = right->Value();
		}
		if (mono->Value())
			Prefs.filter_resample_mono = true;
		else
			Prefs.filter_resample_mono = false;
		Prefs.filter_resample_freq = m_frequency;
		Prefs.filter_resample_bits = m_bits;

		Pool.mainWindow->PostMessage(RESAMPLE_DO);
		Quit();
		break;	

	case SELECT:
		i = list->CurrentSelection();
		if(i < 0)	break;		// nothing selected 
		item = (BStringItem*)list->ItemAt(i);
		
		m_frequency = atof( item->Text() );
		text->SetValue(m_frequency);
		SetList(true);
		list->Invalidate();
		break;

	case SELECT_BITS:
		i = resolution->CurrentSelection();
		if(i < 0)	break;		// nothing selected 
		item = (BStringItem*)resolution->ItemAt(i);
		
		m_bits = atoi( item->Text() );
		bits->SetValue(m_bits);
		SetList(false);
		resolution->Invalidate();
		break;

	case SET_TEXT:
		m_frequency = text->Value();
		SetList(true);
		list->Invalidate();
		break;

	case SET_BITS:
		m_bits = bits->Value();
		SetList(false);
		resolution->Invalidate();
		break;
	
	case CHANGE_CHANNEL:
		if (mono->Value()){			// mono selected
			if (Pool.sample_type == MONO){
				left->SetEnabled(false);
				right->SetEnabled(false);
			}else{
				left->SetEnabled(true);
				right->SetEnabled(true);
				left->SetValue(Prefs.filter_resample_ml);
				right->SetValue(Prefs.filter_resample_mr);
			}
		}else{
			if (Pool.sample_type == STEREO){
				left->SetEnabled(false);
				right->SetEnabled(false);
			}else{
				left->SetEnabled(true);
				right->SetEnabled(true);
				left->SetValue(Prefs.filter_resample_sl);
				right->SetValue(Prefs.filter_resample_sr);
			}
		}
		break;

	default:
		BWindow::MessageReceived(msg);
	}
}

