/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Copyright (C) 2009 - 2010 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file storyscreen/controller.cpp
 * Storyscreen controller (implementation).
 */

#include "global.hpp"
#include "storyscreen/controller.hpp"
#include "storyscreen/part.hpp"
#include "storyscreen/render.hpp"

#include "asserts.hpp"
#include "foreach.hpp"
#include "variable.hpp"

#include "display.hpp"
#include "game_events.hpp"
#include "gamestatus.hpp"
#include "gettext.hpp"
#include "intro.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "widgets/button.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

namespace storyscreen {

controller::controller(display& disp, const vconfig& data, const std::string& scenario_name)
	: disp_(disp)
	, disp_resize_lock_()
	, evt_context_()
	, data_(data)
	, scenario_name_(scenario_name)
	, parts_()
{
	ASSERT_LOG(resources::state_of_game != NULL, "Ouch: gamestate is NULL when initializing storyscreen controller");
	build_parts();
}

void controller::resolve_wml(const vconfig& cfg)
{
	for(vconfig::all_children_iterator i = cfg.ordered_begin(); i != cfg.ordered_end(); i++)
	{
		// i->first and i->second are goddamn temporaries; do not make references
		const std::string key = i->first;
		const vconfig node = i->second;

		if(key == "part" && !node.empty()) {
			part_pointer_type const story_part(new part(node));
			// Use scenario name as part title if the WML doesn't supply a custom one.
			if((*story_part).show_title() && (*story_part).title().empty()) {
				(*story_part).set_title( scenario_name_ );
			}
			parts_.push_back(story_part);
		}
		// [if]
		else if(key == "if") {
			const std::string branch_label =
				game_events::conditional_passed(NULL, node) ?
				"then" : "else";
			const vconfig branch = node.child(branch_label);
			resolve_wml(branch);
		}
		// [switch]
		else if(key == "switch") {
			const std::string var_name = node["variable"];
			const std::string var_actual_value = resources::state_of_game->get_variable_const(var_name);
			bool case_not_found = true;

			for(vconfig::all_children_iterator j = node.ordered_begin(); j != node.ordered_end(); ++j) {
				if(j->first != "case") continue;

				// Enter all matching cases.
				const std::string var_expected_value = (j->second)["value"];
			    if(var_actual_value == var_expected_value) {
					case_not_found = false;
					resolve_wml(j->second);
			    }
			}

			if(case_not_found) {
				for(vconfig::all_children_iterator j = node.ordered_begin(); j != node.ordered_end(); ++j) {
					if(j->first != "else") continue;

					// Enter all elses.
					resolve_wml(j->second);
				}
			}
		}
		// [deprecated_message]
		else if(key == "deprecated_message") {
			// Won't appear until the scenario start event finishes.
			game_events::handle_deprecated_message(node.get_parsed_config());
		}
		// [wml_message]
		else if(key == "wml_message") {
			// Pass to game events handler. As with [deprecated_message],
			// it won't appear until the scenario start event is complete.
			game_events::handle_wml_log_message(node.get_parsed_config());
		}
	}
}

void controller::show()
{
	if(parts_.empty()) {
		LOG_NG << "no storyscreen parts to show\n";
		return;
	}

	gui::button next_button(disp_.video(),_("Next") + std::string(" >"));
	gui::button skip_button(disp_.video(),_("Skip"));
	// TODO:
	//  gui::button back_button(disp_.video(),std::string("< ")+_("Next"));

	// Build renderer cache unless built for a low-memory environment;
	// caching the scaled backgrounds can take over a decent amount of memory.
#ifndef LOW_MEM
	std::vector< render_pointer_type > uis_;
	foreach(part_pointer_type p, parts_) {
		ASSERT_LOG( p != NULL, "Ouch: hit NULL storyscreen part in collection" );
		render_pointer_type const rpt(new part_ui(*p, disp_, next_button, skip_button));
		uis_.push_back(rpt);
	}
#endif

	size_t k = 0;

	while(k < parts_.size()) {
#ifndef LOW_MEM
		render_reference_type render_interface = *uis_[k];
#else
		render_value_type render_interface(*parts_[k], disp_, next_button, skip_button);
#endif

		LOG_NG << "displaying storyscreen part " << k+1 << " of " << parts_.size() << '\n';

		switch(render_interface.show()) {
		case part_ui::NEXT:
			++k;
			break;
		case part_ui::BACK:
			// If we are at the first page we can't go back.
			if(k > 0) {
				--k;
			}
			break;
		case part_ui::SKIP:
			k = parts_.size();
			break;
		default:
			throw quit();
		}
	}
}

} // end namespace storyscreen
