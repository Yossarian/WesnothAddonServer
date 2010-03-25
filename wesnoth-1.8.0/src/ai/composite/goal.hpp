/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/composite/goal.hpp
 */

#ifndef AI_COMPOSITE_GOAL_HPP_INCLUDED
#define AI_COMPOSITE_GOAL_HPP_INCLUDED


#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif


#include "../../global.hpp"

#include "../contexts.hpp"
#include "../game_info.hpp"
#include "../../terrain_filter.hpp"

//included for 'target' markers
#include "../default/contexts.hpp"

#include "component.hpp"

#include <map>
#include <stack>
#include <vector>
#include <deque>

namespace ai {

class goal : public readonly_context_proxy, public component {
public:
	goal(readonly_context &context, const config &cfg);


	virtual ~goal();


	virtual void add_targets(std::back_insert_iterator< std::vector< target > > target_list);


	virtual config to_config() const;


	virtual void on_create();


	bool active() const;


	const std::string& get_id() const;


	const std::string& get_name() const;


	const std::string& get_engine() const;


	bool redeploy(const config &cfg);


protected:
	config cfg_;


};


class target_unit_goal : public goal {
public:
	target_unit_goal(readonly_context &context, const config &cfg);


	virtual void add_targets(std::back_insert_iterator< std::vector< target > > target_list);


	virtual void on_create();

private:
	virtual bool matches_unit(unit_map::const_iterator u);

	double value() const
	{
		return value_;
	}
	double value_;
};


class protect_goal : public goal {
public:
	protect_goal(readonly_context &context, const config &cfg, bool protect_only_own_unit, bool protect_unit);


	virtual void add_targets(std::back_insert_iterator< std::vector< target > > target_list);


	virtual void on_create();

private:

	double value()
	{
		return value_;
	}

	boost::shared_ptr<terrain_filter> filter_ptr_;
	bool protect_only_own_unit_;
	bool protect_unit_;
	int radius_;
	double value_;
};


class protect_location_goal : public protect_goal {
public:
	protect_location_goal(readonly_context &context, const config &cfg)
	: protect_goal(context,cfg,false,false)
	{
	}
};


class protect_unit_goal : public protect_goal {
public:
	protect_unit_goal(readonly_context &context, const config &cfg)
	: protect_goal(context,cfg,false,true)
	{
	}
};


class protect_my_unit_goal : public protect_goal {
public:
	protect_my_unit_goal(readonly_context &context, const config &cfg)
	: protect_goal(context,cfg,true,true)
	{
	}
};


class goal_factory{
public:
	typedef boost::shared_ptr< goal_factory > factory_ptr;
	typedef std::map<std::string, factory_ptr> factory_map;
	typedef std::pair<const std::string, factory_ptr> factory_map_pair;

	static factory_map& get_list() {
		static factory_map *goal_factories;
		if (goal_factories==NULL) {
			goal_factories = new factory_map;
		}
		return *goal_factories;
	}

	virtual goal_ptr get_new_instance( readonly_context &context, const config &cfg ) = 0;

	goal_factory( const std::string &name )
	{
		factory_ptr ptr_to_this(this);
		get_list().insert(make_pair(name,ptr_to_this));
	}

	virtual ~goal_factory() {}
};


template<class GOAL>
class register_goal_factory : public goal_factory {
public:
	register_goal_factory( const std::string &name )
		: goal_factory( name )
	{
	}

	virtual goal_ptr get_new_instance( readonly_context &context, const config &cfg ){
		goal_ptr a(new GOAL(context,cfg));
		a->on_create();
		return a;
	}
};


} //end of namespace ai


#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
