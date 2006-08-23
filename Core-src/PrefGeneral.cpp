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
#include "PrefGeneral.h"
#include "SpinControl.h"

#define BOOL_CHANGED				'shwG'
#define PEAK_LEVEL					'peak'
#define SPIN_CHANGED				'spnC'
#define SET_TIME					'setT'
#define SET_TEMP					'stTP'

/*******************************************************
*   Setup the main view. Add in all the niffty components
*   we have made and get things rolling
*******************************************************/
PrefGeneral::PrefGeneral(BRect frame):BView(frame, "Prefs general", B_FOLLOW_ALL, 0){
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	BRect r = frame;
//	r.right /=2;
	r.top = 5;	r.bottom = 25;
	float right = r.right;
	
	langmenu = new BPopUpMenu(Language.Name());
	BMenuField *menu;
	r.right *= .75;
	AddChild(menu = new BMenuField(r,NULL,Language.get("LANGUAGE"),langmenu));
	r.right = right;
	menu->SetDivider(be_plain_font->StringWidth(Language.get("LANGUAGE")) +10);
	
	
	r.OffsetBy(0,30);
	AddChild(c_grid = new BCheckBox(r, "grid", Language.get("SHOWGRID"), new BMessage(BOOL_CHANGED)));
	if (Prefs.show_grid)	c_grid->SetValue(B_CONTROL_ON);
	r.OffsetBy(0,18);
	AddChild(c_peak = new BCheckBox(r, "peak", Language.get("SHOWPEAK"), new BMessage(BOOL_CHANGED)));
	if (Prefs.show_peak)	c_peak->SetValue(B_CONTROL_ON);
	r.OffsetBy(0,18);
	r.right *= .6;
	AddChild(s_peak = new SpinControl(r, NULL, Language.get("PEAKLEVEL"), new BMessage(PEAK_LEVEL), 1, 100, Prefs.peak*100, 2));
	r.right = right;
	s_peak->SetDivider(r.Width()*.4);

	r.OffsetBy(0,28);
	AddChild(c_paste = new BCheckBox(r, "paste", Language.get("SELECT_PASTE"), new BMessage(BOOL_CHANGED)));
	if (Prefs.select_after_paste)	c_paste->SetValue(B_CONTROL_ON);
	r.OffsetBy(0,18);
	AddChild(c_follow_playing = new BCheckBox(r, "follow", Language.get("FOLLOW_PLAYING"), new BMessage(BOOL_CHANGED)));
	if (Prefs.follow_playing)	c_follow_playing->SetValue(B_CONTROL_ON);
	r.OffsetBy(0,18);
	AddChild(c_play = new BCheckBox(r, "play", Language.get("PLAYONLOAD"), new BMessage(BOOL_CHANGED)));
	if (Prefs.play_when_loaded)	c_play->SetValue(B_CONTROL_ON);
	r.OffsetBy(0,18);
	AddChild(c_double = new BCheckBox(r, "double", Language.get("SELECT_DOUBLE"), new BMessage(BOOL_CHANGED)));
	if (Prefs.select_all_on_double)	c_double->SetValue(B_CONTROL_ON);
	r.OffsetBy(0,18);
	AddChild(c_drag_drop = new BCheckBox(r, "dragndrop", Language.get("DRAG_DROP"), new BMessage(BOOL_CHANGED)));
	if (Prefs.drag_drop)	c_drag_drop->SetValue(B_CONTROL_ON);

	r.OffsetBy(0,28);
	time = new BPopUpMenu(Language.get("TIMEDISPLAY"));
	menu = new BMenuField(r,NULL,Language.get("TIMEDISPLAY"),time);

	BMessage *m = new BMessage(SET_TIME);
	m->AddInt32("time",DISPLAY_SAMPLES);
	time->AddItem(menu_sample = new BMenuItem(Language.get("SAMPLES"), m));
	if (Prefs.display_time == DISPLAY_SAMPLES)	menu_sample->SetMarked(true);

	m = new BMessage(SET_TIME);
	m->AddInt32("time",DISPLAY_TIME);
	time->AddItem(menu_time = new BMenuItem(Language.get("TIME"), m));
	if (Prefs.display_time == DISPLAY_TIME)	menu_time->SetMarked(true);

	menu->SetDivider(be_plain_font->StringWidth(Language.get("TIMEDISPLAY")) +10);
	AddChild(menu);

	r.OffsetBy(0,28);
	r.right *= .98;
	AddChild(temp_file = new BTextControl(r, NULL, Language.get("TEMP_DIR"), Prefs.temp_dir.String(), new BMessage(SET_TEMP) ));
	temp_file->SetDivider(r.Width()*.4);

	r.OffsetBy(0,24);
	AddChild(s_free = new SpinControl(r, NULL, Language.get("KEEP_FREE"), new BMessage(SPIN_CHANGED), 10, 10000, Prefs.keep_free, 10));
	r.right = right;
	s_free->SetDivider(r.Width()*.74);

}

/*******************************************************
*  
*******************************************************/
void PrefGeneral::AddLanguageMenu()
{
	while (langmenu->ItemAt(0))
		langmenu->RemoveItem(langmenu->ItemAt(0));

	app_info ai;
	be_app->GetAppInfo(&ai);
	BEntry entry(&ai.ref);
	BPath path;
	entry.GetPath(&path);
	path.GetParent(&path);
	path.Append("Languages");
	BDirectory directory(path.Path());
	char name[B_FILE_NAME_LENGTH];

	while (directory.GetNextEntry(&entry, false) == B_OK){
		if(entry.IsFile()){
			entry.GetPath(&path);
			entry.GetName(name);
			BMessage *m = new BMessage(CHANGE_LANGUAGE);
			m->AddString("language",name);
			BMenuItem *menuitem = new BMenuItem(name, m, 0, 0);
			menuitem->SetTarget(this);
			langmenu->AddItem(menuitem);
			if(!strcmp(name,Language.Name())){
				menuitem->SetMarked(true);
			}
		}
	}
}

/*******************************************************
*  
*******************************************************/
PrefGeneral::~PrefGeneral()
{
}

/*******************************************************
*  
*******************************************************/
void PrefGeneral::AttachedToWindow(){
	AddLanguageMenu();
	c_grid->SetTarget(this);
	c_paste->SetTarget(this);
	c_play->SetTarget(this);
	c_double->SetTarget(this);
	c_peak->SetTarget(this);
	c_follow_playing->SetTarget(this);
	s_peak->SetTarget(this);
	s_free->SetTarget(this);
	c_drag_drop->SetTarget(this);
	temp_file->SetTarget(this);
	time->SetTargetForItems(this);

	if (Prefs.display_time == DISPLAY_SAMPLES)
	{
		menu_sample->SetMarked(true);
		menu_time->SetMarked(false);
	}
	else
	{
		menu_time->SetMarked(true);
		menu_sample->SetMarked(false);
	}
}

/*******************************************************
*
*******************************************************/
void PrefGeneral::MessageReceived(BMessage *msg){
	char *lname = NULL;
	int32 i;

	switch(msg->what){
	case CHANGE_LANGUAGE:
		if(msg->FindString("language",(const char**)&lname) == B_OK){
			Language.SetName(lname);
			Window()->PostMessage(msg);
			be_app->PostMessage(CHANGE_LANGUAGE);
		}
		break;
	
	case BOOL_CHANGED:
		Prefs.show_grid = (c_grid->Value() == B_CONTROL_ON);
		Prefs.select_after_paste = (c_paste->Value() == B_CONTROL_ON);
		Prefs.select_all_on_double = (c_double->Value() == B_CONTROL_ON);
		Prefs.play_when_loaded = (c_play->Value() == B_CONTROL_ON);
		Prefs.show_peak = (c_peak->Value() == B_CONTROL_ON);
		Prefs.follow_playing = (c_follow_playing->Value() == B_CONTROL_ON);
		Prefs.drag_drop = (c_drag_drop->Value() == B_CONTROL_ON);
		Pool.update_draw_cache = true;	// update the draw cache
		Pool.sample_view_dirty = true;	// update the sample-view
		Pool.RedrawWindow();
		break;
	
	case PEAK_LEVEL:
		Prefs.peak = s_peak->Value()/100.0;
		Pool.sample_view_dirty = true;	// update the sample-view
		Pool.update_draw_cache = true;	// update the draw cache
		Pool.RedrawWindow();
		break;

	case SPIN_CHANGED:
		Prefs.keep_free = s_free->Value();
		break;
	
	case SET_TIME:
		msg->FindInt32("time", &i);
		Prefs.display_time = i;
		Pool.RedrawWindow();
		break;
	
	case SET_TEMP:
		Prefs.temp_dir.SetTo(temp_file->Text());
		(new BAlert(NULL,Language.get("TEMP_CHANGED"),Language.get("OK")))->Go();
		break;

	default:
		BView::MessageReceived(msg);
		break;   
	}
}
