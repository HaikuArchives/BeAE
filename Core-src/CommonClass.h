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

#ifndef _COMMON_H
#define _COMMON_H

#include <List.h>
#include <Looper.h>
#include <OS.h>
#include <Point.h>

class CommonClass{
 public:
	CommonClass();
	~CommonClass();
    
	// Zoom factor methods
//	void SetZoom(int32 precent = 100);
//	int32 GetZoom() const;
   
	// Things to let your plugin give the user 
	// a little feadback to how things are going
	// UpdateProgress takes a precentage from 0-100
//	void SetStatus(const char *format,...) const;
//	void UpdateProgress(int32 precent) const;
   
	// Get a language specific key from the users language file
	// To install a new key append your pluginname_key to the 
	// appropriate language file
	const char* GetLanguageKey(const char *key);
   
	// Add a tool tip to a view
	// a null tip removes its entry
	void AddTip(BView *v, const char *tip = NULL);
	
public:

private:
   CommonClass(const CommonClass&);
   CommonClass &operator=(const CommonClass&);

   sem_id commonSem;
   uint32 _reserved[8];
};

extern CommonClass Common; // Included so you don't have to 
#endif





