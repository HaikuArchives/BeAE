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

#ifndef MY_STRING_ITEM_H
#define MY_STRING_ITEM_H

#include <ListItem.h>
#include <View.h>
#include "MyStringItem.h"

StringItem::StringItem(const char *label, int32 level, bool expanded)
	: BListItem(level, expanded)
{
	m_label = new char[strlen(label)+1];
	strcpy(m_label, label);
}

StringItem::~StringItem()
{
	if (m_label)
		delete[] m_label;
}

const char *StringItem::Label() const
{
	return m_label;
}

void StringItem::DrawItem(BView *view, BRect rect, bool all)
{
	BFont font;
	view->GetFont(&font);

	if (IsSelected())
		view->SetLowColor(150,190,230);
	else
		view->SetLowColor(255,255,255);

	view->FillRect(rect, B_SOLID_LOW);
	view->SetHighColor(0,0,0);
	view->DrawString( m_label, BPoint( rect.left +5, rect.top +font.Size() ));

}

#endif
