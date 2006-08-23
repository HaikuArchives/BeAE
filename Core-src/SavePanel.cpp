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
#include <TranslationKit.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Globals.h"
#include "SavePanel.h"

/*******************************************************
*
*******************************************************/
// ------------------- CodecMenuItem -------------------
CodecMenuItem::CodecMenuItem(media_codec_info *ci, uint32 msg_type)
	: BMenuItem(ci->pretty_name, new BMessage(msg_type))
{
	memcpy(&fCodecInfo, ci, sizeof(fCodecInfo));
}


CodecMenuItem::~CodecMenuItem()
{
}

// ------------------- FileFormatMenuItem -------------------
FileFormatMenuItem::FileFormatMenuItem(media_file_format *format)
	: BMenuItem(format->pretty_name, new BMessage(PANEL_FORMAT))
{
	memcpy(&fFileFormat, format, sizeof(fFileFormat));
}


FileFormatMenuItem::~FileFormatMenuItem()
{
}


/*******************************************************
*
*******************************************************/
SavePanel::SavePanel(BHandler *handler)
	: BFilePanel(B_SAVE_PANEL, new BMessenger(handler), NULL, B_FILE_NODE, false, new BMessage(SAVE_AUDIO), NULL, true, true)
{
	if (Window()->Lock()) {
		float minw, maxw, minh, maxh;
		Window()->GetSizeLimits(&minw, &maxw, &minh, &maxh);
		minw = 350;
		Window()->SetSizeLimits(minw, maxw, minh, maxh);
		Window()->ResizeTo(MAX(Window()->Frame().Width(), minw), Window()->Frame().Height());

		Window()->SetTitle(Language.get("PANEL_SAVE"));

		// Find all the views that are in the way and move up them up 10 pixels
		BView *background = Window()->ChildAt(0);
		BView *poseview = background->FindView("PoseView");
		if (poseview) poseview->ResizeBy(0, -40);
		BButton *insert = (BButton *)background->FindView("default button");
		if (insert) insert->MoveBy(0, -35);
		BButton *cancel = (BButton *)background->FindView("cancel button");
		if (cancel){
			cancel->ResizeTo( insert->Frame().Width(), insert->Frame().Height()-5);
			cancel->MoveTo( insert->Frame().left, insert->Frame().bottom+7);
		}
		BScrollBar *hscrollbar = (BScrollBar *)background->FindView("HScrollBar");
		if (hscrollbar) hscrollbar->MoveBy(0, -40);
		BScrollBar *vscrollbar = (BScrollBar *)background->FindView("VScrollBar");
		if (vscrollbar) vscrollbar->ResizeBy(0, -40);
		BView *countvw = (BView *)background->FindView("CountVw");
		if (countvw) countvw->MoveBy(0, -40);
		BView *textview = (BView *)background->FindView("text view");
		if (textview){
			textview->ResizeBy(50, 0);
			textview->MoveBy(0, -40);
		}
		
		// Add the new BHandler to the window's looper
		Window()->AddHandler(this);
		
		if (!cancel || !textview || !hscrollbar) {
			//printf("Couldn't find necessary controls.\n");
			return;
		}

		// Position the menu field relative to the other GUI elements, and make it the
		// same length as the textview
		BRect rect = textview->Frame();
//		rect.top = hscrollbar->Frame().bottom + 5;
//		rect.bottom = rect.top + 10;
//		rect.right = (rect.right+rect.left)/2;
		rect.OffsetBy(0,26);

		BPopUpMenu *popmenu = new BPopUpMenu(Language.get("FORMAT"));
		fFormatMenu = new BMenuField(rect, NULL, Language.get("FORMAT"), popmenu , B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_NAVIGABLE);
		background->AddChild(fFormatMenu);
	
		rect.OffsetBy(0,24);

		popmenu = new BPopUpMenu(Language.get("CODEC"));
		fAudioMenu = new BMenuField(rect, NULL, Language.get("CODEC"), popmenu,B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_NAVIGABLE);
		background->AddChild(fAudioMenu);

		fFormatMenu->SetDivider(50);
		fAudioMenu->SetDivider(50);
		
		// Make sure the smallest window won't draw the "Settings" button over anything else
//		float min_window_width = Window()->Bounds().right - 10 + textview->Frame().right;
//		Window()->SetSizeLimits(min_window_width, 10000, 250, 10000);
//		if (Window()->Bounds().IntegerWidth() + 1 < min_window_width)
//			Window()->ResizeTo(min_window_width, 300);
	
		BuildFormatMenu();
		BuildAudioMenu();
		
		Window()->Unlock();
	}
}

/*******************************************************
*   Handle messages from controls we've added
*******************************************************/
void SavePanel::MessageReceived(BMessage *message) {
	switch (message->what){
	case PANEL_FORMAT:
		BuildAudioMenu();
		break;

	case PANEL_CODEC:
		break;

	default:
		BHandler::MessageReceived(message);
		break;
	}
}

/*******************************************************
*
*******************************************************/
void SavePanel::BuildFormatMenu() {
	BMenu *menu = fFormatMenu->Menu();
	BMenuItem *item;
	// clear out old format menu items
	while ((item = menu->RemoveItem((int32)0)) != NULL) {
		delete item;
	}

	// add menu items for each file format
	media_file_format mfi;
	int32 cookie = 0;
	FileFormatMenuItem *ff_item;
	while (get_next_file_format(&cookie, &mfi) == B_OK) {
		ff_item = new FileFormatMenuItem(&mfi);
		menu->AddItem(ff_item);
		ff_item->SetTarget(this);
	}
	
	// mark first item
	item = menu->ItemAt(0);
	if (item != NULL) {
		item->SetMarked(true);
		((BInvoker *)item)->Invoke();
	}
}

/*******************************************************
*
*******************************************************/
void SavePanel::BuildAudioMenu()
{
	BMenu *menu = fAudioMenu->Menu();
	BMenuItem *item = NULL;
	// clear out old audio codec menu items
	while ((item = menu->RemoveItem((int32)0)) != NULL) {
		delete item;
	}

	// get selected file format
	FileFormatMenuItem *ffmi = (FileFormatMenuItem*)fFormatMenu->Menu()->FindMarked();
	media_file_format *mf_format = &(ffmi->fFileFormat);

	media_format format, outfmt;
	memset(&format, 0, sizeof(format));
	memset(&outfmt, 0, sizeof(outfmt));
	media_codec_info codec_info;
	int32 cookie = 0;
	CodecMenuItem *cmi;

	// add available audio encoders to menu
	format.type = B_MEDIA_RAW_AUDIO;
	format.u.raw_audio = media_raw_audio_format::wildcard;	
	while (get_next_encoder(&cookie, mf_format, &format, &outfmt, &codec_info) == B_OK) {
		cmi = new CodecMenuItem(&codec_info, PANEL_CODEC);
		menu->AddItem(cmi);
		cmi->SetTarget(this);
		// reset media format struct
		format.type = B_MEDIA_RAW_AUDIO;
		format.u.raw_audio = media_raw_audio_format::wildcard;
	}

	// mark first audio encoder
	item = menu->ItemAt(0);
	if (item != NULL) {
		fAudioMenu->SetEnabled(true);
		item->SetMarked(true);
		((BInvoker *)item)->Invoke();
	} else {
		item = new BMenuItem("None available", NULL);
		menu->AddItem(item);
		item->SetMarked(true);
		fAudioMenu->SetEnabled(false);
	}
}

/*******************************************************
*
*******************************************************/
void SavePanel::GetSelectedFormatInfo(media_file_format **format, media_codec_info **audio)
{
	*format = NULL;
	FileFormatMenuItem *formatItem =
		dynamic_cast<FileFormatMenuItem *>(fFormatMenu->Menu()->FindMarked());
	if (formatItem != NULL) {
		*format = &(formatItem->fFileFormat);
	}
	
	*audio = NULL;
	CodecMenuItem *codecItem =
		dynamic_cast<CodecMenuItem *>(fAudioMenu->Menu()->FindMarked());
	if (codecItem != NULL) {
		*audio =  &(codecItem->fCodecInfo);
	}
}
/*******************************************************
*
*******************************************************/
void SavePanel::SetFormatInfo(media_file_format *format, media_codec_info *audio)
{


}

/*******************************************************
*
*******************************************************/
SavePanel::~SavePanel() {
	
}