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

#include "OpenPanel.h"

#include <File.h>
#include <Messenger.h>
#include <NodeInfo.h>
#include <String.h>
#include <Window.h>
#include <MediaFile.h>
#include <MediaDefs.h>
#include <MediaFormats.h>
#include <MediaTrack.h>
#include "Globals.h"

#include <stdio.h>

const uint32 kPlayStop = 'plst'; // message the Play/Stop button sends
const uint32 kUpdateBtn = 'btup'; // should we update the button title?

OpenPanel::OpenPanel(BHandler* handler)
	: BFilePanel(B_OPEN_PANEL,new BMessenger(handler),NULL,
			B_FILE_NODE,false,NULL,NULL,true,true),
		mSndFile(NULL), mPlayBtn(NULL), mBtnUpdater(NULL)
{
	BView* view;

	if (Window()->Lock()) {
		float minw, maxw, minh, maxh;
		Window()->GetSizeLimits(&minw, &maxw, &minh, &maxh);
		minw = 480;
		Window()->SetSizeLimits(minw, maxw, minh, maxh);
		Window()->ResizeTo(MAX(Window()->Frame().Width(), minw), Window()->Frame().Height());

		// add this object to the window's looper's list of handlers
		// (we inherit from BHandler, too, remember?)
		Window()->AddHandler(this);
		
		// get ahold of the Cancel button so we can position
		// and size the Play/Stop button relative to it
		BView* cancel = Window()->FindView("cancel button");

		if (cancel) {
			// add the Play/Stop button
			view = Window()->ChildAt(0); // get the background view
			if (view != NULL) {
				// Frame() gets the coordinates in the parent view
				BRect r(cancel->Frame());
				// move it left 10 pels less than the distance between
				// Cancel button and the left edge of the panel
				r.OffsetBy(-(r.left - view->Bounds().left - 10),0);
				// make sure to B_FOLLOW_BOTTOM so resizing works right!
				mPlayBtn = new BButton(r,"PlayStop","Play",
				new BMessage(kPlayStop),B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
				// it's essential to set the button's target to this,
				// otherwise our MessageReceived() won't get the button's
				// messages
				mPlayBtn->SetTarget(this);
				mPlayBtn->SetEnabled(false);
				view->AddChild(mPlayBtn);
			
				r.left = r.right+8;
				r.right = cancel->Frame().left;
				r.bottom = r.top + 14;
				r.OffsetBy(0,-1);
				line1 = new BStringView(r, NULL, NULL, B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
				r.OffsetBy(0,14);
				line2 = new BStringView(r, NULL, NULL, B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
//				line1->SetAlignment(B_ALIGN_CENTER);
//				line2->SetAlignment(B_ALIGN_CENTER);
				view->AddChild(line1);
				view->AddChild(line2);
			}
		}
		Window()->Unlock();
	}
	
	SetButtonLabel(B_DEFAULT_BUTTON,Language.get("LOAD"));
	SetButtonLabel(B_CANCEL_BUTTON,Language.get("CANCEL"));
	Window()->SetTitle(Language.get("PANEL_OPEN"));
}

OpenPanel::~OpenPanel(void)
{
	delete RefFilter();
	// deleting BFileGameSound stops it too
	delete mSndFile;
	delete mBtnUpdater;
}

void OpenPanel::SelectionChanged(void)
{
	status_t err;
	entry_ref ref;
	
	if (mSndFile) {
		delete mSndFile;
		mSndFile = NULL;
		mPlayBtn->SetEnabled(false);
		line1->SetText("");
		line2->SetText("");
	}
	// Rewind() is essential to make sure GetNextSelectedRef()
	// gets the first in the list of selected refs --
	// even if there's only one selected!
	Rewind();
	err = GetNextSelectedRef(&ref);
	if (err == B_OK) {
		BNode node(&ref);
		if (!node.IsDirectory()) {
			delete mSndFile;
			mSndFile = new BFileGameSound(&ref,false);
			if (mSndFile->InitCheck() == B_OK) {
				mPlayBtn->SetEnabled(true);

// ============
				char s1[128];
				char s2[128];
				char s[128];
				
				BMediaFile *fMediaFile = new BMediaFile(&ref);
				BMediaTrack *track;
				media_format format;
				memset(&format, 0, sizeof(format));
				media_codec_info codecInfo;
				bool audioDone(false);
				bigtime_t audioDuration(0);
				int32 tracks = fMediaFile->CountTracks();
				for (int32 i = 0; i < tracks && (!audioDone); i++) {
					track = fMediaFile->TrackAt(i);
					if (track != NULL) {
						track->EncodedFormat(&format);

					if (format.IsAudio()) {
						memset(&format, 0, sizeof(format));
						format.type = B_MEDIA_RAW_AUDIO;
						track->DecodedFormat(&format);
						media_raw_audio_format *raf = &(format.u.raw_audio);
						char bytesPerSample = (char)(raf->format & 0xf);
						if (bytesPerSample == 1) {
							sprintf(s2, "8 bit ");
						} else if (bytesPerSample == 2) {
							sprintf(s2, "16 bit ");
						} else {
							sprintf(s2,"%d byte", bytesPerSample);
						}
						sprintf(s, "%.3f kHz", (float)(raf->frame_rate / 1000.0f));
						strcat(s2,s);
						if (raf->channel_count == 2) {
							strcat(s2," stereo");
						} else if (raf->channel_count == 1) {
							strcat(s2," mono");
						} else {
							sprintf(s, "%d channel", (int)raf->channel_count);
							strcat(s2, s);
						}
						track->GetCodecInfo(&codecInfo);
						sprintf(s1, codecInfo.pretty_name);			// codec
						audioDuration = track->Duration();
						audioDone = true;
					}
					fMediaFile->ReleaseTrack(track);
				}	
			}
	//			*duration << (int32)(MAX(audioDuration, videoDuration) / 1000000) << " seconds";
				delete fMediaFile;

//===============
				line1->SetText(s1);
				line2->SetText(s2);
				
			}else{
				line1->SetText(Language.get("UNSUPPORTED"));
				line2->SetText("");
			}
		}
	}
}

void OpenPanel::WasHidden(void)
{
	// This will be called any time the user causes
	// the panel to be hidden, but not if some
	// OpenPanel member function calls Hide().
	// BMessageRunner will be restarted the next time
	// Play button is clicked (see MessageReceived(), below.

	// kill the BMessageRunner
	delete mBtnUpdater;
	// set it to NULL as lazy way out of double-delete trouble
	mBtnUpdater = NULL;
}

void OpenPanel::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
	case kPlayStop:
		if (mSndFile != NULL) {
			if (mSndFile->IsPlaying()) {
				mSndFile->StopPlaying();
				mPlayBtn->SetLabel(Language.get("PLAY"));
			}
			else {
				mSndFile->StartPlaying();
				mPlayBtn->SetLabel(Language.get("STOP"));
				
				// if necessary, start the BMessageRunner
				// that will will periodically check to see if
				// the button label needs to be updated
				if (mBtnUpdater == NULL) {
					mBtnUpdater = new BMessageRunner(
						BMessenger(this),
						new BMessage(kUpdateBtn),
						500000/* every .5 sec */);
				}
			}
		}
		break;
	case kUpdateBtn:
		//fprintf(stderr,"got kUpdateBtn\n");
		if (mSndFile != NULL) {
			if (!mSndFile->IsPlaying()) {
				mPlayBtn->SetLabel(Language.get("PLAY"));
			}
		}
		break;
	}
}

OpenFilter::OpenFilter(void)
	: BRefFilter()
{
}

OpenFilter::~OpenFilter(void)
{
}

bool OpenFilter::Filter(const entry_ref* ref,BNode* node,
	struct stat* st,const char* filetype)
{
	bool admitIt = false;
	char type[256];
	const BString mask("au");
	BNodeInfo nodeInfo(node);

	if (node->IsDirectory()) {
		admitIt = true;
	}
	else {
		nodeInfo.GetType(type);
		// allow all files with supertype "audio"
		admitIt = (mask.Compare(type,mask.CountChars()) == 0);
	}

	return (admitIt);
}
