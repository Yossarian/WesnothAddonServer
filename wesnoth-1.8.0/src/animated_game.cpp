/* $Id$ */
/*
   Copyright (C) 2007 - 2010 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file animated_game.cpp
 * Template instantiations for wesnoth-game.
 */

#include "animated.i"
// Force compilation of the following template instantiations

#include "unit_frame.hpp"
#include "image.hpp"

template class animated< image::locator >;
template class animated< std::string >;
template class animated< unit_frame >;

