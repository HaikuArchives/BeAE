/*
 SpinSlider - a BSlider with an embedded SpinControl

 Version 1.07
 
 Written by John Yanarella (jmy@codecatalyst.com)
 Last Updated 11.16.2000 By YNOP (ynop@acm.org)

 Change log: -------------------------------------------
  Version 1.07 - Update the Defualt colors for the fill and the
                 bar color to the yellow and blue. This is so
                 things look consistant for everyone
 
  Version 1.06 - Write own Mouse handling code to fix bug involving
                 activity outside bar frame incorrectly setting value.
                 The new code respects slider snooze amount and uses
                 a separate thread for invokation.

  Version 1.05 - Fixed a serious bug that crashes an application
                 if the view is removed from a window and then re-added.

  Version 1.04 - Correctly allows/handles decimal input in the text control
                 when in scaled mode, and fixes display bug for input of 
                 values less than min or greater than max

  Version 1.03 - Removed TextControl flicker, fixed
                 SetDivider() (i.e. it actually works now)
 
  Version 1.02 - Added text display value scaling methods,
  				 reworked SetSpinnerWidth() to be like
  				 SetDivider() in BTextControl

  Version 1.01 - Added SetSpinnerWidth() method, 
                 and added invokation on spinner change

  Version 1.0  - Initial release

 -------------------------------------------------------
*/
#include <stdio.h>
#include <LayoutBuilder.h>
#include <Window.h>
#include <StringView.h>


#include "SpinSlider.h"

#define MSG_SPIN_CHANGED 	'spCh'
#define MSG_SLIDER_CHANGED 	'slCh'

SpinSlider::SpinSlider(const char *name, const char *label,
				       BMessage *message, int32 minValue, int32 maxValue, 
				       thumb_style thumbType, uint32 flags)
 : BControl(name, B_EMPTY_STRING, message, flags)
{
	m_slider = new BSlider("SliderControl", B_EMPTY_STRING,
					new BMessage(MSG_SLIDER_CHANGED), minValue, maxValue, B_HORIZONTAL, thumbType);
	m_slider->SetModificationMessage(new BMessage(MSG_SLIDER_CHANGED));
	m_spinner = new BDecimalSpinner("SpinerControl", NULL, new BMessage(MSG_SPIN_CHANGED));
	m_spinner->SetMinValue(minValue);
	m_spinner->SetMaxValue(maxValue);
	m_spinner->SetPrecision(0);
	m_spinner->SetStep(1);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(new BStringView("label", label))
		.AddGroup(B_HORIZONTAL)
			.Add(m_slider, 0.75)
			.Add(m_spinner, 0.25)
		.End();
	
   rgb_color yel = { 192, 192, 0 };
   rgb_color blu = { 0, 0, 216 };
   m_slider->SetBarColor(yel);
   m_slider->UseFillColor(true,&blu);
}



SpinSlider::~SpinSlider() 
{

}

void SpinSlider::AttachedToWindow(void)
{
	BControl::AttachedToWindow();

	m_spinner->SetEnabled(IsEnabled());
	m_spinner->SetTarget(this);
	m_slider->SetEnabled(IsEnabled());
	m_slider->SetTarget(this);

	Invalidate();
}

void SpinSlider::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case MSG_SPIN_CHANGED:
			if (m_spinner) {
				m_slider->SetValue(m_spinner->Value());
				BControl::SetValue(m_spinner->Value());
			}
			Invoke();
			break;
		case MSG_SLIDER_CHANGED:
			if (m_slider) {
				m_spinner->SetValue(m_slider->Value());
				BControl::SetValue(m_slider->Value());
			}
			Invoke();
			break;
		default:
			break;
	}
	BControl::MessageReceived(msg);
}

void SpinSlider::SetValue(int32 value)
{
	if (m_spinner && (m_spinner->Value() != value))
		m_spinner->SetValue(value);
	if (m_slider && (m_slider->Value() != value))
		m_slider->SetValue(value);
	BControl::SetValue(value);
}



void SpinSlider::SetEnabled(bool enabled)
{
	if (m_spinner)
		m_spinner->SetEnabled(enabled);
	if (m_slider)
		m_slider->SetEnabled(enabled);
	BControl::SetEnabled(enabled);
}

void SpinSlider::SetScale(float scale)
{
	m_spinner->SetScale(scale);
}

float SpinSlider::Scale() const
{
	return m_spinner->Scale();
}

float SpinSlider::ScaledValue() const
{
	return Value() / Scale();
}
