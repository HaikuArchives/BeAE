/*******************************************************
*   
*   This file may be used under under the license as
*   stated below. For more information about License
*   types see the included LICENSE file. If you did 
*   not recive a copy of that file contact the author.
*   
*   @author  John Talton
*   @date    Mar 14 2002
*******************************************************/

#include "Globals.h"
#include "ToolTip.h"

#include <stdio.h>

/*******************************************************
*   
*******************************************************/
ToolTip::ToolTip(){
   tips = new BList();
   
   ttWin = new ToolTipWindow();
   
   ttThread = spawn_thread(_ToolTip_Thread_, "ToolTip", B_NORMAL_PRIORITY, (void *)this);
   if(ttThread < 0){
		debugger("Tooltip thread failed");
      // its just tool tips .. forget failing 
   }else{
      resume_thread(ttThread);
   }
}

/*******************************************************
*   
*******************************************************/
ToolTip::~ToolTip(){
   //if(IPref.doToolTips){
   //   snooze(500000);
   //   kill_thread(ttThread);
      ttWin->Lock();
      ttWin->Quit();
   //}
   
   tip_entry *te = NULL;
   
   for(int32 i=tips->CountItems()-1;i > 0;i--){
      te = (tip_entry*)tips->RemoveItem(i);
      if(te && te->tip){
         delete te->tip;
      }
      delete te;
   }
   delete tips;
}

/*******************************************************
*   
*******************************************************/
status_t ToolTip::AddTip(BView *v,const char *tip){
   if(!v){ return B_ERROR; }
   tip_entry *te = NULL;
   if(tip){
      te = new tip_entry;
      te->v = v;
      te->tip = tip;
      tips->AddItem((void*)te);
      
      return B_OK;
   }else{
      for(int32 i = 0;i < tips->CountItems();i++){
         te = (tip_entry*)tips->ItemAt(i);
         if(te && (te->v == v)){
            te = (tip_entry*)tips->RemoveItem(i);
            if(te){
               delete te->tip;
               delete te;
               return B_OK;
            }else{
               return B_ERROR;
            }
         }
      }
   }
   return B_ERROR;
}

/*******************************************************
*   
*******************************************************/
const char* ToolTip::GetTip(BView *v){
   tip_entry *te = NULL;
   for(int32 i = 0;i < tips->CountItems();i++){
      te = (tip_entry*)tips->ItemAt(i);
      if(te && (te->v == v)){
         return te->tip;
      }
   }
   return NULL;
   //return v->Name();
}


/*******************************************************
*   
*******************************************************/
int32 ToolTip::Tool_Thread(){
   BPoint loc;
   BPoint oldloc;
   ulong button;
   int32 count = 0;
   BView *v = NULL;
   
   // Wait until the BApplication becomes valid, in case
   // someone creates this as a global variable.
   while(!be_app_messenger.IsValid()){
      snooze(200000);
   }
   
   // while the app is valid, run. This is a 
   // easy way to let the thread natually die
	while(be_app_messenger.IsValid()){
		if(ttWin->Lock()){
			ttWin->ChildAt(0)->GetMouse(&loc, &button);
			ttWin->ConvertToScreen(&loc);
			if((loc == oldloc) && !button){
				if(count == 5){
					if(ttWin->IsHidden()){
						BPoint tr = loc;
						v = FindView(tr);
						const char *tip = NULL;
						while(v && (tip = GetTip(v)) == NULL){
							v = v->Parent();
						}
						if(tip){
							if(tip && strcmp(tip,"")){
								ttWin->Tip(loc,tip);
							}else{
								//ttWin->Tip(loc,"whats up with the No tip haven tool?");
							}
						}
					}
				}else{
					count++;
				}
			}else{
				count = 0;
				if(!(ttWin->IsHidden())){
					ttWin->Bye();
				}
			}
			oldloc = loc;
			ttWin->Unlock();
			//snooze(250000);
			snooze(150000);
		}else{
			snooze(1000000);
		}
	}
	return B_OK;
}

/*******************************************************
*   
*******************************************************/
BView* ToolTip::FindView(BPoint where){
   BView *winview=NULL;
   BWindow *win = NULL;
   long windex = 0;
   
   while((winview==NULL)&&((win=be_app->WindowAt(windex++))!=NULL)){
      if(win!=ttWin){
         // lock with timeout, in case somebody has a non-running window around
         // in their app.
         if(win->LockWithTimeout(1E6)==B_OK){
            BRect frame=win->Frame();
            if(frame.Contains(where)){
               BPoint winpoint;
               winpoint = where-frame.LeftTop();
               winview = win->FindView(winpoint);
               if(winview){
                   BRegion region;
                   BPoint newpoint=where;
                   winview->ConvertFromScreen(&newpoint);
                   winview->GetClippingRegion(&region);
                   if(!region.Contains(newpoint)){
                      winview=0;
                   }
               }
            }
            win->Unlock();
         }
      }
   }
   return winview;
}




