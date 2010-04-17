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

/**
 * @file tools/key_test.cpp
 * Keyboard-test - Standalone-Utility
 */

#include "key.h"
#include "video.hpp"

int main( void )
{
	SDL_Init(SDL_INIT_VIDEO);
	CVideo video( 640, 480, 16, 0 );
	CKey key;
	printf( "press enter (escape exits)...\n" );
	for(;;) {
		if( key[KEY_RETURN] != 0 )
			printf( "key(ENTER) pressed\n" );
		if( key[SDLK_ESCAPE] != 0 )
			return 1;
	}
}
