#include "Range.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Node.hpp"


namespace godot {


Range::___method_bindings Range::___mb = {};

void Range::___init_method_bindings() {
	___mb.mb_get_as_ratio = godot::api->godot_method_bind_get_method("Range", "get_as_ratio");
	___mb.mb_get_max = godot::api->godot_method_bind_get_method("Range", "get_max");
	___mb.mb_get_min = godot::api->godot_method_bind_get_method("Range", "get_min");
	___mb.mb_get_page = godot::api->godot_method_bind_get_method("Range", "get_page");
	___mb.mb_get_step = godot::api->godot_method_bind_get_method("Range", "get_step");
	___mb.mb_get_value = godot::api->godot_method_bind_get_method("Range", "get_value");
	___mb.mb_is_greater_allowed = godot::api->godot_method_bind_get_method("Range", "is_greater_allowed");
	___mb.mb_is_lesser_allowed = godot::api->godot_method_bind_get_method("Range", "is_lesser_allowed");
	___mb.mb_is_ratio_exp = godot::api->godot_method_bind_get_method("Range", "is_ratio_exp");
	___mb.mb_is_using_rounded_values = godot::api->godot_method_bind_get_method("Range", "is_using_rounded_values");
	___mb.mb_set_allow_greater = godot::api->godot_method_bind_get_method("Range", "set_allow_greater");
	___mb.mb_set_allow_lesser = godot::api->godot_method_bind_get_method("Range", "set_allow_lesser");
	___mb.mb_set_as_ratio = godot::api->godot_method_bind_get_method("Range", "set_as_ratio");
	___mb.mb_set_exp_ratio = godot::api->godot_method_bind_get_method("Range", "set_exp_ratio");
	___mb.mb_set_max = godot::api->godot_method_bind_get_method("Range", "set_max");
	___mb.mb_set_min = godot::api->godot_method_bind_get_method("Range", "set_min");
	___mb.mb_set_page = godot::api->godot_method_bind_get_method("Range", "set_page");
	___mb.mb_set_step = godot::api->godot_method_bind_get_method("Range", "set_step");
	___mb.mb_set_use_rounded_values = godot::api->godot_method_bind_get_method("Range", "set_use_rounded_values");
	___mb.mb_set_value = godot::api->godot_method_bind_get_method("Range", "set_value");
	___mb.mb_share = godot::api->godot_method_bind_get_method("Range", "share");
	___mb.mb_unshare = godot::api->godot_method_bind_get_method("Range", "unshare");
}

real_t Range::get_as_ratio() const {
	return ___godot_icall_float(___mb.mb_get_as_ratio, (const Object *) this);
}

real_t Range::get_max() const {
	return ___godot_icall_float(___mb.mb_get_max, (const Object *) this);
}

real_t Range::get_min() const {
	return ___godot_icall_float(___mb.mb_get_min, (const Object *) this);
}

real_t Range::get_page() const {
	return ___godot_icall_float(___mb.mb_get_page, (const Object *) this);
}

real_t Range::get_step() const {
	return ___godot_icall_float(___mb.mb_get_step, (const Object *) this);
}

real_t Range::get_value() const {
	return ___godot_icall_float(___mb.mb_get_value, (const Object *) this);
}

bool Range::is_greater_allowed() const {
	return ___godot_icall_bool(___mb.mb_is_greater_allowed, (const Object *) this);
}

bool Range::is_lesser_allowed() const {
	return ___godot_icall_bool(___mb.mb_is_lesser_allowed, (const Object *) this);
}

bool Range::is_ratio_exp() const {
	return ___godot_icall_bool(___mb.mb_is_ratio_exp, (const Object *) this);
}

bool Range::is_using_rounded_values() const {
	return ___godot_icall_bool(___mb.mb_is_using_rounded_values, (const Object *) this);
}

void Range::set_allow_greater(const bool allow) {
	___godot_icall_void_bool(___mb.mb_set_allow_greater, (const Object *) this, allow);
}

void Range::set_allow_lesser(const bool allow) {
	___godot_icall_void_bool(___mb.mb_set_allow_lesser, (const Object *) this, allow);
}

void Range::set_as_ratio(const real_t value) {
	___godot_icall_void_float(___mb.mb_set_as_ratio, (const Object *) this, value);
}

void Range::set_exp_ratio(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_exp_ratio, (const Object *) this, enabled);
}

void Range::set_max(const real_t maximum) {
	___godot_icall_void_float(___mb.mb_set_max, (const Object *) this, maximum);
}

void Range::set_min(const real_t minimum) {
	___godot_icall_void_float(___mb.mb_set_min, (const Object *) this, minimum);
}

void Range::set_page(const real_t pagesize) {
	___godot_icall_void_float(___mb.mb_set_page, (const Object *) this, pagesize);
}

void Range::set_step(const real_t step) {
	___godot_icall_void_float(___mb.mb_set_step, (const Object *) this, step);
}

void Range::set_use_rounded_values(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_use_rounded_values, (const Object *) this, enabled);
}

void Range::set_value(const real_t value) {
	___godot_icall_void_float(___mb.mb_set_value, (const Object *) this, value);
}

void Range::share(const Node *with) {
	___godot_icall_void_Object(___mb.mb_share, (const Object *) this, with);
}

void Range::unshare() {
	___godot_icall_void(___mb.mb_unshare, (const Object *) this);
}

}