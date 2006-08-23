/*******************************************************
*   YPreferences
*
*   A niffty simple way to save off prefs fo a app
*   useing a BMessage. Its simple and flexable
*******************************************************/
/*******************************************************
*   Class for saving and loading preference information
*   via BMessages.
*
*   Origanly by Eric Shepherd
*******************************************************/

#ifndef _YPREFERENCES_H
#define _YPREFERENCES_H

#include <Path.h>
#include <Message.h>

class YPreferences : public BMessage {
	public:
					YPreferences(const char *filename);
					~YPreferences();
	status_t		InitCheck(void);
	
	status_t		SetBool(const char *name, bool b);
	status_t		SetInt8(const char *name, int8 i);
	status_t		SetInt16(const char *name, int16 i);
	status_t		SetInt32(const char *name, int32 i);
	status_t		SetInt64(const char *name, int64 i);
	status_t		SetFloat(const char *name, float f);
	status_t		SetDouble(const char *name, double d);
	status_t		SetString(const char *name, const char *string);
	status_t		SetPoint(const char *name, BPoint p);
	status_t		SetRect(const char *name, BRect r);
	status_t		SetMessage(const char *name, const BMessage *message);
	status_t		SetFlat(const char *name, const BFlattenable *obj);
	status_t        SetColor(const char *name, rgb_color c);
	status_t        FindColor(const char *name, rgb_color *c);
	
	private:
	
	BPath			path;
	status_t		status;
};

inline status_t YPreferences::InitCheck(void) {
	return status;
}

#endif
