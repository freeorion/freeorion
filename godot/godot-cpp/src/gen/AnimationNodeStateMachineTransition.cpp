#include "AnimationNodeStateMachineTransition.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AnimationNodeStateMachineTransition::___method_bindings AnimationNodeStateMachineTransition::___mb = {};

void AnimationNodeStateMachineTransition::___init_method_bindings() {
	___mb.mb_get_advance_condition = godot::api->godot_method_bind_get_method("AnimationNodeStateMachineTransition", "get_advance_condition");
	___mb.mb_get_priority = godot::api->godot_method_bind_get_method("AnimationNodeStateMachineTransition", "get_priority");
	___mb.mb_get_switch_mode = godot::api->godot_method_bind_get_method("AnimationNodeStateMachineTransition", "get_switch_mode");
	___mb.mb_get_xfade_time = godot::api->godot_method_bind_get_method("AnimationNodeStateMachineTransition", "get_xfade_time");
	___mb.mb_has_auto_advance = godot::api->godot_method_bind_get_method("AnimationNodeStateMachineTransition", "has_auto_advance");
	___mb.mb_is_disabled = godot::api->godot_method_bind_get_method("AnimationNodeStateMachineTransition", "is_disabled");
	___mb.mb_set_advance_condition = godot::api->godot_method_bind_get_method("AnimationNodeStateMachineTransition", "set_advance_condition");
	___mb.mb_set_auto_advance = godot::api->godot_method_bind_get_method("AnimationNodeStateMachineTransition", "set_auto_advance");
	___mb.mb_set_disabled = godot::api->godot_method_bind_get_method("AnimationNodeStateMachineTransition", "set_disabled");
	___mb.mb_set_priority = godot::api->godot_method_bind_get_method("AnimationNodeStateMachineTransition", "set_priority");
	___mb.mb_set_switch_mode = godot::api->godot_method_bind_get_method("AnimationNodeStateMachineTransition", "set_switch_mode");
	___mb.mb_set_xfade_time = godot::api->godot_method_bind_get_method("AnimationNodeStateMachineTransition", "set_xfade_time");
}

AnimationNodeStateMachineTransition *AnimationNodeStateMachineTransition::_new()
{
	return (AnimationNodeStateMachineTransition *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimationNodeStateMachineTransition")());
}
String AnimationNodeStateMachineTransition::get_advance_condition() const {
	return ___godot_icall_String(___mb.mb_get_advance_condition, (const Object *) this);
}

int64_t AnimationNodeStateMachineTransition::get_priority() const {
	return ___godot_icall_int(___mb.mb_get_priority, (const Object *) this);
}

AnimationNodeStateMachineTransition::SwitchMode AnimationNodeStateMachineTransition::get_switch_mode() const {
	return (AnimationNodeStateMachineTransition::SwitchMode) ___godot_icall_int(___mb.mb_get_switch_mode, (const Object *) this);
}

real_t AnimationNodeStateMachineTransition::get_xfade_time() const {
	return ___godot_icall_float(___mb.mb_get_xfade_time, (const Object *) this);
}

bool AnimationNodeStateMachineTransition::has_auto_advance() const {
	return ___godot_icall_bool(___mb.mb_has_auto_advance, (const Object *) this);
}

bool AnimationNodeStateMachineTransition::is_disabled() const {
	return ___godot_icall_bool(___mb.mb_is_disabled, (const Object *) this);
}

void AnimationNodeStateMachineTransition::set_advance_condition(const String name) {
	___godot_icall_void_String(___mb.mb_set_advance_condition, (const Object *) this, name);
}

void AnimationNodeStateMachineTransition::set_auto_advance(const bool auto_advance) {
	___godot_icall_void_bool(___mb.mb_set_auto_advance, (const Object *) this, auto_advance);
}

void AnimationNodeStateMachineTransition::set_disabled(const bool disabled) {
	___godot_icall_void_bool(___mb.mb_set_disabled, (const Object *) this, disabled);
}

void AnimationNodeStateMachineTransition::set_priority(const int64_t priority) {
	___godot_icall_void_int(___mb.mb_set_priority, (const Object *) this, priority);
}

void AnimationNodeStateMachineTransition::set_switch_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_switch_mode, (const Object *) this, mode);
}

void AnimationNodeStateMachineTransition::set_xfade_time(const real_t secs) {
	___godot_icall_void_float(___mb.mb_set_xfade_time, (const Object *) this, secs);
}

}