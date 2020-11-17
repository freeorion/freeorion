#include "AnimationNodeAdd3.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AnimationNodeAdd3::___method_bindings AnimationNodeAdd3::___mb = {};

void AnimationNodeAdd3::___init_method_bindings() {
	___mb.mb_is_using_sync = godot::api->godot_method_bind_get_method("AnimationNodeAdd3", "is_using_sync");
	___mb.mb_set_use_sync = godot::api->godot_method_bind_get_method("AnimationNodeAdd3", "set_use_sync");
}

AnimationNodeAdd3 *AnimationNodeAdd3::_new()
{
	return (AnimationNodeAdd3 *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimationNodeAdd3")());
}
bool AnimationNodeAdd3::is_using_sync() const {
	return ___godot_icall_bool(___mb.mb_is_using_sync, (const Object *) this);
}

void AnimationNodeAdd3::set_use_sync(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_sync, (const Object *) this, enable);
}

}