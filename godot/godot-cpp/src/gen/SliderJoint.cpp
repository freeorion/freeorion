#include "SliderJoint.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


SliderJoint::___method_bindings SliderJoint::___mb = {};

void SliderJoint::___init_method_bindings() {
	___mb.mb__get_lower_limit_angular = godot::api->godot_method_bind_get_method("SliderJoint", "_get_lower_limit_angular");
	___mb.mb__get_upper_limit_angular = godot::api->godot_method_bind_get_method("SliderJoint", "_get_upper_limit_angular");
	___mb.mb__set_lower_limit_angular = godot::api->godot_method_bind_get_method("SliderJoint", "_set_lower_limit_angular");
	___mb.mb__set_upper_limit_angular = godot::api->godot_method_bind_get_method("SliderJoint", "_set_upper_limit_angular");
	___mb.mb_get_param = godot::api->godot_method_bind_get_method("SliderJoint", "get_param");
	___mb.mb_set_param = godot::api->godot_method_bind_get_method("SliderJoint", "set_param");
}

SliderJoint *SliderJoint::_new()
{
	return (SliderJoint *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"SliderJoint")());
}
real_t SliderJoint::_get_lower_limit_angular() const {
	return ___godot_icall_float(___mb.mb__get_lower_limit_angular, (const Object *) this);
}

real_t SliderJoint::_get_upper_limit_angular() const {
	return ___godot_icall_float(___mb.mb__get_upper_limit_angular, (const Object *) this);
}

void SliderJoint::_set_lower_limit_angular(const real_t lower_limit_angular) {
	___godot_icall_void_float(___mb.mb__set_lower_limit_angular, (const Object *) this, lower_limit_angular);
}

void SliderJoint::_set_upper_limit_angular(const real_t upper_limit_angular) {
	___godot_icall_void_float(___mb.mb__set_upper_limit_angular, (const Object *) this, upper_limit_angular);
}

real_t SliderJoint::get_param(const int64_t param) const {
	return ___godot_icall_float_int(___mb.mb_get_param, (const Object *) this, param);
}

void SliderJoint::set_param(const int64_t param, const real_t value) {
	___godot_icall_void_int_float(___mb.mb_set_param, (const Object *) this, param, value);
}

}