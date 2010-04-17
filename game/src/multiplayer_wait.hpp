/* $Id$ */
/*
   Copyright (C) 2007 - 2010
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef MULTIPLAYER_WAIT_HPP_INCLUDED
#define MULTIPLAYER_WAIT_HPP_INCLUDED

#include "widgets/label.hpp"
#include "widgets/button.hpp"
#include "widgets/menu.hpp"
#include "widgets/combo.hpp"

#include "leader_list.hpp"
#include "gamestatus.hpp"
#include "multiplayer_ui.hpp"

namespace mp {

class wait : public ui
{
public:
	wait(game_display& disp, const config& cfg, chat& c, config& gamelist);
	virtual void process_event();

	void join_game(bool observe);

	const game_state& get_state();

	void start_game();

protected:
	virtual void layout_children(const SDL_Rect& rect);
	virtual void hide_children(bool hide=true);
	virtual void process_network_data(const config& data, const network::connection sock);
private:
	class leader_preview_pane : public gui::preview_pane
	{
	public:
		leader_preview_pane(game_display& disp,
			const std::vector<const config *> &side_list, int color);

		bool show_above() const;
		bool left_side() const;
		void set_selection(int index);
		std::string get_selected_leader();
		std::string get_selected_gender();

		handler_vector handler_members();
	private:
		virtual void draw_contents();
		virtual void process_event();

		std::vector<const config *> side_list_;
		const int color_;
		gui::combo leader_combo_; // Must appear before the leader_list_manager
		gui::combo gender_combo_; // Must appear before the leader_list_manager
		leader_list_manager leaders_;
		size_t  selection_;
	};

	void generate_menu();

	gui::button cancel_button_;
	gui::label start_label_;
	gui::menu game_menu_;

	// int team_;

	config level_;
	game_state state_;

	bool stop_updates_;
};

}
#endif
