#include "Skin.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Skin::___method_bindings Skin::___mb = {};

void Skin::___init_method_bindings() {
	___mb.mb_add_bind = godot::api->godot_method_bind_get_method("Skin", "add_bind");
	___mb.mb_clear_binds = godot::api->godot_method_bind_get_method("Skin", "clear_binds");
	___mb.mb_get_bind_bone = godot::api->godot_method_bind_get_method("Skin", "get_bind_bone");
	___mb.mb_get_bind_count = godot::api->godot_method_bind_get_method("Skin", "get_bind_count");
	___mb.mb_get_bind_pose = godot::api->godot_method_bind_get_method("Skin", "get_bind_pose");
	___mb.mb_set_bind_bone = godot::api->godot_method_bind_get_method("Skin", "set_bind_bone");
	___mb.mb_set_bind_count = godot::api->godot_method_bind_get_method("Skin", "set_bind_count");
	___mb.mb_set_bind_pose = godot::api->godot_method_bind_get_method("Skin", "set_bind_pose");
}

Skin *Skin::_new()
{
	return (Skin *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Skin")());
}
void Skin::add_bind(const int64_t bone, const Transform pose) {
	___godot_icall_void_int_Transform(___mb.mb_add_bind, (const Object *) this, bone, pose);
}

void Skin::clear_binds() {
	___godot_icall_void(___mb.mb_clear_binds, (const Object *) this);
}

int64_t Skin::get_bind_bone(const int64_t bind_index) const {
	return ___godot_icall_int_int(___mb.mb_get_bind_bone, (const Object *) this, bind_index);
}

int64_t Skin::get_bind_count() const {
	return ___godot_icall_int(___mb.mb_get_bind_count, (const Object *) this);
}

Transform Skin::get_bind_pose(const int64_t bind_index) const {
	return ___godot_icall_Transform_int(___mb.mb_get_bind_pose, (const Object *) this, bind_index);
}

void Skin::set_bind_bone(const int64_t bind_index, const int64_t bone) {
	___godot_icall_void_int_int(___mb.mb_set_bind_bone, (const Object *) this, bind_index, bone);
}

void Skin::set_bind_count(const int64_t bind_count) {
	___godot_icall_void_int(___mb.mb_set_bind_count, (const Object *) this, bind_count);
}

void Skin::set_bind_pose(const int64_t bind_index, const Transform pose) {
	___godot_icall_void_int_Transform(___mb.mb_set_bind_pose, (const Object *) this, bind_index, pose);
}

}