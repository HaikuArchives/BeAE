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

#include <Application.h>
#include <InterfaceKit.h>
#include <StorageKit.h>
#include <MediaKit.h>
#include <stdio.h>
#include <stdlib.h>
#include <Cursor.h>

#include "Globals.h"
#include "MainWindow.h"
#include "VUView.h"
#include "main.h"
#include "TransportView.h"
#include "PointersView.h"
#include "ToolBarView.h"
#include "BigView.h"
#include "SampleView.h"
#include "ValuesView.h"
#include "IndexView.h"
#include "TimeBarView.h"
#include "Filters.h"
#include "PrefWindow.h"
#include "FreqWindow.h"
#include "MyClipBoard.h"
#include "Shortcut.h"
#include "FilterDialogs.h"
#include "Analyzers.h"
#include "VMSystem.h"

cookie_record play_cookie;
#define UPDATE	'updt'

// URL to the homepage/Tutorials/others
//char *TUTORIALS_URL = "http://www.xentronix.com/module.php?mod=document";
char *HOMEPAGE_URL = "http://developer.berlios.de/projects/beae/";


class MyMenuBar : public BMenuBar{
  public:
  	MyMenuBar(BRect frame, const char *name);
	virtual void MakeFocus(bool b);
};

MyMenuBar::MyMenuBar(BRect r, const char *name) : BMenuBar(r, name)
{
}

void MyMenuBar::MakeFocus(bool b)
{
	// This one does make sure the MenuBar does NOT get focus !!!
	// To avoid the key-navigation to stop working
}


#ifdef __VM_SYSTEM

void BufferPlayer(void *theCookie, void *buffer, size_t size, const media_raw_audio_format &format)
{
	// We're going to be cheap and only work for floating-point audio 
	if (format.format != media_raw_audio_format::B_AUDIO_FLOAT) { 
		return; 
	}
	
	bool stop_needed = false;
	size_t i; 
	float *buf = (float *) buffer; 
	size_t float_size = size/4; 
	cookie_record *cookie = (cookie_record *) theCookie;
	float left = 0.0, right = 0.0;
	float fraq = fabs(Pool.frequency/Pool.system_frequency);
	
	if ((Pool.selection == NONE) | cookie->end){
		cookie->end_mem = Pool.size*Pool.sample_type;
	}else{
		cookie->end_mem = Pool.r_sel_pointer*Pool.sample_type;
	}
	
	int32 mem_size = float_size * (int32)ceil(fraq);
	float *mem = cookie->buffer;
	VM.ReadCache( mem, mem_size );
//	VM.ReadBlockAt( cookie->mem, mem, mem_size );

	// Now fill the buffer with sound! 

	if (cookie->pause){
		for (i=0; i<float_size; i++) { 
			buf[i] = 0.0;
		}
	}else{
		if (Pool.sample_type == MONO){	//cookie->mono){
			for (i=0; i<float_size; i+=2){
				if (cookie->mem >= cookie->end_mem){
					if (cookie->loop){
						cookie->mem = Pool.pointer*Pool.sample_type;
						VM.SetPlayPointer( cookie->mem );
						VM.ReadCache( mem, mem_size );
					}else{
						buf[i] = 0.0;
						buf[i+1] = 0.0;
						stop_needed = true;
					}
				}
				if (!stop_needed){
					buf[i] = *mem;
					buf[i+1] = *mem;

					cookie->add += fraq;
					if (Pool.frequency>=0){
						while (cookie->add >= 1.0){
							cookie->mem++;
							mem++;
							--cookie->add;
						}
					}else{
						while (cookie->add >= 1.0){
							cookie->mem--;
							mem--;
							--cookie->add;
						}
						if (cookie->mem < 0)
							cookie->mem += cookie->end_mem;
					}
				}
			}
		}else{
			for (i=0; i<float_size; i+=2) { 
				if (cookie->mem >= cookie->end_mem){
					if (cookie->loop){
						cookie->mem = Pool.pointer*Pool.sample_type;
						VM.SetPlayPointer( cookie->mem );
						VM.ReadCache( mem, mem_size );
					}else{
						buf[i] = 0.0f;
						buf[i+1] = 0.0f;
						stop_needed = true;
					}
				}
				if (!stop_needed){
					switch(Pool.selection){
					case NONE:
					case BOTH:
						buf[i] = mem[0];
						buf[i+1] = mem[1];
						break;
					case LEFT:
						buf[i] = mem[0];
						buf[i+1] = mem[0];
						break;
					case RIGHT:
						buf[i] = mem[1];
						buf[i+1] = mem[1];
						break;
					}

					cookie->add += fraq;
					if (Pool.frequency>=0){
						while (cookie->add >= 1.0){
							cookie->mem+= 2;
							mem+=2;
							--cookie->add;
						}
					}else{
						while (cookie->add >= 1.0){
							cookie->mem-= 2;
							mem-=2;
							--cookie->add;
						}
						if (cookie->mem < 0)
							cookie->mem += cookie->end_mem;
					}
				}
			}
		}
	}
	
	Pool.last_pointer = cookie->mem;	// set the last played location
	if (Pool.sample_type == STEREO)
		Pool.last_pointer >>= 1;

	if (stop_needed)
		Pool.mainWindow->PostMessage(TRANSPORT_STOP);

	for (int i=0; i<PLAY_HOOKS; i++){
		if (Pool.BufferHook[i]){
			(Pool.BufferHook[i])(buf, float_size, Pool.BufferCookie[i]);
		}
	}
	

	for (i=0; i<float_size; i+=2) { 
		left = MAX(buf[i], left);
		right = MAX(buf[i+1],right);
	}
	cookie->left = left;
	cookie->right = right;

	// update the visuals
	cookie->count -= size;
	if (cookie->count <0){
		cookie->count = (int)Pool.system_frequency/3;						// 24 times a second
			Pool.mainWindow->PostMessage(UPDATE);
	}
}

#else
void BufferPlayer(void *theCookie, void *buffer, size_t size, const media_raw_audio_format &format)
{
	// We're going to be cheap and only work for floating-point audio 
	if (format.format != media_raw_audio_format::B_AUDIO_FLOAT) { 
		return; 
	}
	
// assumes 44.1Khz stereo floats

	bool stop_needed = false;
	size_t i; 
	float *buf = (float *) buffer; 
	size_t float_size = size/4; 
	cookie_record *cookie = (cookie_record *) theCookie;
	float left = 0.0, right = 0.0;
	double fraq = fabs(Pool.frequency/Pool.system_frequency);
	
	if ((Pool.selection == NONE) | cookie->end){
		cookie->end_mem = Pool.sample_memory + Pool.size*Pool.sample_type;
	}else{
		cookie->end_mem = Pool.sample_memory + Pool.r_sel_pointer*Pool.sample_type;
	}
	
	// Now fill the buffer with sound! 

	if (cookie->pause){
		for (i=0; i<float_size; i++) { 
			buf[i] = 0.0;
		}
	}else{
		if (Pool.sample_type == MONO){	//cookie->mono){
			for (i=0; i<float_size; i+=2){
				if (cookie->mem >= cookie->end_mem){
					if (cookie->loop){
						cookie->mem = Pool.sample_memory + Pool.pointer*Pool.sample_type;
					}else{
						buf[i] = 0.0;
						buf[i+1] = 0.0;
						stop_needed = true;
					}
				}
				if (!stop_needed){
					buf[i] = *cookie->mem;
					buf[i+1] = *cookie->mem;

					cookie->add += fraq;
					if (Pool.frequency>=0){
						while (cookie->add >= 1.0){
							cookie->mem++;
							--cookie->add;
						}
					}else{
						while (cookie->add >= 1.0){
							cookie->mem--;
							--cookie->add;
						}
						if (cookie->mem < Pool.sample_memory)
							cookie->mem += (cookie->end_mem - Pool.sample_memory);
					}
				}
			}
		}else{
			for (i=0; i<float_size; i+=2) { 
				if (cookie->mem >= cookie->end_mem){
					if (cookie->loop){
						cookie->mem = Pool.sample_memory + Pool.pointer*Pool.sample_type;
					}else{
						buf[i] = 0.0;
						buf[i+1] = 0.0;
						stop_needed = true;
					}
				}
				if (!stop_needed){
					switch(Pool.selection){
					case NONE:
					case BOTH:
						buf[i] = cookie->mem[0];
						buf[i+1] = cookie->mem[1];
						break;
					case LEFT:
						buf[i] = cookie->mem[0];
						buf[i+1] = cookie->mem[0];
						break;
					case RIGHT:
						buf[i] = cookie->mem[1];
						buf[i+1] = cookie->mem[1];
						break;
					}

					cookie->add += fraq;
					if (Pool.frequency>=0){
						while (cookie->add >= 1.0){
							cookie->mem+= 2;
							--cookie->add;
						}
					}else{
						while (cookie->add >= 1.0){
							cookie->mem-= 2;
							--cookie->add;
						}
						if (cookie->mem < Pool.sample_memory)
							cookie->mem += (cookie->end_mem - Pool.sample_memory);
					}
					
				}
			}
		}
	}

	Pool.last_pointer = (cookie->mem - Pool.sample_memory);	// set the last played location
	if (Pool.sample_type == STEREO)
		Pool.last_pointer >>= 1;

	if (stop_needed)
		Pool.mainWindow->PostMessage(TRANSPORT_STOP);

	for (int i=0; i<PLAY_HOOKS; i++){
		if (Pool.BufferHook[i]){
			(Pool.BufferHook[i])(buf, float_size, Pool.BufferCookie[i]);
		}
	}
	

	for (i=0; i<float_size; i+=2) { 
		left = MAX(buf[i], left);
		right = MAX(buf[i+1],right);
	}
	cookie->left = left;
	cookie->right = right;

	// update the visuals
	cookie->count -= size;
	if (cookie->count <0){
		cookie->count = (int)Pool.system_frequency/3;						// 24 times a second
			Pool.mainWindow->PostMessage(UPDATE);
	}
}
#endif

MainWindow::MainWindow(BRect frame)
	:BWindow(frame, NULL ,B_TITLED_WINDOW,B_ASYNCHRONOUS_CONTROLS)
{
	Looper()->SetName("BeAE");

	// create global access
	Pool.mainWindow = this;			// handle to this window

	// set MIME types
	Pool.InstallMimeType();

	// init prefs
	Prefs.Init();
	ClipBoard.Init();				// clipboard init
	Hist.Init();					// Undo init
#ifdef __VM_SYSTEM
	VM.Init();
#endif

	Pool.mouseArrow = new BCursor(IMouse_Arrow);
	Pool.mouseArrowLeft = new BCursor(IMouse_ArrowLeft);
	Pool.mouseArrowRight = new BCursor(IMouse_ArrowRight);
	Pool.mousePencil = new BCursor(IMouse_Pencil);
	Pool.mouseMove = new BCursor(IMouse_Move);
	Pool.mouseArrowMove = new BCursor(IMouse_MoveArrow);
	Pool.mouseLeftRight = new BCursor(IMouse_LeftRight);

	ResizeTo( Prefs.frame.Width(), Prefs.frame.Height() );
	MoveTo( Prefs.frame.left, Prefs.frame.top );

	// Set Language
	Language.SetName(Prefs.lang_name.String());
	
	// Now init the keyBindings
	KeyBind.Init();
	
	// Set up the tool tipper
	// befor we load any other tools or windows
	Pool.tt = new ToolTip();
	Pool.progress = new ProgressWindow(BRect(0,0,300,30));
	Pool.PrefWin = new PrefWindow();
	
	// create the player and recorder nodes
	Pool.InitBufferPlayer( 44100 );	

	// This is the Pool .. it loads layers and tools and floaters
	// and all kinds of groovy stuff
	Pool.Init();

	// Set the last filter
	FiltersInit();

	char s[255];
	sprintf(s, "BeAE - %s", Language.get("UNTITLED"));
	SetTitle(s);

// GUI
	mainMenuBar = NULL;
	AddMenu();

	BRect r(0,0,31,62);				// VU meters
	r.OffsetTo(229,Bounds().bottom - 62);
	AddChild(VU_view = new VUView(r));
	Pool.m_VU_View = VU_view;		// add a pointer for the player

	r.Set(0,0,228,62);				// Transport buttons
	r.OffsetTo(0,Bounds().bottom - 62);
	AddChild(transport_view = new TransportView(r));

	r.Set(0,0,259,62);				// Numers at right
	r.OffsetTo(Bounds().right - 259,Bounds().bottom - 62);
	AddChild(pointer_view = new PointersView(r));

	r = Bounds();					// Toolbar
	r.top += 20;
	r.bottom = r.top+32;
	r.right = 3000;
	AddChild(toolBar = new ToolBarView(r));

	r = Bounds();					// Big LCD
	r.top = r.bottom -62;
	r.left = 261;
	r.right -= 260;
	AddChild(new BigView(r));


// sample Views
	BRect sv = Bounds();
	sv.top += (21+32);
	sv.bottom -= 63;

	r = sv;							// wave display
	r.left += VALUEBAR_WIDTH;
	r.bottom -= TIMEBAR_HEIGHT;
	r.top += INDEXVIEW_HEIGHT;
	AddChild(sample_view = new SampleView(r));
	Pool.m_SampleView = sample_view;	// for the player
	((SampleView*)sample_view)->Init();
	
	r = sv;							// left sample heights
//	r.top += INDEXVIEW_HEIGHT;
	r.right = VALUEBAR_WIDTH-1;
	AddChild(new ValuesView(r));
	
	r = sv;							// bottom timescale
	r.left = VALUEBAR_WIDTH;
	r.top = r.bottom - TIMEBAR_HEIGHT+1;
	AddChild(new TimeBarView(r));

	r = sv;							// scroll_view / overview
	r.left = VALUEBAR_WIDTH;
	r.bottom = r.top + INDEXVIEW_HEIGHT-1;
	AddChild(index_view = new IndexView(r));
	
//	r = sv;
//	r.bottom = r.top + INDEXVIEW_HEIGHT-1;
//	AddChild(new IndexView(r));


	Pool.SetLoop(  (transport_view->loop->Value() == B_CONTROL_ON) );
	SetSizeLimits(MIN_W ,MAX_W, MIN_H , MAX_H);
	SetPulseRate(50000);
}

void MainWindow::AddMenu()
{
	BMenu		*menu;
	BMenuItem	*menuItem;
	BRect		menuBarRect;
	
	menuBarRect.Set(0.0, 0.0, 10000.0, 18.0);
	mainMenuBar = new MyMenuBar(menuBarRect, "MenuBar");
	AddChild(mainMenuBar);
	
	menu = new BMenu(Language.get("FILE_MENU"));
	mainMenuBar->AddItem(menu);
	menu->AddItem(menuItem = new BMenuItem(Language.get("FILE_NEW"), new BMessage(NEW), KeyBind.GetKey("FILE_NEW"), KeyBind.GetMod("FILE_NEW")));

	recent_menu = new BMenu(Language.get("FILE_OPEN"));
	UpdateRecent();
	menu->AddItem(recent_menu);
	BMenuItem *openitem = menu->FindItem(Language.get("FILE_OPEN"));
	openitem->SetShortcut(KeyBind.GetKey("FILE_OPEN"),KeyBind.GetMod("FILE_OPEN"));
	openitem->SetMessage(new BMessage(OPEN));
//	openitem->SetShortcut('O', 0);

	menu->AddItem(menuItem = new BMenuItem(Language.get("FILE_INSERT"), new BMessage(INSERT), KeyBind.GetKey("FILE_INSERT"), KeyBind.GetMod("FILE_INSERT")));
	menuItem->SetEnabled(false);
	menu->AddItem(menuItem = new BMenuItem(Language.get("FILE_APPEND"), new BMessage(APPEND), KeyBind.GetKey("FILE_APPEND"), KeyBind.GetMod("FILE_APPEND")));
	menuItem->SetEnabled(false);
	menu->AddItem(menuItem = new BMenuItem(Language.get("FILE_MIX"), new BMessage(OPEN_MIX), KeyBind.GetKey("FILE_MIX"), KeyBind.GetMod("FILE_MIX")));
	menuItem->SetEnabled(false);
	menu->AddSeparatorItem();
	menu->AddItem(menuItem = new BMenuItem(Language.get("FILE_SAVE"), new BMessage(SAVE), KeyBind.GetKey("FILE_SAVE"), KeyBind.GetMod("FILE_SAVE")));
	Pool.mn_save = menuItem;
	menu->AddItem(menuItem = new BMenuItem(Language.get("FILE_SAVE_AS"), new BMessage(SAVE_AS), KeyBind.GetKey("FILE_SAVE_AS"), KeyBind.GetMod("FILE_SAVE_AS")));
	Pool.mn_save_as = menuItem;
	menu->AddItem(menuItem = new BMenuItem(Language.get("FILE_SAVE_SELECTION"), new BMessage(SAVE_SELECTION), KeyBind.GetKey("FILE_SAVE_SELECTION"), KeyBind.GetMod("FILE_SAVE_SELECTION")));
	Pool.mn_save_sel = menuItem;
	menu->AddSeparatorItem();
	menu->AddItem(menuItem = new BMenuItem(Language.get("PREFERENCES"), new BMessage(PREFERENCES), KeyBind.GetKey("PREFERENCES"), KeyBind.GetMod("PREFERENCES")));
	menu->AddSeparatorItem();
	menu->AddItem(menuItem = new BMenuItem(Language.get("FILE_QUIT"), new BMessage(B_QUIT_REQUESTED), KeyBind.GetKey("FILE_QUIT"), KeyBind.GetMod("FILE_QUIT")));

	menu = new BMenu(Language.get("EDIT_MENU"));
	Pool.menu_edit = menu;
	mainMenuBar->AddItem(menu);
	menu->AddItem(menuItem = new BMenuItem(Language.get("UNDO"), new BMessage(UNDO), KeyBind.GetKey("UNDO"), KeyBind.GetMod("UNDO")));
	Pool.mn_undo = menuItem;
#ifdef __SAMPLE_STUDIO_LE
	menu->AddItem(menuItem = new BMenuItem(Language.get("REDO"), new BMessage(REDO), KeyBind.GetKey("REDO"), KeyBind.GetMod("REDO")));
	Pool.mn_redo = menuItem;
#endif
	menu->AddItem(menuItem = new BMenuItem(Language.get("UNDO_ENABLE"), new BMessage(UNDO_ENABLE), KeyBind.GetKey("UNDO_ENABLE"), KeyBind.GetMod("UNDO_ENABLE")));
	Pool.mn_undo_enable = menuItem;
	menuItem->SetMarked(Prefs.save_undo);
	menu->AddSeparatorItem();
	menu->AddItem(menuItem = new BMenuItem(Language.get("COPY"), new BMessage(B_COPY), KeyBind.GetKey("COPY"), KeyBind.GetMod("COPY")));
	Pool.mn_copy = menuItem;
	menu->AddItem(menuItem = new BMenuItem(Language.get("COPY_SILENCE"), new BMessage(COPY_SILENCE), KeyBind.GetKey("COPY_SILENCE"), KeyBind.GetMod("COPY_SILENCE")));
	Pool.mn_copy_silence = menuItem;
	menu->AddItem(menuItem = new BMenuItem(Language.get("CUT"), new BMessage(B_CUT), KeyBind.GetKey("CUT"), KeyBind.GetMod("CUT")));
	Pool.mn_cut = menuItem;
	menu->AddItem(menuItem = new BMenuItem(Language.get("PASTE"), new BMessage(B_PASTE), KeyBind.GetKey("PASTE"), KeyBind.GetMod("PASTE")));
	Pool.mn_paste = menuItem;
	menu->AddItem(menuItem = new BMenuItem(Language.get("PASTE_NEW"), new BMessage(PASTE_NEW), KeyBind.GetKey("PASTE_NEW"), KeyBind.GetMod("PASTE_NEW")));
	Pool.mn_paste_new = menuItem;
#ifdef __SAMPLE_STUDIO_LE
	menu->AddItem(menuItem = new BMenuItem(Language.get("EDIT_PASTE_MIX"), new BMessage(PASTE_MIXED), KeyBind.GetKey("EDIT_PASTE_MIX"), KeyBind.GetMod("EDIT_PASTE_MIX")));
	menuItem->SetEnabled(false);
	Pool.mn_paste_mix = menuItem;
	menu->AddItem(menuItem = new BMenuItem(Language.get("COPY_TO_STACK"), new BMessage(TO_STACK), KeyBind.GetKey("COPY_TO_STACK"), KeyBind.GetMod("COPY_TO_STACK")));
	Pool.mn_copy_to_stack = menuItem;
#endif
	menu->AddSeparatorItem();
	menu->AddItem(menuItem = new BMenuItem(Language.get("SELECT_ALL"), new BMessage(B_SELECT_ALL), KeyBind.GetKey("SELECT_ALL"), KeyBind.GetMod("SELECT_ALL")));
	Pool.mn_select_all = menuItem;
	menu->AddItem(menuItem = new BMenuItem(Language.get("UNSELECT_ALL"), new BMessage(UNSELECT_ALL), KeyBind.GetKey("UNSELECT_ALL"), KeyBind.GetMod("UNSELECT_ALL")));
	Pool.mn_unselect = menuItem;
	menu->AddSeparatorItem();
	BMenu *sub = new BMenu(Language.get("ZERO_CROSS"));
	Pool.menu_zero = sub;
	sub->AddItem(new BMenuItem(Language.get("ZERO_IN"), new BMessage(ZERO_IN), KeyBind.GetKey("ZERO_IN"), KeyBind.GetMod("ZERO_IN")));
	sub->AddItem(new BMenuItem(Language.get("ZERO_OUT"), new BMessage(ZERO_OUT), KeyBind.GetKey("ZERO_OUT"), KeyBind.GetMod("ZERO_OUT")));
	sub->AddItem(new BMenuItem(Language.get("ZERO_LL"), new BMessage(ZERO_LL), KeyBind.GetKey("ZERO_LL"), KeyBind.GetMod("ZERO_LL")));
	sub->AddItem(new BMenuItem(Language.get("ZERO_LR"), new BMessage(ZERO_LR), KeyBind.GetKey("ZERO_LR"), KeyBind.GetMod("ZERO_LR")));
	sub->AddItem(new BMenuItem(Language.get("ZERO_RL"), new BMessage(ZERO_RL), KeyBind.GetKey("ZERO_RL"), KeyBind.GetMod("ZERO_RL")));
	sub->AddItem(new BMenuItem(Language.get("ZERO_RR"), new BMessage(ZERO_RR), KeyBind.GetKey("ZERO_RR"), KeyBind.GetMod("ZERO_RR")));
	menu->AddItem(sub);
	
	menu->AddSeparatorItem();
	menu->AddItem(menuItem = new BMenuItem(Language.get("CLEAR"), new BMessage(CLEAR), KeyBind.GetKey("CLEAR"), KeyBind.GetMod("CLEAR")));
	Pool.mn_clear = menuItem;
	menu->AddItem(menuItem = new BMenuItem(Language.get("TRIM"), new BMessage(TRIM), KeyBind.GetKey("TRIM"), KeyBind.GetMod("TRIM")));
	Pool.mn_trim = menuItem;
	menu->AddItem(menuItem = new BMenuItem(Language.get("SET_FREQ"), new BMessage(SET_FREQUENCY), KeyBind.GetKey("SET_FREQ"), KeyBind.GetMod("SET_FREQ")));
	Pool.mn_set_freq = menuItem;
	menu->AddItem(menuItem = new BMenuItem(Language.get("RESAMPLE"), new BMessage(RESAMPLE), KeyBind.GetKey("RESAMPLE"), KeyBind.GetMod("RESAMPLE")));
	Pool.mn_resample = menuItem;

	menu = new BMenu(Language.get("TRANSFORM_MENU"));
	Pool.menu_transform = menu;
	mainMenuBar->AddItem(menu);

	menu = new BMenu(Language.get("ANALYZE_MENU"));
	Pool.menu_analyze = menu;
	mainMenuBar->AddItem(menu);

	#ifdef __SAMPLE_STUDIO_LE
		menu = new BMenu(Language.get("GENERATE_MENU"));
		Pool.menu_generate = menu;
		mainMenuBar->AddItem(menu);
	#endif

	menu = new BMenu(Language.get("HELP_MENU"));
	mainMenuBar->AddItem(menu);
	menu->AddItem(menuItem = new BMenuItem(Language.get("HELP"), new BMessage(HELP), KeyBind.GetKey("HELP"), KeyBind.GetMod("HELP")));
//	menu->AddSeparatorItem();
	menu->AddItem(menuItem = new BMenuItem(Language.get("HISTORY"), new BMessage(HISTORY), KeyBind.GetKey("HISTORY"), KeyBind.GetMod("HISTORY")));
	menu->AddItem(menuItem = new BMenuItem(Language.get("HOMEPAGE"), new BMessage(HOMEPAGE), KeyBind.GetKey("HOMEPAGE"), KeyBind.GetMod("HOMEPAGE")));
	menu->AddSeparatorItem();
	menu->AddItem(menuItem = new BMenuItem(Language.get("ABOUT"), new BMessage(ABOUT), KeyBind.GetKey("ABOUT"), KeyBind.GetMod("ABOUT")));

	SetKeyMenuBar(NULL);
}

MainWindow::~MainWindow()
{
}

void MainWindow::UpdateRecent()
{
	BMenuItem *menuItem;
	BMessage msg;
	BMessage *msgout = NULL;
	entry_ref eref;
	BEntry e;
	int32 i = 0;
	char name[B_FILE_NAME_LENGTH];
	
	while(recent_menu->ItemAt(0))
		recent_menu->RemoveItem(recent_menu->ItemAt(0));

	be_roster->GetRecentDocuments(&msg,10,"audio");
	while(msg.FindRef("refs",i,&eref) == B_OK){
		e.SetTo(&eref);
		if(e.InitCheck() == B_OK){
			e.GetName(name);
			msgout = new BMessage(B_REFS_RECEIVED);//DO_OPEN);
			msgout->AddRef("refs",&eref);
			recent_menu->AddItem(menuItem = new BMenuItem(name,msgout));
			menuItem->SetTarget(be_app);
		}
		i++;
	}
}

bool MainWindow::QuitRequested()
{
	if (!Pool.IsChanged(2)){
		Prefs.frame = Frame();
		Pool.StopPlaying();
		be_app->PostMessage(B_QUIT_REQUESTED);
		return true;
	}else{
		return false;
	}
}

void MainWindow::MessageReceived(BMessage *message)
{
	RealtimeFilter *filter = NULL;
	int32 x, key, mod, raw_key;
	uint32 msg;
	float y;

//	message->PrintToStream();

	switch (message->what){
	case CHANGE_LANGUAGE:
		if (mainMenuBar){
			mainMenuBar->RemoveSelf();
			delete mainMenuBar;
		}
		AddMenu();
		Pool.UpdateMenu();
		break;

	case TRANSPORT_PAUSE_MAN:
		if (transport_view->pause->Value() == B_CONTROL_ON)
			transport_view->pause->SetValue(B_CONTROL_OFF);
		else
			transport_view->pause->SetValue(B_CONTROL_ON);
		// no break as TRANSPORT_PAUSE needs to follow
	case TRANSPORT_PAUSE:
		play_cookie.pause = (transport_view->pause->Value() == B_CONTROL_ON);
		break;

	case TRANSPORT_TOGGLE:
		if (Pool.IsPlaying())	PostMessage(TRANSPORT_STOP);
		else					PostMessage(TRANSPORT_PLAYS);
		break;
	
	case TRANSPORT_SET:
		Pool.pointer = Pool.last_pointer;
		Pool.sample_view_dirty = true;	// update the sample-view
		Pool.RedrawWindow();
		break;

	case TRANSPORT_PLAY:
		if (Pool.size == 0)	break;
		if (Pool.sample_type == NONE){
			transport_view->play_sel->SetValue(B_CONTROL_OFF);
			transport_view->play->SetValue(B_CONTROL_OFF);
			break;
		}
		transport_view->stop->SetValue(B_CONTROL_OFF);
		transport_view->play->SetValue(B_CONTROL_OFF);
		transport_view->play_sel->SetValue(B_CONTROL_ON);

//		Pool.SetLoop(  (transport_view->loop->Value() == B_CONTROL_ON) );
		Pool.StartPlaying(Pool.pointer*Pool.sample_type, true);	// play till end
		break;

	case TRANSPORT_PLAYS:
		if (Pool.size == 0)	break;
		if (Pool.sample_type == NONE){
			transport_view->play_sel->SetValue(B_CONTROL_OFF);
			transport_view->play->SetValue(B_CONTROL_OFF);
			break;
		}
		transport_view->stop->SetValue(B_CONTROL_OFF);
		transport_view->play_sel->SetValue(B_CONTROL_OFF);
		transport_view->play->SetValue(B_CONTROL_ON);

//		Pool.SetLoop(  (transport_view->loop->Value() == B_CONTROL_ON) );
		Pool.StartPlaying(Pool.pointer*Pool.sample_type, false);
		break;

	case TRANSPORT_STOP:
		transport_view->play->SetValue(B_CONTROL_OFF);
		transport_view->play_sel->SetValue(B_CONTROL_OFF);
		transport_view->stop->SetValue(B_CONTROL_ON);
		play_cookie.mem = play_cookie.start_mem;
		Pool.StopPlaying();
		FindView("Sample view")->Pulse();
		FindView("Index view")->Invalidate();
		break;

	case TRANSPORT_LOOP_MAN:
		if (transport_view->loop->Value() == B_CONTROL_ON)
			transport_view->loop->SetValue(B_CONTROL_OFF);
		else
			transport_view->loop->SetValue(B_CONTROL_ON);
		// no break as TRANSPORT_LOOP needs to follow
	case TRANSPORT_LOOP:
		Pool.SetLoop(  (transport_view->loop->Value() == B_CONTROL_ON) );
		break;

	case OPEN:
		if (!Pool.IsChanged())
			((MyApplication*)be_app)->fOpenPanel->Show();
		break;
	
	case SAVE:			// need to add default setting in the save-panel for this
	case SAVE_AS:
		if (Pool.sample_type == NONE)	return;
		Pool.save_selection = false;
		((MyApplication*)be_app)->fSavePanel->Window()->SetTitle(Language.get("PANEL_SAVE"));
		((MyApplication*)be_app)->fSavePanel->Show();
		break;
	
	case SAVE_SELECTION:
		if (Pool.selection == NONE || Pool.sample_type == NONE)	return;
		((MyApplication*)be_app)->fSavePanel->Window()->SetTitle(Language.get("PANEL_SAVE_SELECTION"));
		Pool.save_selection = true;
		((MyApplication*)be_app)->fSavePanel->Show();
		break;
	
	case UNDO:
		Pool.Undo();
		break;
		
	case B_SELECT_ALL:
		if (Pool.size == 0)	break;
		Pool.SelectAll();
		break;

	case UNSELECT_ALL:
		if (Pool.size == 0)	break;
		Pool.DeSelectAll();
		break;

	case B_COPY:
		ClipBoard.Copy();
		break;
	
	case COPY_SILENCE:
		ClipBoard.Cut(false);		// silence cut
		break;
	
	case B_CUT:
		ClipBoard.Cut(true);		// delete cut
		break;
	
	case B_PASTE:
		ClipBoard.Paste();
		break;

	case DROP_PASTE:
	{
		BPoint p;
		message->FindPoint("_drop_point_", &p);
		p = sample_view->ConvertFromScreen(p);
		BRect r = sample_view->Bounds();
		if (r.Contains(p)){
			Pool.selection = NONE;
			Pool.pointer = (int32)(Pool.l_pointer + p.x * (Pool.r_pointer - Pool.l_pointer)/Bounds().Width());
			ClipBoard.Paste();
		}
	}	break;
	
	case PASTE_MIXED:
		ClipBoard.PasteMix();		// ClipBoard handles redrawing and dialogs
		break;

	case B_MOUSE_WHEEL_CHANGED:
		message->FindFloat("be:wheel_delta_y", &y);
		if (y==-1)	PostMessage(ZOOM_IN);
		if (y==1)	PostMessage(ZOOM_OUT);
		break;

	case ZOOM_IN:
		if (Pool.size == 0)	break;
		x = Pool.r_pointer - Pool.l_pointer;
		
		if (x < Pool.m_SampleView->Bounds().Width()/64)	break;
		
		x /= 2;
		if (x < 1)	x = 1;

		Pool.l_pointer = Pool.l_pointer +x/2;				// window to selection
		if (Pool.l_pointer<0)	Pool.l_pointer = 0;
		Pool.r_pointer = Pool.l_pointer + x;
		if (Pool.r_pointer > Pool.size){
			Pool.r_pointer = Pool.size;
			Pool.l_pointer = Pool.r_pointer - x;
			if (Pool.l_pointer<0)
				Pool.l_pointer = 0;
		}
		Pool.update_index = true;
		Pool.UpdateMenu();
		Pool.RedrawWindow();
		break;
	case ZOOM_OUT:
		if (Pool.size == 0)	break;
		x = (Pool.r_pointer - Pool.l_pointer)+1;
		x *= 2;
		if (x > Pool.size)	x = Pool.size;

		Pool.l_pointer = Pool.l_pointer -x/4;				// window to selection
		if (Pool.l_pointer<0)	Pool.l_pointer = 0;
		Pool.r_pointer = Pool.l_pointer + x;
		if (Pool.r_pointer > Pool.size){
			Pool.r_pointer = Pool.size;
			Pool.l_pointer = Pool.r_pointer - x;
			if (Pool.l_pointer<0)
				Pool.l_pointer = 0;
		}
		Pool.UpdateMenu();
		Pool.update_index = true;
		Pool.RedrawWindow();
		break;
	case ZOOM_FULL:
		if (Pool.size == 0)	break;
		Pool.l_pointer = 0;
		Pool.r_pointer = Pool.size;
		Pool.UpdateMenu();
		Pool.update_index = true;
		Pool.RedrawWindow();
		break;
	case ZOOM_SELECTION:
		if (Pool.size == 0)	break;
		if (Pool.selection != NONE){
			Pool.l_pointer = Pool.pointer;
			Pool.r_pointer = Pool.r_sel_pointer;
		}
		Pool.UpdateMenu();
		Pool.update_index = true;
		Pool.RedrawWindow();
		break;
	case ZOOM_LEFT:
		if (Pool.size == 0 || Pool.selection==NONE)	break;
		x = sample_view->Bounds().IntegerWidth()/6;
		Pool.l_pointer = Pool.pointer -x/2;				// window to selection
		if (Pool.l_pointer<0)	Pool.l_pointer = 0;
		Pool.r_pointer = Pool.l_pointer + x;

		Pool.UpdateMenu();
		Pool.update_index = true;
		Pool.RedrawWindow();
		break;
	case ZOOM_RIGHT:
		if (Pool.size == 0 || Pool.selection==NONE)	break;
		x = sample_view->Bounds().IntegerWidth()/6;
		Pool.l_pointer = Pool.r_sel_pointer - x/2;		// window to selection
		if (Pool.l_pointer<0)	Pool.l_pointer = 0;
		Pool.r_pointer = Pool.l_pointer + x;
		if (Pool.r_pointer > Pool.size){
			Pool.r_pointer = Pool.size;
			Pool.l_pointer = Pool.r_pointer - x;
		}

		Pool.UpdateMenu();
		Pool.update_index = true;
		Pool.RedrawWindow();
		break;
		
	case EDIT_L:
		if (Pool.selection != NONE)
			Pool.selection = LEFT;
		Pool.UpdateMenu();
		sample_view->Draw(sample_view->Bounds());
		break;	
	case EDIT_R:
		if (Pool.selection != NONE)
			Pool.selection = RIGHT;
		Pool.UpdateMenu();
		sample_view->Draw(sample_view->Bounds());
		break;	
	case EDIT_B:
		if (Pool.selection != NONE)
			Pool.selection = BOTH;
		Pool.UpdateMenu();
		sample_view->Draw(sample_view->Bounds());
		break;	

	case TRANSPORT_REW:
		x = Pool.r_pointer - Pool.l_pointer;
		Pool.pointer -= x/40;
		if (Pool.pointer <0)	Pool.pointer = 0;
		if (Pool.pointer < Pool.l_pointer){
			Pool.l_pointer -= x/10;
			if (Pool.l_pointer <0)	Pool.l_pointer = 0;
		}
		Pool.r_pointer = Pool.l_pointer + x;
		Pool.update_index = true;
		Pool.RedrawWindow();
		break;

	case TRANSPORT_REW_ALL:
		x = Pool.r_pointer - Pool.l_pointer;
		Pool.pointer = 0;
		Pool.l_pointer = 0;
		Pool.r_pointer = x;
		Pool.update_index = true;
		Pool.RedrawWindow();
		break;

	case TRANSPORT_FWD:
		x = Pool.r_pointer - Pool.l_pointer;
		Pool.pointer += x/40;
		if (Pool.pointer >Pool.size)	Pool.pointer = Pool.size;
		if (Pool.pointer > Pool.r_pointer){
			Pool.r_pointer += x/10;
			if (Pool.r_pointer >Pool.size)	Pool.r_pointer = Pool.size;
		}
		Pool.l_pointer = Pool.r_pointer - x;
		Pool.update_index = true;
		Pool.RedrawWindow();
		break;

	case TRANSPORT_FWD_ALL:
		x = Pool.r_pointer - Pool.l_pointer;
		Pool.pointer = Pool.size;;
		Pool.r_pointer = Pool.pointer;
		Pool.l_pointer = Pool.pointer - x;
		Pool.update_index = true;
		Pool.RedrawWindow();
		break;
	
	case TRANSPORT_HOME:
		Pool.pointer = Pool.l_pointer;
		Pool.update_index = true;
		Pool.RedrawWindow();
		break;
	
	case TRANSPORT_END:
		Pool.pointer = Pool.r_pointer;
		Pool.update_index = true;
		Pool.RedrawWindow();
		break;
	
	case TRANSPORT_LEFT:
		x = Pool.r_pointer - Pool.l_pointer;
		Pool.l_pointer -= x/2;
		if (Pool.l_pointer<0)	Pool.l_pointer = 0;
		Pool.r_pointer = Pool.l_pointer + x;
		Pool.update_index = true;
		Pool.RedrawWindow();
		break;

	case TRANSPORT_RIGHT:
		x = Pool.r_pointer - Pool.l_pointer;
		Pool.l_pointer += x/2;
		if (Pool.l_pointer>(Pool.size-x))	Pool.l_pointer = Pool.size-x;
		Pool.r_pointer = Pool.l_pointer + x;
		Pool.update_index = true;
		Pool.RedrawWindow();
		break;
	
	case UNDO_ENABLE:
		Prefs.save_undo = !Prefs.save_undo;
		Pool.UpdateMenu();
		break;
	
	case ABOUT:
		Pool.DoAbout();
		break;
		
	case HELP:
{
		BPath path;
		app_info ai;
		be_app->GetAppInfo(&ai);
		BEntry entry(&ai.ref);
		entry.GetPath(&path);
		path.GetParent(&path);
		path.Append("Help/help.html");
		char *help = new char[strlen(path.Path())+1];
		sprintf(help, path.Path());
		be_roster->Launch("text/html",1, &help);
		delete help;
}		break;
		
	case NEW:
	{	app_info info;
		be_app->GetAppInfo(&info);
		be_roster->Launch(info.signature);
	}	break;
		
	case PASTE_NEW:
	{	app_info info;
		be_app->GetAppInfo(&info);
		be_roster->Launch(info.signature, new BMessage(B_PASTE));
	}	break;
		
	case HISTORY:
{
		BPath path;
		app_info ai;
		be_app->GetAppInfo(&ai);
		BEntry entry(&ai.ref);
		entry.GetPath(&path);
		path.GetParent(&path);
		path.Append("Help/history.html");
		char *history = new char[strlen(path.Path())+1];
		sprintf(history, path.Path());
		be_roster->Launch("text/html",1, &history);
		delete history;
}		break;
		
	case HOMEPAGE:
		be_roster->Launch("text/html",1, &HOMEPAGE_URL);
		break;
		
	case PREFERENCES:
		if(Pool.PrefWin != NULL){
			Pool.PrefWin->Show();
		}else{
			//Create a new Prefs windos?!
		}
		break;

	case UPDATE:
		FindView("Sample view")->Pulse();
		FindView("Big view")->Pulse();
		FindView("Index view")->Invalidate();
		break;
	
	case REDRAW:
		Pool.sample_view_dirty = true;	// update the sample-view
		Pool.update_index = true;
		Pool.ResetIndexView();
		Pool.RedrawWindow();
		break;
	
	case TOOL_SELECT:
		Pool.tool_mode = SELECT_TOOL;
		Pool.UpdateMenu();
		break;

	case TOOL_DRAW:
		Pool.tool_mode = DRAW_TOOL;
		Pool.UpdateMenu();
		break;

	case TOOL_PLAY:
		Pool.tool_mode = PLAY_TOOL;
		Pool.UpdateMenu();
		break;

	case TOOL_JOGG:
		Pool.tool_mode = SCRUB_TOOL;
		Pool.UpdateMenu();
		break;

	case SPECTRUM:
		(new SpectrumWindow());
		break;
		
	case SAMPLE_SCOPE:
		(new SampleScopeWindow());
		break;
		
	case SET_FREQUENCY:
		(new FreqWindow(BPoint( (Frame().left+Frame().right)/2, (Frame().top+Frame().bottom)/2)));
		break;

	case RESAMPLE_DO:
		DoResample();
		break;

	case RESAMPLE:
		(new ResampleWindow(BPoint( (Frame().left+Frame().right)/2, (Frame().top+Frame().bottom)/2)));
		break;

	case CLEAR:
		if (!Pool.PrepareFilter())	break;
		ClipBoard.DoSilence();
		Pool.changed = true;
		Pool.UpdateMenu();
		Pool.HideProgress();
		Pool.ResetIndexView();
		Pool.RedrawWindow();
		break;
		
	case RUN_FILTER:							// run a filter with or without GUI
{		if (Pool.size == 0) break;
		const char *tag = NULL;
		if (message->FindInt32("filter", &mod) == B_OK){
			RunFilter(mod);
		}
		else if (message->FindString("language_key", &tag) == B_OK){
			RunFilter(tag);
		}
}		break;
	
	case EXE_FILTER:							// apply filter, this is send by the filter or repeat function
		if (message->FindPointer("filter", (void**)&filter) == B_OK)
			ExecuteFilter(filter);
		break;
	
	case CANCEL_FILTER:							// apply filter, this is send by the filter or repeat function
		if (message->FindPointer("filter", (void**)&filter) == B_OK)
			CancelFilter(filter);
		break;
	
	case RUN_LAST_FILTER:							// run a filter with or without GUI
		RunLastFilter();
		break;
	
	case TRIM:
		DoTrim();
		break;
	
	case ZERO_IN:
		ZeroLR();
		ZeroRL();
		Pool.RedrawWindow();
		break;
	case ZERO_OUT:
		ZeroLL();
		ZeroRR();
		Pool.RedrawWindow();
		break;
	case ZERO_LL:
		ZeroLL();
		Pool.RedrawWindow();
		break;
	case ZERO_LR:
		ZeroLR();
		Pool.RedrawWindow();
		break;
	case ZERO_RL:
		ZeroRL();
		Pool.RedrawWindow();
		break;
	case ZERO_RR:
		ZeroRR();
		Pool.RedrawWindow();
		break;
	
	case SET_TIME:
		message->FindInt32("time", &x);
		Prefs.display_time = x;
		Pool.RedrawWindow();
		Pool.PrefWin->PostMessage(CHANGE_LANGUAGE);
		break;

	case B_KEY_DOWN:
//		message->PrintToStream();
		message->FindInt32("key", &raw_key);
		message->FindInt32("modifiers", &mod);
		message->FindInt32("raw_char", &key);

		// now do some conversions for combinations
		if (key == B_FUNCTION_KEY){
			key = 12+raw_key;
		}else if (key>='a' && key<='z')
			key -= ('a'-'A');
		
		msg = KeyBind.GetMessage(key, mod);
		if (msg){
			BMessage new_message(msg);
			new_message.AddString("language_key", KeyBind.GetID(key, mod));
			PostMessage(&new_message);
		}
		break;

	case B_MIME_DATA:			// let the app parse drops
	case B_SIMPLE_DATA:
		be_app->PostMessage(message);
//		message->PrintToStream();
		break;				

	default:
//		message->PrintToStream();
		BWindow::MessageReceived(message);
	}
	
	if (CurrentFocus())
		CurrentFocus()->MakeFocus(false);
}
