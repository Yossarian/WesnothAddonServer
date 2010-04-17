/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file preferences.cpp
 *  Get and set user-preferences.
 */

#include "global.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "filesystem.hpp"
#include "gui/widgets/settings.hpp"
#include "hotkeys.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "sound.hpp"
#include "video.hpp" // non_interactive()
#include "serialization/parser.hpp"

#include <sys/stat.h> // for setting the permissions of the preferences file

static lg::log_domain log_filesystem("filesystem");
#define ERR_FS LOG_STREAM(err, log_filesystem)

namespace {

bool colour_cursors = false;

bool no_preferences_save = false;

bool fps = false;

int draw_delay_ = 20;

config prefs;
}

namespace preferences {

base_manager::base_manager()
{
	scoped_istream stream = istream_file(get_prefs_file());
	read(prefs, *stream);
}

base_manager::~base_manager()
{
	if (no_preferences_save) return;

	// Set the 'hidden' preferences.
	prefs["scroll_threshold"] =
			lexical_cast<std::string>(mouse_scroll_threshold());

	write_preferences();
}

void write_preferences()
{
    #ifndef _WIN32

    bool prefs_file_existed = access(get_prefs_file().c_str(), F_OK) == 0;

    #endif

	try {
		scoped_ostream prefs_file = ostream_file(get_prefs_file());
		write(*prefs_file, prefs);
	} catch(io_exception&) {
		ERR_FS << "error writing to preferences file '" << get_prefs_file() << "'\n";
	}


    #ifndef _WIN32

    if(!prefs_file_existed) {

        if(chmod(get_prefs_file().c_str(), 0600) == -1) {
			ERR_FS << "error setting permissions of preferences file '" << get_prefs_file() << "'\n";
        }

    }

    #endif


}

void set(const std::string& key, std::string value) {
	prefs[key] = value;
}

void clear(const std::string& key)
{
	prefs.recursive_clear_value(key);
}

void set_child(const std::string& key, const config& val) {
	prefs.clear_children(key);
	prefs.add_child(key, val);
}

const config &get_child(const std::string& key)
{
	return prefs.child(key);
}

void erase(const std::string& key) {
	prefs.remove_attribute(key);
}

std::string get(const std::string& key) {
	return prefs[key];
}

void disable_preferences_save() {
	no_preferences_save = true;
}

config* get_prefs(){
	config* pointer = &prefs;
	return pointer;
}

bool fullscreen()
{
	return utils::string_bool(get("fullscreen"), false);
}

void _set_fullscreen(bool ison)
{
	prefs["fullscreen"] = (ison ? "yes" : "no");
}

bool scroll_to_action()
{
	return utils::string_bool(get("scroll_to_action"), true);
}

void _set_scroll_to_action(bool ison)
{
	prefs["scroll_to_action"] = (ison ? "yes" : "no");
}

int min_allowed_width()
{
#ifdef USE_TINY_GUI
	return 320;
#endif

	// Most things (not all) in the new widgets library work properly on tiny
	// gui resolution. So allow them, which makes initial testing easier.
	if(gui2::new_widgets) {
		return 320;
	}

	return 800;
}

int min_allowed_height()
{
#ifdef USE_TINY_GUI
	return 240;
#endif

	// Most things (not all) in the new widgets library work properly on tiny
	// gui resolution. So allow them, which makes initial testing easier.
	if(gui2::new_widgets) {
		return 240;
	}

	if (game_config::small_gui)
		return 480;

	return 600;
}

std::pair<int,int> resolution()
{
	const std::string postfix = fullscreen() ? "resolution" : "windowsize";
	std::string x = prefs['x' + postfix], y = prefs['y' + postfix];
	if (!x.empty() && !y.empty()) {
		std::pair<int,int> res(std::max(atoi(x.c_str()), min_allowed_width()),
		                       std::max(atoi(y.c_str()), min_allowed_height()));

		// Make sure resolutions are always divisible by 4
		//res.first &= ~3;
		//res.second &= ~3;
		return res;
	} else {
		return std::pair<int,int>(1024,768);
	}
}

bool turbo()
{
	if(non_interactive()) {
		return true;
	}

	return  utils::string_bool(get("turbo"), false);
}

void _set_turbo(bool ison)
{
	prefs["turbo"] = (ison ? "yes" : "no");
}

double turbo_speed()
{
	return lexical_cast_default<double>(get("turbo_speed"), 2.0);
}

void save_turbo_speed(const double speed)
{
	preferences::set("turbo_speed", lexical_cast<std::string>(speed));
}

bool idle_anim()
{
	return  utils::string_bool(get("idle_anim"), true);
}

void _set_idle_anim(const bool ison)
{
	prefs["idle_anim"] = (ison ? "yes" : "no");
}

int idle_anim_rate()
{
	return lexical_cast_default<int>(get("idle_anim_rate"), 0);
}

void _set_idle_anim_rate(const int rate)
{
	preferences::set("idle_anim_rate", lexical_cast<std::string>(rate));
}

const std::string& language()
{
	return prefs["locale"];
}

void set_language(const std::string& s)
{
	preferences::set("locale", s);
}

bool ellipses()
{
	return utils::string_bool(get("show_side_colours"), false);
}

void _set_ellipses(bool ison)
{
	preferences::set("show_side_colours",  (ison ? "yes" : "no"));
}

bool grid()
{
	return utils::string_bool(get("grid"), false);
}

void _set_grid(bool ison)
{
	preferences::set("grid",  (ison ? "yes" : "no"));
}

size_t sound_buffer_size()
{
	// Sounds don't sound good on Windows unless the buffer size is 4k,
	// but this seems to cause crashes on other systems...
	#ifdef _WIN32
		const size_t buf_size = 4096;
	#else
		const size_t buf_size = 1024;
	#endif

	return lexical_cast_default<size_t>(get("sound_buffer_size"), buf_size);
}

void save_sound_buffer_size(const size_t size)
{
	#ifdef _WIN32
		const char* buf_size = "4096";
	#else
		const char* buf_size = "1024";
	#endif

	const std::string new_size = lexical_cast_default<std::string>(size, buf_size);
	if (get("sound_buffer_size") == new_size)
		return;

	preferences::set("sound_buffer_size", new_size);

	sound::reset_sound();
}

int music_volume()
{
	return lexical_cast_default<int>(get("music_volume"), 100);
}

void set_music_volume(int vol)
{
	if(music_volume() == vol) {
		return;
	}

	preferences::set("music_volume", lexical_cast_default<std::string>(vol, "100"));
	sound::set_music_volume(music_volume());
}

int sound_volume()
{
	return lexical_cast_default<int>(get("sound_volume"), 100);
}

void set_sound_volume(int vol)
{
	if(sound_volume() == vol) {
		return;
	}

	preferences::set("sound_volume", lexical_cast_default<std::string>(vol, "100"));
	sound::set_sound_volume(sound_volume());
}

int bell_volume()
{
	return lexical_cast_default<int>(get("bell_volume"), 100);
}

void set_bell_volume(int vol)
{
	if(bell_volume() == vol) {
		return;
	}

	preferences::set("bell_volume", lexical_cast_default<std::string>(vol, "100"));
	sound::set_bell_volume(bell_volume());
}

int UI_volume()
{
	return lexical_cast_default<int>(get("UI_volume"), 100);
}

void set_UI_volume(int vol)
{
	if(UI_volume() == vol) {
		return;
	}

	preferences::set("UI_volume", lexical_cast_default<std::string>(vol, "100"));
	sound::set_UI_volume(UI_volume());
}

bool turn_bell()
{
	return utils::string_bool(get("turn_bell"), true);
}

bool set_turn_bell(bool ison)
{
	if(!turn_bell() && ison) {
		preferences::set("turn_bell", "yes");
		if(!music_on() && !sound_on() && !UI_sound_on()) {
			if(!sound::init_sound()) {
				preferences::set("turn_bell", "no");
				return false;
			}
		}
	} else if(turn_bell() && !ison) {
		preferences::set("turn_bell", "no");
		sound::stop_bell();
		if(!music_on() && !sound_on() && !UI_sound_on())
			sound::close_sound();
	}
	return true;
}

bool UI_sound_on()
{
	return utils::string_bool(get("UI_sound"), true);
}

bool set_UI_sound(bool ison)
{
	if(!UI_sound_on() && ison) {
		preferences::set("UI_sound", "yes");
		if(!music_on() && !sound_on() && !turn_bell()) {
			if(!sound::init_sound()) {
				preferences::set("UI_sound", "no");
				return false;
			}
		}
	} else if(UI_sound_on() && !ison) {
		preferences::set("UI_sound", "no");
		sound::stop_UI_sound();
		if(!music_on() && !sound_on() && !turn_bell())
			sound::close_sound();
	}
	return true;
}

bool message_bell()
{
	return utils::string_bool(get("message_bell"), true);
}

bool sound_on()
{
	return utils::string_bool(get("sound"), true);
}

bool set_sound(bool ison) {
	if(!sound_on() && ison) {
		preferences::set("sound", "yes");
		if(!music_on() && !turn_bell() && !UI_sound_on()) {
			if(!sound::init_sound()) {
				preferences::set("sound", "no");
				return false;
			}
		}
	} else if(sound_on() && !ison) {
		preferences::set("sound", "no");
		sound::stop_sound();
		if(!music_on() && !turn_bell() && !UI_sound_on())
			sound::close_sound();
	}
	return true;
}

bool music_on()
{
	return utils::string_bool(get("music"), true);
}

bool set_music(bool ison) {
	if(!music_on() && ison) {
		preferences::set("music", "yes");
		if(!sound_on() && !turn_bell() && !UI_sound_on()) {
			if(!sound::init_sound()) {
				preferences::set("music", "no");
				return false;
			}
		}
		else
			sound::play_music();
	} else if(music_on() && !ison) {
		preferences::set("music", "no");
		if(!sound_on() && !turn_bell() && !UI_sound_on())
			sound::close_sound();
		else
			sound::stop_music();
	}
	return true;
}

namespace {
	double scroll = 0.2;
}

int scroll_speed()
{
	const int value = lexical_cast_in_range<int>(get("scroll"), 50, 1, 100);
	scroll = value/100.0;

	return value;
}

void set_scroll_speed(const int new_speed)
{
	prefs["scroll"] = lexical_cast<std::string>(new_speed);
	scroll = new_speed / 100.0;
}

bool middle_click_scrolls()
{
	return utils::string_bool(get("middle_click_scrolls"), true);
}

bool mouse_scroll_enabled()
{
	return utils::string_bool(get("mouse_scrolling"), true);
}

void enable_mouse_scroll(bool value)
{
	set("mouse_scrolling", value ? "yes" : "no");
}

int mouse_scroll_threshold()
{
	return lexical_cast_default<int>(prefs["scroll_threshold"], 10);
}

bool animate_map()
{
	return utils::string_bool(preferences::get("animate_map"), true);
}

bool show_standing_animations()
{
	return utils::string_bool(preferences::get("unit_standing_animations"), true);
}

bool show_fps()
{
	return fps;
}

void set_show_fps(bool value)
{
	fps = value;
}

int draw_delay()
{
	return draw_delay_;
}

void set_draw_delay(int value)
{
	draw_delay_ = value;
}

bool use_colour_cursors()
{
	return colour_cursors;
}

void _set_colour_cursors(bool value)
{
	preferences::set("colour_cursors", value ? "yes" : "no");
	colour_cursors = value;
}

void load_hotkeys() {
	hotkey::load_hotkeys(prefs);
}
void save_hotkeys() {
	hotkey::save_hotkeys(prefs);
}

void add_alias(const std::string &alias, const std::string &command)
{
	config &alias_list = prefs.child_or_add("alias");
	alias_list[alias] = command;
}


const config &get_alias()
{
	return get_child("alias");
}

unsigned int sample_rate()
{
	return lexical_cast_default<unsigned int>(preferences::get("sample_rate"), 44100);
}

void save_sample_rate(const unsigned int rate)
{
	if (sample_rate() == rate)
		return;

	preferences::set("sample_rate", lexical_cast<std::string>(rate));

	// If audio is open, we have to re set sample rate
	sound::reset_sound();
}

} // end namespace preferences

