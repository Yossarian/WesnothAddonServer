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
 * @file display.hpp
 *
 * map_display and display: classes which take care of
 * displaying the map and game-data on the screen.
 *
 * The display is divided into two main sections:
 * - the game area, which displays the tiles of the game board, and units on them,
 * - and the side bar, which appears on the right hand side.
 * The side bar display is divided into three sections:
 * - the minimap, which is displayed at the top right
 * - the game status, which includes the day/night image,
 *   the turn number, information about the current side,
 *   and information about the hex currently moused over (highlighted)
 * - the unit status, which displays an image and stats
 *   for the current unit.
 */

#ifndef DISPLAY_H_INCLUDED
#define DISPLAY_H_INCLUDED

class config;
class terrain_builder;
class map_labels;

#include "generic_event.hpp"
#include "image.hpp"
#include "font.hpp"
#include "key.hpp"
#include "map_location.hpp"
#include "reports.hpp"
#include "team.hpp"
#include "time_of_day.hpp"
#include "theme.hpp"
#include "video.hpp"
#include "widgets/button.hpp"

#include "SDL.h"

#include <map>
#include <set>
#include <string>

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

class gamemap;

class display
{
public:
	display(CVideo& video, const gamemap* map, const config& theme_cfg,
			const config& level);
	virtual ~display();

	/**
	 * Updates internals that cache map size. This should be called when the map
	 * size has changed.
	 */
	void reload_map();

	void change_map(const gamemap* m);

	static Uint32 rgb(Uint8 red, Uint8 green, Uint8 blue)
		{ return 0xFF000000 | (red << 16) | (green << 8) | blue; }
	static Uint8 red(Uint32 color)
		{ return (color & 0x00FF0000) >> 16;}
	static Uint8 green(Uint32 color)
		{ return (color & 0x0000FF00) >> 8;}
	static Uint8 blue(Uint32 color)
		{ return (color & 0x000000FF) ;}
	static Uint32 max_rgb(Uint32 first,Uint32 second)
		{ return rgb(std::max(red(first),red(second)),std::max(green(first),green(second)),std::max(blue(first),blue(second))) ; }

	/** Gets the underlying screen object. */
	CVideo& video() { return screen_; }

	/** return the screen surface or the surface used for map_screenshot. */
	surface get_screen_surface() { return map_screenshot_ ? map_screenshot_surf_ : screen_.getSurface();}

	virtual bool in_game() const { return false; }
	virtual bool in_editor() const { return false; }

	/**
	 * the dimensions of the display. x and y are width/height.
	 * mapx is the width of the portion of the display which shows the game area.
	 * Between mapx and x is the sidebar region.
	 */
	int w() const { return screen_.getx(); }	/**< width */
	int h() const { return screen_.gety(); }	/**< height */
	const SDL_Rect& minimap_area() const
		{ return theme_.mini_map_location(screen_area()); }
	const SDL_Rect& unit_image_area() const
		{ return theme_.unit_image_location(screen_area()); }

	SDL_Rect screen_area() const
		{ const SDL_Rect res = {0,0,w(),h()}; return res; }

	/**
	 * Returns the maximum area used for the map
	 * regardless to resolution and view size
	 */
	const SDL_Rect& max_map_area() const;

	/**
	 * Returns the area used for the map
	 */
	const SDL_Rect& map_area() const;

	/**
	 * Returns the available area for a map, this may differ
	 * from the above. This area will get the background area
	 * applied to it.
	 */
	const SDL_Rect& map_outside_area() const { return map_screenshot_ ?
		max_map_area() : theme_.main_map_location(screen_area()); }

	/** Check if the bbox of the hex at x,y has pixels outside the area rectangle. */
	bool outside_area(const SDL_Rect& area, const int x,const int y) const;

	/**
	 * Function which returns the width of a hex in pixels,
	 * up to where the next hex starts.
	 * (i.e. not entirely from tip to tip -- use hex_size()
	 * to get the distance from tip to tip)
	 */
	int hex_width() const { return (zoom_*3)/4; }

	/**
	 * Function which returns the size of a hex in pixels
	 * (from top tip to bottom tip or left edge to right edge).
	 */
	int hex_size() const { return zoom_; }

	/** Returns the current zoom factor. */
	double get_zoom_factor() const { return double(zoom_)/double(image::tile_size); }

	/**
	 * given x,y co-ordinates of an onscreen pixel, will return the
	 * location of the hex that this pixel corresponds to.
	 * Returns an invalid location if the mouse isn't over any valid location.
	 */
	const map_location hex_clicked_on(int x, int y) const;

	/**
	 * given x,y co-ordinates of a pixel on the map, will return the
	 * location of the hex that this pixel corresponds to.
	 * Returns an invalid location if the mouse isn't over any valid location.
	 */
	const map_location pixel_position_to_hex(int x, int y) const;

	/**
	 * given x,y co-ordinates of the mouse, will return the location of the
	 * hex in the minimap that the mouse is currently over, or an invalid
	 * location if the mouse isn't over the minimap.
	 */
	map_location minimap_location_on(int x, int y);

	const map_location& selected_hex() const { return selectedHex_; }
	const map_location& mouseover_hex() const { return mouseoverHex_; }

	virtual void select_hex(map_location hex);
	virtual void highlight_hex(map_location hex);

	/** Function to invalidate the game status displayed on the sidebar. */
	void invalidate_game_status() { invalidateGameStatus_ = true; }

	/** Functions to get the on-screen positions of hexes. */
	int get_location_x(const map_location& loc) const;
	int get_location_y(const map_location& loc) const;

	/**
	 * Rectangular area of hexes, allowing to decide how the top and bottom
	 * edges handles the vertical shift for each parity of the x coordinate
	 */
	struct rect_of_hexes{
		int left;
		int right;
		int top[2]; // for even and odd values of x, respectively
		int bottom[2];

		/**  very simple iterator to walk into the rect_of_hexes */
		struct iterator {
			iterator(const map_location &loc, const rect_of_hexes &rect)
				: loc_(loc), rect_(rect){};

			/** increment y first, then when reaching bottom, increment x */
			void operator++();
			bool operator==(const iterator &that) const { return that.loc_ == loc_; }
			bool operator!=(const iterator &that) const { return that.loc_ != loc_; }
			const map_location& operator*() const {return loc_;};

			typedef std::forward_iterator_tag iterator_category;
			typedef map_location value_type;
			typedef int difference_type;
			typedef const map_location *pointer;
			typedef const map_location &reference;

			private:
				map_location loc_;
				const rect_of_hexes &rect_;
		};
		typedef iterator const_iterator;

		iterator begin() const;
		iterator end() const;
	};

	/** Return the rectangular area of hexes overlapped by r (r is in screen coordinates) */
	const rect_of_hexes hexes_under_rect(const SDL_Rect& r) const;

	/** Returns the rectangular area of visible hexes */
	const rect_of_hexes get_visible_hexes() const {return hexes_under_rect(map_area());};

	/** Returns true if location (x,y) is covered in shroud. */
	bool shrouded(const map_location& loc) const {
		return viewpoint_ && viewpoint_->shrouded(loc);
	}
	/** Returns true if location (x,y) is covered in fog. */
	bool fogged(const map_location& loc) const {
		return viewpoint_ && viewpoint_->fogged(loc);
	}

	/**
	 * Determines whether a grid should be overlayed on the game board.
	 * (to more clearly show where hexes are)
	 */
	void set_grid(const bool grid) { grid_ = grid; }

	/** Getter for the x,y debug overlay on tiles */
	bool get_draw_coordinates() const { return draw_coordinates_; }
	/** Setter for the x,y debug overlay on tiles */
	void set_draw_coordinates(bool value) { draw_coordinates_ = value; }

	/** Getter for the terrain code debug overlay on tiles */
	bool get_draw_terrain_codes() const { return draw_terrain_codes_; }
	/** Setter for the terrain code debug overlay on tiles */
	void set_draw_terrain_codes(bool value) { draw_terrain_codes_ = value; }

	/** Save a (map-)screenshot and return the estimated file size */
	int screenshot(std::string filename, bool map_screenshot = false);

	/** Invalidates entire screen, including all tiles and sidebar. Calls redraw observers. */
	void redraw_everything();

	/** Adds a redraw observer, a function object to be called when redraw_everything is used */
	void add_redraw_observer(boost::function<void(display&)> f);

	/** Clear the redraw observers */
	void clear_redraw_observers();

	theme& get_theme() { return theme_; }
	gui::button* find_button(const std::string& id);
	gui::button::TYPE string_to_button_type(std::string type);
	void create_buttons();
	void invalidate_theme() { panelsDrawn_ = false; }

	void refresh_report(reports::TYPE report_num, reports::report report);

	// Will be overridden in the display subclass
	virtual void draw_minimap_units() {};

	/** Function to invalidate all tiles. */
	void invalidate_all();

	/** Function to invalidate a specific tile for redrawing. */
	bool invalidate(const map_location& loc);

	bool invalidate(const std::set<map_location>& locs);

	/**
	 * If this set is partially invalidated, invalidate all its hexes.
	 * Returns if any new invalidation was needed
	 */
	bool propagate_invalidation(const std::set<map_location>& locs);

	/** invalidate all hexes under the rectangle rect (in screen coordinates) */
	bool invalidate_locations_in_rect(const SDL_Rect& rect);
	bool invalidate_visible_locations_in_rect(const SDL_Rect& rect);

	/**
	 * Function to invalidate animated terrains which may have changed.
	 */
	virtual void invalidate_animations();

	/**
	 * Per-location invalidation called by invalidate_animations()
	 * defaults to no action, overriden by derived classes
	 */
	virtual void invalidate_animations_location(const map_location& /*loc*/) {}

	/**
	 * What hex are currently invalidated (read only)
	 * used for some fine grained invalidation algorithm which need recurstion
	 */
	const std::set<map_location> & get_invalidated() { return invalidated_; }



	const gamemap& get_map() const { return *map_; }

	/**
	 * The last action in drawing a tile is adding the overlays.
	 * These overlays are drawn in the following order:
	 * hex_overlay_			if the drawn location is in the map
	 * selected_hex_overlay_	if the drawn location is selected
	 * mouseover_hex_overlay_	if the drawn location is underneath the mouse
	 *
	 * These functions require a prerendered surface.
	 * Since they are drawn at the top, they are not influenced by TOD, shroud etc.
	 */
	void set_selected_hex_overlay(const surface& image)
		{ selected_hex_overlay_ = image; }

	void clear_selected_hex_overlay()
		{ selected_hex_overlay_ = NULL; }

	void set_mouseover_hex_overlay(const surface& image)
		{ mouseover_hex_overlay_ = image; }

	void clear_mouseover_hex_overlay()
		{ mouseover_hex_overlay_ = NULL; }

	/**
	 * Debug function to toggle the "sunset" mode.
	 * The map area become progressively darker,
	 * except where hexes are refreshed.
	 * delay is the number of frames between each darkening
	 * (0 to toggle).
	 */
	void sunset(const size_t delay = 0);

	/** Toogle to continuously redraw the screen. */
	void toggle_benchmark();

	void flip();

	/** Copy the backbuffer to the framebuffer. */
	void update_display();

	/** Rebuild all dynamic terrain. */
	void rebuild_all();

	const theme::menu* menu_pressed();

	/**
	 * Finds the menu which has a given item in it,
	 * and enables or disables it.
	 */
	void enable_menu(const std::string& item, bool enable);

	void set_diagnostic(const std::string& msg);

	/** Delay routines: use these not SDL_Delay (for --nogui). */
	void delay(unsigned int milliseconds) const;

	/**
	 * Set/Get whether 'turbo' mode is on.
	 * When turbo mode is on, everything moves much faster.
	 */
	void set_turbo(const bool turbo) { turbo_ = turbo; }

	double turbo_speed() const;

	void set_turbo_speed(const double speed) { turbo_speed_ = speed; }

	/** control unit idle animations and their frequency */
	void set_idle_anim(bool ison) { idle_anim_ = ison; }
	bool idle_anim() const { return idle_anim_; }
	void set_idle_anim_rate(int rate);
	double idle_anim_rate() const { return idle_anim_rate_; }

	void bounds_check_position();
	void bounds_check_position(int& xpos, int& ypos);

	/**
	 * Scrolls the display by xmov,ymov pixels.
	 * Invalidation and redrawing will be scheduled.
	 * @return true if the map actually moved.
	 */
	bool scroll(int xmov, int ymov);

	/**
	 * Zooms the display by the specified amount.
	 * Negative values zoom out.
	 * Note the amount should be a multiple of four,
	 * otherwise the images might start to look odd
	 * (hex_width() gets rounding errors).
	 */
	void set_zoom(int amount);

	/** Sets the zoom amount to the default. */
	void set_default_zoom();

	enum SCROLL_TYPE { SCROLL, WARP, ONSCREEN };

	/**
	 * Scroll such that location loc is on-screen.
	 * WARP jumps to loc; SCROLL uses scroll speed;
	 * ONSCREEN only scrolls if x,y is offscreen
	 * force : scroll even if prefferences tell us not to
	 */
	void scroll_to_tile(const map_location& loc, SCROLL_TYPE scroll_type=ONSCREEN, bool check_fogged=true,bool force = true);

	/**
	 * Scroll such that location loc1 is on-screen.
	 * It will also try to make it such that loc2 is on-screen,
	 * but this is not guaranteed. For ONSCREEN scrolls add_spacing
	 * sets the desired minimum distance from the border in hexes.
	 */
	void scroll_to_tiles(map_location loc1, map_location loc2,
	                     SCROLL_TYPE scroll_type=ONSCREEN, bool check_fogged=true,
			     double add_spacing=0.0, bool force = true);

	/** Scroll to fit as many locations on-screen as possible, starting with the first. */
	void scroll_to_tiles(const std::vector<map_location>& locs,
	                     SCROLL_TYPE scroll_type=ONSCREEN, bool check_fogged=true,
	                     bool only_if_possible=false,
			     double add_spacing=0.0, bool force = true);

	/** Expose the event, so observers can be notified about map scrolling. */
	events::generic_event &scroll_event() const { return scroll_event_; }

	/** Check if a tile is fully visible on screen. */
	bool tile_fully_on_screen(const map_location& loc);

	/** Checks if location @a loc or one of the adjacent tiles is visible on screen. */
	bool tile_nearly_on_screen(const map_location &loc) const;

	/**
	 * Draws invalidated items.
	 * If update is true, will also copy the display to the frame buffer.
	 * If force is true, will not skip frames, even if running behind.
	 * Not virtual, since it gathers common actions. Calls various protected
	 * virtuals (further below) to allow specialized behaviour in derived classes.
	 */
	void draw(bool update=true, bool force=false);

	map_labels& labels();
	const map_labels& labels() const;

	/** Announce a message prominently. */
	void announce(const std::string& msg,
		       const SDL_Color& colour = font::GOOD_COLOUR);

	/**
	 * Schedule the minimap for recalculation.
	 * Useful if any terrain in the map has changed.
	 */
	void recalculate_minimap() {minimap_ = NULL; redrawMinimap_ = true; };

	/**
	 * Schedule the minimap to be redrawn.
	 * Useful if units have moved about on the map.
	 */
	void redraw_minimap() { redrawMinimap_ = true; }

	/**
	 * Set what will be shown for the report with type which_report.
	 * Note that this only works for some reports,
	 * i.e. reports that can not be deducted
	 * from the supplied arguments to generate_report,
	 * currently: SELECTED_TERRAIN, EDIT_LEFT_BUTTON_FUNCTION
	 */
	void set_report_content(const reports::TYPE which_report, const std::string &content);
	std::map<reports::TYPE, std::string> get_report_contents() {return report_;};

protected:
	/** Clear the screen contents */
	void clear_screen();

	/**
	 * Called near the beginning of each draw() call.
	 * Derived classes can use this to add extra actions before redrawing
	 * invalidated hexes takes place. No action here by default.
	 */
	virtual void pre_draw() {}

	/**
	 * Get the clipping rectangle for drawing.
	 * Virtual since the editor might use a slightly different approach.
	 */
	virtual const SDL_Rect& get_clip_rect();

	/**
	 * Only called when there's actual redrawing to do. Loops through
	 * invalidated locations and redraws them. Derived classes can override
	 * this, possibly to insert pre- or post-processing around a call to the
	 * base class's function.
	 */
	virtual void draw_invalidated();

	/**
	 * Hook for actions to take right after draw() calls drawing_buffer_commit
	 * No action here by default.
	 */
	virtual void post_commit() {}

	/**
	 * Redraws a single gamemap location.
	 */
	virtual void draw_hex(const map_location& loc);

	/**
	 * @returns the image type to be used for the passed hex
	 * (mostly to do with brightening like for mouseover)
	 */
	virtual image::TYPE get_image_type(const map_location& loc);

	/**
	 * Update time of day member to the tod to be used in the current drawing
	 */
	virtual void update_time_of_day();

	/**
	 * Called near the end of a draw operation, derived classes can use this
	 * to render a specific sidebar. Very similar to post_commit.
	 */
	virtual void draw_sidebar();

	/**
	 * Draws the border tile overlay.
	 * The routine determines by itself which border it is on
	 * and draws an overlay accordingly. The definition of the
	 * border is stored in the 'main_map_border' part of the theme.
	 *
	 * @param loc	the map location of the tile
	 * @param xpos	the on-screen pixels x coordinate of the tile
	 * @param ypos	the on-screen pixels y coordinate of the tile
	 */
	virtual void draw_border(const map_location& loc,
		const int xpos, const int ypos);

	void draw_minimap();

	enum ADJACENT_TERRAIN_TYPE { ADJACENT_BACKGROUND, ADJACENT_FOREGROUND, ADJACENT_FOGSHROUD };

	std::vector<surface> get_terrain_images(const map_location &loc,
					const std::string& timeid,
					image::TYPE type,
					ADJACENT_TERRAIN_TYPE terrain_type);

	std::vector<std::string> get_fog_shroud_graphics(const map_location& loc);

	void draw_image_for_report(surface& img, SDL_Rect& rect);

	void scroll_to_xy(int screenxpos, int screenypos, SCROLL_TYPE scroll_type,bool force = true);

	CVideo& screen_;
	const gamemap* map_;
	const team *viewpoint_;
	int xpos_, ypos_;
	theme theme_;
	int zoom_;
	static int last_zoom_;
	boost::scoped_ptr<terrain_builder> builder_;
	surface minimap_;
	SDL_Rect minimap_location_;
	bool redrawMinimap_;
	bool redraw_background_;
	bool invalidateAll_;
	bool grid_;
	int diagnostic_label_;
	bool panelsDrawn_;
	double turbo_speed_;
	bool turbo_;
	bool invalidateGameStatus_;
	boost::scoped_ptr<map_labels> map_labels_;
	std::string shroud_image_;
	std::string fog_image_;
	time_of_day tod_;

	/** Event raised when the map is being scrolled */
	mutable events::generic_event scroll_event_;

	/**
	 * Holds the tick count for when the next drawing event is scheduled.
	 * Drawing shouldn't occur before this time.
	 */
	int nextDraw_;

	// Not set by the initializer:
	SDL_Rect reportRects_[reports::NUM_REPORTS];
	surface reportSurfaces_[reports::NUM_REPORTS];
	reports::report reports_[reports::NUM_REPORTS];
	std::map<reports::TYPE, std::string> report_;
	std::vector<gui::button> buttons_;
	std::set<map_location> invalidated_;
	std::set<map_location> previous_invalidated_;
	surface selected_hex_overlay_;
	surface mouseover_hex_overlay_;
	map_location selectedHex_;
	map_location mouseoverHex_;
	CKey keys_;

public:
	/** Helper structure for rendering the terrains. */
	struct tblit{
		tblit(const int x, const int y) :
			x(x),
			y(y),
			surf(),
			clip()
			{}

		tblit(const int x, const int y, const surface& surf,
				const SDL_Rect& clip = SDL_Rect()) :
			x(x),
			y(y),
			surf(1, surf),
			clip(clip)
			{}

		tblit(const int x, const int y, const std::vector<surface>& surf,
				const SDL_Rect& clip = SDL_Rect()) :
			x(x),
			y(y),
			surf(surf),
			clip(clip)
			{}


		int x;                      /**< x screen coordinate to render at. */
		int y;                      /**< y screen coordinate to render at. */
		std::vector<surface> surf;  /**< surface(s) to render. */
		SDL_Rect clip;              /**<
		                             * The clipping area of the source if
		                             * ommitted the entire source is used.
		                             */
	};

	/**
	 * The layers to render something on. This value should never be stored
	 * it's the internal drawing order and adding removing and reordering
	 * the layers should be safe.
	 * If needed in WML use the name and map that to the enum value.
	 */
	enum tdrawing_layer{
		LAYER_TERRAIN_BG,          /**<
		                            * Layer for the terrain drawn behind the
		                            * unit.
		                            */
		LAYER_TERRAIN_TMP_BG,      /**<
		                            * Layer which holds stuff that needs to be
		                            * sorted out further, but under units.
		                            */
		LAYER_UNIT_FIRST,          /**< Reserve layeres to be selected for WML. */
		LAYER_UNIT_BG = LAYER_UNIT_FIRST+10,             /**< Used for the ellipse behind the unit. */
		LAYER_UNIT_DEFAULT=LAYER_UNIT_FIRST+40,/**<default layer for drawing units */
		LAYER_TERRAIN_FG = LAYER_UNIT_FIRST+50, /**<
		                            * Layer for the terrain drawn in front of
		                            * the unit.
		                            */
		LAYER_UNIT_MOVE_DEFAULT=LAYER_UNIT_FIRST+60/**<default layer for drawing moving units */,
		LAYER_UNIT_FG =  LAYER_UNIT_FIRST+80, /**<
		                            * Used for the ellipse in front of the
		                            * unit.
		                            */
		LAYER_UNIT_MISSILE_DEFAULT = LAYER_UNIT_FIRST+90, /**< default layer for missile frames*/
		LAYER_UNIT_LAST=LAYER_UNIT_FIRST+100,
		LAYER_REACHMAP,            /**< "black stripes" on unreachable hexes. */
		LAYER_FOG_SHROUD,          /**< Fog and shroud. */
		LAYER_UNIT_BAR,            /**<
		                            * Unit bars and overlays are drawn on this
		                            * layer (for testing here).
		                            */
		LAYER_ATTACK_INDICATOR,    /**< Layer which holds the attack indicator. */
		LAYER_MOVE_INFO,           /**< Movement info (defense%, ect...). */
		LAYER_LINGER_OVERLAY,      /**< The overlay used for the linger mode. */
		LAYER_BORDER,              /**< The border of the map. */

		LAYER_LAST_LAYER           /**<
		                            * Don't draw to this layer it's a dummy to
		                            * size the vector.
		                            */
		};

	/**
	 * Draw the image of a unit at a certain location.
	 * x,y: pixel location on screen to draw the unit
	 * image: the image of the unit
	 * reverse: if the unit should be flipped across the x axis
	 * greyscale: used when the unit is petrified
	 * alpha: the merging to use with the background
	 * blendto: blend to this colour using blend_ratio
	 * submerged: the amount of the unit out of 1.0 that is submerged
	 *            (presumably under water) and thus shouldn't be drawn
	 */
	void render_unit_image(int x, int y, const display::tdrawing_layer drawing_layer,
			const map_location& loc, surface image,
			bool hreverse=false, bool greyscale=false,
			fixed_t alpha=ftofxp(1.0), Uint32 blendto=0,
			double blend_ratio=0, double submerged=0.0,bool vreverse =false);

	/**
	 * Draw text on a hex. (0.5, 0.5) is the center.
	 * The font size is adjusted to the zoom factor
	 * and divided by 2 for tiny-gui.
	 */
	void draw_text_in_hex(const map_location& loc,
		const tdrawing_layer layer, const std::string& text, size_t font_size,
		SDL_Color color, double x_in_hex=0.5, double y_in_hex=0.5);

protected:

	// Initially tdrawing_buffer was a vector but profiling showed that a map
	// was more efficient. Tested with the LAYER_UNIT_LAST for various values
	// and different types the results were. (Tested with oprofile.)

 	// container    unit layers    counts
	// vector       100            3748
	// vector       10000          147338
	// map          10000          3362

	// Since a map with 10000 items was more efficient I didn't test the map
	// with 100 items. I want to retest once more is converted, since with
	// a different usage it numbers might differ so the old code is disabled
	// with TDRAWING_BUFFER_USES_VECTOR
	//     20080308 -- Mordante

	/**
	 * Compare functor for the blitting.
	 *
	 * The blitting order must be sorted on y value first instead of x so added
	 * another functor since map_location::operator< sorts on x value.
	 */
	struct draw_order
	{
		bool operator()(const map_location& lhs, const map_location& rhs) const
		{
			return lhs.y < rhs.y || (lhs.y == rhs.y && lhs.x < rhs.x);
		}
	};

	/**
	 * * Surfaces are rendered per level in a map.
	 * * Per level the items are rendered per location these locations are
	 *   stored in the drawing order required for units.
	 * * every location has a vector with surfaces, each with its own screen
	 *   coordinate to render at.
	 * * every vector element has a vector with surfaces to render.
	 */
	typedef std::map<map_location, std::vector<tblit>, draw_order> tblit_map;
#if TDRAWING_BUFFER_USES_VECTOR
	typedef std::vector<tblit_map> tdrawing_buffer;
#else
	typedef std::map<tdrawing_layer, tblit_map> tdrawing_buffer;
#endif
	tdrawing_buffer drawing_buffer_;

public:

	/**
	 * Add an item to the drawing buffer.
	 *
	 * @param layer              The layer to draw on.
	 * @param loc                The hex the image belongs to, needed for the
	 *                           drawing order.
	 * @param blit               The structure to blit.
	 */
	void drawing_buffer_add(const tdrawing_layer layer,
			const map_location& loc, const tblit& blit)
	{
		drawing_buffer_[layer][loc].push_back(blit);
	}

protected:

	/** Draws the drawing_buffer_ and clears it. */
	void drawing_buffer_commit();

	/** Clears the drawing buffer. */
	void drawing_buffer_clear();

	/** redraw all panels associated with the map display */
	void draw_all_panels();

	/**
	 * Strict weak ordering to sort a STL-set of hexes
	 * for drawing using the z-order.
	 * (1000 are just to weight the y compare to x)
	 */
	struct ordered_draw : public std::binary_function<map_location, map_location, bool> {
		bool operator()(map_location a, map_location b) {
			return (a.y*2 + a.x%2) * 1024 + a.x < (b.y*2 + b.x%2) * 1024 + b.x;
		}
	};

	/**
	 * Initiate a redraw.
	 *
	 * Invalidate controls and panels when changed after they have been drawn
	 * initially. Useful for dynamic theme modification.
	 */
	bool draw_init();
	void draw_wrap(bool update,bool force,bool changed);

	/** Used to indicate to drawing funtions that we are doing a map screenshot */
	bool map_screenshot_;

private:
	/** Handle for the label which displays frames per second. */
	int fps_handle_;
	/** Count work done for the debug info displayed under fps */
	int invalidated_hexes_;
	int drawn_hexes_;

	bool idle_anim_;
	double idle_anim_rate_;

	surface map_screenshot_surf_;

	std::vector<boost::function<void(display&)> > redraw_observers_;

	/** Debug flag - overlay x,y coords on tiles */
	bool draw_coordinates_;
	/** Debug flag - overlay terrain codes on tiles */
	bool draw_terrain_codes_;
};

#endif

