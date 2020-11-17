#include "DampedSpringJoint2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


DampedSpringJoint2D::___method_bindings DampedSpringJoint2D::___mb = {};

void DampedSpringJoint2D::___init_method_bindings() {
	___mb.mb_get_damping = godot::api->godot_method_bind_get_method("DampedSpringJoint2D", "get_damping");
	___mb.mb_get_length = godot::api->godot_method_bind_get_method("DampedSpringJoint2D", "get_length");
	___mb.mb_get_rest_length = godot::api->godot_method_bind_get_method("DampedSpringJoint2D", "get_rest_length");
	___mb.mb_get_stiffness = godot::api->godot_method_bind_get_method("DampedSpringJoint2D", "get_stiffness");
	___mb.mb_set_damping = godot::api->godot_method_bind_get_method("DampedSpringJoint2D", "set_damping");
	___mb.mb_set_length = godot::api->godot_method_bind_get_method("DampedSpringJoint2D", "set_length");
	___mb.mb_set_rest_length = godot::api->godot_method_bind_get_method("DampedSpringJoint2D", "set_rest_length");
	___mb.mb_set_stiffness = godot::api->godot_method_bind_get_method("DampedSpringJoint2D", "set_stiffness");
}

DampedSpringJoint2D *DampedSpringJoint2D::_new()
{
	return (DampedSpringJoint2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"DampedSpringJoint2D")());
}
real_t DampedSpringJoint2D::get_damping() const {
	return ___godot_icall_float(___mb.mb_get_damping, (const Object *) this);
}

real_t DampedSpringJoint2D::get_length() const {
	return ___godot_icall_float(___mb.mb_get_length, (const Object *) this);
}

real_t DampedSpringJoint2D::get_rest_length() const {
	return ___godot_icall_float(___mb.mb_get_rest_length, (const Object *) this);
}

real_t DampedSpringJoint2D::get_stiffness() const {
	return ___godot_icall_float(___mb.mb_get_stiffness, (const Object *) this);
}

void DampedSpringJoint2D::set_damping(const real_t damping) {
	___godot_icall_void_float(___mb.mb_set_damping, (const Object *) this, damping);
}

void DampedSpringJoint2D::set_length(const real_t length) {
	___godot_icall_void_float(___mb.mb_set_length, (const Object *) this, length);
}

void DampedSpringJoint2D::set_rest_length(const real_t rest_length) {
	___godot_icall_void_float(___mb.mb_set_rest_length, (const Object *) this, rest_length);
}

void DampedSpringJoint2D::set_stiffness(const real_t stiffness) {
	___godot_icall_void_float(___mb.mb_set_stiffness, (const Object *) this, stiffness);
}

}