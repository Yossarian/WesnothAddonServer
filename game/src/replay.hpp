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
 *  @file replay.hpp
 *  Replay control code.
 */

#ifndef REPLAY_H_INCLUDED
#define REPLAY_H_INCLUDED

#include "config.hpp"
#include "map_location.hpp"
#include "rng.hpp"

#include <deque>

class game_display;
class terrain_label;
class unit_map;
class play_controller;
struct time_of_day;

class replay: public rand_rng::rng
{
public:
	replay();
	explicit replay(const config& cfg);

	void append(const config& cfg);

	void set_skip(bool skip);
	bool is_skipping() const;

	void add_start();
	void add_recruit(int unit_index, const map_location& loc);
	void add_recall(const std::string& unit_id, const map_location& loc);
	void add_disband(const std::string& unit_id);
	void add_countdown_update(int value,int team);
	void add_movement(const std::vector<map_location>& steps);
	void add_attack(const map_location& a, const map_location& b,
		int att_weapon, int def_weapon, const std::string& attacker_type_id,
		const std::string& defender_type_id, int attacker_lvl,
		int defender_lvl, const size_t turn, const time_of_day &t);
	void add_seed(const char* child_name, rand_rng::seed_t seed);
	void choose_option(int index);
	void text_input(std::string input);
	void set_random_value(const std::string& choice);
	void add_label(const terrain_label*);
	void clear_labels(const std::string&);
	void add_rename(const std::string& name, const map_location& loc);
	void init_side();
	void end_turn();
	void add_event(const std::string& name,
		const map_location& loc=map_location::null_location);
	void add_unit_checksum(const map_location& loc,config* const cfg);
	void add_checksum_check(const map_location& loc);
	void add_log_data(const std::string &key, const std::string &var);
	void add_log_data(const std::string &category, const std::string &key, const std::string &var);
	void add_log_data(const std::string &category, const std::string &key, const config& c);

	/**
	 * Mark an expected advancement adding it to the queue
	 */
	void add_expected_advancement(const map_location& loc);

	/**
	 * Access to the expected advancements queue.
	 */
	const std::deque<map_location>& expected_advancements() const;

	/**
	 * Remove the front expected advancement from the queue
	 */
	void pop_expected_advancement();

	/**
	 * Adds an advancement to the replay, the following option command
	 * determines which advancement option has been choosen
	 */
	void add_advancement(const map_location& loc);

	void add_chat_message_location();
	void speak(const config& cfg);
	std::string build_chat_log();

	//get data range will get a range of moves from the replay system.
	//if data_type is 'ALL_DATA' then it will return all data in this range
	//except for undoable data that has already been sent. If data_type is
	//NON_UNDO_DATA, then it will only retrieve undoable data, and will mark
	//it as already sent.
	//undoable data includes moves such as placing a label or speaking, which is
	//ignored by the undo system.
	enum DATA_TYPE { ALL_DATA, NON_UNDO_DATA };
	config get_data_range(int cmd_start, int cmd_end, DATA_TYPE data_type=ALL_DATA);
	config get_last_turn(int num_turns=1);
	const config& get_replay_data() const { return cfg_; }

	void undo();

	void start_replay();
	void revert_action();
	config* get_next_action();
	void pre_replay();

	bool at_end() const;
	void set_to_end();

	void clear();
	bool empty();

	enum MARK_SENT { MARK_AS_UNSENT, MARK_AS_SENT };
	void add_config(const config& cfg, MARK_SENT mark=MARK_AS_UNSENT);

	int ncommands();

	static void process_error(const std::string& msg);

private:
	//generic for add_movement and add_attack
	void add_pos(const std::string& type,
	             const map_location& a, const map_location& b);

	void add_value(const std::string& type, int value);

	void add_chat_log_entry(const config &speak, std::ostream &str) const;

	const config::child_list& commands() const;
	void remove_command(int);
	/** Adds a new empty command to the command list.
	 *
	 * @param update_random_context  If set to false, do not update the
	 *           random context variables: all random generation will take
	 *           place in the previous random context. Used for commands
	 *           for which "random context" is pointless, and which can be
	 *           issued while some other commands are still taking place,
	 *           like, for example, messages during combats.
	 *
	 * @return a pointer to the added command
	 */
	config* add_command(bool update_random_context=true);
	config cfg_;
	unsigned int pos_;

	config* current_;

	bool skip_;

	std::vector<int> message_locations;

	/**
	 * A queue of units (locations) that are supposed to advance but the
	 * relevant advance (choice) message has not yet been received
	 */
	std::deque<map_location> expected_advancements_;
};

replay& get_replay_source();

extern replay recorder;

//replays up to one turn from the recorder object
//returns true if it got to the end of the turn without data running out
bool do_replay(int side_num, replay *obj = NULL);

bool do_replay_handle(int side_num, const std::string &do_untill);

//an object which can be made to undo a recorded move
//unless the transaction is confirmed
struct replay_undo
{
	replay_undo(replay& obj) : obj_(&obj) {}
	~replay_undo() { if(obj_) obj_->undo(); }
	void confirm_transaction() { obj_ = NULL; }

private:
	replay* obj_;
};

class replay_network_sender
{
public:
	replay_network_sender(replay& obj);
	~replay_network_sender();

	void sync_non_undoable();
	void commit_and_sync();
private:
	replay& obj_;
	int upto_;
};

#endif
