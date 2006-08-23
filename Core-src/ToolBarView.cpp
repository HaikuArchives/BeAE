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
#include <PictureButton.h>
#include <View.h>
#include <Cursor.h>
#include "CommonPool.h"

#include "ToolBarView.h"
#include "CommonPool.h"
#include "MyClipBoard.h"

BPictureButton *ToolBarButton(BRect buttonsize, int32 index, const char *tip, BMessage *msg, uint32 state, BView *view)
{
	BBitmap *icon = BTranslationUtils::GetBitmapFile("./Bitmaps/ToolIcons.png");
	BBitmap *up = BTranslationUtils::GetBitmapFile("./Bitmaps/EmptyButton.png");
	BBitmap *dis = new BBitmap(up);
	
	uint8 *in = (uint8*)icon->Bits() +index*25*4;
	uint8 *out = (uint8*)up->Bits();
	uint8 *dout = (uint8*)dis->Bits();
	
	uint32 add = up->BytesPerRow() + 104;
	uint8 d;
	
	for(int32 y=0; y<22; y++){
		for(int32 x=0; x<25; x++){
			if (in[0]==0 && in[1]==255 && in[2]==0){
				in+=4;
				out+=4;
				dout+=4;
			}else{
				d = (uint8)(in[0]*0.15+in[1]*0.25+in[2]*0.1) + 100;
				out[add] = in[0];
				out[add+1] = in[1];
				out[add+2] = in[2];
				*out++ = *in++;
				*out++ = *in++;
				*out++ = *in++;
				out++;	in++;
				dout[add] = d;
				dout[add+1] = d;
				dout[add+2] = d;
				*dout++ = d;
				*dout++ = d;
				*dout++ = d;
				dout++;
			}
		}
		in += icon->BytesPerRow() -25*4;
		out += up->BytesPerRow() -25*4;
		dout += up->BytesPerRow() -25*4;
	}
	
	BPicture *Up,*Down, *DUp, *DDown;

	view->BeginPicture(new BPicture);
	view->DrawBitmap(dis, BRect(0,0,24,22), BRect(0,0,24,22) );
	DUp = view->EndPicture();
   
	view->BeginPicture(new BPicture);
	view->DrawBitmap(dis, BRect(25,0,49,22), BRect(0,0,24,22) );
	DDown = view->EndPicture();

	view->BeginPicture(new BPicture);
	view->DrawBitmap(up, BRect(0,0,24,22), BRect(0,0,24,22) );
	Up = view->EndPicture();
   
	view->BeginPicture(new BPicture);
	view->DrawBitmap(up, BRect(25,0,49,22), BRect(0,0,24,22) );
	Down = view->EndPicture();
	view->Sync();

	BPictureButton *PBut = new BPictureButton(buttonsize, NULL, Up, Down, msg, state, B_FOLLOW_ALL, B_WILL_DRAW);
	Pool.AddTip(PBut, tip);
	
	PBut->SetDisabledOff(DUp);
	PBut->SetDisabledOn(DDown);

	delete up;
	delete Up;
	delete Down;
	delete dis;
	delete DDown;
	delete DUp;
	
	return PBut;
}

ToolBarView::ToolBarView(BRect r) : 
	BView(r, "ToolBar", B_FOLLOW_NONE, B_WILL_DRAW)
{
	SetViewColor(216,216,216);
}

//*****************************************************
ToolBarView::~ToolBarView()
{
}

//*****************************************************
void ToolBarView::Update()
{
	tool[3]->SetEnabled(Pool.selection != NONE);					// save selection
	tool[2]->SetEnabled(Pool.sample_type != NONE && Pool.changed);	// save
	tool[10]->SetEnabled(Pool.selection != NONE);					// cut
	tool[11]->SetEnabled(Pool.selection != NONE);					// copy

	tool[8]->SetEnabled(Hist.HasUndo());							// need history class for this

#ifdef __SAMPLE_STUDIO_LE
	tool[9]->SetEnabled(Hist.HasRedo());							// need history class for this
	tool[14]->SetEnabled(Pool.selection != NONE);					// copy to stack
	tool[13]->SetEnabled(ClipBoard.HasClip());
#endif

	tool[12]->SetEnabled(ClipBoard.HasClip());
	
	if (Pool.sample_type != STEREO || Pool.selection == NONE){
		tool[5]->SetEnabled(false);	// L
		tool[6]->SetEnabled(false);	// R
		tool[7]->SetEnabled(false);	// B
	}else{
		tool[5]->SetEnabled(Pool.selection != LEFT);
		tool[6]->SetEnabled(Pool.selection != RIGHT);
		tool[7]->SetEnabled(Pool.selection != BOTH);
	}
	
	bool draw = false;
	BView *view = Window()->FindView("Sample view");
	if (view){
		float step = (Pool.r_pointer-Pool.l_pointer)/view->Bounds().Width();
		if (step < 64) draw = true;
	}
	if (Pool.tool_mode == DRAW_TOOL && !draw)	// make sure that the correct mode is active
		Pool.tool_mode = SELECT_TOOL;

	tool[16]->SetEnabled(Pool.sample_type != NONE);
	tool[16]->SetValue(Pool.tool_mode == SELECT_TOOL && Pool.sample_type != NONE);
	tool[17]->SetEnabled(Pool.sample_type != NONE && draw);
	tool[17]->SetValue(Pool.tool_mode == DRAW_TOOL);
	tool[18]->SetEnabled(Pool.sample_type != NONE);
	tool[18]->SetValue(Pool.tool_mode == PLAY_TOOL);
	tool[19]->SetEnabled(Pool.sample_type != NONE);
	tool[19]->SetValue(Pool.tool_mode == SCRUB_TOOL);

}

//*****************************************************
void ToolBarView::AttachedToWindow()
{
	BRect r(0,0,24,22);
	r.OffsetTo(4,5);
	AddChild( tool[0] = ToolBarButton( r, 0, Language.get("TIP_NEW"), new BMessage(NEW), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild( tool[1] = ToolBarButton( r, 1, Language.get("TIP_OPEN"), new BMessage(OPEN), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild( tool[22] = ToolBarButton( r, 22, Language.get("TIP_INSERT"), new BMessage(INSERT), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild( tool[2] = ToolBarButton( r, 2, Language.get("TIP_SAVE"), new BMessage(SAVE), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild( tool[3] = ToolBarButton( r, 3, Language.get("TIP_SAVE_SELECTION"), new BMessage(SAVE_SELECTION), B_ONE_STATE_BUTTON, this));

	r.OffsetBy(25+8,0);
	AddChild( tool[5] = ToolBarButton( r, 5, Language.get("EDIT_L"), new BMessage(EDIT_L), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild( tool[6] = ToolBarButton( r, 6, Language.get("EDIT_R"), new BMessage(EDIT_R), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild( tool[7] = ToolBarButton( r, 7, Language.get("EDIT_B"), new BMessage(EDIT_B), B_ONE_STATE_BUTTON, this));

	r.OffsetBy(25+8,0);
	AddChild( tool[8] = ToolBarButton( r, 8, Language.get("UNDO"), new BMessage(UNDO), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
#ifdef __SAMPLE_STUDIO_LE
	AddChild( tool[9] = ToolBarButton( r, 9, Language.get("REDO"), new BMessage(REDO), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
#endif
	AddChild( tool[10] = ToolBarButton( r, 10, Language.get("CUT"), new BMessage(B_CUT), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild( tool[11] = ToolBarButton( r, 11, Language.get("COPY"), new BMessage(B_COPY), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild( tool[12] = ToolBarButton( r, 12, Language.get("PASTE"), new BMessage(B_PASTE), B_ONE_STATE_BUTTON, this));
#ifdef __SAMPLE_STUDIO_LE
	r.OffsetBy(25,0);
	AddChild( tool[13] = ToolBarButton( r, 13, Language.get("PASTE_MIX"), new BMessage(PASTE_MIXED), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild( tool[14] = ToolBarButton( r, 14, Language.get("COPY_TO_STACK"), new BMessage(TO_STACK), B_ONE_STATE_BUTTON, this));
#endif
//	r.OffsetBy(25,0);
//	AddChild( tool[15] = ToolBarButton( r, 15, Language.get("SET_LOOP"), new BMessage(SET_LOOP), B_ONE_STATE_BUTTON, this));
	r.OffsetBy(25+8,0);
	AddChild( tool[16] = ToolBarButton( r, 23, Language.get("TOOL_SELECT"), new BMessage(TOOL_SELECT), B_TWO_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild( tool[17] = ToolBarButton( r, 24, Language.get("TOOL_DRAW"), new BMessage(TOOL_DRAW), B_TWO_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild( tool[18] = ToolBarButton( r, 25, Language.get("TOOL_PLAY"), new BMessage(TOOL_PLAY), B_TWO_STATE_BUTTON, this));
	r.OffsetBy(25,0);
	AddChild( tool[19] = ToolBarButton( r, 26, Language.get("TOOL_JOGG"), new BMessage(TOOL_JOGG), B_TWO_STATE_BUTTON, this));

	
}

//*****************************************************
void ToolBarView::Draw(BRect r)
{
	r = Bounds();
	SetHighColor(255,255,255);
	StrokeLine(BPoint(r.left, r.top), BPoint(r.right, r.top));
	SetHighColor(64,64,64);
	StrokeLine(BPoint(r.left, r.bottom), BPoint(r.right, r.bottom));

}
