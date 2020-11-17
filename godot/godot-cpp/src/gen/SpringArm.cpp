#include "SpringArm.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Shape.hpp"


namespace godot {


SpringArm::___method_bindings SpringArm::___mb = {};

void SpringArm::___init_method_bindings() {
	___mb.mb_add_excluded_object = godot::api->godot_method_bind_get_method("SpringArm", "add_excluded_object");
	___mb.mb_clear_excluded_objects = godot::api->godot_method_bind_get_method("SpringArm", "clear_excluded_objects");
	___mb.mb_get_collision_mask = godot::api->godot_method_bind_get_method("SpringArm", "get_collision_mask");
	___mb.mb_get_hit_length = godot::api->godot_method_bind_get_method("SpringArm", "get_hit_length");
	___mb.mb_get_length = godot::api->godot_method_bind_get_method("SpringArm", "get_length");
	___mb.mb_get_margin = godot::api->godot_method_bind_get_method("SpringArm", "get_margin");
	___mb.mb_get_shape = godot::api->godot_method_bind_get_method("SpringArm", "get_shape");
	___mb.mb_remove_excluded_object = godot::api->godot_method_bind_get_method("SpringArm", "remove_excluded_object");
	___mb.mb_set_collision_mask = godot::api->godot_method_bind_get_method("SpringArm", "set_collision_mask");
	___mb.mb_set_length = godot::api->godot_method_bind_get_method("SpringArm", "set_length");
	___mb.mb_set_margin = godot::api->godot_method_bind_get_method("SpringArm", "set_margin");
	___mb.mb_set_shape = godot::api->godot_method_bind_get_method("SpringArm", "set_shape");
}

SpringArm *SpringArm::_new()
{
	return (SpringArm *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"SpringArm")());
}
void SpringArm::add_excluded_object(const RID RID) {
	___godot_icall_void_RID(___mb.mb_add_excluded_object, (const Object *) this, RID);
}

void SpringArm::clear_excluded_objects() {
	___godot_icall_void(___mb.mb_clear_excluded_objects, (const Object *) this);
}

int64_t SpringArm::get_collision_mask() {
	return ___godot_icall_int(___mb.mb_get_collision_mask, (const Object *) this);
}

real_t SpringArm::get_hit_length() {
	return ___godot_icall_float(___mb.mb_get_hit_length, (const Object *) this);
}

real_t SpringArm::get_length() const {
	return ___godot_icall_float(___mb.mb_get_length, (const Object *) this);
}

real_t SpringArm::get_margin() {
	return ___godot_icall_float(___mb.mb_get_margin, (const Object *) this);
}

Ref<Shape> SpringArm::get_shape() const {
	return Ref<Shape>::__internal_constructor(___godot_icall_Object(___mb.mb_get_shape, (const Object *) this));
}

bool SpringArm::remove_excluded_object(const RID RID) {
	return ___godot_icall_bool_RID(___mb.mb_remove_excluded_object, (const Object *) this, RID);
}

void SpringArm::set_collision_mask(const int64_t mask) {
	___godot_icall_void_int(___mb.mb_set_collision_mask, (const Object *) this, mask);
}

void SpringArm::set_length(const real_t length) {
	___godot_icall_void_float(___mb.mb_set_length, (const Object *) this, length);
}

void SpringArm::set_margin(const real_t margin) {
	___godot_icall_void_float(___mb.mb_set_margin, (const Object *) this, margin);
}

void SpringArm::set_shape(const Ref<Shape> shape) {
	___godot_icall_void_Object(___mb.mb_set_shape, (const Object *) this, shape.ptr());
}

}