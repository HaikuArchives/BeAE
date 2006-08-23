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

#include <FindDirectory.h>
#include <Directory.h>
#include <Screen.h>

#include <stdio.h>

#include "Globals.h"
#include "Preferences.h"
#include "YPreferences.h"

// our Global def
Preferences Prefs;

#define BUFFER_SIZE		8192*8
#define CACHE_SIZE		16*256	// size of the VM Cache (4Mb)

/*******************************************************
*
*******************************************************/
Preferences::Preferences(){
   FactorySettings();
}


/*******************************************************
*
*******************************************************/
void Preferences::Init(){
   BPath path;
   BDirectory dir;
   
   FactorySettings();
   
   // Get that damn file 
   YPreferences prefs(SETTINGS_DIR"/BeAE");
   if (prefs.InitCheck() != B_OK) {
      find_directory(B_USER_SETTINGS_DIRECTORY, &path);
      path.Append(SETTINGS_DIR);
      dir.CreateDirectory(path.Path(),&dir);
   } 
   
   // Get the language name
	if(prefs.FindString("Language_Name",&lang_name) != B_OK)
		lang_name.SetTo("English");
   
   // Get the temp dir
	if(prefs.FindString("temp_dir",&temp_dir) != B_OK)
		temp_dir.SetTo("/tmp");
   
   // Get the window size and position
	if(prefs.FindRect("window_frame",&frame)!=B_OK)
		frame.Set(50,50,800,600);
		
	if(prefs.FindRect("sample_scope_frame", &sample_scope_pos)!=B_OK)
		sample_scope_pos.Set(50, 50, 270, 150);

	if(prefs.FindRect("spectrum_analyzer_frame", &spectrum_analyzer_pos)!=B_OK)
	   	spectrum_analyzer_pos.Set(50,200,270, 300);
	
	char s[255];
	for (int i=0; i<40; i++){
		sprintf(s, "filter%d_frame", i);
		if(prefs.FindRect(s, &filter_pos[i])!=B_OK)
			filter_pos[i].Set(200+i*5,100+i*10,400+i*5,200+i*10);
	}
	
   if(prefs.FindBool("show_peak", &show_peak) != B_OK)
      show_peak = true;
   if(prefs.FindBool("show_grid", &show_grid) != B_OK)
      show_grid = true;
   if(prefs.FindBool("follow_playing", &follow_playing) != B_OK)
      follow_playing = true;
   if(prefs.FindBool("save_undo", &save_undo) != B_OK)
      save_undo = true;
   if(prefs.FindBool("play_on_load", &play_when_loaded) != B_OK)
      play_when_loaded = true;
   if(prefs.FindBool("select_all", &select_all_on_double) != B_OK)
      select_all_on_double = true;
   if(prefs.FindBool("select_paste", &select_after_paste) != B_OK)
      select_after_paste = true;

   if(prefs.FindBool("drag_drop", &drag_drop) != B_OK)
      drag_drop = true;

   if(prefs.FindInt32("buffer_size", &buffer_size) != B_OK)
      buffer_size = BUFFER_SIZE;

   if(prefs.FindInt32("cache_size", &cache_size) != B_OK)
      cache_size = CACHE_SIZE;

   if(prefs.FindInt32("keep_free", &keep_free) != B_OK)
      keep_free = 500;
   if(prefs.FindInt32("display_time", &display_time) != B_OK)
      display_time = DISPLAY_TIME;

	if(prefs.FindFloat("peak", &peak) != B_OK)	peak = 0.85;

	if(prefs.FindColor("back_color", &back_color) != B_OK){
		SetColorScheme();		// set default
	}else{						// load the rest of the colors
		prefs.FindColor("back_color2", &back_color2);
		prefs.FindColor("back_selected_color", &back_selected_color);
		prefs.FindColor("back_selected_color2", &back_selected_color2);

		prefs.FindColor("index_back_color", &index_back_color);
		prefs.FindColor("index_back_color2", &index_back_color2);
		prefs.FindColor("index_mid_color", &index_mid_color);
		prefs.FindColor("index_left_color", &index_left_color);
		prefs.FindColor("index_back_selected_color", &index_back_selected_color);
		prefs.FindColor("index_left_selected_color", &index_left_selected_color);
		prefs.FindColor("index_left_color2", &index_left_color2);
		prefs.FindColor("index_back_selected_color2", &index_back_selected_color2);
		prefs.FindColor("index_left_selected_color2", &index_left_selected_color2);
		prefs.FindColor("index_mid_selected_color", &index_mid_selected_color);
		prefs.FindColor("index_pointer_color", &index_pointer_color);

		prefs.FindColor("left_color", &left_color);
		prefs.FindColor("left_selected_color", &left_selected_color);
		prefs.FindColor("left_color2", &left_color2);
		prefs.FindColor("left_selected_color2", &left_selected_color2);

		prefs.FindColor("right_color", &right_color);
		prefs.FindColor("right_selected_color", &right_selected_color);
		prefs.FindColor("right_color2", &right_color2);
		prefs.FindColor("right_selected_color2", &right_selected_color2);

		prefs.FindColor("grid_color", &grid_color);
		prefs.FindColor("grid_selected_color", &grid_selected_color);
		prefs.FindColor("peak_color", &peak_color);
		prefs.FindColor("peak_selected_color", &peak_selected_color);

		prefs.FindColor("mid_left_color", &mid_left_color);
		prefs.FindColor("mid_right_color", &mid_right_color);
		prefs.FindColor("mid_left_selected_color", &mid_left_selected_color);
		prefs.FindColor("mid_right_selected_color", &mid_right_selected_color);

		prefs.FindColor("pointer_color", &pointer_color);
		prefs.FindColor("time_back_color", &time_back_color);
		prefs.FindColor("time_marks_color", &time_marks_color);
		prefs.FindColor("time_small_marks_color", &time_small_marks_color);
		prefs.FindColor("time_text_color", &time_text_color);
	}

	// repeat action
	if(prefs.FindInt32("repeat_message", (int32*)&repeat_message) != B_OK)		repeat_message = 0;
	if(prefs.FindString("repeat_tag", &repeat_tag) != B_OK)						repeat_tag.SetTo("");

	// for the filters
	if(prefs.FindInt32("f_normalize", &filter_normalize) != B_OK)				filter_normalize = 98;
	if(prefs.FindInt32("f_resample_ml", &filter_resample_ml) != B_OK)			filter_resample_ml = 50;
	if(prefs.FindInt32("f_resample_mr", &filter_resample_mr) != B_OK)			filter_resample_mr = 50;
	if(prefs.FindInt32("f_resample_sl", &filter_resample_sl) != B_OK)			filter_resample_sl = 100;
	if(prefs.FindInt32("f_resample_sr", &filter_resample_sr) != B_OK)			filter_resample_sr = 100;
	if(prefs.FindInt32("f_resample_bits", &filter_resample_bits) != B_OK)		filter_resample_bits = 16;
	if(prefs.FindBool("f_resample_mono", &filter_resample_mono) != B_OK)		filter_resample_mono = true;
	if(prefs.FindFloat("f_resample_freq", &filter_resample_freq) != B_OK)		filter_resample_freq = 44100;

	if(prefs.FindFloat("f_delay_delay", &filter_delay_delay) != B_OK)			filter_delay_delay = 0.036;
	if(prefs.FindFloat("f_delay_gain", &filter_delay_gain) != B_OK)				filter_delay_gain = 0.5;

	if(prefs.FindFloat("f_reverb_delay", &filter_reverb_delay) != B_OK)			filter_reverb_delay = 0.036;
	if(prefs.FindFloat("f_reverb_gain", &filter_reverb_gain) != B_OK)			filter_reverb_gain = 0.5;

	if(prefs.FindFloat("f_room_delay", &filter_room_delay) != B_OK)				filter_room_delay = 0.04;
	if(prefs.FindFloat("f_room_gain", &filter_room_gain) != B_OK)				filter_room_gain = 0.5;
	if(prefs.FindFloat("f_room_damping", &filter_room_damping) != B_OK)			filter_room_damping = 0.4;

	if(prefs.FindInt32("f_bassboost_frequency", &filter_bassboost_frequency) != B_OK)	filter_bassboost_frequency = 250;
	if(prefs.FindInt32("f_bassboost_boost", &filter_bassboost_boost) != B_OK)		filter_bassboost_boost = 8;

//	if(prefs.FindInt32("f_limiter_value", &filter_limiter_value) != B_OK)		filter_limiter_value = 50;
//	if(prefs.FindInt32("f_limiter_mix", &filter_limiter_mix) != B_OK)			filter_limiter_mix = 100;

	if(prefs.FindInt32("f_amplifier_value", &filter_amplifier_value) != B_OK)	filter_amplifier_value = 100;

	if(prefs.FindBool("f_compressor_rms", &filter_compressor_rms) != B_OK)		filter_compressor_rms = true;
	if(prefs.FindFloat("f_compressor_ratio", &filter_compressor_ratio) != B_OK)	filter_compressor_ratio = 8.0;
	if(prefs.FindFloat("f_compressor_attac", &filter_compressor_attac) != B_OK)	filter_compressor_attac = 0.1;
	if(prefs.FindFloat("f_compressor_decay", &filter_compressor_decay) != B_OK)	filter_compressor_decay = 0.1;
	if(prefs.FindFloat("f_compressor_treshold", &filter_compressor_treshold) != B_OK) filter_compressor_treshold = -32;
	if(prefs.FindFloat("f_compressor_gain", &filter_compressor_gain) != B_OK)	filter_compressor_gain = 4.0;
}

/*******************************************************
*
*******************************************************/
void Preferences::Sync(){
	// Save the prefs for the App
	YPreferences prefs(SETTINGS_DIR"/BeAE");
	if (prefs.InitCheck() != B_OK)	return;
   
	prefs.SetString("Language_Name",Language.Name());
	prefs.SetString("temp_dir",temp_dir.String());
	prefs.SetRect("window_frame", frame);

	prefs.SetRect("sample_scope_frame", sample_scope_pos);
	prefs.SetRect("spectrum_analyzer_frame", spectrum_analyzer_pos);
	
	char s[255];
	for (int i=0; i<40; i++){
		sprintf(s, "filter%d_frame", i);
		prefs.SetRect(s, filter_pos[i]);
	}

	prefs.SetBool("show_peak", show_peak);
	prefs.SetBool("drag_drop", drag_drop);
	prefs.SetBool("show_grid", show_grid);
	prefs.SetBool("save_undo", save_undo);
	prefs.SetBool("play_on_load", play_when_loaded);
	prefs.SetBool("select_all", select_all_on_double);
	prefs.SetBool("select_paste", select_after_paste);
	prefs.SetBool("follow_playing", follow_playing);

	prefs.SetInt32("buffer_size",buffer_size);
	prefs.SetInt32("cache_size",cache_size);
	prefs.SetInt32("keep_free",keep_free);
	prefs.SetInt32("display_time",display_time);
	prefs.SetFloat("peak", peak);

	prefs.SetColor("back_color", back_color);
	prefs.SetColor("back_selected_color", back_selected_color);
	prefs.SetColor("back_color2", back_color2);
	prefs.SetColor("back_selected_color2", back_selected_color2);

	prefs.SetColor("index_back_color", index_back_color);
	prefs.SetColor("index_back_color2", index_back_color2);
	prefs.SetColor("index_mid_color", index_mid_color);
	prefs.SetColor("index_left_color", index_left_color);
	prefs.SetColor("index_left_color2", index_left_color2);
	prefs.SetColor("index_back_selected_color", index_back_selected_color);
	prefs.SetColor("index_back_selected_color2", index_back_selected_color2);
	prefs.SetColor("index_left_selected_color", index_left_selected_color);
	prefs.SetColor("index_left_selected_color2", index_left_selected_color2);
	prefs.SetColor("index_mid_selected_color", index_mid_selected_color);
	prefs.SetColor("index_pointer_color", index_pointer_color);

	prefs.SetColor("left_color", left_color);
	prefs.SetColor("left_selected_color", left_selected_color);
	prefs.SetColor("left_color2", left_color2);
	prefs.SetColor("left_selected_color2", left_selected_color2);

	prefs.SetColor("right_color", right_color);
	prefs.SetColor("right_selected_color", right_selected_color);
	prefs.SetColor("right_color2", right_color2);
	prefs.SetColor("right_selected_color2", right_selected_color2);

	prefs.SetColor("grid_color", grid_color);
	prefs.SetColor("grid_selected_color", grid_selected_color);
	prefs.SetColor("peak_color", peak_color);
	prefs.SetColor("peak_selected_color", peak_selected_color);

	prefs.SetColor("mid_left_color", mid_left_color);
	prefs.SetColor("mid_right_color", mid_right_color);
	prefs.SetColor("mid_left_selected_color", mid_left_selected_color);
	prefs.SetColor("mid_right_selected_color", mid_right_selected_color);

	prefs.SetColor("pointer_color", pointer_color);
	prefs.SetColor("time_back_color", time_back_color);
	prefs.SetColor("time_marks_color", time_marks_color);
	prefs.SetColor("time_small_marks_color", time_small_marks_color);
	prefs.SetColor("time_text_color", time_text_color);

	// repeat action
	prefs.SetInt32("repeat_message",(int32)repeat_message);
	prefs.SetString("repeat_tag",repeat_tag.String());

	// for the filters
	prefs.SetInt32("f_normalize",filter_normalize);
	prefs.SetInt32("f_resample_ml",filter_resample_ml);
	prefs.SetInt32("f_resample_mr",filter_resample_mr);
	prefs.SetInt32("f_resample_sl",filter_resample_sl);
	prefs.SetInt32("f_resample_sr",filter_resample_sr);
	prefs.SetInt32("f_resample_bits", filter_resample_bits);
	prefs.SetBool("f_resample_mono", filter_resample_mono);
	prefs.SetFloat("f_resample_freq", filter_resample_freq);

	prefs.SetFloat("f_delay_delay", filter_delay_delay);
	prefs.SetFloat("f_delay_gain", filter_delay_gain);
	prefs.SetFloat("f_reverb_delay", filter_reverb_delay);
	prefs.SetFloat("f_reverb_gain", filter_reverb_gain);

	prefs.SetFloat("f_room_delay", filter_room_delay);
	prefs.SetFloat("f_room_gain", filter_room_gain);
	prefs.SetFloat("f_room_damping", filter_room_damping);

	prefs.SetInt32("f_bassboost_frequency", filter_bassboost_frequency);
	prefs.SetInt32("f_bassboost_boost", filter_bassboost_boost);

//	prefs.FindInt32("f_limiter_value", filter_limiter_value);
//	prefs.FindInt32("f_limiter_mix", filter_limiter_mix);

	prefs.FindInt32("f_amplifier_value", &filter_amplifier_value);

	prefs.FindBool("f_compressor_rms", filter_compressor_rms);
	prefs.FindFloat("f_compressor_ratio", filter_compressor_ratio);
	prefs.FindFloat("f_compressor_attac", filter_compressor_attac);
	prefs.FindFloat("f_compressor_decay", filter_compressor_decay);
	prefs.FindFloat("f_compressor_treshold", filter_compressor_treshold);
	prefs.FindFloat("f_compressor_gain", filter_compressor_gain);
}

/*******************************************************
*
*******************************************************/
Preferences::~Preferences(){
	Sync();
}

/*******************************************************
*
*******************************************************/
void Preferences::FactorySettings(){
	lang_name.SetTo("English");
	temp_dir.SetTo("/tmp");

	frame.Set(50,50,800,600);

	SetColorScheme();		// Add the default scheme
	
	peak = .85;	// 90% boundary lines
	display_time = DISPLAY_TIME;
//	display_time = DISPLAY_SAMPLES;
	
	follow_playing = true;
	save_undo = true;
	show_grid = true;
	show_peak = true;
	drag_drop = true;
	play_when_loaded = true;
	select_all_on_double = true;
	select_after_paste = true;
	
	keep_free = 500;		// keep 500 Mb free disk space
	buffer_size = BUFFER_SIZE;
	cache_size = CACHE_SIZE;
	
	spectrum_analyzer_pos.Set(50,200,270, 300);
	sample_scope_pos.Set(50, 50, 270, 150);
	
	for (int i=0; i<40; i++)
		filter_pos[i].Set(200+i*5,100+i*10,400+i*5,200+i*10);
}

/*******************************************************
*
*******************************************************/
void Preferences::SetColorScheme(int32 i){
	switch(i){
	case 1:
	// GUI Cool Edit
		index_back_color = (rgb_color){46,93,31};
		index_back_color2 = (rgb_color){28,200,149};
		index_back_selected_color = (rgb_color){0, 0, 0};
		index_back_selected_color2 = (rgb_color){117,136,59};

		index_mid_color = (rgb_color){70,115,46};
		index_left_selected_color = (rgb_color){227,99,50};
		index_left_selected_color2 = (rgb_color){133,80,0};
		index_left_color = (rgb_color){238,107,0};
		index_left_color2 = (rgb_color){85,0,0};
		index_mid_selected_color = (rgb_color){80,80,80};
		index_pointer_color = (rgb_color){255,255,255};

		back_color = 				(rgb_color){119,119,119};
		back_color2 = 				(rgb_color){0, 0, 0};
		back_selected_color = 		(rgb_color){228,228,228};
		back_selected_color2 = 		(rgb_color){255,255,255};

		left_color = 				(rgb_color){28, 129,77};
		left_color2 = 				(rgb_color){106, 240, 171};
		left_selected_color = 		(rgb_color){14, 31, 49};
		left_selected_color2 = 		(rgb_color){32,80,117};

		right_color = 				(rgb_color){28, 129,77};
		right_color2 = 				(rgb_color){106, 240, 171};
		right_selected_color = 		(rgb_color){14, 31, 49};
		right_selected_color2 =		(rgb_color){32,80,117};

		grid_color = 				(rgb_color){55,55,81};
		grid_selected_color = 		(rgb_color){89,104,95};

		peak_color = 				(rgb_color){111, 135, 195};
		peak_selected_color = 		(rgb_color){111, 135, 195};

		mid_left_color = 			(rgb_color){164, 38, 38};
		mid_right_color = 			(rgb_color){164, 38, 38};
		mid_left_selected_color =	(rgb_color){164, 141, 38};
		mid_right_selected_color = 	(rgb_color){164, 141, 38};
		pointer_color = 			(rgb_color){255,255,0};

		time_back_color = (rgb_color){35,70,100};
		time_marks_color = (rgb_color){255, 255, 255};
		time_small_marks_color = (rgb_color){200, 200, 200};
		time_text_color = (rgb_color){210, 210, 210};
		break;

	case 2:
	// GUI Black & White
		index_back_color = (rgb_color){140, 140, 140};
		index_back_color2 = (rgb_color){255,255,255};
		index_mid_color = (rgb_color){128,128,128};
		index_left_color = (rgb_color){0, 0, 0};
		index_left_color2 = (rgb_color){255, 255, 255};
		index_back_selected_color = (rgb_color){40,40,40};
		index_back_selected_color2 = (rgb_color){220,220,220};
		index_left_selected_color = (rgb_color){32,32,32};
		index_left_selected_color2 = (rgb_color){32,32,32};
		index_mid_selected_color = (rgb_color){255,255,255};
		index_pointer_color = (rgb_color){0,0,255};

		back_color = 				(rgb_color){140, 140, 140};
		back_color2 = 				(rgb_color){00, 00, 0};
		back_selected_color = 		(rgb_color){255, 255, 255};
		back_selected_color2 = 		(rgb_color){255, 255, 255};

		left_color = 				(rgb_color){128, 128, 128};
		left_color2 = 				(rgb_color){255, 255, 255};
		left_selected_color = 		(rgb_color){192,192,192};
		left_selected_color2 = 		(rgb_color){0, 0, 0};

		right_color = 				(rgb_color){128, 128, 128};
		right_color2 = 				(rgb_color){255, 255, 255};
		right_selected_color = 		(rgb_color){192,192,192};
		right_selected_color2 = 	(rgb_color){0, 0, 0};

		grid_color = 				(rgb_color){128, 128, 128};
		grid_selected_color = 		(rgb_color){200,200,200};

		peak_color = 				(rgb_color){200, 200, 200};
		peak_selected_color = 		(rgb_color){100, 100, 100};

		mid_left_color = 			(rgb_color){192, 192, 192};
		mid_right_color = 			(rgb_color){192, 192, 192};
		mid_left_selected_color =	(rgb_color){192, 192, 192};
		mid_right_selected_color = 	(rgb_color){192, 192, 192};
		pointer_color = 			(rgb_color){255,255,255};

		time_back_color = (rgb_color){60,100,110};
		time_marks_color = (rgb_color){255, 255, 255};
		time_small_marks_color = (rgb_color){200, 200, 200};
		time_text_color = (rgb_color){240, 240, 240};
		break;

	case 3:
	// GUI SoftTones
		index_back_color = (rgb_color){255, 213, 114};
		index_back_color2 = (rgb_color){167, 121, 53};
		index_mid_color = (rgb_color){128,128,128};
		index_left_color = (rgb_color){69, 32, 0};
		index_left_color2 = (rgb_color){209, 87, 35};
		index_back_selected_color = (rgb_color){141,159,177};
		index_back_selected_color2 = (rgb_color){8,69,115};
		index_left_selected_color = (rgb_color){3,15,100};
		index_left_selected_color2 = (rgb_color){6,34,200};
		index_mid_selected_color = (rgb_color){80,80,80};
		index_pointer_color = (rgb_color){215,100,74};

		back_color =				(rgb_color){53, 95, 130};
		back_color2 =				(rgb_color){235, 234, 235};
		back_selected_color =		(rgb_color){122,110,123};
		back_selected_color2 =		(rgb_color){255,247,201};

		left_color = 				(rgb_color){57, 49, 53};
		left_color2 = 				(rgb_color){193, 152, 152};
		left_selected_color = 		(rgb_color){29,23,28};
		left_selected_color2 = 		(rgb_color){155,114,114};

		right_color = 				(rgb_color){163, 74, 49};
		right_color2 = 				(rgb_color){190, 201, 100};
		right_selected_color = 		(rgb_color){61, 46, 14};
		right_selected_color2 = 	(rgb_color){159, 136, 98};

		grid_color = 				(rgb_color){57, 96, 132};
		grid_selected_color = 		(rgb_color){57, 96, 132};

		peak_color = 				(rgb_color){160, 190, 220};
		peak_selected_color = 		(rgb_color){160, 190, 220};

		mid_left_color = 			(rgb_color){91, 175, 180};
		mid_right_color = 			(rgb_color){180, 180, 170};
		mid_left_selected_color =	(rgb_color){0, 80, 80};
		mid_right_selected_color = 	(rgb_color){80,80,0};
		pointer_color = 			(rgb_color){255,255,0};

		time_back_color = (rgb_color){163,132,114};
		time_marks_color = (rgb_color){255, 255, 255};
		time_small_marks_color = (rgb_color){200, 200, 200};
		time_text_color = (rgb_color){210, 210, 210};
		break;

	case 4:
	// GUI ColdCut
		index_back_color = (rgb_color){250, 232, 171};
		index_back_color2 = (rgb_color){231, 208, 168};
		index_mid_color = (rgb_color){255,255,196};
		index_left_selected_color = (rgb_color){89,88,132};
		index_left_selected_color2 = (rgb_color){19,87,87};
		index_left_color = (rgb_color){156,113,122};
		index_left_color2 = (rgb_color){123,66,95};

		index_back_selected_color = (rgb_color){192,196,208};
		index_back_selected_color2 = (rgb_color){159,159,172};
		index_mid_selected_color = (rgb_color){80,80,80};
		index_pointer_color = (rgb_color){255,0,64};

		back_color =				(rgb_color){243, 243, 243};
		back_color2 =				(rgb_color){255, 255, 255};
		back_selected_color =		(rgb_color){193,197,206};
		back_selected_color2 =		(rgb_color){246,246,255};

		left_color = 				(rgb_color){55,57,62};
		left_color2 = 				(rgb_color){140,142,162};
		left_selected_color = 		(rgb_color){0,0,0};
		left_selected_color2 = 		(rgb_color){78,80,102};

		right_color = 				(rgb_color){55,57,62};
		right_color2 = 				(rgb_color){140,142,162};
		right_selected_color = 		(rgb_color){0,0,0};
		right_selected_color2 =		(rgb_color){78,80,102};

		grid_color = 				(rgb_color){223,224,235};
		grid_selected_color = 		(rgb_color){177,177,186};

		peak_color = 				(rgb_color){230,231,235};
		peak_selected_color = 		(rgb_color){183,187,201};

		mid_left_color = 			(rgb_color){91, 175, 180};
		mid_right_color = 			(rgb_color){180, 180, 170};
		mid_left_selected_color =	(rgb_color){0, 80, 80};
		mid_right_selected_color = 	(rgb_color){80, 80, 0};
		pointer_color = 			(rgb_color){255,20,62};

		time_back_color = (rgb_color){216,216,216};
		time_marks_color = (rgb_color){150,150,150};
		time_small_marks_color = (rgb_color){180,180,180};
		time_text_color = (rgb_color){118,119,114};
		break;

	default:
	// GUI Default
		index_back_color = (rgb_color){255, 249, 221};
		index_back_color2 = (rgb_color){248, 219, 128};
		index_mid_color = (rgb_color){196,196,196};
		index_left_selected_color = (rgb_color){3, 15, 100};
		index_left_selected_color2 = (rgb_color){6, 34, 200};
		index_left_color = (rgb_color){90,0,0};
		index_left_color2 = (rgb_color){180,0,0};

		index_back_selected_color = (rgb_color){0,145,170};
		index_back_selected_color2 = (rgb_color){0,96,170};
		index_mid_selected_color = (rgb_color){80,80,80};
		index_pointer_color = (rgb_color){255,0,0};

		back_color = 				(rgb_color){0,115,157};
		back_color2 = 				(rgb_color){27,51,65};
		back_selected_color = 		(rgb_color){255,239,136};
		back_selected_color2 = 		(rgb_color){255,242,204};

		left_color = 				(rgb_color){219, 122, 19};
		left_color2 = 				(rgb_color){255, 235, 176};
		left_selected_color = 		(rgb_color){23,  46,  65};
		left_selected_color2 = 		(rgb_color){40, 106, 166};

		right_color = 				(rgb_color){125, 74, 49};
		right_color2 = 				(rgb_color){255, 211, 129};
		right_selected_color = 		(rgb_color){35,47,55};
		right_selected_color2 =		(rgb_color){91,138,194};

		grid_color = 				(rgb_color){57, 96, 132};
		grid_selected_color = 		(rgb_color){164, 190, 190};

		peak_color = 				(rgb_color){160, 190, 220};
		peak_selected_color = 		(rgb_color){100, 100, 180};

		mid_left_color = 			(rgb_color){91, 175, 180};
		mid_right_color = 			(rgb_color){180, 180, 170};
		mid_left_selected_color =	(rgb_color){0, 80, 80};
		mid_right_selected_color = 	(rgb_color){80, 80, 0};
		pointer_color = 			(rgb_color){255,255,0};

		time_back_color = (rgb_color){35,100,160};
		time_marks_color = (rgb_color){255, 255, 255};
		time_small_marks_color = (rgb_color){200, 200, 200};
		time_text_color = (rgb_color){210, 210, 210};
		break;
	}
}
