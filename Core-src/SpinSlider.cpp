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

#include <Window.h>

#include "SpinSlider.h"

#define MSG_SPIN_CHANGED 	'spCh'

SpinSlider::SpinSlider(BRect frame, const char *name, const char *label, 
				       BMessage *message, int32 minValue, int32 maxValue, 
				       thumb_style thumbType, uint32 resizingMode, uint32 flags)
 : BSlider(frame, name, label, message, minValue, maxValue, thumbType, resizingMode, flags)
{
	m_min = minValue; m_max = maxValue;
	m_divider = frame.Width() - (frame.Width() / 3.0);
	m_fillfunc = NULL;
	m_thread_id = -1;
	m_pressed = false;

//	SetSnoozeAmount(1000000);

	m_spinner = new SpinControl(frame, "SpinControl",
								NULL, new BMessage(MSG_SPIN_CHANGED),
								minValue, maxValue, Value(), 1);
	AddChild(m_spinner);
	
   rgb_color yel = { 192, 192, 0 };
   rgb_color blu = { 0, 0, 216 };
   SetBarColor(yel);
   UseFillColor(true,&blu);
	
}

SpinSlider::~SpinSlider() 
{
	if (m_thread_id > 0)
	{
		kill_thread(m_thread_id);
		m_thread_id = -1;
	}
}

void SpinSlider::AttachedToWindow(void)
{
	BSlider::AttachedToWindow();

	SetDivider(m_divider);

	m_spinner->SetEnabled(IsEnabled());
	m_spinner->SetTarget(this);
	
	Invalidate();
}

void SpinSlider::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case MSG_SPIN_CHANGED:
			if (m_spinner) 
				SetValue(m_spinner->Value());
			Invoke();			
		default:
			BSlider::MessageReceived(msg);
			break;
	}
}

void SpinSlider::SetValue(int32 value)
{
	if (m_spinner && (m_spinner->Value() != value))
		m_spinner->SetValue(value);
	BSlider::SetValue(value);
}

void SpinSlider::MouseDown(BPoint pt)
{
	if (IsEnabled())
	{
		SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY); 
	
		BRect frame = BarFrame();
		
		if (pt.x >= frame.left && pt.x <= frame.right)
		{
			SetValue(ValueForPoint(pt));
			Invoke();
			m_pressed = true;
		}
	}
}

void SpinSlider::MouseMoved(BPoint pt, uint32 transit, const BMessage *message)
{
	if (IsEnabled() && m_pressed)
	{
		if (m_thread_id < 0)
		{
			m_thread_id = spawn_thread(SpinSlider::invoker_thread, "...the whites in their eyes...", B_NORMAL_PRIORITY, this);
			if(m_thread_id >= 0)
				resume_thread(m_thread_id);		
		}
		
		BRect frame = BarFrame();
		
		if (pt.x < frame.left)
			pt.x = frame.left;
		else if (pt.x > frame.right)
			pt.x = frame.right;
		
		int32 value = ValueForPoint(pt);
		if (value != Value())
		{
			SetValue(value);
			Invoke();
		}
//		snooze(SnoozeAmount());
	}
}

void SpinSlider::MouseUp(BPoint pt)
{
	if (IsEnabled() && m_pressed)
	{
		BRect frame = BarFrame();

		if (m_thread_id > 0)
		{
			kill_thread(m_thread_id);
			m_thread_id = -1;
		}
		
		if (pt.x >= frame.left && pt.x <= frame.right)
		{
			SetValue(ValueForPoint(pt));
			Invoke();
		}
		
		m_pressed = false;
	}
}

int32 SpinSlider::invoker_thread(void* data)
{
	SpinSlider* slider = (SpinSlider*)data;
	
	int32 last_value = slider->Value();
	
	while (true)
	{
		int32 value = slider->Value();
		if (last_value != value)
		{
			//slider->Looper()->Lock();
			slider->LockLooper();
			slider->Invoke();
			//slider->Looper()->Unlock();
			slider->UnlockLooper();
			last_value = value;
		}
		snooze(slider->SnoozeAmount());
	}
}

void SpinSlider::SetEnabled(bool enabled)
{
	if (m_spinner)
		m_spinner->SetEnabled(enabled);
	BSlider::SetEnabled(enabled);
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

BRect SpinSlider::BarFrame(void) const 
{
	BRect frame = BSlider::BarFrame();
	frame.right = m_divider - 5.0;
	return frame;
}

void SpinSlider::DrawBar(void)
{
	if (!m_fillfunc)
		BSlider::DrawBar();
	else
		m_fillfunc(OffscreenView(), BarFrame());
}

void SpinSlider::SetFillFunc(fill_callback func)
{
	m_fillfunc = func;
}

void SpinSlider::SetDivider(float xCoordinate)
{
	m_divider = xCoordinate;
	if (Window())
	{
		BRect frame = Bounds();
		BRect barframe = BarFrame();
		frame.left = m_divider + 1;
		frame.top = (barframe.top + barframe.bottom) / 2.0 - 8.0;
	
		m_spinner->MoveTo(frame.LeftTop());
		m_spinner->ResizeTo(frame.Width(), frame.Height());
		
		Window()->Lock();
		DrawSlider();	
		Window()->Unlock();	
		Invalidate();
	}
}

float SpinSlider::Divider() const
{
	return m_divider;
}