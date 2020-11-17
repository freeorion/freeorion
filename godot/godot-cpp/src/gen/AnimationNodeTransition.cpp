#include "AnimationNodeTransition.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AnimationNodeTransition::___method_bindings AnimationNodeTransition::___mb = {};

void AnimationNodeTransition::___init_method_bindings() {
	___mb.mb_get_cross_fade_time = godot::api->godot_method_bind_get_method("AnimationNodeTransition", "get_cross_fade_time");
	___mb.mb_get_enabled_inputs = godot::api->godot_method_bind_get_method("AnimationNodeTransition", "get_enabled_inputs");
	___mb.mb_get_input_caption = godot::api->godot_method_bind_get_method("AnimationNodeTransition", "get_input_caption");
	___mb.mb_is_input_set_as_auto_advance = godot::api->godot_method_bind_get_method("AnimationNodeTransition", "is_input_set_as_auto_advance");
	___mb.mb_set_cross_fade_time = godot::api->godot_method_bind_get_method("AnimationNodeTransition", "set_cross_fade_time");
	___mb.mb_set_enabled_inputs = godot::api->godot_method_bind_get_method("AnimationNodeTransition", "set_enabled_inputs");
	___mb.mb_set_input_as_auto_advance = godot::api->godot_method_bind_get_method("AnimationNodeTransition", "set_input_as_auto_advance");
	___mb.mb_set_input_caption = godot::api->godot_method_bind_get_method("AnimationNodeTransition", "set_input_caption");
}

AnimationNodeTransition *AnimationNodeTransition::_new()
{
	return (AnimationNodeTransition *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimationNodeTransition")());
}
real_t AnimationNodeTransition::get_cross_fade_time() const {
	return ___godot_icall_float(___mb.mb_get_cross_fade_time, (const Object *) this);
}

int64_t AnimationNodeTransition::get_enabled_inputs() {
	return ___godot_icall_int(___mb.mb_get_enabled_inputs, (const Object *) this);
}

String AnimationNodeTransition::get_input_caption(const int64_t input) const {
	return ___godot_icall_String_int(___mb.mb_get_input_caption, (const Object *) this, input);
}

bool AnimationNodeTransition::is_input_set_as_auto_advance(const int64_t input) const {
	return ___godot_icall_bool_int(___mb.mb_is_input_set_as_auto_advance, (const Object *) this, input);
}

void AnimationNodeTransition::set_cross_fade_time(const real_t time) {
	___godot_icall_void_float(___mb.mb_set_cross_fade_time, (const Object *) this, time);
}

void AnimationNodeTransition::set_enabled_inputs(const int64_t amount) {
	___godot_icall_void_int(___mb.mb_set_enabled_inputs, (const Object *) this, amount);
}

void AnimationNodeTransition::set_input_as_auto_advance(const int64_t input, const bool enable) {
	___godot_icall_void_int_bool(___mb.mb_set_input_as_auto_advance, (const Object *) this, input, enable);
}

void AnimationNodeTransition::set_input_caption(const int64_t input, const String caption) {
	___godot_icall_void_int_String(___mb.mb_set_input_caption, (const Object *) this, input, caption);
}

}