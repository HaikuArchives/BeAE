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

#include <Alert.h>
#include <Message.h>
#include <Messenger.h>
#include <Application.h>
#include <stdio.h>

#include "Globals.h"
#include "CommonClass.h"

CommonClass Common;

/*******************************************************
*   
*******************************************************/
CommonClass::CommonClass(){
	if((commonSem = create_sem(1, "Common Sem")) < 0){
		debugger("Create Common Sem failed");
	}
}

/*******************************************************
*   
*******************************************************/
CommonClass::~CommonClass(){
	delete_sem(commonSem);
}

/*******************************************************
*   
*******************************************************/
void CommonClass::AddTip(BView *v,const char *tip){
	Pool.AddTip(v,tip);
}

/*******************************************************
*   
*******************************************************/
/*
void CommonClass::SetStatus(const char *s){
   acquire_sem(commonSem);
   if(s == NULL){return;}
   project_entry *pje = NULL;
   int32 pid = IPool.GetCurrentProj();
   pje = IPool.GetProject(pid);
   if(pje == NULL){ return; }
   if(pje->doc == NULL){ return; }
   BMessage *status = new BMessage(STATUS);
   status->AddString("Istatus",s);
   (pje->doc)->PostMessage(status);
   release_sem(commonSem);
}
*/

/*******************************************************
*   
*******************************************************/
/*
void CommonClass::UpdateProgress(int32 precentcomplete){
   acquire_sem(commonSem);
   project_entry *pje = NULL;
   int32 pid = IPool.GetCurrentProj();
   pje = IPool.GetProject(pid);
   if(pje == NULL){ return; }
   if(pje->doc == NULL){ return; }
   BMessage *status = new BMessage(PROGRESS);
   status->AddInt32("Iprogress",precentcomplete);
   (pje->doc)->PostMessage(status);   
   release_sem(commonSem);
}
*/

/*******************************************************
*   
*******************************************************/
const char* CommonClass::GetLanguageKey(const char *key){
   return Language.get(key);
}

/*******************************************************
*   This should set the zoom for the current document
*   and not global alowing much fun!
*******************************************************/
/*
void CommonClass::SetZoom(double z){
   acquire_sem(commonSem);
   project_entry *pje = IPool.GetProject(IPool.GetCurrentProj());
   if(pje == NULL){
      //Cant set zoom of a null proj
      return;
   }
   if(z <= 1){
      // thats a little too small for me 
      return;
   }
   
   pje->doc->SetZoom(z);
   SendMessage(BROADCAST,new BMessage(I_COMMON_CHANGED));
   release_sem(commonSem);
}
*/

/*******************************************************
*   
*******************************************************/
/*
double CommonClass::GetZoom(){
   project_entry *pje = IPool.GetProject(IPool.GetCurrentProj());
   if(pje == NULL){
      //Cant get zoom of a null proj
      return 100;
   }
   if(pje->doc == NULL){
      return 100;
   }
   
   return pje->doc->GetZoom();
}
*/
