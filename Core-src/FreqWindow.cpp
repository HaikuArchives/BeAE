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

#include "Globals.h"
#include "FreqWindow.h"
#include "SpinControl.h"
#include "MyStringItem.h"

#define QUIT			'quit'
#define SET				'setF'
#define SET_TEXT		'setT'
#define SELECT			'slct'

/*******************************************************
*   
*******************************************************/
FreqWindow::FreqWindow(BPoint p) : BWindow(BRect(p.x,p.y,p.x,p.y),Language.get("FREQ_WINDOW"),B_FLOATING_WINDOW_LOOK,B_FLOATING_APP_WINDOW_FEEL, B_NOT_RESIZABLE|B_NOT_ZOOMABLE)
{
	BRect r(0,0,180,180);
	ResizeTo(r.Width(), r.Height());
	MoveBy(-r.Width()/2, -r.Height()/2);

	view = new BView(r, NULL, B_FOLLOW_ALL, B_WILL_DRAW);
	r.InsetBy(8,8);
	r.right = 70;

	r.top += 28;		// space for the textbox
	list = new BListView(r,"Freq list");
	BScrollView *sv = new BScrollView("scroll", list, B_FOLLOW_ALL_SIDES, B_WILL_DRAW, false, true, B_PLAIN_BORDER);
	sv->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	sv->MakeFocus(false);
	view->AddChild(sv);
	
	r.Set(4,8,85,28);
	text = new SpinControl(r, NULL, NULL, new BMessage(SET_TEXT), 4000, 96000, 44100, 500);
	view->AddChild(text);

	r = Bounds();
	r.left = r.right - 85;
	r.top = r.bottom - 32;
	r.bottom -=8;
	r.right -= 8;
	view->AddChild(new BButton(r, NULL, Language.get("OK"), new BMessage(SET)) );
	r.OffsetBy(0,-30);
	view->AddChild(new BButton(r, NULL, Language.get("CANCEL"), new BMessage(QUIT)) );

	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(view);

	StringItem *it;
	list->AddItem(it = new StringItem("96000"));
	list->AddItem(it = new StringItem("64000"));
	list->AddItem(it = new StringItem("48000"));
	list->AddItem(it = new StringItem("44100"));
	list->AddItem(it = new StringItem("32000"));
	list->AddItem(it = new StringItem("22050"));
	list->AddItem(it = new StringItem("16000"));
	list->AddItem(it = new StringItem("12500"));
	list->AddItem(it = new StringItem("11025"));
	list->AddItem(it = new StringItem("8000"));
	list->SetSelectionMessage(new BMessage(SELECT));
	list->SetInvocationMessage(new BMessage(SELECT));
	SetList();

	m_old = Pool.frequency;
	Run();
	Show();
}

/*******************************************************
*   
*******************************************************/
void FreqWindow::SetList(){

	BStringItem *it = NULL;
	for (int32 i=0; i<list->CountItems(); i++){
		it = (BStringItem*)list->ItemAt(i);
		it->Deselect();
		float frequency = atof( it->Text() );
		if (Pool.frequency == frequency){	it->Select();	text->SetValue(Pool.frequency);	}
	}
}

/*******************************************************
*   
*******************************************************/
bool FreqWindow::QuitRequested(){
	Pool.frequency = m_old;
	return true;
}

/*******************************************************
*   
*******************************************************/
void FreqWindow::MessageReceived(BMessage* msg){
	int32 i;
	BStringItem *item = NULL;

	switch(msg->what){
	case QUIT:
		Pool.frequency = m_old;
		Quit();
		break;
	
	case SET:
		// set the freq
		Pool.InitBufferPlayer( Pool.frequency );
		Quit();
		break;	

	case SELECT:
		i = list->CurrentSelection();
		if(i < 0)	break;		// nothing selected 
		item = (BStringItem*)list->ItemAt(i);
		
		Pool.frequency = atof( item->Text() );
		text->SetValue(Pool.frequency);
		SetList();
		list->Invalidate();
		break;

	case SET_TEXT:
		Pool.frequency = text->Value();
		SetList();
		list->Invalidate();
		break;

	default:
		BWindow::MessageReceived(msg);
	}
}
