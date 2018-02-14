/*
 SpinSlider - a BSlider with an embedded SpinControl

 Version 1.07

 Written by John Yanarella (jmy@codecatalyst.com)
 Last Updated 11.16.2000 By YNOP (ynop@acm.org)
*/

#ifndef _SPIN_SLIDER_H
#define _SPIN_SLIDER_H

#include <Slider.h>
#include <DecimalSpinner.h>

class SpinSlider : public BControl
{
 public:

	SpinSlider(const char *name, const char *label,
			BMessage *message, int32 minValue, int32 maxValue,
			thumb_style thumbType = B_TRIANGLE_THUMB,
			uint32 flags = B_NAVIGABLE | B_WILL_DRAW | B_FRAME_EVENTS);

	virtual ~SpinSlider();

	virtual void AttachedToWindow();
	virtual void MessageReceived(BMessage* msg);

	virtual void SetValue(int32 value);
	virtual void SetEnabled(bool enabled);

	void SetScale(float scale);
	float Scale() const;
	float ScaledValue() const;


 private:
 
	BSlider* m_slider;
	BSpinner* m_spinner;
};

#endif
