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

#include <LayoutBuilder.h>
#include <Window.h>
#include <View.h>
#include <InterfaceKit.h>
#include <stdlib.h>
#include <stdio.h>
#include <Entry.h>
#include <Path.h>
#include <File.h>
#include <Volume.h>
#include <StringItem.h>

#include "Globals.h"
#include "FilterDialogs.h"

#define SET_TEXT		'setT'
#define SET_BITS		'setB'
#define SELECT			'slct'
#define CHANGE_CHANNEL	'chgC'
#define SELECT_BITS		'selB'

/*******************************************************
*   
*******************************************************/
ResampleWindow::ResampleWindow(BPoint p) : BWindow(BRect(p.x,p.y,p.x,p.y),
	Language.get("RESAMPLE_WINDOW"), B_FLOATING_WINDOW_LOOK,
	B_MODAL_APP_WINDOW_FEEL, B_NOT_RESIZABLE | B_NOT_ZOOMABLE
	| B_AUTO_UPDATE_SIZE_LIMITS)
{
	m_frequency = Pool.frequency;


// The SampleRate Box
	view = new BView(NULL, B_WILL_DRAW);
	BBox *rate_box = new BBox(B_EMPTY_STRING);
	rate_box->SetLabel(Language.get("RATE"));

	list = new BListView("Freq list");
	BScrollView *sv = new BScrollView("scroll", list, B_WILL_DRAW, false, true, B_PLAIN_BORDER);
	sv->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	sv->MakeFocus(false);

	text = new BSpinner(NULL, NULL, new BMessage(SET_TEXT));
	text->SetRange(4000, 48000);
	text->SetValue(44100);

	BGroupLayout* rateBoxLayout = BLayoutBuilder::Group<>(B_VERTICAL)
		.SetInsets(B_USE_WINDOW_INSETS, B_USE_WINDOW_INSETS,
			B_USE_WINDOW_INSETS, B_USE_WINDOW_INSETS)
		.Add(text)
		.Add(sv);

	rate_box->AddChild(rateBoxLayout->View());

// The Channels
	BBox *c_box = new BBox(B_EMPTY_STRING);
	c_box->SetLabel(Language.get("CHANNELS"));
	mono = new BRadioButton(NULL, Language.get("MONO"), new BMessage(CHANGE_CHANNEL));

	if (Pool.sample_type == MONO)	mono->SetValue(B_CONTROL_ON);
	stereo = new BRadioButton(NULL, Language.get("STEREO"), new BMessage(CHANGE_CHANNEL));

	if (Pool.sample_type == STEREO)	stereo->SetValue(B_CONTROL_ON);

	left = new BSpinner(NULL, Language.get("LEFT_MIX"), NULL);
	left->SetRange(0, 400);
	left->SetValue(Pool.sample_type == MONO ? Prefs.filter_resample_sl : Prefs.filter_resample_sr);

	right = new BSpinner(NULL, Language.get("RIGHT_MIX"), NULL);
	right->SetRange(0, 400);
	right->SetValue(Pool.sample_type == MONO ? Prefs.filter_resample_ml : Prefs.filter_resample_mr);

	left->SetEnabled(false);
	right->SetEnabled(false);

	BGroupLayout* channelBoxLayout = BLayoutBuilder::Group<>(B_VERTICAL)
		.SetInsets(B_USE_WINDOW_INSETS, B_USE_WINDOW_INSETS,
			B_USE_WINDOW_INSETS, B_USE_WINDOW_INSETS)
		.Add(mono)
		.Add(stereo)
		.AddGrid()
			.Add(left->CreateLabelLayoutItem(), 0, 0)
			.Add(left->CreateTextViewLayoutItem(), 1, 0)
			.Add(right->CreateLabelLayoutItem(), 0, 1)
			.Add(right->CreateTextViewLayoutItem(), 1, 1)
		.End();

	c_box->AddChild(channelBoxLayout->View());

// the resolution
	BBox *r_box = new BBox(B_EMPTY_STRING);
	r_box->SetLabel(Language.get("RESOLUTION"));
	resolution = new BListView("Bits list");
	sv = new BScrollView("scroll", resolution, B_WILL_DRAW, false, true, B_PLAIN_BORDER);
	sv->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	sv->MakeFocus(false);
	
	bits = new BSpinner(NULL, NULL, new BMessage(SET_BITS));
	bits->SetRange(4, 32);
	bits->SetValue(Pool.sample_bits);

	BStringItem *it;
	resolution->AddItem(it = new BStringItem("8"));
	if (Pool.sample_bits <= 8)	it->Select();
	resolution->AddItem(it = new BStringItem("16"));
	if (Pool.sample_bits <= 16 && Pool.sample_bits >8)	it->Select();
	resolution->AddItem(it = new BStringItem("32"));
	if (Pool.sample_bits <= 32 && Pool.sample_bits >16)	it->Select();
	resolution->SetSelectionMessage(new BMessage(SELECT_BITS));
	resolution->SetInvocationMessage(new BMessage(SELECT_BITS));
	m_bits = Pool.sample_bits;

	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

//	list->AddItem(new BStringItem("96000"));
//	list->AddItem(new BStringItem("64000"));
	list->AddItem(new BStringItem("48000"));
	list->AddItem(new BStringItem("44100"));
	list->AddItem(new BStringItem("32000"));
	list->AddItem(new BStringItem("22050"));
	list->AddItem(new BStringItem("16000"));
	list->AddItem(new BStringItem("12500"));
	list->AddItem(new BStringItem("11025"));
	list->AddItem(new BStringItem("8000"));
	list->SetSelectionMessage(new BMessage(SELECT));
	list->SetInvocationMessage(new BMessage(SELECT));
	SetList(true);

	BGroupLayout* resolutionBoxLayout = BLayoutBuilder::Group<>(B_VERTICAL)
		.SetInsets(B_USE_WINDOW_INSETS, B_USE_WINDOW_INSETS,
			B_USE_WINDOW_INSETS, B_USE_WINDOW_INSETS)
		.Add(bits)
		.Add(sv);

	r_box->AddChild(resolutionBoxLayout->View());

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING)
		.AddGroup(B_HORIZONTAL)
			.Add(rate_box)
			.Add(c_box)
			.Add(r_box)
		.End()
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(new BButton( NULL, Language.get("CANCEL"), new BMessage(B_QUIT_REQUESTED)))
			.Add(new BButton(NULL, Language.get("APPLY"), new BMessage(SET)))
		.End();
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

