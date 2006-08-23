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

// How long until this compiled ver expires
// if they are all zero then never expire
#define EXPIRES_D 17
#define EXPIRES_M 5
#define EXPIRES_Y 2002

#include <Application.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <Message.h>
#include <NodeMonitor.h>
#include <Path.h>
#include <Roster.h>
#include <SoundPlayer.h>
#include <Cursor.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> // header with crypt in it :)

#include "Globals.h"
#include "CommonPool.h"
#include "ProgressWindow.h"
#include "ToolTip.h"
#include "YPreferences.h"
#include "YLanguageClass.h"
#include "AboutBox.h"
#include "MyClipBoard.h"
#include "MainWindow.h"
#include "Shortcut.h"
#include "PeakFile.h"
#include "VMSystem.h"
#include "Filters.h"

// this is the list with filters for the basic version
extern filter_info __FilterList[];

extern void BufferPlayer(void *theCookie, void *buffer, size_t size, const media_raw_audio_format &format);

// our Global def
CommonPool Pool;
extern cookie_record play_cookie;

extern const char *APP_SIGNATURE;

/*******************************************************
*   The Common pool is a group of all plugins/addons/functions
*******************************************************/
CommonPool::CommonPool(){
	expired = Expired();

	if (expired)	printf("expired !\n");

	// To be save set them to nil 
	importmenu = NULL;
	tt = NULL;				// ToolTips
	progress = NULL;
	player = NULL;
	PrefWin = NULL;
	
	m_playing = false;
	play_pointer = 0;
	changed = false;
	save_mode = 0;
	last_pointer = 0;
	sample_view_dirty = true;	// update the sample-view
	update_draw_cache = true;	// update the draw cache
	update_peak = false;
	update_index = true;
	play_cookie.buffer = NULL;

	tool_mode = SELECT_TOOL;

	for (int i=0; i<8; i++)
		BufferHook[i] = NULL;
}

/*******************************************************
*   
*******************************************************/
void CommonPool::Init(){
	if(expired){
		(new BAlert(NULL,"This software has expired. Tool/Floaters will nolonger load. Please register.","Oh"))->Go();
	}
   
//   thread_id tid = spawn_thread(_LoadFilters_, I_FILTER_LOADER, B_NORMAL_PRIORITY, (void *)this);
//	if(tid < 0){
//		debugger(THREAD_FAIL_MSG);
//	}else{
//		resume_thread(tid);
//	}
	play_mode = NONE;
	pointer = 0;
	play_pointer = 0;
	l_pointer = 0;
	r_pointer = 0;
	r_sel_pointer = 0;
	size = 0;
	selection = NONE;
	sample_type = NONE;
	sample_bits = 16;
	frequency = 41400.0;

	play_cookie.buffer = new float[16384];
}

/*******************************************************
*   
*******************************************************/
CommonPool::~CommonPool(){
	if (progress)
		progress->Quit();
	
	if (IsPlaying())
		StopPlaying();

	if (play_cookie.buffer)
		delete [] play_cookie.buffer;

#ifndef __VM_SYSTEM				// RAM
	if (sample_memory)
		free(sample_memory);
#endif

	if (PrefWin)
		PrefWin->Quit();
	
//	if (tt)	delete tt;
}

// refills the PeakFile Cache
void CommonPool::ResetIndexView()
{
	if (sample_type == NONE) return;

	StartProgress(Language.get("INDEXING"), size);

	Peak.Init(size+1, (Pool.sample_type == MONO) );
	Peak.CreatePeaks(0, size+1, size+1);
	Pool.update_index = true;		// update the draw cache

	HideProgress();
}

/*******************************************************
*   SoundPlayer
*******************************************************/
void CommonPool::InitBufferPlayer(float f)
{
	media_raw_audio_format format;
	memset(&format, 0, sizeof(format));
	format.frame_rate = f;									// see if you can set frequency
	format.format = media_raw_audio_format::B_AUDIO_FLOAT;	// 32 bit float
	format.channel_count = 2;								// stereo
	format.buffer_size = Prefs.buffer_size;					// media buffer
	format.byte_order = 2;
	
	play_cookie.pause = false;

	if (player){
		player->Stop();
		delete player;
	}
	player = new BSoundPlayer(&format, "BeAE", BufferPlayer, NULL, &play_cookie);

	format = Pool.player->Format();
	system_frequency = format.frame_rate;
}

#ifndef __VM_SYSTEM				// RAM
	void CommonPool::StartPlaying(int64 p, bool end)
	{
		m_playing = true;
		if (p){
			play_cookie.mem = sample_memory +p*4;
		}else{
			play_cookie.mem = sample_memory;
		}
		play_cookie.mono = (sample_type == MONO);
		play_cookie.start_mem = play_cookie.mem;

		if (end | (selection == NONE))
			play_cookie.end_mem = sample_memory + size*sample_type;
		else
			play_cookie.end_mem = sample_memory + r_sel_pointer*sample_type;

		play_cookie.end = end;
		play_cookie.frequency = frequency;
		play_cookie.add = 0;
		last_pointer = 0;

		player->Start();
		player->SetHasData(true);
	}
#else	// VM
	void CommonPool::StartPlaying(int64 p, bool end)
	{
		m_playing = true;
		if (p){
			play_cookie.mem = p;
		}else{
			play_cookie.mem = 0;
		}
		play_cookie.mono = (sample_type == MONO);
		play_cookie.start_mem = play_cookie.mem;

		if (end | (selection == NONE))
			play_cookie.end_mem = size*sample_type;
		else
			play_cookie.end_mem = r_sel_pointer*sample_type;

		play_cookie.end = end;
		play_cookie.frequency = frequency;
		play_cookie.add = 0;
		last_pointer = 0;

		// set play cache
		VM.SetPlayPointer(play_cookie.mem);

		player->Start();
		player->SetHasData(true);
	}
#endif

void CommonPool::StopPlaying()
{
	m_playing = false;

	#ifdef __VM_SYSTEM
	VM.StopCache();
	#endif

	player->Stop();
}

bool CommonPool::IsPlaying()
{
	return m_playing;
}

bool CommonPool::SetLoop(bool l)
{
	bool ret = play_cookie.loop;
	play_cookie.loop = l;
	return ret;
}

bool CommonPool::PrepareFilter()
{
	if (sample_type == NONE)
		return false;

	player->Stop();
	mainWindow->PostMessage(TRANSPORT_STOP);		// stop playing

	if (selection == NONE)		SelectAll();	// select all if noe is selected
	if (Prefs.save_undo)		SaveUndo();			// save undo data

	return true;
}

int32 CommonPool::SetPlayHook(void (*in)(float*, size_t, void*), int32 index, void *cookie)
{
	for (int i=index; i<PLAY_HOOKS; i++){
		if (BufferHook[i] == NULL){
			BufferHook[i] = in;
			BufferCookie[i] = cookie;
			return i;	// return installed handler
		}
	}
	return -1;		// error
}

void CommonPool::RemovePlayHook(void (*in)(float*, size_t, void *), int32 index)
{
	if (index==-1){
		for (int i=0; i<PLAY_HOOKS; i++){
			if (BufferHook[i] == in){
				BufferHook[i] = NULL;
				BufferCookie[i] = NULL;
				return;
			}
		}
	}else{
		if (BufferHook[index] == in){
			BufferHook[index] = NULL;
			BufferCookie[index] = NULL;
			return;
		}
	}
}

/*******************************************************
*   Select All
*******************************************************/
void CommonPool::SelectAll()
{
	if (sample_type != NONE){
		pointer = 0;
		r_sel_pointer = size;
		selection = BOTH;
		UpdateMenu();
		RedrawWindow();
	}
}

/*******************************************************
*   DeSelect All
*******************************************************/
void CommonPool::DeSelectAll()
{
	if (sample_type != NONE && Pool.selection != NONE){
		selection = NONE;
		r_sel_pointer = 0;
		UpdateMenu();
		RedrawWindow();
	}
}

/*******************************************************
*   Redraw Main Window
*******************************************************/
void CommonPool::RedrawWindow()
{
	mainWindow->Lock();
//	for (int32 i = 0; i<mainWindow->CountChildren();i++)
//		mainWindow->ChildAt(i)->Invalidate();

	mainWindow->FindView("Pointers view")->Invalidate();
	mainWindow->FindView("Index view")->Invalidate();
	mainWindow->FindView("Big view")->Invalidate();
	sample_view_dirty = true;	// update the sample-view
	mainWindow->FindView("Sample view")->Invalidate();
	mainWindow->FindView("TimeBar view")->Invalidate();
	mainWindow->FindView("Values view")->Invalidate();

	mainWindow->Unlock();
}

/*******************************************************
*   Update the menus
*******************************************************/
void CommonPool::UpdateMenu()
{
	BMenuItem *menuItem = NULL;
	mainWindow->Lock();

	while (menu_transform->ItemAt(0)){
		menuItem = menu_transform->ItemAt(0);
		menu_transform->RemoveItem(menuItem);
		delete menuItem;
	}
	
	if (Prefs.repeat_message)
		menu_transform->AddItem(menuItem = new BMenuItem(Language.get(Prefs.repeat_tag.String()), new BMessage(RUN_LAST_FILTER), KeyBind.GetKey("REPEAT_ACTION"), KeyBind.GetMod("REPEAT_ACTION")));

// transform menu

	BMessage *filter_msg;
	int32 filter = 0;
	char name[255];
	while(__FilterList[filter].name != NULL)
	{
		if (strcmp(__FilterList[filter].name, "---") == 0)
		{
			menu_transform->AddSeparatorItem();
		}
		else
		{
			// can do some stuff to organise menu here
		 	filter_msg = new BMessage(RUN_FILTER);
			filter_msg->AddInt32("filter", filter);
			sprintf(name, Language.get(__FilterList[filter].name));
			if ( __FilterList[filter].type & FILTER_GUI )
				strcat(name, "...");
			menu_transform->AddItem(menuItem = new BMenuItem(name, filter_msg, KeyBind.GetKey(__FilterList[filter].name), KeyBind.GetMod(__FilterList[filter].name)));
			menuItem->SetEnabled( __FilterList[filter].type & Pool.sample_type );
		}
		filter++;
	}

	while (menu_analyze->ItemAt(0)){
		menuItem = menu_analyze->ItemAt(0);
		menu_analyze->RemoveItem(menuItem);
		delete menuItem;
	}
	
	menu_analyze->AddItem(menuItem = new BMenuItem(Language.get("SPECTRUM_ANALYZER"), new BMessage(SPECTRUM), KeyBind.GetKey("SPECTRUM_ANALYZER"), KeyBind.GetMod("SPECTRUM_ANALYZER")));
	menu_analyze->AddItem(menuItem = new BMenuItem(Language.get("SAMPLE_SCOPE"), new BMessage(SAMPLE_SCOPE), KeyBind.GetKey("SAMPLE_SCOPE"), KeyBind.GetMod("SAMPLE_SCOPE")));

	menu_transform->SetEnabled(sample_type != NONE);	// transform menu
	menu_analyze->SetEnabled(sample_type != NONE);		// analyzers menu

#ifdef __SAMPLE_STUDIO_LE
	menu_generate->SetEnabled(false);					// generation menu
#endif

	menu_zero->SetEnabled(sample_type != NONE);			// zero cross menu
	mn_trim->SetEnabled(selection != NONE);				// trim
	mn_save_sel->SetEnabled(selection != NONE);			// save selection
	mn_save->SetEnabled(sample_type != NONE && Pool.changed);			// save
	mn_save_as->SetEnabled(sample_type != NONE);		// save as
	mn_set_freq->SetEnabled(sample_type != NONE);		// set frequency
	mn_resample->SetEnabled(sample_type != NONE);		// resample
	mn_select_all->SetEnabled(sample_type != NONE);		// select all
	mn_unselect->SetEnabled(selection != NONE);			// DeSelect all
	mn_cut->SetEnabled(selection != NONE);				// cut
	mn_copy->SetEnabled(selection != NONE);				// copy
	mn_copy_silence->SetEnabled(selection != NONE);		// copy & Silence
	mn_clear->SetEnabled(selection != NONE);			// clear
	mn_undo_enable->SetMarked(Prefs.save_undo);			// undo enabled
	mn_undo->SetEnabled(Hist.HasUndo());				// need history class for this
	mn_paste->SetEnabled(ClipBoard.HasClip());
	mn_paste_new->SetEnabled(ClipBoard.HasClip());
#ifdef __SAMPLE_STUDIO_LE
	mn_paste_mix->SetEnabled(ClipBoard.HasClip());
	mn_redo->SetEnabled(Hist.HasRedo());				// need history class for this
	mn_copy_to_stack->SetEnabled(selection != NONE);	// copy to stack
#endif

	((MainWindow*)mainWindow)->toolBar->Update();
	mainWindow->Unlock();
}

/*******************************************************
*   Check for and handle changed files
*******************************************************/
bool CommonPool::IsChanged(int32 mode)
{
	if (changed){
		int32 k = (new BAlert(NULL,Language.get("SAVE_NEEDED"),Language.get("SAVE"),Language.get("DISCARD"),Language.get("CANCEL")))->Go();
		switch(k){
		case 0:
			save_selection = false;
			save_mode = mode;
			mainWindow->PostMessage(SAVE_AS);
			return true;
			break;
		case 1:
			return false;
			break;
		default:
			return true;
		}
	}else{
		return false;
	}
}

/*******************************************************
*   Handle UNDO
*******************************************************/
void CommonPool::SaveUndo()
{
	if (selection==NONE || !Prefs.save_undo)	return;

	Hist.Save(H_REPLACE, pointer, r_sel_pointer);
	UpdateMenu();
}

void CommonPool::Undo()
{
	Hist.Restore();
	Pool.ResetIndexView();
	UpdateMenu();
	RedrawWindow();
}

/*******************************************************
*   Progress Window
*******************************************************/
void CommonPool::StartProgress(const char *label, int32 max)
{
	if(progress->Lock()){
		progress->StartProgress(label, max);
		progress->Unlock();
	}
}

void CommonPool::ProgressUpdate(int32 delta)
{
	if(progress->Lock()){
		progress->SetProgress(delta);
		progress->Unlock();
	}
}

void CommonPool::SetProgressName(const char *name)
{
	if(progress->Lock()){
		progress->SetTitle(name);
		progress->Unlock();
	}
}

void CommonPool::HideProgress()
{
	if(progress->Lock()){
		if (!progress->IsHidden())
			progress->Hide();
		progress->Unlock();
	}
}

/*******************************************************
*   Dont forget to update the IDF_MIME_VER if any changes
*   are made to the structure or the Mime installation
*******************************************************/
status_t CommonPool::InstallMimeType(bool force){
/*	//Set Mime type for Inferno Document File
	BMimeType mime(DOCUMENT_MIME);
	bool inst = true;
	// if we need to force update then delete the old 
	// mime type out of the database
	if(force){
		mime.Delete();
	}
   
   // if its installed already then we should check if its 
   // the current version.  This is not really needed but 
   // it makes life easyer down the road if we ever want 
   // to change the type up or anything.
   if(mime.IsInstalled()){
      // if its not the current vir we should delete the thingy
      BMessage info;
      inst = false;
      if(mime.GetAttrInfo(&info) == B_NO_ERROR){
         // We tag each Mime setup with a ver number
         // so we can keep it uptoday if it ever changes
         // and we need to reinstall it
         // this is porly writen code as it does not realy
         // handle the cases correctly .. but thats ok for now
         int32 ver = -1;
         if(info.FindInt32("idf_mime_ver",&ver) != B_OK){ ver = -1; }
         if(ver < IDF_MIME_VER){
            inst = true;
         } 
      }
   }
   
   // We should now install it .. becase of a force or a First time run
   // or a wrong Version.
   if(inst){
      // Mime is already set to the IDF signature
      mime.Install();
      
      // install a pritty Icon that goes iwth our file
      BBitmap large_icon(BRect(0, 0, B_LARGE_ICON - 1, B_LARGE_ICON - 1), B_COLOR_8_BIT);
      BBitmap mini_icon(BRect(0, 0, B_MINI_ICON - 1, B_MINI_ICON - 1), B_COLOR_8_BIT);
      large_icon.SetBits(LargeInfernoIcon, large_icon.BitsLength(), 0, B_COLOR_8_BIT);
      mini_icon.SetBits(SmallInfernoIcon, mini_icon.BitsLength(), 0, B_COLOR_8_BIT);
      mime.SetIcon(&large_icon, B_LARGE_ICON);
      mime.SetIcon(&mini_icon, B_MINI_ICON);
      
      // Set up the Name and Description feilds just to be nice
      mime.SetShortDescription(IDF_MIME_NAME);
      mime.SetLongDescription(IDF_MIME_DESC);
      
      // This is VERRY important to do as it the reson we are installing the mime type
      mime.SetPreferredApp(APP_SIGNATURE);
      
      // Build the extentions list.  This is good if we are writeing to 
      // a NON-BFS disk. Mime types will not work there... but file
      // extentions DO!
      BMessage extmsg;
      extmsg.AddString("extensions","IDF");
      extmsg.AddString("extensions","idf");
      extmsg.AddString("extensions","Inferno");
      mime.SetFileExtensions(&extmsg);
      
      // add relevant Inferno fields to meta-mime type      
      // This is wher all the niffty Attributs get added.
      // This is not needed except if we dont then Tracker
      // will not pick up on them and the user will be upset
      BMessage msg;
            
      // first off add our current ver number
      msg.AddInt32("idf_mime_ver",IDF_MIME_VER);
      
      // Name
      msg.AddString("attr:public_name", "Image Name");
      msg.AddString("attr:name", NAME_ATTR);
      msg.AddInt32("attr:type", B_STRING_TYPE);
      msg.AddBool("attr:viewable", true);
      msg.AddBool("attr:editable", true);
      msg.AddInt32("attr:width", 120);
      msg.AddInt32("attr:alignment", B_ALIGN_LEFT);
      msg.AddBool("attr:extra", false);
      
      // Description
      msg.AddString("attr:public_name", "Description");
      msg.AddString("attr:name", DESC_ATTR);
      msg.AddInt32("attr:type", B_STRING_TYPE);
      msg.AddBool("attr:viewable", true);
      msg.AddBool("attr:editable", true);
      msg.AddInt32("attr:width", 120);
      msg.AddInt32("attr:alignment", B_ALIGN_LEFT);
      msg.AddBool("attr:extra", false);
      
      // Width
      msg.AddString("attr:public_name", "Width");
      msg.AddString("attr:name", WIDTH_ATTR);
      msg.AddInt32("attr:type", B_INT32_TYPE);
      msg.AddBool("attr:viewable", true);
      msg.AddBool("attr:editable", false);
      msg.AddInt32("attr:width", 120);
      msg.AddInt32("attr:alignment", B_ALIGN_LEFT);
      msg.AddBool("attr:extra", false);
      
      // Height
      msg.AddString("attr:public_name", "Height");
      msg.AddString("attr:name", HEIGHT_ATTR);
      msg.AddInt32("attr:type", B_INT32_TYPE);
      msg.AddBool("attr:viewable", true);
      msg.AddBool("attr:editable", false);
      msg.AddInt32("attr:width", 120);
      msg.AddInt32("attr:alignment", B_ALIGN_LEFT);
      msg.AddBool("attr:extra", false);
      
      // Now actaully add all of that stuff to the mime type .. 
      // and return ...
      return mime.SetAttrInfo(&msg);
   }
*/
	return B_OK;
}

/*******************************************************
*   
*******************************************************/
void CommonPool::DoAbout(){
	BPoint p;
	BRect r = mainWindow->Frame();
	
	p.x = (r.left+r.right)/2;
	p.y = (r.top+r.bottom)/2;
	
	(new AboutBox(p));
}

/*******************************************************
*   
*******************************************************/
void CommonPool::AddTip(BView *v,const char *tip){
	if(tt){
		tt->AddTip(v,tip);
	}
}

/*******************************************************
*
*******************************************************/
/*
void CommonPool::LoadFilters()
	app_info ai;
	be_app->GetAppInfo(&ai);
	BEntry entry(&ai.ref);
	BPath path;
	entry.GetPath(&path);
	path.GetParent(&path);
	path.Append(FILTERS_DIR);
	BDirectory directory(path.Path());

	while (directory.GetNextEntry(&entry, true) == B_OK) {
		AddFilter(entry);
	}
	if(ss){ ss->EnableIcon(4,true) ;}
}
*/

/*******************************************************
*   
*******************************************************/
/*
status_t CommonPool::AddFilter(BEntry entry){
	if(expired){ return B_ERROR; }

	BPath path;
	char name[B_FILE_NAME_LENGTH];
   
	entry.GetPath(&path);
	entry.GetName(name);
   
   image_id image = load_add_on(path.Path());
   if(image == B_ERROR){
      return B_ERROR;
   }
      
   int32 *ver = NULL;
   if(get_image_symbol(image, "FILTER_API_VERSION", B_SYMBOL_TYPE_ANY,(void **)&ver) != B_OK){
      BString tmp;
      tmp.SetTo("CommonPool: ");
      tmp.Append(path.Leaf());
      tmp.Append(" - faild to find FILTER_API_VERSION");
      ICommon.Log(tmp.String());
      unload_add_on(image);
      return B_ERROR;
   }
   if(*ver != FILTER_API_VER){
      BString tmp;
      tmp.SetTo("CommonPool: Filter ");
      tmp.Append(path.Leaf());
      tmp.Append(" has Bad API ver");
      ICommon.Log(tmp.String());
      unload_add_on(image);
      return B_ERROR;
   }
   
   char *author = NULL;
   if(get_image_symbol(image, "FILTER_AUTHOR", B_SYMBOL_TYPE_DATA,(void **)&author) != B_OK){
      // Damn error but we handle it :)
   }
   char *email = NULL;
   if(get_image_symbol(image, "FILTER_EMAIL", B_SYMBOL_TYPE_DATA,(void **)&email) != B_OK){
      // Damn error but we handle it :)
   }
   char *copy = NULL;
   if(get_image_symbol(image, "FILTER_COPYRIGHT", B_SYMBOL_TYPE_DATA,(void **)&copy) != B_OK){
      // Damn error but we handle it :)
   }
   char *version = NULL;
   if(get_image_symbol(image, "FILTER_VERSION", B_SYMBOL_TYPE_DATA,(void **)&version) != B_OK){
      // Damn error but we handle it :)
   }
   char *cat = NULL;
   if(get_image_symbol(image, "FILTER_CATAGORY", B_SYMBOL_TYPE_DATA,(void **)&cat) != B_OK){
      // Damn error but we handle it :)
   }
   char *desc = NULL;
   if(get_image_symbol(image, "FILTER_DESCRIPTION", B_SYMBOL_TYPE_DATA,(void **)&desc) != B_OK){
      // Damn error but we handle it :)
   }
   
   // strip the filter name as it has %hex values in it.
   StripHex(name);
   
   filter_entry *fe = new filter_entry;
   fe->name = new char[strlen(name)+1]; strcpy(fe->name,name);
   StripHex(fe->name); // this shouldn't be needed !
   
   if(author && *author){
      fe->author = new char[strlen(author)+1]; strcpy(fe->author,author);
   }else{
      fe->author = NULL;
   }
   if(email && *email){
      fe->email = new char[strlen(email)+1]; strcpy(fe->email,email);
   }else{
      fe->email = NULL;
   }
   if(copy && *copy){
      fe->copyright = new char[strlen(copy)+1]; strcpy(fe->copyright,copy);
   }else{
      fe->copyright = NULL;
   }
   if(version && *version){
      fe->version = new char[strlen(version)+1]; strcpy(fe->version,version);
   }else{
      fe->version = NULL;
   }
   if(cat && *cat){
      fe->catagory = new char[strlen(cat)+1]; strcpy(fe->catagory,cat);
   }else{
      BString tmp;
      tmp.SetTo("CommonPool: Warning - Filter \"");
      tmp.Append(name);
      tmp.Append("\" does not have a Catagory set, Adding to General");
      ICommon.Log(tmp.String());
      fe->catagory = new char[strlen(Language.get("GENERAL_FILTER_MENU"))+1]; strcpy(fe->catagory,Language.get("GENERAL_FILTER_MENU"));
   }
   if(desc && *desc){
      fe->desc = new char[strlen(desc)+1]; strcpy(fe->desc,desc);
   }else{
      fe->desc = NULL;
   }
   entry.GetRef(&(fe->eref));
   
   
   unload_add_on(image);
   
   Filters.AddItem(fe);
   PWin->AddFilterConfig(fe);
  
   return B_OK;
}
*/

/*******************************************************
*   This is not a build menu becasue the Bar is loaded
*   befor the Converters .. thus we must add entryes to
*   a existing menu. Should be names SetImpotMenu;
*******************************************************/
/*
void CommonPool::BuildImportMenu(BMenu *menu){
//	importmenu = menu;
}
*/

/*******************************************************
*   
*******************************************************/
/*
void CommonPool::BuildExportMenu(BMenu*){
}
*/

/*******************************************************
*   
*******************************************************/
/*
void CommonPool::StartImporter(entry_ref importer,entry_ref file_ref){
   InfernoWindow *proj = NULL;
   project_entry *pje = NULL;
   BPath path;
   BEntry entry(&importer,true);
   IConvFunc instantiate_conv;
   
   entry.GetPath(&path);
   image_id image = load_add_on(path.Path());
   if(image == B_ERROR){
      return;
   }
   
   if(get_image_symbol(image, "instantiate_converter", B_SYMBOL_TYPE_TEXT,(void **)&instantiate_conv) != B_OK){
      BString tmp;
      tmp.SetTo("CommonPool: ");
      tmp.Append(path.Leaf());
      tmp.Append(" - can't Instantiate Converter (Import)");
      ICommon.Log(tmp.String());
      unload_add_on(image);
      return;
   }

   BMessage msg;
   InfernoConverter *converter = (*instantiate_conv)(&msg,&ICommon);
   if(converter == NULL){ return; }
   
   InfernoDocument *IDoc = NULL;
   BFile infile(&file_ref,B_READ_ONLY);
   
   if(converter->Import(&infile,&IDoc) == B_OK){
      if(IDoc == NULL){
         return;
      }
      
      // Should we set the name or leave it to the 
      // converter? It does not know the file name
      entry.SetTo(&file_ref,true);
      entry.GetPath(&path);
      IDoc->ImageName.SetTo(path.Leaf());
      
      proj = new InfernoWindow(BRect(100,100,500,500),IDoc->ImageName.String(),IDoc);
      proj->Show();
      pje = new project_entry;
      pje->doc = proj;
      pje->info = IDoc;
      pje->refcount = 0;
      IPool.AddProject(pje);
   }else{
      // Could not import !!
   }
}
*/

/*******************************************************
*   
*******************************************************/
/*
filter_entry* CommonPool::FindFilter(entry_ref ref){
   filter_entry *fe = NULL;
   for(int32 i = 0;i < Filters.CountItems();i++){
      fe = (filter_entry*)Filters.ItemAt(i);
      if(fe && (fe->eref == ref)){
         return fe;
      }
   }

   return NULL;
}
*/

/*******************************************************
*   
*******************************************************/
/*
void CommonPool::StartFilter(entry_ref ref){
   BPath path;
   BEntry entry(&ref,true);
   IFilterFunc instantiate_filter;
   
   entry.GetPath(&path);
   image_id image = load_add_on(path.Path());
   if(image == B_ERROR){
      return;
   }
   
   if(get_image_symbol(image, "instantiate_filter", B_SYMBOL_TYPE_TEXT,(void **)&instantiate_filter) != B_OK){
      BString tmp;
      tmp.SetTo("CommonPool: ");
      tmp.Append(path.Leaf());
      tmp.Append(" - can't Instantiate Filter");
      ICommon.Log(tmp.String());
      unload_add_on(image);
      return;
   }
   
   project_entry *pje = NULL;
   pje = GetProject();
   if(pje == NULL){
      return;
   }
   
   InfernoDocument *IDoc = pje->info;
   
   if(IDoc){
      InfernoFilter *filter = NULL;
      filter_entry *fe = FindFilter(ref);
      if(fe == NULL){ 
         // WHAT THE?
         // Can't load prefs file!
         BString tmp;
         tmp.SetTo("CommonPool: ");
         tmp.Append("Can't fine filter_entry, Loading tool without Prefs file");
         ICommon.Log(tmp.String());
         BMessage msg;
         msg.AddPointer("InfernoDocument",(void*)IDoc);
         filter = (*instantiate_filter)(&msg,&ICommon);//,IDoc);
      }else{
         char prefsname[B_FILE_NAME_LENGTH];
         sprintf(prefsname,ISETTINGS_DIR"/%s_prefs",fe->name);
         YPreferences prefs(prefsname);
         prefs.AddPointer("InfernoDocument",(void*)IDoc);
         filter = (*instantiate_filter)(&prefs,&ICommon);//,IDoc);
      }
      
      
      BView *v = filter->GetSettingsView();
      if(v){
         //Run as a active windowed filter
         // Modal so it blocks
         if(fiterwin){
            fiterwin->Lock();
            fiterwin->Quit();
            fiterwin = NULL;
         }
         BEntry ent(&ref,true);
         char name[B_FILE_NAME_LENGTH];
         ent.GetName(name);
         StripHex(name);
         fiterwin = new FilterConfigWindow(filter,image,v,name);
      }else{
         // Run Once filter
         RunFilter(pje,filter);
         
         if(fe == NULL){
         }else{
            YPreferences *Fprefs = NULL;
            BString s(fe->name);
            s.Prepend(ISETTINGS_DIR"/");
            s.Append("_prefs");
            Fprefs = new YPreferences(s.String());
            Fprefs->MakeEmpty();
            filter->SaveState(Fprefs);
            delete Fprefs;
         }
         delete filter;
         unload_add_on(image); // Trash the memory now!! this is a must
      }
   }
}
*/

/*******************************************************
*   
*******************************************************/
/*
status_t CommonPool::RunFilter(project_entry *pje, InfernoFilter *filter){
   if(!filter) return B_ERROR;
   
   if(pje && pje->info){
      InfernoLayer *lay = pje->info->LayerAt();
      
      //IHist.SaveLayer(lay);
      filter_mode fm;
      
      if(lay){
         // build fm
         filter_action a = (filter_action)IPref.defaultfilterAction;
         if(fiterwin){
            a = fiterwin->Action();
         }
         
         if((a == ACTION_SELECT) || (a == ACTION_SELECT_NEW)){
            fm.useSelection = true;
            fm.BoundingBox = SelectionBounds(pje->info->Selection);//lay->Bounds();
         }else{
            fm.useSelection = false;
            fm.BoundingBox = lay->Bounds();
         }
         
         
         
         fm.selection = pje->info->Selection;
         
         status_t s = filter->Filter(lay,fm,I_PROOF_HIGH_QUALITY);
         
         // depending on the mode we shoudl copy the layer
         // to a new docuemtn or to a new layer.
         
         // we should also check s == B_OK to see if we
         // had some sorta error
         
         if((a == ACTION_SELECT_NEW) || (a == ACTION_WHOLE_NEW)){
            // Create a new layer and Init it with lay
            // then use the tmp layer to reinsert the old
            // layer back into the doc
            (new BAlert(NULL,"cant creat new layer - yet","OK"))->Go();
         }else{
            // We dont have to do anything here except clean
            // up the tmp layers and stuff we used
         }
         
         
         return s;
      }
   }
   
   return B_ERROR;
}
*/

/*******************************************************
*   
*******************************************************/
/*
void CommonPool::BuildFilterMenu(BMenu *fmenu){
   BMessage *msg = NULL;
   filter_entry *fe = NULL;
   BMenuItem *item = NULL;
   BMenu *nmenu = NULL;
   
   // this is a place holder for the 'last fitler run'
   // menu item. We should do something kewl here.
   fmenu->AddItem(item = new BMenuItem("none",NULL));
   item->SetEnabled(false);
   fmenu->AddSeparatorItem();
   
   // Image menu and Color menu are defualt
   fmenu->AddItem(nmenu = new BMenu(COLOR_CATAGORY));
   fmenu->AddItem(nmenu = new BMenu(IMAGE_CATAGORY));
   fmenu->AddSeparatorItem();
   
   // you know what we should do right now .. 
   // we should alphabatize the filter list
   // so that everthing is easy to find.
   Filters.SortItems(FilterSorter);
   
   for(int32 i = 0; i < Filters.CountItems();i++){
      fe = (filter_entry*)Filters.ItemAt(i);
      if(fe){
         item = NULL;
         msg = new BMessage(DO_FILTER);
         msg->AddRef("Ifilter_ref",&(fe->eref));
         if(fe->catagory == NULL){
            // catagory was null
            // we should actaully add to general filter cat
            continue;
         }
         item = (fmenu->FindItem(fe->catagory));
         if(item){
            nmenu = item->Submenu();
            if(nmenu){
               //nmenu->AddItem(item = new FilterMenuItem(fe->name,"",msg));
               nmenu->AddItem(item = new BMenuItem(fe->name,msg));
               item->SetTarget(be_app);
            }else{
               // we found the cat but it didn't have a sub menu.
               // what should we do
            }
         }else{
            fmenu->AddItem(nmenu = new BMenu(fe->catagory));
            //nmenu->AddItem(item = new FilterMenuItem(fe->name,"",msg));
            nmenu->AddItem(item = new BMenuItem(fe->name,msg));
            item->SetTarget(be_app);
         }
      }
   }
}
*/

/*******************************************************
*   
*******************************************************/
bool CommonPool::Expired(){
   if(EXPIRES_M == EXPIRES_Y == EXPIRES_D == 0){ return false; }
      
   char Sm[4],Sd[3],Sy[5];
   sscanf(__DATE__,"%s %s %s",Sm,Sd,Sy);
   int32 d = atoi(Sd);
   int32 y = atoi(Sy);
   int32 m = -1;
   if(strcmp(Sm,"Jan") == 0){
      m = 0;
   }else if(strcmp(Sm,"Feb") == 0){
      m = 1;
   }else if(strcmp(Sm,"Mar") == 0){
      m = 2;
   }else if(strcmp(Sm,"Apr") == 0){
      m = 3;
   }else if(strcmp(Sm,"May") == 0){
      m = 4;
   }else if(strcmp(Sm,"Jun") == 0){
      m = 5;
   }else if(strcmp(Sm,"Jul") == 0){
      m = 6;
   }else if(strcmp(Sm,"Aug") == 0){
      m = 7;
   }else if(strcmp(Sm,"Sep") == 0){
      m = 8;
   }else if(strcmp(Sm,"Oct") == 0){
      m = 9;
   }else if(strcmp(Sm,"Nov") == 0){
      m = 10;
   }else if(strcmp(Sm,"Dec") == 0){
      m = 11;
   }
   
   m = (m + EXPIRES_M) % 12;
   y = (y + EXPIRES_Y);
   d = (d + EXPIRES_D) % 30; // this could be smarter :p

   time_t timer = time(NULL);
   struct tm *now = localtime(&timer);

   if(y <= (now->tm_year+1900)){ return true; }
   if(m <= now->tm_mon){ return true; }
   if(d  <= now->tm_mday){ return true; }
   
   return false;
}
