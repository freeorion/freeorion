#include "ConeTwistJoint.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ConeTwistJoint::___method_bindings ConeTwistJoint::___mb = {};

void ConeTwistJoint::___init_method_bindings() {
	___mb.mb__get_swing_span = godot::api->godot_method_bind_get_method("ConeTwistJoint", "_get_swing_span");
	___mb.mb__get_twist_span = godot::api->godot_method_bind_get_method("ConeTwistJoint", "_get_twist_span");
	___mb.mb__set_swing_span = godot::api->godot_method_bind_get_method("ConeTwistJoint", "_set_swing_span");
	___mb.mb__set_twist_span = godot::api->godot_method_bind_get_method("ConeTwistJoint", "_set_twist_span");
	___mb.mb_get_param = godot::api->godot_method_bind_get_method("ConeTwistJoint", "get_param");
	___mb.mb_set_param = godot::api->godot_method_bind_get_method("ConeTwistJoint", "set_param");
}

ConeTwistJoint *ConeTwistJoint::_new()
{
	return (ConeTwistJoint *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ConeTwistJoint")());
}
real_t ConeTwistJoint::_get_swing_span() const {
	return ___godot_icall_float(___mb.mb__get_swing_span, (const Object *) this);
}

real_t ConeTwistJoint::_get_twist_span() const {
	return ___godot_icall_float(___mb.mb__get_twist_span, (const Object *) this);
}

void ConeTwistJoint::_set_swing_span(const real_t swing_span) {
	___godot_icall_void_float(___mb.mb__set_swing_span, (const Object *) this, swing_span);
}

void ConeTwistJoint::_set_twist_span(const real_t twist_span) {
	___godot_icall_void_float(___mb.mb__set_twist_span, (const Object *) this, twist_span);
}

real_t ConeTwistJoint::get_param(const int64_t param) const {
	return ___godot_icall_float_int(___mb.mb_get_param, (const Object *) this, param);
}

void ConeTwistJoint::set_param(const int64_t param, const real_t value) {
	___godot_icall_void_int_float(___mb.mb_set_param, (const Object *) this, param, value);
}

}