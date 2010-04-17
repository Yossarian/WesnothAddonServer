/* $Id$ */
/*
   Copyright (C) 2005 - 2010 by Rusty Russell <rusty@rustcorp.com.au>
   Copyright (C) 2009 - 2010 by Greg Shikhman <cornmander@cornmander.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef UPLOAD_LOG_H_INCLUDED
#define UPLOAD_LOG_H_INCLUDED

#include <string>

#include "config.hpp"
#include "thread.hpp"

class config;
class display;
class game_state;
class t_string;
class team;
class unit_map;

struct upload_log
{
public:
	struct manager {
		manager() {upload_log::manager_ =  this;}
		~manager();
		void manage();
	};

	// We only enable logging when playing campaigns.
	upload_log(bool enable);
	~upload_log();

	// User starts a game (may be new campaign or saved).
	void start(game_state &state, const team &team,
			const t_string &turn, int num_turns,
			const std::string& map_data);
	void start(game_state &state, const std::string& map_data);

	// User finishes a level.
	void defeat(int turn);
	void victory(int turn, int gold);
	void quit(int turn);
	void read_replay();

	// Argument passed to upload thread.
	struct thread_info {
		thread_info() :
			t(0),
			shutdown(false),
			lastfile()
		{}

		threading::thread *t;
		bool shutdown;
		std::string lastfile;
	};

private:
	config &add_game_result(const std::string &str, int turn);
	bool game_finished(config *game);

	static struct thread_info thread_;
	static manager* manager_;
	friend struct manager;

	config config_;
	config *game_;
	std::string filename_;
	bool enabled_;
};

namespace upload_log_dialog
{
	// Please please please upload stats?
	void show_beg_dialog(display& disp);
}

#endif // UPLOAD_LOG_H_INCLUDED
