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

#include "Globals.h"
#include "PointersView.h"
#include "MainWindow.h"
#include "Preferences.h"

class EntryView : public BTextView
{
  public:
	EntryView(BRect r, const char *name, BView *view = NULL);
	~EntryView();
	
	virtual void Draw(BRect r);
	virtual void MakeFocus(bool f);
	virtual void KeyDown(const char *bytes, int32 numBytes);
	
	void SetPointer( int64 p );
	void ConvertToPointer();

  private:
	int64 m_number;
	BView *parent;
};

//==========================
EntryView::EntryView(BRect r, const char *name, BView *view) :
	BTextView(r, name, BRect(5, 2, 68, 16), B_FOLLOW_ALL, B_NAVIGABLE | B_WILL_DRAW)
{
	parent = view;
	SetFontSize(14);
}

EntryView::~EntryView()
{
}

void EntryView::SetPointer( int64 p)
{
	m_number = p;
	
	char s[255];
	// create text
	if (Prefs.display_time == DISPLAY_SAMPLES){
		sprintf(s, "%d", (int)p);
	}else
	if (Prefs.display_time == DISPLAY_TIME){
		float time = p / Pool.frequency;
		float seconds = floor(time);
		time -= seconds;	time *= 1000;
		float minutes = floor(seconds / 60);
		seconds -= minutes*60;

		sprintf(s, "%d:%d.%d", (int)minutes, (int)seconds, (int)time);
	}
	SetText(s);
}

void EntryView::ConvertToPointer()
{
	int64 p;
	// create text
	if (Prefs.display_time == DISPLAY_SAMPLES){
		sscanf(Text(), "%Ld", &p);
	}else
	if (Prefs.display_time == DISPLAY_TIME){
		char t[255], s[255];
		int seconds = 0;
		int minutes = 0;
		float time = 0;
		sscanf(Text(), "%d:%d.%s", &minutes, &seconds, &t);
		sprintf(s, "0.");
		strcat(s, t);
		sscanf(s, "%f", &time);

		if (time<0) time = 0;
		if (minutes<0) minutes = 0;
		if (seconds<0) seconds = 0;
		if (minutes>59) minutes = 0;
		if (seconds>59) seconds = 0;

		p = (int64)(Pool.frequency * (minutes*60+seconds+time));
	}
	SetPointer(p);
}

void EntryView::Draw(BRect r)
{
	StrokeRect(Bounds());
	
	BTextView::Draw(r);
}

void EntryView::MakeFocus(bool focus)
{
	BTextView::MakeFocus( focus );
	if (!focus)
	{
		MoveTo(-10000,-10000);
	}
	else
		SelectAll();
}

void EntryView::KeyDown(const char *bytes, int32 numBytes)
{
	if (numBytes == 1){
		switch (bytes[0]){
		case B_ENTER:
		case B_TAB:
			ConvertToPointer();
			MakeFocus(false);
			if (parent){		// set the number
				((PointersView*)parent)->SetDigits(m_number);
			}
			break;
		
		case B_ESCAPE:
			MakeFocus(false);
			break;
		
		case B_LEFT_ARROW:
		case B_RIGHT_ARROW:
		case B_UP_ARROW:
		case B_DOWN_ARROW:
		case B_HOME:
		case B_END:
		case B_DELETE:
		case B_BACKSPACE:
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '.':
		case ':':
			BTextView::KeyDown(bytes, numBytes);
			break;

		default:
			break;
		}
	} else {
		BTextView::KeyDown(bytes, numBytes);
	}
}

//=======================================================
PointersView::PointersView(BRect r) : 
	BView(r, "Pointers view", B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT, B_WILL_DRAW)
{
	digits = BTranslationUtils::GetBitmapFile("./Bitmaps/SmallDigits.png");
	SetViewColor(B_TRANSPARENT_COLOR);

	pointer = 0;
	full_update = true;

	m_update = 0;
	BRect r(31, 14, 102, 32);
	AddChild(new EntryView(r, "pointer", this));
	FindView("pointer")->MoveTo(-100,-100);
}

//*****************************************************
PointersView::~PointersView()
{
	delete digits;
}

//*****************************************************
void PointersView::AttachedToWindow()
{
	SetViewBitmap(BTranslationUtils::GetBitmapFile("./Bitmaps/PointerView.png"), B_FOLLOW_ALL);
}

//*****************************************************
void PointersView::Pulse()
{
	int64 p = 0;
	if (Pool.sample_type){
		if (Pool.IsPlaying()){
			p = Pool.last_pointer;
		}else
			p = Pool.pointer;
	}

	if (p!=pointer){
		pointer = p;
		full_update = false;
		Draw(Bounds());
		full_update = true;
	}
}

void PointersView::DrawDigits(uint32 x, uint32 y, uint32 count){
	if (Prefs.display_time == DISPLAY_SAMPLES){
		int32 c;
		int32 div = 10000000;
		bool first = true;
		for(int32 p=0; p<8; p++){
			c = count/div +1;
			count -= (c-1)*div;
			div /= 10;
			
			if (c!=1)	first = false;
			if (c==1 && first && p!=7)	c--;
			
			BRect num(8*c, 0, 8*c +7, 12);
			BRect dest(x + p*8, Bounds().top +y, x + p*8 +7, Bounds().top +y +12);
			DrawBitmap(digits, num, dest);
		}
	}else
	if (Prefs.display_time == DISPLAY_TIME){
		float time = count / Pool.frequency;
		float seconds = floor(time);
		time -= seconds;	time *= 1000;
		float minutes = floor(seconds / 60);
		seconds -= minutes*60;
				
		BRect num, dest;
				
		if (floor(minutes/10))
			num.Set(8* (floor(minutes/10)+1), 0, 8*(floor(minutes/10)+1)+7, 12);
		else
			num.Set(0, 0, 7, 12);
		dest.Set(x, Bounds().top +y, x +7, Bounds().top +y+12);
		DrawBitmap(digits, num, dest);
		minutes -= (floor(minutes/10)*10);
		x+=8;

		num.Set(8* (minutes+1), 0, 8*(minutes+1)+7, 12);
		dest.Set(x, Bounds().top +y, x +7, Bounds().top +y+12);
		DrawBitmap(digits, num, dest);
		x+=8;

		num.Set(8* 11+4, 0, 8*11+7, 12);	// :
		dest.Set(x, Bounds().top +y, x +3, Bounds().top +y+12);
		DrawBitmap(digits, num, dest);
		x += 4;

		num.Set(8* (floor(seconds/10)+1), 0, 8*(floor(seconds/10)+1)+7, 12);
		dest.Set(x, Bounds().top +y, x + 7, Bounds().top +y+12);
		DrawBitmap(digits, num, dest);
		x+=8;

		num.Set(8* ((seconds- floor(seconds/10)*10)+1), 0, 8*((seconds - floor(seconds/10)*10)+1)+7, 12);
		dest.Set(x, Bounds().top +y, x + 7, Bounds().top +y+12);
		DrawBitmap(digits, num, dest);
		x+=8;

		num.Set(8* 11+12, 0, 8*11+12+3, 12);	// .
		dest.Set(x, Bounds().top +y, x +3, Bounds().top +y+12);
		DrawBitmap(digits, num, dest);
		x += 4;

		num.Set(8* (floor(time/100)+1), 0, 8*(floor(time/100)+1)+7, 12);
		dest.Set(x, Bounds().top +y, x +7, Bounds().top +y+12);
		DrawBitmap(digits, num, dest);
		time -= (floor(time/100)*100);
		x+=8;

		num.Set(8* (floor(time/10)+1), 0, 8*(floor(time/10)+1)+7, 12);
		dest.Set(x, Bounds().top +y, x +7, Bounds().top +y+12);
		DrawBitmap(digits, num, dest);
		time -= (floor(time/10)*10);
		x+=8;

		num.Set(8* (floor(time)+1), 0, 8*(floor(time)+1)+7, 12);
		dest.Set(x, Bounds().top +y, x +7, Bounds().top +y+12);
		DrawBitmap(digits, num, dest);
	}
}


//*****************************************************
void PointersView::Draw(BRect rect)
{
	if (Pool.sample_type != NONE){
		DrawDigits( 34, 18, Pool.l_pointer);
		DrawDigits( 109,18, Pool.r_pointer);
		DrawDigits( 184,18, Pool.r_pointer - Pool.l_pointer+1);

		DrawDigits( 34, 41, Pool.pointer);
		if (Pool.selection != NONE){
			DrawDigits( 109,41, Pool.r_sel_pointer);
			DrawDigits( 184,41, Pool.r_sel_pointer - Pool.pointer+1);
		}else{
			DrawDigits( 184,41, 0);
		}
//		Sync();
	}
}

//*****************************************************
void PointersView::MouseDown(BPoint p)
{
	if (Pool.size == 0) return;

	uint32 buttons;
	GetMouse(&p, &buttons);
	BMenuItem *it = NULL;
	if (buttons & B_SECONDARY_MOUSE_BUTTON){
		BMenuItem *selected;
		BPopUpMenu *popUp = new BPopUpMenu("");

		BMessage *m = new BMessage(SET_TIME);
		m->AddInt32("time",DISPLAY_SAMPLES);
		popUp->AddItem(it = new BMenuItem(Language.get("SAMPLES"), m));
		if (Prefs.display_time == DISPLAY_SAMPLES)	it->SetMarked(true);

		m = new BMessage(SET_TIME);
		m->AddInt32("time",DISPLAY_TIME);
		popUp->AddItem(it = new BMenuItem(Language.get("TIME"), m));
		if (Prefs.display_time == DISPLAY_TIME)	it->SetMarked(true);
		
		ConvertToScreen(&p);
		selected = popUp->Go(p); 
		if ( selected )
			Window()->PostMessage(selected->Message());
	}
	else
	{
		EntryView *view = (EntryView*)FindView("pointer");

		if (BRect(34, 18, 100, 34).Contains(p))
		{
			m_update = 1;
			view->MoveTo(31,14);
			view->SetPointer(Pool.l_pointer);
			view->MakeFocus(true);
		}else
		if (BRect(106, 14, 177, 32).Contains(p))
		{
			m_update = 2;
			view->MoveTo(106,14);
			view->SetPointer(Pool.r_pointer);
			view->MakeFocus(true);
		}else
		if (BRect(181, 14, 252, 32).Contains(p))
		{
			m_update = 3;
			view->MoveTo(181,14);
			view->SetPointer(Pool.r_pointer - Pool.l_pointer+1);
			view->MakeFocus(true);
		}else
		if (BRect(31, 37, 102, 55).Contains(p))
		{
			m_update = 4;
			view->MoveTo(31,37);
			view->SetPointer(Pool.pointer);
			view->MakeFocus(true);
		}else
		if (BRect(106, 37, 177, 55).Contains(p))
		{
			m_update = 5;
			view->MoveTo(106,37);
			view->SetPointer(Pool.r_sel_pointer);
			view->MakeFocus(true);
		}else
		if (BRect(181, 37, 252, 55).Contains(p))
		{
			m_update = 6;
			view->MoveTo(181,37);
			if (Pool.selection != NONE)
				view->SetPointer(Pool.r_sel_pointer - Pool.pointer+1);
			else
				view->SetPointer(0);
			view->MakeFocus(true);
		}
		else
		{
			view->MoveTo(-10000,-10000);
		}
	}
}


//*****************************************************
void PointersView::SetDigits(int64 p)
{
	if (p < 0)	p = 0;
	if (p > Pool.size)	p = Pool.size;

	switch(m_update){
	case 1:
		if (p < Pool.r_pointer)
			Pool.l_pointer = p;
		break;
	case 2:
		if (p > Pool.l_pointer)
			Pool.r_pointer = p;
		break;	
	case 3:
		if (p < 1)	break;
		Pool.r_pointer = Pool.l_pointer + p -1;
		if (Pool.r_pointer > Pool.size){
			Pool.r_pointer = Pool.size;
			Pool.l_pointer = Pool.r_pointer - p +1;
		}
		break;	
	case 4:
		Pool.pointer = p;
		if (p > Pool.r_sel_pointer){
			Pool.selection = NONE;
			Pool.r_sel_pointer = 0;
		}
		break;	
	case 5:
		Pool.r_sel_pointer = p;
		if (Pool.r_sel_pointer < Pool.pointer){
			int64 t = Pool.r_sel_pointer;
			Pool.r_sel_pointer = Pool.pointer;
			Pool.pointer = t;
		}
		if (Pool.selection == NONE)	Pool.selection = BOTH;
		break;	
	case 6:
		if (p < 1)	break;
		Pool.r_sel_pointer = Pool.pointer + p -1;
		if (Pool.r_sel_pointer > Pool.size){
			Pool.r_sel_pointer = Pool.size;
			Pool.pointer = Pool.r_sel_pointer - p +1;
		}
		if (Pool.selection == NONE)	Pool.selection = BOTH;
		break;	
	}

	Invalidate();
	Pool.update_index = true;
	Window()->FindView("Index view")->Invalidate();
	Window()->FindView("Sample view")->Invalidate();
	Window()->FindView("TimeBar view")->Invalidate();
}

