#include "Generic6DOFJoint.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Generic6DOFJoint::___method_bindings Generic6DOFJoint::___mb = {};

void Generic6DOFJoint::___init_method_bindings() {
	___mb.mb__get_angular_hi_limit_x = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "_get_angular_hi_limit_x");
	___mb.mb__get_angular_hi_limit_y = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "_get_angular_hi_limit_y");
	___mb.mb__get_angular_hi_limit_z = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "_get_angular_hi_limit_z");
	___mb.mb__get_angular_lo_limit_x = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "_get_angular_lo_limit_x");
	___mb.mb__get_angular_lo_limit_y = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "_get_angular_lo_limit_y");
	___mb.mb__get_angular_lo_limit_z = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "_get_angular_lo_limit_z");
	___mb.mb__set_angular_hi_limit_x = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "_set_angular_hi_limit_x");
	___mb.mb__set_angular_hi_limit_y = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "_set_angular_hi_limit_y");
	___mb.mb__set_angular_hi_limit_z = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "_set_angular_hi_limit_z");
	___mb.mb__set_angular_lo_limit_x = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "_set_angular_lo_limit_x");
	___mb.mb__set_angular_lo_limit_y = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "_set_angular_lo_limit_y");
	___mb.mb__set_angular_lo_limit_z = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "_set_angular_lo_limit_z");
	___mb.mb_get_flag_x = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "get_flag_x");
	___mb.mb_get_flag_y = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "get_flag_y");
	___mb.mb_get_flag_z = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "get_flag_z");
	___mb.mb_get_param_x = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "get_param_x");
	___mb.mb_get_param_y = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "get_param_y");
	___mb.mb_get_param_z = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "get_param_z");
	___mb.mb_get_precision = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "get_precision");
	___mb.mb_set_flag_x = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "set_flag_x");
	___mb.mb_set_flag_y = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "set_flag_y");
	___mb.mb_set_flag_z = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "set_flag_z");
	___mb.mb_set_param_x = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "set_param_x");
	___mb.mb_set_param_y = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "set_param_y");
	___mb.mb_set_param_z = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "set_param_z");
	___mb.mb_set_precision = godot::api->godot_method_bind_get_method("Generic6DOFJoint", "set_precision");
}

Generic6DOFJoint *Generic6DOFJoint::_new()
{
	return (Generic6DOFJoint *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Generic6DOFJoint")());
}
real_t Generic6DOFJoint::_get_angular_hi_limit_x() const {
	return ___godot_icall_float(___mb.mb__get_angular_hi_limit_x, (const Object *) this);
}

real_t Generic6DOFJoint::_get_angular_hi_limit_y() const {
	return ___godot_icall_float(___mb.mb__get_angular_hi_limit_y, (const Object *) this);
}

real_t Generic6DOFJoint::_get_angular_hi_limit_z() const {
	return ___godot_icall_float(___mb.mb__get_angular_hi_limit_z, (const Object *) this);
}

real_t Generic6DOFJoint::_get_angular_lo_limit_x() const {
	return ___godot_icall_float(___mb.mb__get_angular_lo_limit_x, (const Object *) this);
}

real_t Generic6DOFJoint::_get_angular_lo_limit_y() const {
	return ___godot_icall_float(___mb.mb__get_angular_lo_limit_y, (const Object *) this);
}

real_t Generic6DOFJoint::_get_angular_lo_limit_z() const {
	return ___godot_icall_float(___mb.mb__get_angular_lo_limit_z, (const Object *) this);
}

void Generic6DOFJoint::_set_angular_hi_limit_x(const real_t angle) {
	___godot_icall_void_float(___mb.mb__set_angular_hi_limit_x, (const Object *) this, angle);
}

void Generic6DOFJoint::_set_angular_hi_limit_y(const real_t angle) {
	___godot_icall_void_float(___mb.mb__set_angular_hi_limit_y, (const Object *) this, angle);
}

void Generic6DOFJoint::_set_angular_hi_limit_z(const real_t angle) {
	___godot_icall_void_float(___mb.mb__set_angular_hi_limit_z, (const Object *) this, angle);
}

void Generic6DOFJoint::_set_angular_lo_limit_x(const real_t angle) {
	___godot_icall_void_float(___mb.mb__set_angular_lo_limit_x, (const Object *) this, angle);
}

void Generic6DOFJoint::_set_angular_lo_limit_y(const real_t angle) {
	___godot_icall_void_float(___mb.mb__set_angular_lo_limit_y, (const Object *) this, angle);
}

void Generic6DOFJoint::_set_angular_lo_limit_z(const real_t angle) {
	___godot_icall_void_float(___mb.mb__set_angular_lo_limit_z, (const Object *) this, angle);
}

bool Generic6DOFJoint::get_flag_x(const int64_t flag) const {
	return ___godot_icall_bool_int(___mb.mb_get_flag_x, (const Object *) this, flag);
}

bool Generic6DOFJoint::get_flag_y(const int64_t flag) const {
	return ___godot_icall_bool_int(___mb.mb_get_flag_y, (const Object *) this, flag);
}

bool Generic6DOFJoint::get_flag_z(const int64_t flag) const {
	return ___godot_icall_bool_int(___mb.mb_get_flag_z, (const Object *) this, flag);
}

real_t Generic6DOFJoint::get_param_x(const int64_t param) const {
	return ___godot_icall_float_int(___mb.mb_get_param_x, (const Object *) this, param);
}

real_t Generic6DOFJoint::get_param_y(const int64_t param) const {
	return ___godot_icall_float_int(___mb.mb_get_param_y, (const Object *) this, param);
}

real_t Generic6DOFJoint::get_param_z(const int64_t param) const {
	return ___godot_icall_float_int(___mb.mb_get_param_z, (const Object *) this, param);
}

int64_t Generic6DOFJoint::get_precision() const {
	return ___godot_icall_int(___mb.mb_get_precision, (const Object *) this);
}

void Generic6DOFJoint::set_flag_x(const int64_t flag, const bool value) {
	___godot_icall_void_int_bool(___mb.mb_set_flag_x, (const Object *) this, flag, value);
}

void Generic6DOFJoint::set_flag_y(const int64_t flag, const bool value) {
	___godot_icall_void_int_bool(___mb.mb_set_flag_y, (const Object *) this, flag, value);
}

void Generic6DOFJoint::set_flag_z(const int64_t flag, const bool value) {
	___godot_icall_void_int_bool(___mb.mb_set_flag_z, (const Object *) this, flag, value);
}

void Generic6DOFJoint::set_param_x(const int64_t param, const real_t value) {
	___godot_icall_void_int_float(___mb.mb_set_param_x, (const Object *) this, param, value);
}

void Generic6DOFJoint::set_param_y(const int64_t param, const real_t value) {
	___godot_icall_void_int_float(___mb.mb_set_param_y, (const Object *) this, param, value);
}

void Generic6DOFJoint::set_param_z(const int64_t param, const real_t value) {
	___godot_icall_void_int_float(___mb.mb_set_param_z, (const Object *) this, param, value);
}

void Generic6DOFJoint::set_precision(const int64_t precision) {
	___godot_icall_void_int(___mb.mb_set_precision, (const Object *) this, precision);
}

}