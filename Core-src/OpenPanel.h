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

#ifndef _SNDFILEPANEL_H_
#define _SNDFILEPANEL_H_

#include <Button.h>
#include <Handler.h>
#include <FileGameSound.h>
#include <FilePanel.h>
#include <MessageRunner.h>
#include <StringView.h>
#include <String.h>
#include <File.h>
#include <NodeInfo.h>

class OpenPanel : public BFilePanel, public BHandler {
public:
	OpenPanel(BHandler* handler);
	virtual ~OpenPanel(void);
	
	virtual void SelectionChanged(void);
	virtual void WasHidden(void);
	
	virtual void MessageReceived(BMessage* msg);

private:
	BFileGameSound* mSndFile;
	BButton* mPlayBtn;
	BStringView *line1, *line2;
	// use a BMessageRunner to periodically remind us
	// to check whether mSndFile has stopped playing so
	// we can reset mPlayBtn's label to "Play".
	BMessageRunner* mBtnUpdater;
};

class OpenFilter : public BRefFilter {
public:
	OpenFilter(void);
	virtual ~OpenFilter(void);
	
	virtual bool Filter(const entry_ref* ref,BNode* node,
		struct stat* st,const char* filetype);
};

#endif // #ifndef _SNDFILEPANEL_H_