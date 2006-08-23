/*
 SpinSlider - a BSlider with an embedded SpinControl

 Version 1.07

 Written by John Yanarella (jmy@codecatalyst.com)
 Last Updated 11.16.2000 By YNOP (ynop@acm.org)
*/

#ifndef _SPIN_SLIDER_H
#define _SPIN_SLIDER_H

#include <Slider.h>
#include "SpinControl.h"

typedef void (*fill_callback) (BView* view, BRect frame);

class SpinSlider : public BSlider
{
 public:
	SpinSlider(BRect frame, const char *name, const char *label, 
			BMessage *message, int32 minValue, int32 maxValue, 
			thumb_style thumbType = B_TRIANGLE_THUMB, 
			uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP, 
			uint32 flags = B_NAVIGABLE | B_WILL_DRAW | B_FRAME_EVENTS);

	virtual ~SpinSlider();

	virtual void MouseDown(BPoint pt);
	virtual void MouseMoved(BPoint pt, uint32 transit, const BMessage* msg);
	virtual void MouseUp(BPoint pt);
			
	virtual void AttachedToWindow();
	virtual void MessageReceived(BMessage* msg);

	virtual void SetValue(int32 value);
	virtual void SetEnabled(bool enabled);

	virtual BRect BarFrame(void) const;
	virtual void DrawBar(void);

	virtual void SetFillFunc(fill_callback);

	void SetScale(float scale);
	float Scale() const;
	float ScaledValue() const;

	virtual void SetDivider(float xCoordinate);
	virtual float Divider() const;

 private:
 	static int32 invoker_thread(void* data);
	thread_id m_thread_id;
 
 	SpinControl* m_spinner;
	fill_callback m_fillfunc;

	float m_min, m_max;
	float m_divider;
	bool m_pressed;
};

#endif
