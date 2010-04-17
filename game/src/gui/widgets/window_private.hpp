/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_WINDOW_PRIVATE_HPP_INCLUDED
#define GUI_WIDGETS_WINDOW_PRIVATE_HPP_INCLUDED

/**
 * @file gui/widgets/window_private.hpp
 * Helper for header for the window.
 *
 * @note This file should only be included by window.cpp.
 *
 * This file is being used for a small experiment similar like
 * gui/widgets/grid_private.hpp.
 */

#include "gui/widgets/window.hpp"

namespace gui2 {

/**
 * Helper to implement private functions without modifing the header.
 *
 * The class is a helper to avoid recompilation and only has static
 * functions.
 */
struct twindow_implementation
{
	/**
	 * Layouts the window.
	 *
	 * This part handles the actual layouting of the window.
	 *
	 * @see layout_algorihm for more information.
	 *
	 * @param window              The window to operate upon.
	 * @param maximum_width       The maximum width of the window.
	 * @param maximum_height      The maximum height of the window.
	 */
	static void layout(twindow& window,
			const unsigned maximum_width, const unsigned maximum_height);

};

} // namespace gui2

#endif