#include "HingeJoint.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


HingeJoint::___method_bindings HingeJoint::___mb = {};

void HingeJoint::___init_method_bindings() {
	___mb.mb__get_lower_limit = godot::api->godot_method_bind_get_method("HingeJoint", "_get_lower_limit");
	___mb.mb__get_upper_limit = godot::api->godot_method_bind_get_method("HingeJoint", "_get_upper_limit");
	___mb.mb__set_lower_limit = godot::api->godot_method_bind_get_method("HingeJoint", "_set_lower_limit");
	___mb.mb__set_upper_limit = godot::api->godot_method_bind_get_method("HingeJoint", "_set_upper_limit");
	___mb.mb_get_flag = godot::api->godot_method_bind_get_method("HingeJoint", "get_flag");
	___mb.mb_get_param = godot::api->godot_method_bind_get_method("HingeJoint", "get_param");
	___mb.mb_set_flag = godot::api->godot_method_bind_get_method("HingeJoint", "set_flag");
	___mb.mb_set_param = godot::api->godot_method_bind_get_method("HingeJoint", "set_param");
}

HingeJoint *HingeJoint::_new()
{
	return (HingeJoint *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"HingeJoint")());
}
real_t HingeJoint::_get_lower_limit() const {
	return ___godot_icall_float(___mb.mb__get_lower_limit, (const Object *) this);
}

real_t HingeJoint::_get_upper_limit() const {
	return ___godot_icall_float(___mb.mb__get_upper_limit, (const Object *) this);
}

void HingeJoint::_set_lower_limit(const real_t lower_limit) {
	___godot_icall_void_float(___mb.mb__set_lower_limit, (const Object *) this, lower_limit);
}

void HingeJoint::_set_upper_limit(const real_t upper_limit) {
	___godot_icall_void_float(___mb.mb__set_upper_limit, (const Object *) this, upper_limit);
}

bool HingeJoint::get_flag(const int64_t flag) const {
	return ___godot_icall_bool_int(___mb.mb_get_flag, (const Object *) this, flag);
}

real_t HingeJoint::get_param(const int64_t param) const {
	return ___godot_icall_float_int(___mb.mb_get_param, (const Object *) this, param);
}

void HingeJoint::set_flag(const int64_t flag, const bool enabled) {
	___godot_icall_void_int_bool(___mb.mb_set_flag, (const Object *) this, flag, enabled);
}

void HingeJoint::set_param(const int64_t param, const real_t value) {
	___godot_icall_void_int_float(___mb.mb_set_param, (const Object *) this, param, value);
}

}