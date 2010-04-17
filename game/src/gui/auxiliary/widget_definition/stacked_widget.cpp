/* $Id$ */
/*
   Copyright (C) 2007 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/widget_definition/stacked_widget.hpp"

#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "wml_exception.hpp"

namespace gui2 {

tstacked_widget_definition::tstacked_widget_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing stacked widget " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

tstacked_widget_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, grid(NULL)
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_stacked_widget
 *
 * == Multi page ==
 *
 * The documentation is not written yet.
 */

	// Add a dummy state since every widget needs a state.
	static config dummy ("draw");
	state.push_back(tstate_definition(dummy));

	const config &child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = new tbuilder_grid(child);
}

} // namespace gui2

