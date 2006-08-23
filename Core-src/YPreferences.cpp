/*******************************************************
*   YPreferences
*
*   A niffty simple way to save off prefs fo a app
*   useing a BMessage. Its simple and flexable
*
*   @author  YNOP(ynop@acm.org) 
*   @version 1.0.0
*   @date    May 4 2000
*******************************************************/
/*******************************************************
*   Class for saving and loading preference information
*   via BMessages.
*
*   Origanly by Eric Shepherd
*******************************************************/
#include <Alert.h>
#include <File.h>
#include <FindDirectory.h>
#include <Message.h>
#include <Messenger.h>
#include <String.h>

#include <stdio.h>

#include "YPreferences.h"

/*******************************************************
*   Open the settings file and read the data in.
********************************************************/
YPreferences::YPreferences(const char *filename) : BMessage('pref'){
   BFile file;
   
   status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
   if (status != B_OK) {
      //return;
      debugger("cant find_directory for B_USER_SETTINGS_DIRECTORY - thats bad");
   }
   
   // should we automaticly build the directory
   // that we refer to if it does not exist?
   // that way it will be there next to to write prefs
   // into .. this is a good idea
   
   path.Append(filename);
   status = file.SetTo(path.Path(), B_READ_ONLY);
   if(status == B_OK) {
      status = Unflatten(&file);	
   }
}


/*******************************************************
*   And finaly write them to disk
*******************************************************/
YPreferences::~YPreferences() {
   BFile file;
	
	if (file.SetTo(path.Path(), B_WRITE_ONLY | B_CREATE_FILE) == B_OK) {
		Flatten(&file);
	}
}

/*******************************************************
*   
*******************************************************/
status_t YPreferences::SetBool(const char *name, bool b) {
	if (HasBool(name)) {
		return ReplaceBool(name, 0, b);
	}
	return AddBool(name, b);
}
/*******************************************************
*   
*******************************************************/
status_t YPreferences::SetInt8(const char *name, int8 i) {
	if (HasInt8(name)) {
		return ReplaceInt8(name, 0, i);
	}
	return AddInt8(name, i);
}

/*******************************************************
*   
*******************************************************/
status_t YPreferences::SetInt16(const char *name, int16 i) {
	if (HasInt16(name)) {
		return ReplaceInt16(name, 0, i);
	}
	return AddInt16(name, i);
}

/*******************************************************
*   
*******************************************************/
status_t YPreferences::SetInt32(const char *name, int32 i) {
	if (HasInt32(name)) {
		return ReplaceInt32(name, 0, i);
	}
	return AddInt32(name, i);
}

/*******************************************************
*   
*******************************************************/
status_t YPreferences::SetInt64(const char *name, int64 i) {
	if (HasInt64(name)) {
		return ReplaceInt64(name, 0, i);
	}
	return AddInt64(name, i);
}

/*******************************************************
*   
*******************************************************/
status_t YPreferences::SetFloat(const char *name, float f) {
	if (HasFloat(name)) {
		return ReplaceFloat(name, 0, f);
	}
	return AddFloat(name, f);
}

/*******************************************************
*   
*******************************************************/
status_t YPreferences::SetDouble(const char *name, double f) {
	if (HasDouble(name)) {
		return ReplaceDouble(name, 0, f);
	}
	return AddDouble(name, f);
}

/*******************************************************
*   
*******************************************************/
status_t YPreferences::SetString(const char *name, const char *s) {
	if (HasString(name)) {
		return ReplaceString(name, 0, s);
	}
	return AddString(name, s);
}

/*******************************************************
*   
*******************************************************/
status_t YPreferences::SetPoint(const char *name, BPoint p) {
	if (HasPoint(name)) {
		return ReplacePoint(name, 0, p);
	}
	return AddPoint(name, p);
}

/*******************************************************
*   
*******************************************************/
status_t YPreferences::SetRect(const char *name, BRect r) {
	if (HasRect(name)) {
		return ReplaceRect(name, 0, r);
	}
	return AddRect(name, r);
}

/*******************************************************
*   
*******************************************************/
status_t YPreferences::SetMessage(const char *name, const BMessage *message) {
	if (HasMessage(name)) {
		return ReplaceMessage(name, 0, message);
	}
	return AddMessage(name, message);
}

/*******************************************************
*   
*******************************************************/
status_t YPreferences::SetFlat(const char *name, const BFlattenable *obj) {
	if (HasFlat(name, obj)) {
		return ReplaceFlat(name, 0, (BFlattenable *) obj);
	}
	return AddFlat(name, (BFlattenable *) obj);
}

/*******************************************************
*   
*******************************************************/
status_t YPreferences::SetColor(const char *name, rgb_color c){
   // the way this works is we break it down and
   // save off each color
   BString s;
   s.SetTo(name);
   s.Append("R");
   if(HasInt8(s.String())){
      ReplaceInt8(s.String(),0,c.red);
   }else{
      AddInt8(s.String(),c.red);
   }
   s.SetTo(name);
   s.Append("G");
   if(HasInt8(s.String())){
      ReplaceInt8(s.String(),0,c.green);
   }else{
      AddInt8(s.String(),c.green);
   }
   s.SetTo(name);
   s.Append("B");
   if(HasInt8(s.String())){
      ReplaceInt8(s.String(),0,c.blue);
   }else{
      AddInt8(s.String(),c.blue);
   }
   s.SetTo(name);
   s.Append("A");
   if(HasInt8(s.String())){
      ReplaceInt8(s.String(),0,c.alpha);
   }else{
      AddInt8(s.String(),c.alpha);
   }
   return B_OK;
}

/*******************************************************
*   
*******************************************************/
status_t YPreferences::FindColor(const char *name, rgb_color *c){
   BString s;
   s.SetTo(name);
   s.Append("R");
   if(FindInt8(s.String(),(int8*)&(c->red)) != B_OK){ return B_ERROR; }
   s.SetTo(name);
   s.Append("G");
   if(FindInt8(s.String(),(int8*)&(c->green)) != B_OK){ return B_ERROR; }
   s.SetTo(name);
   s.Append("B");
   if(FindInt8(s.String(),(int8*)&(c->blue)) != B_OK){ return B_ERROR; }
   s.SetTo(name);
   s.Append("A");
   if(FindInt8(s.String(),(int8*)&(c->alpha)) != B_OK){ return B_ERROR; }
   return B_OK;
}

















