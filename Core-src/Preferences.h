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

#ifndef _PREFS_H
#define _PREFS_H

#include <String.h>
#include <Rect.h>
#include <InterfaceDefs.h>
#include <GraphicsDefs.h>

enum{ DISPLAY_SAMPLES, DISPLAY_TIME };

class Preferences{
 public:
	Preferences();
	void Init();
	void Sync();
	~Preferences();
	void FactorySettings();
	
	void SetColorScheme(int32 i = 0);
 public:
	// Non-user set prefs
	BString lang_name, temp_dir;

	rgb_color	index_back_color, index_left_color, index_mid_color, index_mid_selected_color;
	rgb_color	index_back_selected_color, index_left_selected_color;
	rgb_color	index_back_color2, index_left_color2;
	rgb_color	index_back_selected_color2, index_left_selected_color2;

	rgb_color	back_color, back_selected_color, left_color, right_color, left_selected_color, right_selected_color;
	rgb_color	grid_color, grid_selected_color, peak_color, peak_selected_color, mid_left_color, mid_right_color;
	rgb_color	back_color2, back_selected_color2, left_color2, right_color2, left_selected_color2, right_selected_color2;

	rgb_color	mid_left_selected_color, mid_right_selected_color, pointer_color, index_pointer_color;
	rgb_color	time_back_color, time_marks_color, time_small_marks_color, time_text_color;

	bool	select_all_on_double;	// use double click to select all ?
	bool	save_undo;				// undo enabled ?
	bool	show_grid;				// show grid ?
	BRect	frame;					// window frame
	bool	play_when_loaded;		// play after a file drop ?
	bool	select_after_paste;		// set selection after a paste
	bool	show_peak;				// peak_level lines
	bool	follow_playing;			// follow the playcursor on display
	bool	drag_drop;				// enable drag & drop

	float	peak;
	int32	display_time;
	int32	keep_free;				// keep free diskspace

	int32	buffer_size;			// media buffer-size
	int32	cache_size;				// size of the VM cache


	BRect	filter_pos[40];			// position of the filters
	BRect	spectrum_analyzer_pos;
	BRect	sample_scope_pos;
	
// prefs for build in filters
	int32	filter_normalize;
	int32	filter_resample_ml, filter_resample_mr, filter_resample_sl, filter_resample_sr, filter_resample_bits;
	bool	filter_resample_mono;
	float	filter_resample_freq;
	float	filter_reverb_delay, filter_reverb_gain;
	float	filter_delay_delay, filter_delay_gain;
	float	filter_room_delay, filter_room_gain, filter_room_damping;

	int32	filter_bassboost_frequency, filter_bassboost_boost;
	int32	filter_limiter_value, filter_limiter_mix;
	int32	filter_amplifier_value;

	bool	filter_compressor_rms;
	float	filter_compressor_ratio, filter_compressor_attac, filter_compressor_decay, filter_compressor_treshold, filter_compressor_gain;

	uint32	repeat_message;
	BString	repeat_tag;

 private:
};

extern Preferences Prefs; // Included so you don't have too 
#endif
