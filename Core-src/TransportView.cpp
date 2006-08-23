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
#include <View.h>
#include <stdio.h>
#include <PictureButton.h>

#include "Globals.h"
#include "TransportView.h"
#include "ToolBarView.h"


BPictureButton *TransportButton(BRect buttonsize, const char *file, BMessage *msg, uint32 state, BView *view)
{
	BBitmap *up = BTranslationUtils::GetBitmapFile(file);
	BPicture *Up,*Down;

	view->BeginPicture(new BPicture);
	view->DrawBitmap(up, BRect(0,0,24,22), BRect(0,0,24,22) );
	Up = view->EndPicture();
   
	view->BeginPicture(new BPicture);
	view->DrawBitmap(up, BRect(25,0,49,22), BRect(0,0,24,22));
	Down = view->EndPicture();
	view->Sync();

//	MyPictureButton *PBut = new MyPictureButton(buttonsize, Up, Down, msg, state);
	BPictureButton *PBut = new BPictureButton(buttonsize, NULL, Up, Down, msg, state, B_FOLLOW_ALL, B_WILL_DRAW);

	delete up;
	delete Up;
	delete Down;
	
	return PBut;
}


TransportView::TransportView(BRect r) : 
	BView(r, "Transport view", B_FOLLOW_BOTTOM, B_WILL_DRAW)
{
	SetViewColor(192,192,192);
}

//*****************************************************
TransportView::~TransportView()
{
}

//*****************************************************
void TransportView::AttachedToWindow()
{
	SetViewBitmap(BTranslationUtils::GetBitmapFile("./Bitmaps/Transport.png"), B_FOLLOW_ALL);

	BRect r(0,0,24,22);
	r.OffsetTo(9,9);
	AddChild(stop = TransportButton( r, "./Bitmaps/StopButton.png", new BMessage(TRANSPORT_STOP), B_TWO_STATE_BUTTON, this));
	stop->SetValue(B_CONTROL_ON);
	r.OffsetBy(25,0);
	AddChild(play_sel = TransportButton( r, "./Bitmaps/PlaySelectionButton.png", new BMessage(TRANSPORT_PLAY), B_TWO_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild(play = TransportButton( r, "./Bitmaps/PlayButton.png", new BMessage(TRANSPORT_PLAYS), B_TWO_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild(pause = TransportButton( r, "./Bitmaps/PauseButton.png", new BMessage(TRANSPORT_PAUSE), B_TWO_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild(rec = TransportButton( r, "./Bitmaps/RecButton.png", new BMessage(TRANSPORT_REC), B_ONE_STATE_BUTTON, this));

	r.OffsetTo(9,31);
	AddChild(rew_all = TransportButton( r, "./Bitmaps/BeginButton.png", new BMessage(TRANSPORT_REW_ALL), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild(rew = TransportButton( r, "./Bitmaps/RewButton.png", new BMessage(TRANSPORT_REW), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild(fwd = TransportButton( r, "./Bitmaps/FwdButton.png", new BMessage(TRANSPORT_FWD), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild(fwd_all = TransportButton( r, "./Bitmaps/EndButton.png", new BMessage(TRANSPORT_FWD_ALL), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild(loop = TransportButton( r, "./Bitmaps/LoopButton.png", new BMessage(TRANSPORT_LOOP), B_TWO_STATE_BUTTON, this));

	r.OffsetTo(144,9);
	AddChild( ToolBarButton( r, 16, Language.get("ZOOM_IN"), new BMessage(ZOOM_IN), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild( ToolBarButton( r, 21, Language.get("ZOOM_SELECTION"), new BMessage(ZOOM_SELECTION), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild( ToolBarButton( r, 18, Language.get("ZOOM_LEFT"), new BMessage(ZOOM_LEFT), B_ONE_STATE_BUTTON, this));

	r.OffsetTo(144,31);
	AddChild( ToolBarButton( r, 17, Language.get("ZOOM_OUT"), new BMessage(ZOOM_OUT), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild( ToolBarButton( r, 20, Language.get("ZOOM_FULL"), new BMessage(ZOOM_FULL), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild( ToolBarButton( r, 19, Language.get("ZOOM_RIGHT"), new BMessage(ZOOM_RIGHT), B_ONE_STATE_BUTTON, this));
}

//*****************************************************
void TransportView::Draw(BRect rect)
{
}
