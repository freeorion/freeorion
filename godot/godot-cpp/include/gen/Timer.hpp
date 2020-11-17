#ifndef GODOT_CPP_TIMER_HPP
#define GODOT_CPP_TIMER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Timer.hpp"

#include "Node.hpp"
namespace godot {


class Timer : public Node {
	struct ___method_bindings {
		godot_method_bind *mb_get_time_left;
		godot_method_bind *mb_get_timer_process_mode;
		godot_method_bind *mb_get_wait_time;
		godot_method_bind *mb_has_autostart;
		godot_method_bind *mb_is_one_shot;
		godot_method_bind *mb_is_paused;
		godot_method_bind *mb_is_stopped;
		godot_method_bind *mb_set_autostart;
		godot_method_bind *mb_set_one_shot;
		godot_method_bind *mb_set_paused;
		godot_method_bind *mb_set_timer_process_mode;
		godot_method_bind *mb_set_wait_time;
		godot_method_bind *mb_start;
		godot_method_bind *mb_stop;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Timer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum TimerProcessMode {
		TIMER_PROCESS_PHYSICS = 0,
		TIMER_PROCESS_IDLE = 1,
	};

	// constants


	static Timer *_new();

	// methods
	real_t get_time_left() const;
	Timer::TimerProcessMode get_timer_process_mode() const;
	real_t get_wait_time() const;
	bool has_autostart() const;
	bool is_one_shot() const;
	bool is_paused() const;
	bool is_stopped() const;
	void set_autostart(const bool enable);
	void set_one_shot(const bool enable);
	void set_paused(const bool paused);
	void set_timer_process_mode(const int64_t mode);
	void set_wait_time(const real_t time_sec);
	void start(const real_t time_sec = -1);
	void stop();

};

}

#endif