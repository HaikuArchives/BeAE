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

#include <Window.h>
#include <stdio.h>
#include "Globals.h"
#include "ToolTipWindow.h"


/*******************************************************
*   Our wonderful BWindow, ya its kewl like that.
*   we dont do much here but set up the menubar and 
*   let the view take over.  We also nead some message
*   redirection and handling
*******************************************************/
ToolTipWindow::ToolTipWindow() : BWindow(BRect(0,0,100,19),"Tool Tip",
      B_NO_BORDER_WINDOW_LOOK,
      B_FLOATING_ALL_WINDOW_FEEL,
      B_ASYNCHRONOUS_CONTROLS|B_NOT_RESIZABLE|B_AVOID_FRONT|B_AVOID_FOCUS){
   
   Looper()->SetName(TOOL_TIP_WINDOW);
   
   BView *v = new BView(Bounds(),"",B_FOLLOW_ALL,0);
   v->SetViewColor(255,255,120);
   AddChild(v);
   
   BRect b = v->Bounds();
   b.Set(b.left-1,b.top-1,b.right+1,b.bottom+1);
   BBox *bb = new BBox(b,"",B_FOLLOW_ALL);
   v->AddChild(bb);
   
   b = bb->Frame();
   b.InsetBy(1,1);
   b.left = 4;
   b.bottom --;
   tipv = new BStringView(b,"","...",B_FOLLOW_ALL);
   tipv->SetAlignment(B_ALIGN_CENTER);
   bb->AddChild(tipv);
   
   Run();
}

/*******************************************************
*   
*******************************************************/
void ToolTipWindow::Tip(BPoint p,const char *tipname){
   if(Lock()){
      tipv->SetText(tipname);
      int32 W = (int32)ceil(tipv->StringWidth(tipname));
      ResizeTo(W+16,be_plain_font->Size() + 8);
      p.x += 16;
      p.y += 16;
      MoveTo(p);
      while(IsHidden()){ Show(); }
      Unlock();
   }
}

/*******************************************************
*   
*******************************************************/
void ToolTipWindow::Bye(){
   Lock();
   while(!IsHidden()){ Hide(); }
   Unlock();
}

/*******************************************************
*   
*******************************************************/
void ToolTipWindow::MessageReceived(BMessage* msg){
  
   switch(msg->what){

   default:
      BWindow::MessageReceived(msg);
   }
}

/*******************************************************
*   
*******************************************************/
bool ToolTipWindow::QuitRequested(){
   return true;
}
