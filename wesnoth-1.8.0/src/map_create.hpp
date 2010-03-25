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

#ifndef MAP_CREATE_HPP_INCLUDED
#define MAP_CREATE_HPP_INCLUDED

class config;
class map_generator;

#include <string>

std::string random_generate_map(const std::string& parms, const config &cfg);
config random_generate_scenario(const std::string& parms, const config &cfg);

map_generator* create_map_generator(const std::string& name, const config &cfg);

#endif
