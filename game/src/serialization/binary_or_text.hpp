/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2010 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file serialization/binary_or_text.hpp */

#ifndef SERIALIZATION_BINARY_OR_TEXT_HPP_INCLUDED
#define SERIALIZATION_BINARY_OR_TEXT_HPP_INCLUDED

#include "preprocessor.hpp"

#include <iosfwd>
#include <string>

#include <boost/iostreams/filtering_stream.hpp>

class config;

/**
 * Reads a file, and detects it is compressed before reading it.
 *
 * If it is not a valid file at all, it will throw an error as if it was trying
 * to read it as text WML.
 *
 * @returns                       True iff the format is compressed.
 */
bool detect_format_and_read(config &cfg, std::istream &in, std::string* error_log=NULL); // Throws config::error

/** Class for writing a config out to a file in pieces. */
class config_writer
{
public:
	config_writer(std::ostream &out, bool compress, int level = -1);

	void write(const config &cfg);

	void write_child(const std::string &key, const config &cfg);
	void write_key_val(const std::string &key, const std::string &value);
	void open_child(const std::string &key);
	void close_child(const std::string &key);
	bool good() const;

private:
	boost::iostreams::filtering_stream<boost::iostreams::output> filter_;
	std::ostream *out_ptr_;
	std::ostream &out_;
	bool compress_;
	unsigned int level_;
	std::string textdomain_;
};

#endif
