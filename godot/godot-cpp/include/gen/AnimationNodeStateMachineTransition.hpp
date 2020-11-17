#ifndef GODOT_CPP_ANIMATIONNODESTATEMACHINETRANSITION_HPP
#define GODOT_CPP_ANIMATIONNODESTATEMACHINETRANSITION_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "AnimationNodeStateMachineTransition.hpp"

#include "Resource.hpp"
namespace godot {


class AnimationNodeStateMachineTransition : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_get_advance_condition;
		godot_method_bind *mb_get_priority;
		godot_method_bind *mb_get_switch_mode;
		godot_method_bind *mb_get_xfade_time;
		godot_method_bind *mb_has_auto_advance;
		godot_method_bind *mb_is_disabled;
		godot_method_bind *mb_set_advance_condition;
		godot_method_bind *mb_set_auto_advance;
		godot_method_bind *mb_set_disabled;
		godot_method_bind *mb_set_priority;
		godot_method_bind *mb_set_switch_mode;
		godot_method_bind *mb_set_xfade_time;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AnimationNodeStateMachineTransition"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum SwitchMode {
		SWITCH_MODE_IMMEDIATE = 0,
		SWITCH_MODE_SYNC = 1,
		SWITCH_MODE_AT_END = 2,
	};

	// constants


	static AnimationNodeStateMachineTransition *_new();

	// methods
	String get_advance_condition() const;
	int64_t get_priority() const;
	AnimationNodeStateMachineTransition::SwitchMode get_switch_mode() const;
	real_t get_xfade_time() const;
	bool has_auto_advance() const;
	bool is_disabled() const;
	void set_advance_condition(const String name);
	void set_auto_advance(const bool auto_advance);
	void set_disabled(const bool disabled);
	void set_priority(const int64_t priority);
	void set_switch_mode(const int64_t mode);
	void set_xfade_time(const real_t secs);

};

}

#endif