#include "AnimationNodeAnimation.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AnimationNodeAnimation::___method_bindings AnimationNodeAnimation::___mb = {};

void AnimationNodeAnimation::___init_method_bindings() {
	___mb.mb_get_animation = godot::api->godot_method_bind_get_method("AnimationNodeAnimation", "get_animation");
	___mb.mb_set_animation = godot::api->godot_method_bind_get_method("AnimationNodeAnimation", "set_animation");
}

AnimationNodeAnimation *AnimationNodeAnimation::_new()
{
	return (AnimationNodeAnimation *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimationNodeAnimation")());
}
String AnimationNodeAnimation::get_animation() const {
	return ___godot_icall_String(___mb.mb_get_animation, (const Object *) this);
}

void AnimationNodeAnimation::set_animation(const String name) {
	___godot_icall_void_String(___mb.mb_set_animation, (const Object *) this, name);
}

}