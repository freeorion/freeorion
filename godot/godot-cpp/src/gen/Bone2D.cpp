#include "Bone2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Bone2D::___method_bindings Bone2D::___mb = {};

void Bone2D::___init_method_bindings() {
	___mb.mb_apply_rest = godot::api->godot_method_bind_get_method("Bone2D", "apply_rest");
	___mb.mb_get_default_length = godot::api->godot_method_bind_get_method("Bone2D", "get_default_length");
	___mb.mb_get_index_in_skeleton = godot::api->godot_method_bind_get_method("Bone2D", "get_index_in_skeleton");
	___mb.mb_get_rest = godot::api->godot_method_bind_get_method("Bone2D", "get_rest");
	___mb.mb_get_skeleton_rest = godot::api->godot_method_bind_get_method("Bone2D", "get_skeleton_rest");
	___mb.mb_set_default_length = godot::api->godot_method_bind_get_method("Bone2D", "set_default_length");
	___mb.mb_set_rest = godot::api->godot_method_bind_get_method("Bone2D", "set_rest");
}

Bone2D *Bone2D::_new()
{
	return (Bone2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Bone2D")());
}
void Bone2D::apply_rest() {
	___godot_icall_void(___mb.mb_apply_rest, (const Object *) this);
}

real_t Bone2D::get_default_length() const {
	return ___godot_icall_float(___mb.mb_get_default_length, (const Object *) this);
}

int64_t Bone2D::get_index_in_skeleton() const {
	return ___godot_icall_int(___mb.mb_get_index_in_skeleton, (const Object *) this);
}

Transform2D Bone2D::get_rest() const {
	return ___godot_icall_Transform2D(___mb.mb_get_rest, (const Object *) this);
}

Transform2D Bone2D::get_skeleton_rest() const {
	return ___godot_icall_Transform2D(___mb.mb_get_skeleton_rest, (const Object *) this);
}

void Bone2D::set_default_length(const real_t default_length) {
	___godot_icall_void_float(___mb.mb_set_default_length, (const Object *) this, default_length);
}

void Bone2D::set_rest(const Transform2D rest) {
	___godot_icall_void_Transform2D(___mb.mb_set_rest, (const Object *) this, rest);
}

}