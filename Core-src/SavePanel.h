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

#ifndef SAVEPANEL_H
#define SAVEPANEL_H

#include <storage/FilePanel.h>
#include <InterfaceKit.h>
#include <MediaKit.h>

#define PANEL_FORMAT 	 'sfpf'
#define PANEL_CODEC		 'sfps'
#define SAVE_AUDIO		'sveA'

// ------------------- FileFormatMenuItem -------------------

class FileFormatMenuItem : public BMenuItem
{
public:
				FileFormatMenuItem(media_file_format *format);
	virtual		~FileFormatMenuItem();
	
	media_file_format fFileFormat;
};

// ------------------- CodecMenuItem -------------------

class CodecMenuItem : public BMenuItem
{
public:
				CodecMenuItem(media_codec_info *ci, uint32 msg_type);
	virtual		~CodecMenuItem();
	
	media_codec_info fCodecInfo;
};

// ------------------- Save Panel -------------------

class SavePanel : public BFilePanel, public BHandler {
  public:
	SavePanel(BHandler *handler);
	void MessageReceived(BMessage *message);
	~SavePanel();
	void GetSelectedFormatInfo(media_file_format **format, media_codec_info **audio);
	void SetFormatInfo(media_file_format *format, media_codec_info *audio);

  private:
	void BuildFormatMenu();
	void BuildAudioMenu();

	BMenuField	*fFormatMenu;
	BMenuField	*fAudioMenu;
};

#endif
