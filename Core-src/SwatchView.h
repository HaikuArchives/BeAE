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

#ifndef I_SWATCH_VIEW_H
#define I_SWATCH_VIEW_H

#include <Window.h>
#include <Control.h>
#include <Bitmap.h>

class SwatchView : public BControl
{
 public:
 	SwatchView(BRect frame, const char *name, BMessage* msg = NULL);
	SwatchView(BRect frame, const char *name, rgb_color color, BMessage* msg = NULL);
	virtual ~SwatchView();
	
	virtual	void Draw(BRect update_rect);
	virtual void MouseDown(BPoint point);
	
	virtual void SetEnabled(bool enabled);

	virtual void MessageReceived(BMessage* msg);
	virtual void AttachedToWindow();
	virtual void DetachedFromWindow();

	virtual void SetColor(const rgb_color &color);
	rgb_color Color(void) const;

	void SetActive(bool c);

 protected:
 	virtual void DrawOffscreen();
	
 private:
	virtual void Init(void);
	BBitmap* make_bitmap(void);	
	bool active;

	const char *m_name;
 	BView* m_view;
 	BBitmap* m_bitmap;
	rgb_color m_color;
};

#endif
