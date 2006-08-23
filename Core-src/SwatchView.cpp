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

#include <String.h>
#include "SwatchView.h"

#define SET_COLOR	'setC'

char* RGBtoText(const rgb_color& color) 
{
	static char* p="0123456789ABCDEF";

	char *rgbtxt = new char[8];

	rgbtxt[0]='#';
	rgbtxt[1]=p[(color.red & 0xF0)>>4];
	rgbtxt[2]=p[(color.red & 0x0F)];
	rgbtxt[3]=p[(color.green & 0xF0)>>4];
	rgbtxt[4]=p[(color.green & 0x0F)];
	rgbtxt[5]=p[(color.blue & 0xF0)>>4];
	rgbtxt[6]=p[(color.blue & 0x0F)];
	rgbtxt[7]='\0';

	return rgbtxt;
}

SwatchView::SwatchView(BRect frame, const char *name, BMessage* msg)
 : BControl(frame, "", "", msg, B_FOLLOW_NONE, B_WILL_DRAW)
{
	Init();
	m_name = name;
	active = false;
}

SwatchView::SwatchView(BRect frame, const char *name, rgb_color color, BMessage* msg = NULL)
 : BControl(frame, "", "", msg, B_FOLLOW_NONE, B_WILL_DRAW)
{
	Init();
	m_color = color;
	m_name = name;
	DrawOffscreen();
}

SwatchView::~SwatchView()
{
	m_bitmap->RemoveChild(m_view);
	delete m_view;
	delete m_bitmap;
}

void SwatchView::SetActive(bool c)
{
	active = c;
}

void SwatchView::Init()
{
	BRect frame = Bounds();
	m_bitmap = new BBitmap(frame, B_RGB32, true);
	m_view = new BView(frame, "offscreen", B_FOLLOW_NONE, 0);
	m_bitmap->AddChild(m_view);
	m_color.red = m_color.green = m_color.blue = 0;
	DrawOffscreen();
}

void SwatchView::AttachedToWindow()
{
}

void SwatchView::DetachedFromWindow()
{
//	ICommon->AddTip(this,NULL);
}

void SwatchView::Draw(BRect update_rect)
{
	if (m_bitmap){
		SetDrawingMode(B_OP_COPY);
		DrawBitmap(m_bitmap, Bounds());
	}
}

void SwatchView::MouseDown(BPoint where)
{
	if (!IsEnabled())
		return;

//	BMessage *mesg = Window()->CurrentMessage();
//	int32 clicks = mesg->FindInt32("clicks");

//	if (m_clicks == clicks)
//		SetColorWindow(ICommon, m_name, m_color, this);

	BPoint w2;
	uint32 mods;
	while(true)
	{
		GetMouse(&w2, &mods);
		if (!mods) //releasing the buttons without moving means no drag
		{
			Invoke();
			return;	
		}
		if (w2 != where)
			break;
	}
	snooze(40000);

	BMessage msg(B_PASTE);
	msg.AddData("RGBColor",B_RGB_COLOR_TYPE,(const void**)&m_color,sizeof(rgb_color));
	BString s = RGBtoText(m_color);
	msg.AddData("text/plain",B_MIME_DATA,(const void**)s.String(),s.Length());
	msg.AddString("be:types", "text/plain");
	msg.AddInt32("be:actions", B_COPY_TARGET);
	msg.AddInt32("be:actions", B_TRASH_TARGET);
	  
	BBitmap *bitmap = make_bitmap();
	 
	BPoint pt(bitmap->Bounds().Width()/2.0, bitmap->Bounds().Height()/2);
	DragMessage(&msg, bitmap, B_OP_ALPHA, pt, Window());
}

BBitmap* SwatchView::make_bitmap(void)
{
	BRect rect(0.0, 0.0, 12.0, 12.0);
	
	BBitmap *bitmap = new BBitmap(rect, B_RGB32, true);
	BView *view = new BView(rect, "", B_FOLLOW_NONE, B_WILL_DRAW);

	bitmap->Lock();

	bitmap->AddChild(view);

	view->SetDrawingMode(B_OP_ALPHA);
	view->SetHighColor(m_color);
	view->FillRect(rect);
	
	view->SetDrawingMode(B_OP_COPY);
	view->SetHighColor(0, 0, 0, 255);
	view->StrokeRect(rect);
	view->Sync();

	bitmap->RemoveChild(view);
	delete view;

	bitmap->Unlock();
	
	return bitmap;
}

void SwatchView::SetColor(const rgb_color &color)
{
	m_color = color;
	DrawOffscreen();
}

void SwatchView::SetEnabled(bool enabled)
{
	BControl::SetEnabled(enabled);
	DrawOffscreen();
}

void SwatchView::DrawOffscreen()
{
	BRect rect, frame;
	rect = frame = m_bitmap->Bounds();

	m_bitmap->Lock();
	
	// draw frame
	rect.InsetBy(1.0, 1.0);

	m_view->SetDrawingMode(B_OP_COPY);

	if (IsEnabled())
	{
		if (!active){
			// draw frame
			m_view->SetHighColor(184,184,184,255);
			m_view->StrokeLine(BPoint(rect.left-1.0, rect.top-1.0), BPoint(rect.left-1.0, rect.bottom+1.0));
			m_view->StrokeLine(BPoint(rect.left-1.0, rect.top-1.0), BPoint(rect.right+1.0, rect.top-1.0));
			m_view->SetHighColor(255,255,255,255);
			m_view->StrokeLine(BPoint(rect.left, rect.bottom+1.0), BPoint(rect.right+1.0, rect.bottom+1.0));
			m_view->StrokeLine(BPoint(rect.right+1.0, rect.bottom+1.0), BPoint(rect.right+1.0, rect.top));
			m_view->SetHighColor(216,216,216,216);
			m_view->StrokeLine(rect.LeftBottom(), rect.RightBottom());
			m_view->StrokeLine(rect.RightTop(), rect.RightBottom());
			m_view->SetHighColor(96,96,96,255);
			m_view->StrokeLine(rect.LeftBottom(), rect.LeftTop());
			m_view->StrokeLine(rect.LeftTop(), rect.RightTop());
	
			// draw checkerboard
			rect.InsetBy(1.0, 1.0);	
			m_view->SetLowColor(255,255,255,255);
			m_view->SetHighColor(226,226,226,255);
			pattern checkerboard = (pattern){ 240, 240, 240, 240, 15, 15, 15, 15 };
			m_view->FillRect(rect, checkerboard);
	
			// draw color (with alpha)
			m_view->SetDrawingMode(B_OP_ALPHA);
			m_view->SetHighColor(m_color);
			m_view->FillRect(rect, B_SOLID_HIGH);
		}else{
			rect.InsetBy(-1.0, -1.0);	
			m_view->SetHighColor(0,0,0);
			m_view->StrokeRect(rect);
			rect.InsetBy(1.0, 1.0);	
			m_view->SetHighColor(255,255,255);
			m_view->StrokeRect(rect);
			m_view->SetHighColor(0,0,0);
			rect.InsetBy(1.0, 1.0);	
			m_view->StrokeRect(rect);

			// draw frame
/*			m_view->SetHighColor(184,184,184,255);
			m_view->StrokeLine(BPoint(rect.left-1.0, rect.top-1.0), BPoint(rect.left-1.0, rect.bottom+1.0));
			m_view->StrokeLine(BPoint(rect.left-1.0, rect.top-1.0), BPoint(rect.right+1.0, rect.top-1.0));
			m_view->SetHighColor(255,255,255,255);
			m_view->StrokeLine(BPoint(rect.left, rect.bottom+1.0), BPoint(rect.right+1.0, rect.bottom+1.0));
			m_view->StrokeLine(BPoint(rect.right+1.0, rect.bottom+1.0), BPoint(rect.right+1.0, rect.top));
			m_view->SetHighColor(216,216,216,216);
			m_view->StrokeLine(rect.LeftBottom(), rect.RightBottom());
			m_view->StrokeLine(rect.RightTop(), rect.RightBottom());
			m_view->SetHighColor(96,96,96,255);
			m_view->StrokeLine(rect.LeftBottom(), rect.LeftTop());
			m_view->StrokeLine(rect.LeftTop(), rect.RightTop());
*/			
			// draw checkerboard
			rect.InsetBy(1.0, 1.0);	
			m_view->SetLowColor(255,255,255,255);
			m_view->SetHighColor(226,226,226,255);
			pattern checkerboard = (pattern){ 240, 240, 240, 240, 15, 15, 15, 15 };
			m_view->FillRect(rect, checkerboard);
	
			// draw color (with alpha)
			m_view->SetDrawingMode(B_OP_ALPHA);
			m_view->SetHighColor(m_color);
			m_view->FillRect(rect, B_SOLID_HIGH);
		}
	}	
	else
	{
		m_view->SetDrawingMode(B_OP_COPY);

		m_view->SetHighColor(239,239,239,255);
		m_view->StrokeLine(BPoint(rect.left, rect.bottom+1.0), BPoint(rect.right+1.0, rect.bottom+1.0));
		m_view->StrokeLine(BPoint(rect.right+1.0, rect.bottom+1.0), BPoint(rect.right+1.0, rect.top));
		m_view->SetHighColor(152,152,152,255);
		m_view->StrokeLine(rect.LeftBottom(), rect.LeftTop());
		m_view->StrokeLine(rect.LeftTop(), rect.RightTop());

		rect.InsetBy(1.0, 1.0);	

		m_view->SetHighColor(239, 239, 239);
		m_view->FillRect(rect);
	}
	
	m_view->Sync();
	m_bitmap->Unlock();

	// blit to screen if attached to window
	if (Window())
	{
		if(Window()->Lock()){
			Draw(Bounds());
			Window()->Unlock();
		}
	}
}

rgb_color SwatchView::Color(void) const
{
	return m_color;
}

void SwatchView::MessageReceived(BMessage *msg){
	switch(msg->what) {
	case B_PASTE:
	case B_OK:
		{
			if (!IsEnabled())
				return;

			rgb_color *color;
			ssize_t s;
			msg->FindData("RGBColor",B_RGB_COLOR_TYPE,(const void**)&color,&s);
			SetColor(*color);
			Invoke();
		}
		break;

	case SET_COLOR:
{		const char *title;
		rgb_color *color;
		ssize_t n = sizeof(struct rgb_color);
		if (msg->FindData("RGBColor", (type_code) B_RGB_COLOR_TYPE, (const void **) &color, &n) == B_OK
			&& msg->FindString("Title", (const char **) &title) == B_OK) {
			
			SetColor(*color);
			Invoke();
		}
}		break;


	default:
		BControl::MessageReceived(msg);
	}
}

