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

#ifndef GUI_AUXILIARY_WIDGET_DEFINITION_VERTICAL_SCROLLBAR_HPP_INCLUDED
#define GUI_AUXILIARY_WIDGET_DEFINITION_VERTICAL_SCROLLBAR_HPP_INCLUDED

#include "gui/auxiliary/widget_definition.hpp"

namespace gui2 {

struct tvertical_scrollbar_definition
	: public tcontrol_definition
{
	tvertical_scrollbar_definition(const config& cfg);

	struct tresolution
		: public tresolution_definition_
	{
		tresolution(const config& cfg);

		unsigned minimum_positioner_length;
		unsigned maximum_positioner_length;

		unsigned top_offset;
		unsigned bottom_offset;
	};

};

} // namespace gui2

#endif

