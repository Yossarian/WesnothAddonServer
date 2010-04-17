/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_SLIDER_HPP_INCLUDED
#define GUI_WIDGETS_SLIDER_HPP_INCLUDED

#include "gui/widgets/integer_selector.hpp"
#include "gui/widgets/scrollbar.hpp"

namespace gui2 {

/** A slider. */
class tslider : public tscrollbar_, public tinteger_selector_
{
public:

	tslider() :
		tscrollbar_(),
		best_slider_length_(0),
		minimum_value_(0),
		minimum_value_label_(),
		maximum_value_label_(),
		value_labels_()
	{
	}

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

private:
	/** Inherited from tcontrol. */
	tpoint calculate_best_size() const;
public:

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** Inherited from tinteger_selector_. */
	void set_value(const int value);

	/** Inherited from tinteger_selector_. */
	int get_value() const
		{ return minimum_value_ + get_item_position() * get_step_size(); }

	/** Inherited from tinteger_selector_. */
	void set_minimum_value(const int minimum_value);

	/** Inherited from tinteger_selector_. */
	int get_minimum_value() const { return minimum_value_; }

	/** Inherited from tinteger_selector_. */
	void set_maximum_value(const int maximum_value);

	/** Inherited from tinteger_selector_. */
	int get_maximum_value() const
		// The number of items needs to include the begin and end so count - 1.
		{ return minimum_value_ + get_item_count() - 1; }

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_best_slider_length(const unsigned length)
		{ best_slider_length_ = length; set_dirty(); }

	void set_minimum_value_label(const t_string& minimum_value_label)
		{ minimum_value_label_ = minimum_value_label; }

	void set_maximum_value_label(const t_string& maximum_value_label)
		{ maximum_value_label_ = maximum_value_label; }

	void set_value_labels(const std::vector<t_string>& value_labels)
		{ value_labels_ = value_labels; }

	/**
	 * Returns the label shown for the current value.
	 *
	 * @returns                   The label for the current value, if no label
	 *                            for the current label is defined, it returns
	 *                            the result of get_value().
	 */
	t_string get_value_label() const;
protected:

	/** Inherited from tscrollbar. */
	void child_callback_positioner_moved();

private:

	/** The best size for the slider part itself, if 0 ignored. */
	unsigned best_slider_length_;

	/**
	 * The minimum value the slider holds.
	 *
	 * The maximum value is minimum + item_count_.
	 * The current value is minimum + item_position_.
	 */
	int minimum_value_;

	/** Inherited from tscrollbar. */
	unsigned get_length() const { return get_width(); }

	/** Inherited from tscrollbar. */
	unsigned minimum_positioner_length() const;

	/** Inherited from tscrollbar. */
	unsigned maximum_positioner_length() const;

	/** Inherited from tscrollbar. */
	unsigned offset_before() const;

	/** Inherited from tscrollbar. */
	unsigned offset_after() const;

	/** Inherited from tscrollbar. */
	bool on_positioner(const tpoint& coordinate) const;

	/** Inherited from tscrollbar. */
	int on_bar(const tpoint& coordinate) const;

	/** Inherited from tscrollbar. */
	int get_length_difference(const tpoint& original, const tpoint& current) const
		{ return current.x - original.x; }

	/** Inherited from tscrollbar. */
	void update_canvas();

	/**
	 * When the slider shows the minimum value can show a special text.
	 * If this text is not empty this text is shown else the minimum value.
	 */
	t_string minimum_value_label_;

	/**
	 * When the slider shows the maximum value can show a special text.
	 * If this text is not empty this text is shown else the maximum value.
	 */
	t_string maximum_value_label_;

	/**
	 * This allows the slider to show custom texts instead of the values.
	 * This vector should have the same amount of items as options for the
	 * sliders. When set these texts are shown instead of the values. It also
	 * overrides minimum_value_label_ and maximum_value_label_.
	 */
	std::vector<t_string> value_labels_;

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const;
};

} // namespace gui2

#endif

