#include "PinJoint.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PinJoint::___method_bindings PinJoint::___mb = {};

void PinJoint::___init_method_bindings() {
	___mb.mb_get_param = godot::api->godot_method_bind_get_method("PinJoint", "get_param");
	___mb.mb_set_param = godot::api->godot_method_bind_get_method("PinJoint", "set_param");
}

PinJoint *PinJoint::_new()
{
	return (PinJoint *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PinJoint")());
}
real_t PinJoint::get_param(const int64_t param) const {
	return ___godot_icall_float_int(___mb.mb_get_param, (const Object *) this, param);
}

void PinJoint::set_param(const int64_t param, const real_t value) {
	___godot_icall_void_int_float(___mb.mb_set_param, (const Object *) this, param, value);
}

}