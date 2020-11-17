#include "AnimationNodeBlend3.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AnimationNodeBlend3::___method_bindings AnimationNodeBlend3::___mb = {};

void AnimationNodeBlend3::___init_method_bindings() {
	___mb.mb_is_using_sync = godot::api->godot_method_bind_get_method("AnimationNodeBlend3", "is_using_sync");
	___mb.mb_set_use_sync = godot::api->godot_method_bind_get_method("AnimationNodeBlend3", "set_use_sync");
}

AnimationNodeBlend3 *AnimationNodeBlend3::_new()
{
	return (AnimationNodeBlend3 *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimationNodeBlend3")());
}
bool AnimationNodeBlend3::is_using_sync() const {
	return ___godot_icall_bool(___mb.mb_is_using_sync, (const Object *) this);
}

void AnimationNodeBlend3::set_use_sync(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_sync, (const Object *) this, enable);
}

}