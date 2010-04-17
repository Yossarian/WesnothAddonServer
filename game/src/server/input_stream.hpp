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

#ifndef INPUT_STREAM_HPP_INCLUDED
#define INPUT_STREAM_HPP_INCLUDED

#include <deque>
#include <string>

class input_stream
{
public:
	input_stream(const std::string& path);
	~input_stream();

	bool read_line(std::string& str);
	void stop();

private:
	input_stream(const input_stream&);
	void operator=(const input_stream&);

	int fd_;
	std::string path_;
	std::deque<char> data_;
};

#endif
