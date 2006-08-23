/*******************************************************
*   Refraction©
*   
*   This file may be used under under the license as
*   stated below. For more information about License
*   types see the included LICENSE file. If you did 
*   not recive a copy of that file contact the author.
*   
*   @author  YNOP (ynop@acm.org)
*   @version beta
*   @date    Feb 5 2000
*   @license Refraction Core Code
*******************************************************/
#ifndef _TOOL_TIP_H
#define _TOOL_TIP_H

#include "ToolTipWindow.h"

struct tip_entry{
   BView *v;
   const char *tip;
};

class ToolTip{
public:
   ToolTip();
   ~ToolTip();
   status_t AddTip(BView *v,const char *tip = NULL);
   const char* GetTip(BView*);
private:
   static int32 _ToolTip_Thread_(void *data){
      return ((ToolTip*)(data))->Tool_Thread();
   }
   int32 Tool_Thread();
   BView* FindView(BPoint);
private:
   thread_id ttThread;
   ToolTipWindow *ttWin;
   BList *tips;
};
#endif