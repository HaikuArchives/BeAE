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
#include <View.h>
#include <InterfaceKit.h>
#include <stdlib.h>
#include <stdio.h>

#include "Globals.h"
#include "RealtimeFilter.h"
#include "CompressorFilter.h"

/*******************************************************
*   
*******************************************************/
CompressorFilter::CompressorFilter(bool b) : RealtimeFilter(Language.get("COMPRESSOR"), b)
{

}

/*******************************************************
*   
*******************************************************/
BView *CompressorFilter::ConfigView()
{
	BRect r(0,0,200,240);

	BView *view = new BView(r, NULL, B_FOLLOW_ALL, B_WILL_DRAW);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	r.InsetBy(8,8);
	r.bottom = r.top + 23;
	rms = new BCheckBox(r, NULL, Language.get("RMS"), new BMessage(CONTROL_CHANGED));
	rms->SetValue(Prefs.filter_compressor_rms);
	view->AddChild(rms);

	r.OffsetBy(0,20);
	attac = new SpinSlider(r, NULL, Language.get("ATTAC"), new BMessage(CONTROL_CHANGED), 0, 1000);
	attac->SetValue(Prefs.filter_compressor_attac * 1000);
	view->AddChild(attac);

	r.OffsetBy(0,40);
	r.bottom = r.top + 23;
	decay = new SpinSlider(r, NULL, Language.get("DECAY"), new BMessage(CONTROL_CHANGED), 0, 1000);
	decay->SetValue(Prefs.filter_compressor_decay * 1000);
	view->AddChild(decay);

	r.OffsetBy(0,40);
	treshold = new SpinSlider(r, NULL, Language.get("CMP_TRESHOLD"), new BMessage(CONTROL_CHANGED), -60, 18);
	treshold->SetValue(Prefs.filter_compressor_treshold);
	view->AddChild(treshold);

	r.OffsetBy(0,40);
	ratio = new SpinSlider(r, NULL, Language.get("CMP_RATIO"), new BMessage(CONTROL_CHANGED), 1, 100);
	ratio->SetValue(Prefs.filter_compressor_ratio);
	view->AddChild(ratio);

	r.OffsetBy(0,40);
	gain = new SpinSlider(r, NULL, Language.get("GAIN_DB"), new BMessage(CONTROL_CHANGED), 0, 24);
	gain->SetValue(Prefs.filter_compressor_gain);
	view->AddChild(gain);

	return view;
}

void CompressorFilter::UpdateValues()
{
	Prefs.filter_compressor_rms = rms->Value();
	Prefs.filter_compressor_attac = attac->Value()/1000.0;
	Prefs.filter_compressor_decay = decay->Value()/1000.0;
	Prefs.filter_compressor_treshold = treshold->Value();
	Prefs.filter_compressor_ratio = ratio->Value();
	Prefs.filter_compressor_gain = gain->Value();

	for (int32 i=0; i<buffer_size; i++)
		delay_buffer[i] = 0;
}

/*******************************************************
*   Init & exit
*******************************************************/
bool CompressorFilter::InitFilter(float f, int32 c, int32 pass, int32 size)
{
	RealtimeFilter::InitFilter(f, c, pass, size);

	buffer_size = (int32)(m_frequency +.5) * m_channels;
	
	delay_buffer = new float[ buffer_size ];
	for (int32 i=0; i<buffer_size; i++)	
		delay_buffer[i] = 0.0;

	mRMSSumL = 0.0;
	mMultL = 1.0;
	mRMSSumR = 0.0;
	mMultR = 1.0;
	pBuffer = 0;

	return true;
}

void CompressorFilter::DeAllocate()
{
	delete[] delay_buffer;
}

/*******************************************************
*   
*******************************************************/
void CompressorFilter::FilterBuffer(float *buffer, size_t size)
{
	mDecayMult = exp(log(0.1)/(Prefs.filter_compressor_decay*m_frequency));
	mThreshold = pow(10.0, Prefs.filter_compressor_treshold/10);
	mGain = pow(10.0, Prefs.filter_compressor_gain/10);
	mInvRatio = 1.0 - 1.0 / Prefs.filter_compressor_ratio;


	float mult, levelL, levelR;
 
	if (m_channels == 2){
// Stereo
		for (size_t i=0; i<size; i+=2){
			if (Prefs.filter_compressor_rms) {
				// Calculate current level from root-mean-squared of
				// circular buffer ("RMS")
				mRMSSumL -= delay_buffer[pBuffer];
				delay_buffer[pBuffer] = buffer[i] * buffer[i];
				mRMSSumL += delay_buffer[pBuffer];
				levelL = sqrt(mRMSSumL/buffer_size);
				pBuffer = (pBuffer+1)%buffer_size;

				mRMSSumR -= delay_buffer[pBuffer];
				delay_buffer[pBuffer] = buffer[i+1] * buffer[i+1];
				mRMSSumR += delay_buffer[pBuffer];
				levelR = sqrt(mRMSSumR/buffer_size);
				pBuffer = (pBuffer+1)%buffer_size;
			}else{
				// Calculate current level from value at other end of
				// circular buffer ("Peak")
				levelL = delay_buffer[pBuffer];
				delay_buffer[pBuffer] = buffer[i]>0? buffer[i]: -buffer[i];
				pBuffer = (pBuffer+1)%buffer_size;

				levelR = delay_buffer[pBuffer];
				delay_buffer[pBuffer] = buffer[i+1]>0? buffer[i+1]: -buffer[i+1];
				pBuffer = (pBuffer+1)%buffer_size;
			}
   
			if (levelL > mThreshold)	mult = mGain * pow(mThreshold/levelL, mInvRatio);
			else						mult = 1.0;
			mMultL = mult*mDecayMult + mMultL*(1.0-mDecayMult);

			if (levelR > mThreshold)	mult = mGain * pow(mThreshold/levelR, mInvRatio);
			else						mult = 1.0;
			mMultR = mult*mDecayMult + mMultR*(1.0-mDecayMult);

			buffer[i] *= mMultL;
			buffer[i+1] *= mMultR;
		}
	}else if (m_channels ==1 ){
// Mono	
		for (size_t i=0; i<size; i++){
			if (Prefs.filter_compressor_rms) {
				// Calculate current level from root-mean-squared of
				// circular buffer ("RMS")
				mRMSSumL -= delay_buffer[pBuffer];
				delay_buffer[pBuffer] = buffer[i] * buffer[i];
				mRMSSumL += delay_buffer[pBuffer];
				levelL = sqrt(mRMSSumL/buffer_size);
				pBuffer = (pBuffer+1)%buffer_size;
			}else{
				// Calculate current level from value at other end of
				// circular buffer ("Peak")
				levelL = delay_buffer[pBuffer];
				delay_buffer[pBuffer] = buffer[i]>0? buffer[i]: -buffer[i];
				pBuffer = (pBuffer+1)%buffer_size;
			}
   
			if (levelL > mThreshold)	mult = mGain * pow(mThreshold/levelL, mInvRatio);
			else						mult = 1.0;
			mMultL = mult*mDecayMult + mMultL*(1.0-mDecayMult);

			buffer[i] *= mMultL;
		}
	}
}
