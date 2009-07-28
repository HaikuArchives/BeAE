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
#include "PrefView.h"
#include "PrefGeneral.h"
#include "PrefKeys.h"
#include "PrefColors.h"
#include "MyStringItem.h"

#define PREF_SELECT		'psel'

/*******************************************************
*   Setup the main view. Add in all the niffty components
*   we have made and get things rolling
*******************************************************/
PrefView::PrefView(BRect frame):BView(frame, "Prefs view", B_FOLLOW_ALL_SIDES, B_WILL_DRAW){
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// add the prefs list at the left
	BRect r = Bounds();
	r.right = 130;
	r.top += 8;	r.left += 8;
	r.bottom -= 40;
	list = new BListView(r,"Prefs list");
	BScrollView *sv = new BScrollView("scroll", list, B_FOLLOW_ALL_SIDES, B_WILL_DRAW, false, true, B_PLAIN_BORDER);
	sv->SetLowColor(255,0,0);
	sv->MakeFocus(false);
	AddChild(sv);  

	r.left = sv->Bounds().right + 15;
	r.right = Bounds().right - 8;
	configBox = new BBox(r,"configbox");
	configBox->SetLabel(" - ");
	AddChild(configBox);

	list->AddItem(new StringItem(Language.get("GENERAL")));
	list->AddItem(new StringItem(Language.get("COLORSET")));
	list->AddItem(new StringItem(Language.get("KEYBINDINGS")));

	r = Bounds();
	AddChild(new BButton(BRect(r.right-120,r.bottom-32,r.right-8,r.bottom-8), NULL, Language.get("OK"), new BMessage(QUIT)) );
	AddChild(new BButton(BRect(8,r.bottom-32,146,r.bottom-8), NULL, Language.get("FACTORY"), new BMessage(SET_FACTORY)) );

	r = configBox->Bounds();
	r.InsetBy(5,5); r.top += 10;
	configBox->AddChild(new PrefGeneral(r));
	configBox->SetLabel(Language.get("GENERAL"));
}

/*******************************************************
*  
*******************************************************/
PrefView::~PrefView()
{
	BView *tmpV = configBox->ChildAt(0);
	if(tmpV != NULL){
		tmpV->LockLooper();
		tmpV->RemoveSelf();
		delete tmpV;
	}
}

/*******************************************************
*  
*******************************************************/
void PrefView::AttachedToWindow(){
	list->SetTarget(this);
	list->SetSelectionMessage(new BMessage(PREF_SELECT));
	list->SetInvocationMessage(new BMessage(PREF_SELECT));
}

/*******************************************************
*
*******************************************************/
void PrefView::MessageReceived(BMessage *msg){
	int32 i;
	BView *tmpV = NULL;
	BRect r = configBox->Bounds();
	r.InsetBy(5,5); r.top += 10;

	switch(msg->what){
	case PREF_SELECT:
		i = list->CurrentSelection();
		if(i < 0)
			break; // nothign selected 

		tmpV = configBox->ChildAt(0);
		if (tmpV != NULL){
			tmpV->RemoveSelf();
			delete tmpV;
		}

		switch(i){
		case 0:		// general
			configBox->AddChild(new PrefGeneral(r));
			configBox->SetLabel(Language.get("GENERAL"));
			break;
		case 2:		// keys
			configBox->AddChild(new PrefKeys(r));
			configBox->SetLabel(Language.get("KEYBINDINGS"));
			break;
		case 1:		// colors
			configBox->AddChild(new PrefColors(r));
			configBox->SetLabel(Language.get("COLORSET"));
			break;
		}
		break;

	default:
		BView::MessageReceived(msg);
		break;   
	}
}
