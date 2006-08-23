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

#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <AppKit.h>
#include <map>
#include <string.h>

struct key_bind{
	const char *ID;		// lookup ID (TAG that can be converted by Language.get()
	char key;			// the key
	int32 mod;			// its modifyers
	char keyAlt;		// the 2nd key
	int32 modAlt;		// its 2nd modifyers
	uint32 message;		// Message to be posted to the window
	bool menuItem;		// is a menu item ??? Has to have B_COMMAND_KEY
};

class Shortcut{
  public:
	Shortcut();
	~Shortcut();
   
	// called by system
	void Init();
   
	// request the ID for a binding
	char *GetID(int32 i) const;
	char *GetID(char key, int32 mod) const;
	
	// number of bindings
	int32 CountBindings() const;

	// Gets the key that linked to this
	// spesifier. If no match then 0 is
	// returned
	char GetKey(const char *ID);
	char GetKeyAlt(const char *ID);
	
	// returns wether it is a menu item
	bool IsMenuItem(const char *ID);
   
	// Gets the mod that is linked to this
	// spesifier. if no match then 0 is
	// returned
	int32 GetMod(const char *ID);
	int32 GetModAlt(const char *ID);
   
	// Gets the message 'what' that is linked to this
	// spesifier. if no match then 0 is
	// returned
	uint32 GetMessage(const char *ID);
	uint32 GetMessage(char key, int32 mod);

	// checks to see if a key is installed
	// although this is not needed as install
	// auto checks this for you.
	bool IsInstalled(const char *ID);
   
	// Installs a new Spesifier with a pritty name
	// if that spesifier alreay exists it does 
	// nothing.
	void Install(bool menu, const char *ID, char key, int32 mod, char keyAlt, int32 modAlt, uint32 message);
	
	void InstallDefaults();
   
  private:
	key_bind* FindKB(const char *ID);

	BList kbind;
	key_bind *lastkb;
};
extern Shortcut KeyBind; // Included so you don't have too
#endif








