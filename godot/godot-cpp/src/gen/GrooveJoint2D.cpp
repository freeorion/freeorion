#include "GrooveJoint2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


GrooveJoint2D::___method_bindings GrooveJoint2D::___mb = {};

void GrooveJoint2D::___init_method_bindings() {
	___mb.mb_get_initial_offset = godot::api->godot_method_bind_get_method("GrooveJoint2D", "get_initial_offset");
	___mb.mb_get_length = godot::api->godot_method_bind_get_method("GrooveJoint2D", "get_length");
	___mb.mb_set_initial_offset = godot::api->godot_method_bind_get_method("GrooveJoint2D", "set_initial_offset");
	___mb.mb_set_length = godot::api->godot_method_bind_get_method("GrooveJoint2D", "set_length");
}

GrooveJoint2D *GrooveJoint2D::_new()
{
	return (GrooveJoint2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"GrooveJoint2D")());
}
real_t GrooveJoint2D::get_initial_offset() const {
	return ___godot_icall_float(___mb.mb_get_initial_offset, (const Object *) this);
}

real_t GrooveJoint2D::get_length() const {
	return ___godot_icall_float(___mb.mb_get_length, (const Object *) this);
}

void GrooveJoint2D::set_initial_offset(const real_t offset) {
	___godot_icall_void_float(___mb.mb_set_initial_offset, (const Object *) this, offset);
}

void GrooveJoint2D::set_length(const real_t length) {
	___godot_icall_void_float(___mb.mb_set_length, (const Object *) this, length);
}

}