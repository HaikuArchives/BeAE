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

#include <FindDirectory.h>
#include <Path.h>
#include <InterfaceDefs.h>
#include <stdio.h>

#include "Globals.h"
#include "Shortcut.h"
#include "YPreferences.h"
#include "Filters.h"

// our Global def
Shortcut KeyBind;

// this is the list with filters for the basic version
extern filter_info __FilterList[];

/*******************************************************
*   
*******************************************************/
Shortcut::Shortcut(){
   lastkb = NULL;
}

/*******************************************************
*   
*******************************************************/
Shortcut::~Shortcut(){
	// delete everthing in the list :P
	YPreferences prefs(SETTINGS_DIR"/KeyBindings");
	prefs.MakeEmpty();
   
	key_bind *kb = NULL;
   
	for(int32 i = 0; i < kbind.CountItems();i++){
		//kb = (key_bind*)kbind.RemoveItem(i);
		kb = (key_bind*)kbind.ItemAt(i);
		if(kb){
			prefs.AddString("IkbID",kb->ID);
			prefs.AddInt32("IkbKey",(int32)kb->key);
			prefs.AddInt32("IkbMod",kb->mod);
			prefs.AddInt32("IkbKeyAlt",(int32)kb->keyAlt);
			prefs.AddInt32("IkbModAlt",kb->modAlt);
			prefs.AddInt32("IkbMessage",kb->message);
			prefs.AddBool("IkbMenu",kb->menuItem);
			// FREE THE 2 STRINGS HERE !!! lable and ID ??
			delete kb;
		}
	}
}

/*******************************************************
*   
*******************************************************/
void Shortcut::Init(){
	lastkb = NULL;

	// install defaults
	InstallDefaults();

	YPreferences prefs(SETTINGS_DIR"/KeyBindings");
	if(prefs.InitCheck() == B_OK){
		// Init from prefs file
		const char *ID = NULL;
		int32 key, keyAlt;
		int32 mod, modAlt;
		uint32 message;
		bool menuItem;
		int32 i = 0;
		while(prefs.FindString("IkbID",i,&ID) == B_OK){
			if(prefs.FindInt32("IkbKey",i,&key) != B_OK)			key = 0;
			if(prefs.FindInt32("IkbMod",i,&mod) != B_OK)					mod = 0;
			if(prefs.FindInt32("IkbKeyAlt",i,&keyAlt) != B_OK)		keyAlt = 0;
			if(prefs.FindInt32("IkbModAlt",i,&modAlt) != B_OK)				modAlt = 0;
			if(prefs.FindInt32("IkbMessage",i,(int32*)&message) != B_OK)	message = 0;
			if(prefs.FindBool("IkbMenu",i,&menuItem) != B_OK)				menuItem = 0;

			char *I = new char[strlen(ID)+1]; strcpy(I,ID);
			KeyBind.Install(menuItem, I, key, mod, keyAlt, modAlt, message);
			i++;
		}
	}
}

/*******************************************************
*   Get the keys
*******************************************************/
char Shortcut::GetKey(const char *ID){
	key_bind* kb = NULL;
	kb = FindKB(ID);
	if(kb)	return kb->key;

	return 0;
}
   
char Shortcut::GetKeyAlt(const char *ID){
	key_bind* kb = NULL;
	kb = FindKB(ID);
	if(kb)	return kb->keyAlt;

	return 0;
}
   
/*******************************************************
*   Get the ID
*******************************************************/
char *Shortcut::GetID(int32 i) const{
	key_bind* kb = NULL;
	if (i < kbind.CountItems()){
		kb = (key_bind*)kbind.ItemAt(i);
		if(kb){
			char *ID = new char[strlen(kb->ID)+1]; strcpy(ID,kb->ID);
			return ID;
		}
	}

	return NULL;
}

char *Shortcut::GetID(char key, int32 mod) const{
	key_bind* kb = NULL;
	int32 i = 0;
	
	mod = mod & (B_SHIFT_KEY | B_CONTROL_KEY | B_COMMAND_KEY | B_OPTION_KEY);		// mask left / right stuff
	while (i != kbind.CountItems()){
		kb = (key_bind*)kbind.ItemAt(i);
		if(kb){
			if (((kb->key == key) && (kb->mod == mod)) ||
				((kb->keyAlt == key) && (kb->modAlt == mod))){
				char *ID = new char[strlen(kb->ID)+1]; strcpy(ID,kb->ID);
				return ID;
			}
		}
		i++;
	}

	return NULL;
}

/*******************************************************
*   Get the number of Bindings
*******************************************************/
int32 Shortcut::CountBindings() const{
	return kbind.CountItems();
}

/*******************************************************
*   Is a menuItem ?
*******************************************************/
bool Shortcut::IsMenuItem(const char *ID){
	key_bind* kb = NULL;
	kb = FindKB(ID);
	if(kb)	return kb->menuItem;

	return false;
}

/*******************************************************
*   Get the modifiers
*******************************************************/
int32 Shortcut::GetMod(const char *ID){
	key_bind* kb = NULL;
	kb = FindKB(ID);
	if(kb)	return kb->mod;

	return 0;
}

int32 Shortcut::GetModAlt(const char *ID){
	key_bind* kb = NULL;
	kb = FindKB(ID);
	if(kb)	return kb->modAlt;

	return 0;
}

/*******************************************************
*   Get the message
*******************************************************/
uint32 Shortcut::GetMessage(const char *ID){
	key_bind* kb = NULL;
	kb = FindKB(ID);
	if(kb)	return kb->message;

	return 0;
}

uint32 Shortcut::GetMessage(char key, int32 mod){
	key_bind* kb = NULL;
	int32 i = 0;
	
	mod = mod & (B_SHIFT_KEY | B_CONTROL_KEY | B_COMMAND_KEY | B_OPTION_KEY);		// mask left / right stuff
	while (i != kbind.CountItems()){
		kb = (key_bind*)kbind.ItemAt(i);
		if(kb){
//				printf("%s\t  %c  %x  %c  %x  : %c  %x\n", kb->ID, kb->key, kb->mod, kb->keyAlt, kb->modAlt, key, mod);
			if (((kb->key == key) && (kb->mod == mod)) ||
				((kb->keyAlt == key) && (kb->modAlt == mod))){
				return kb->message;
			}
		}
		i++;
	}

	return 0;
}

/*******************************************************
*   
*******************************************************/
void Shortcut::Install(bool menu, const char *ID, char key, int32 mod, char keyAlt, int32 modAlt, uint32 message){
	key_bind* kb = NULL;
	kb = FindKB(ID);
	if(kb == NULL){
		// do install
		key_bind *kb = new key_bind;
		kb->ID = ID;
		kb->key = key;
		kb->mod = mod;
		kb->keyAlt = keyAlt;
		kb->modAlt = modAlt;
		kb->message = message;
		kb->menuItem = menu;
		kbind.AddItem(kb);
	}else{
//		printf("%s\t  %c  %x  %c  %x\n", ID, key, mod, keyAlt, modAlt);
		// replace the item
		kb->key = key;
		kb->mod = mod;
		kb->keyAlt = keyAlt;
		kb->modAlt = modAlt;
		kb->message = message;
		kb->menuItem = menu;
	}
}

/*******************************************************
*   
*******************************************************/
bool Shortcut::IsInstalled(const char *ID){
	if(FindKB(ID))	return true;

	return false;
}

/*******************************************************
*   
*******************************************************/
key_bind* Shortcut::FindKB(const char *ID){
	if(lastkb){
		if(strcmp(lastkb->ID,ID) == 0)	return lastkb;
	}
	key_bind *kb = NULL;
	for(int32 i = 0;i < kbind.CountItems();i++){
		kb = (key_bind*)kbind.ItemAt(i);
		if(kb){
			if(strcmp(kb->ID,ID) == 0){
				// match
				lastkb = kb;
				return kb;
			}
		}
	}
	lastkb = NULL;
	return NULL;
}

/*******************************************************
*	Default Shortcuts
*	F1 - F12	14 - 25
*******************************************************/
void Shortcut::InstallDefaults(){

	// The FileMenu
	Install(0,"FILE_MENU2",	  		  0, 0,								  0, 0,							SPLITTER		);
	Install(1,"FILE_NEW",			'N', B_COMMAND_KEY,					  0, 0,							NEW				);
	Install(1,"FILE_OPEN",			'O', B_COMMAND_KEY,					'O', 0,							OPEN			);
	Install(1,"FILE_INSERT",   	    'I', B_COMMAND_KEY,			   B_INSERT, B_SHIFT_KEY,				INSERT			);
	Install(1,"FILE_APPEND",		  0, 0,								  0, 0,							APPEND			);
	Install(1,"FILE_MIX",			'M', B_COMMAND_KEY,					  0, 0,							OPEN_MIX		);
	Install(1,"FILE_SAVE",			'S', B_COMMAND_KEY,					  0, 0,							SAVE			);
	Install(1,"FILE_SAVE_AS",		'S', B_COMMAND_KEY | B_SHIFT_KEY,	  0, 0,							SAVE_AS			);
	Install(1,"FILE_SAVE_SELECTION",'S', B_COMMAND_KEY | B_CONTROL_KEY,	'S', B_SHIFT_KEY,				SAVE_SELECTION	);
	Install(1,"PREFERENCES",		'P', B_COMMAND_KEY,					'P', B_SHIFT_KEY,				PREFERENCES		);
	Install(1,"FILE_QUIT",			'Q', B_COMMAND_KEY,					  0, 0,							B_QUIT_REQUESTED);

	// edit menu
	Install(0,"EDIT_MENU2",	  		  0, 0,								  0, 0,							SPLITTER		);
	Install(1,"UNDO",				'Z', B_COMMAND_KEY,					  0, 0,							UNDO			);
#ifdef __SAMPLE_STUDIO_LE
	Install(1,"REDO",				'Z', B_COMMAND_KEY | B_SHIFT_KEY,	  0, 0,							REDO			);
#endif
	Install(1,"UNDO_ENABLE",		'Z', B_COMMAND_KEY | B_CONTROL_KEY,	  0, 0,							UNDO_ENABLE		);
	Install(1,"COPY",				'C', B_COMMAND_KEY,					  0, 0,							B_COPY			);
	Install(1,"COPY_SILENCE",		'X', B_COMMAND_KEY | B_SHIFT_KEY,	  0, 0,							COPY_SILENCE	);
	Install(1,"CUT",				'X', B_COMMAND_KEY,					  0, 0,							B_CUT			);
	Install(1,"PASTE",			  	'V', B_COMMAND_KEY,					  0, 0,							B_PASTE			);
	Install(1,"PASTE_NEW",			'V', B_COMMAND_KEY | B_SHIFT_KEY,	  0, 0,							PASTE_NEW		);
#ifdef __SAMPLE_STUDIO_LE
	Install(1,"EDIT_PASTE_MIX",		'V', B_COMMAND_KEY | B_CONTROL_KEY,	  0, 0,							PASTE_MIXED		);
#endif
	Install(1,"CLEAR",				'B', B_COMMAND_KEY,			   B_DELETE, 0,							CLEAR			);
#ifdef __SAMPLE_STUDIO_LE
	Install(1,"COPY_TO_STACK",		'C', B_COMMAND_KEY | B_SHIFT_KEY,	  0, 0,							TO_STACK		);
#endif
	Install(1,"SELECT_ALL",			'A', B_COMMAND_KEY,					  0, 0,							B_SELECT_ALL	);
	Install(1,"UNSELECT_ALL",		'U', B_COMMAND_KEY,					  0, 0,							UNSELECT_ALL	);
	Install(1,"ZERO_IN",			  0, 0,								  0, 0,							ZERO_IN			);
	Install(1,"ZERO_OUT",			  0, 0,								  0, 0,							ZERO_OUT		);
	Install(1,"ZERO_LL",			  0, 0,								  0, 0,							ZERO_LL			);
	Install(1,"ZERO_LR",			  0, 0,								  0, 0,							ZERO_LR			);
	Install(1,"ZERO_RL",			  0, 0,								  0, 0,							ZERO_RL			);
	Install(1,"ZERO_RR",			  0, 0,								  0, 0,							ZERO_RR			);
	Install(1,"TRIM",				  0, 0,								  0, 0,							TRIM			);
	Install(1,"SET_FREQ",	  		  0, 0,								  0, 0,							SET_FREQUENCY	);
	Install(1,"RESAMPLE",	  		  0, 0,								  0, 0,							RESAMPLE		);

	// help
	Install(0,"HELP_MENU2",	  		  0, 0,								  0, 0,							SPLITTER		);
	Install(1,"HELP",	       		'H', B_COMMAND_KEY,					 14, 0,							HELP			);
	Install(1,"ABOUT",			      0, 0,								  0, 0,							ABOUT			);
	
	// The Transporter
	Install(0,"TRANSPORT",	  		  0, 0,					  0, 0,							SPLITTER		);
	Install(0,"TRANSPORT_PLAYS",	' ', B_SHIFT_KEY,		  0, 0,							TRANSPORT_PLAYS		);
	Install(0,"TRANSPORT_PLAY",		' ', B_CONTROL_KEY,		  0, 0,							TRANSPORT_PLAY		);
	Install(0,"TRANSPORT_TOGGLE",	' ', 0,					  0, 0,							TRANSPORT_TOGGLE	);
	Install(0,"TRANSPORT_STOP",		  0, 0,					  0, 0,							TRANSPORT_STOP		);
	Install(0,"TRANSPORT_PAUSE",	'P', 0,					  0, 0,							TRANSPORT_PAUSE_MAN	);
	Install(0,"TRANSPORT_REW",		  0, 0,					  0, 0,							TRANSPORT_REW		);
	Install(0,"TRANSPORT_REW_ALL",B_HOME, B_SHIFT_KEY,		  0, 0,							TRANSPORT_REW_ALL	);
	Install(0,"TRANSPORT_FWD",		  0, 0,					  0, 0,							TRANSPORT_FWD		);
	Install(0,"TRANSPORT_FWD_ALL",B_END, B_SHIFT_KEY,		  0, 0,							TRANSPORT_FWD_ALL	);
	Install(0,"TRANSPORT_REC",		  0, 0,					  0, 0,							TRANSPORT_REC		);
	Install(0,"TRANSPORT_LOOP",		'L', 0,					  0, 0,							TRANSPORT_LOOP_MAN	);
	Install(0,"TRANSPORT_HOME",  B_HOME, 0,				 	  0, 0,							TRANSPORT_HOME		);
	Install(0,"TRANSPORT_END",	  B_END, 0,				  	  0, 0,							TRANSPORT_END		);
	Install(0,"TRANSPORT_LEFT",B_LEFT_ARROW, 0,			 	  0, 0,							TRANSPORT_LEFT		);
	Install(0,"TRANSPORT_RIGHT",B_RIGHT_ARROW, 0,		  	  0, 0,							TRANSPORT_RIGHT		);
	Install(0,"TRANSPORT_SET",  B_ENTER, 0,				  	'S', 0,							TRANSPORT_SET		);

	//zoom
	Install(0,"ZOOM_FUNCTIONS",	 	  0, 0,								  0, 0,							SPLITTER		);
	Install(0,"ZOOM_IN",			  B_UP_ARROW, 0,				0,0,				ZOOM_IN				);
	Install(0,"ZOOM_OUT",		  	  B_DOWN_ARROW, 0,				0,0,				ZOOM_OUT			);
	Install(0,"ZOOM_FULL",		  	  B_UP_ARROW, B_SHIFT_KEY,		0,0,				ZOOM_FULL			);
	Install(0,"ZOOM_SELECTION",	  	  B_DOWN_ARROW, B_SHIFT_KEY,	0,0,				ZOOM_SELECTION		);
	Install(0,"ZOOM_LEFT",		  	  B_LEFT_ARROW, B_SHIFT_KEY,	0,0,				ZOOM_LEFT			);
	Install(0,"ZOOM_RIGHT",		  	  B_RIGHT_ARROW, B_SHIFT_KEY,	0,0,				ZOOM_RIGHT			);

	//channel selection
	Install(0,"CHANNELS",			  0, 0,					  0, 0,							SPLITTER		);
	Install(0,"EDIT_L",				  0, 0,					  0, 0,							EDIT_L				);
	Install(0,"EDIT_R",				  0, 0,					  0, 0,							EDIT_R				);
	Install(0,"EDIT_B",				  0, 0,					  0, 0,							EDIT_B				);
		
	// transform -- when done
	Install(0,"TRANSFORM_MENU2",	  0, 0,								  0, 0,						SPLITTER		);
	Install(1,"REPEAT_ACTION",		'R', B_COMMAND_KEY,				  0, 0,							RUN_LAST_FILTER	);

	// add the filters for the basic version
	int32 filter = 0;
	while(__FilterList[filter].name != NULL)
	{
		Install(1,__FilterList[filter].name,0, 0,						  0, 0,						RUN_FILTER		);
		filter++;
	}

	// analyze menu
	Install(0,"ANALYZE_MENU2",		  0, 0,							  0, 0,							SPLITTER		);
	Install(1,"SPECTRUM_ANALYZER",	  0, 0,							  0, 0,							SPECTRUM		);
	Install(1,"SAMPLE_SCOPE",	 	  0, 0,							  0, 0,							SAMPLE_SCOPE	);

	// The sample-tools
	Install(0,"SAMPLE_TOOLS",		  0, 0,								  0, 0,						SPLITTER		);
	Install(0,"TOOL_SELECT",				  0, 0,					'1', 0,							TOOL_SELECT		);
	Install(0,"TOOL_DRAW",					  0, 0,					'2', 0,							TOOL_DRAW		);
	Install(0,"TOOL_PLAY",					  0, 0,					'3', 0,							TOOL_PLAY		);
	Install(0,"TOOL_JOGG",					  0, 0,					'4', 0,							TOOL_JOGG		);

//	Install(1,"FILE_MIX",			  0, B_COMMAND_KEY,					  0, 0,											);

}
