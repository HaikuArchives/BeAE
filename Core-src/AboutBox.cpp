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

#include "AboutBox.h"

#include <Window.h>
#include <View.h>
#include <TranslationKit.h>
#include <Bitmap.h>

class AboutView : public BView
{
  public:
	AboutView(BRect r);
	virtual void MouseDown(BPoint);
	virtual void Draw(BRect r);
	virtual void Pulse();
  private:
	int mode;
	int count;
	char *p;
	int line;
	uint8 r,g,b;
};

enum {FADE_IN, FADE_OUT, PAUSE};

#define FADE_STEPS	20
#define PAUSE_STEPS	30

const char *version = "Version 1.1 August 17, 2009";

static char *txt[] = {
"BeAE - Audio Editing for the BeOS, Haiku and Zeta",
"Created by Frans van Nispen (frans@xentronix.com)",
"© 2000 Xentronix Software http://www.xentronix.com",
"Now hosted on Berlios.de, maintained by Cian Duffy & Scott McCreary.",
"Special thanks goes to:",
"Preference & Language classes: John 'YNOP' Talton,",
"Nicholas Blachford, for helping with the filters and effects,",
"Axel Dörfler & David McPaul for helping out with the MediaKit.",
"Stephan Assmus - German translation and ColdCut color scheme,",
"Gilles Richard - French translation,",
"Giuseppe Gargaro - Italian translation,",
"Sergei Dolgov - Russian translation,",
"Carlos Alberto G-M Costa - Spanish translation,",
"Zsolt Bihari - Hungarian translation,",
"Bruno G. Albuquerque - Portuguese translation,",
"Eugenia Loli-Queru & the members of the beta-program.",
"And those who helped pay the $250 Open Source Fee.",
"Patches welcome...",

NULL};

AboutView::AboutView(BRect rect) : BView(rect, NULL, B_FOLLOW_ALL, B_WILL_DRAW | B_PULSE_NEEDED)
{
	mode = FADE_IN;
	count = FADE_STEPS;
	line = 0;
	p = txt[0];
	SetViewColor(224,238,252);
	r=224; g=238; b=252;
}

void AboutView::MouseDown(BPoint p)
{
	Window()->Quit();
}

void AboutView::Draw(BRect rect)
{
	rect = Bounds();
	BFont font;
	GetFont(&font);
	SetLowColor(255, 255, 255);
	SetHighColor(0,0,0);
	DrawString(version, BPoint(rect.right - font.StringWidth(version)-5, rect.bottom-5)); 
	DrawString("http://developer.berlios.de/projects/beae", BPoint(rect.left+5, rect.bottom-5)); 
	
	SetLowColor(224,238,252);
	SetHighColor(r,g,b);
	DrawString(p, BPoint(rect.Width()/2 - font.StringWidth(p)/2, 208+font.Size()/2)); 

}

void AboutView::Pulse()
{
	BRect rect = Bounds();
	rect.top = 200;
	rect.bottom = 217;

	switch (mode){
	case FADE_IN:
		count --;
		r=224*count/FADE_STEPS;
		g=238*count/FADE_STEPS;
		b=252*count/FADE_STEPS;
		if (count==0){
			count = PAUSE_STEPS;
			mode = PAUSE;
		}
		break;
	case FADE_OUT:
		count --;
		r=224-224*count/FADE_STEPS;
		g=238-238*count/FADE_STEPS;
		b=252-252*count/FADE_STEPS;
		if (count==0){
			Draw(rect);
			count = FADE_STEPS;
			mode = FADE_IN;
			
			line++;
			if (txt[line]==NULL)
				line = 0;
			p = txt[line];
			
		}
		break;
	case PAUSE:
		count --;
		if (count==0){
			count = FADE_STEPS;
			mode = FADE_OUT;
		}
		break;
	}
	if (mode!=PAUSE)
		Draw(rect);
}


/*******************************************************
*   
*******************************************************/
AboutBox::AboutBox(BPoint p) : BWindow(BRect(p.x,p.y,p.x,p.y),"About",B_MODAL_WINDOW_LOOK,B_MODAL_APP_WINDOW_FEEL,B_ASYNCHRONOUS_CONTROLS|B_NOT_RESIZABLE){//B_NOT_ANCHORED_ON_ACTIVATE|
	BView *view;

	BBitmap *bitmap = BTranslationUtils::GetBitmapFile("./Bitmaps/SplashBasic.png");
	if (!bitmap)	bitmap = new BBitmap(BRect(0,0,100,100), B_RGB32);

	BRect r = bitmap->Bounds();
	ResizeTo(r.Width(), r.Height());
	MoveBy(-r.Width()/2, -r.Height()/2);

	AddChild(view = new AboutView(r));
	view->SetViewColor(255,0,0);
	view->SetViewBitmap(bitmap, B_FOLLOW_ALL);
	Run();
	Show();
	
	SetPulseRate(100000);
}



/*******************************************************
*   
*******************************************************/
void AboutBox::MessageReceived(BMessage* msg){
	switch(msg->what){
	default:
		BWindow::MessageReceived(msg);
	}
}

bool AboutBox::QuitRequested(){
	return true;
}
