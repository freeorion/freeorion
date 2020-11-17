#include "KinematicBody.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "KinematicCollision.hpp"


namespace godot {


KinematicBody::___method_bindings KinematicBody::___mb = {};

void KinematicBody::___init_method_bindings() {
	___mb.mb_get_axis_lock = godot::api->godot_method_bind_get_method("KinematicBody", "get_axis_lock");
	___mb.mb_get_floor_normal = godot::api->godot_method_bind_get_method("KinematicBody", "get_floor_normal");
	___mb.mb_get_floor_velocity = godot::api->godot_method_bind_get_method("KinematicBody", "get_floor_velocity");
	___mb.mb_get_safe_margin = godot::api->godot_method_bind_get_method("KinematicBody", "get_safe_margin");
	___mb.mb_get_slide_collision = godot::api->godot_method_bind_get_method("KinematicBody", "get_slide_collision");
	___mb.mb_get_slide_count = godot::api->godot_method_bind_get_method("KinematicBody", "get_slide_count");
	___mb.mb_is_on_ceiling = godot::api->godot_method_bind_get_method("KinematicBody", "is_on_ceiling");
	___mb.mb_is_on_floor = godot::api->godot_method_bind_get_method("KinematicBody", "is_on_floor");
	___mb.mb_is_on_wall = godot::api->godot_method_bind_get_method("KinematicBody", "is_on_wall");
	___mb.mb_move_and_collide = godot::api->godot_method_bind_get_method("KinematicBody", "move_and_collide");
	___mb.mb_move_and_slide = godot::api->godot_method_bind_get_method("KinematicBody", "move_and_slide");
	___mb.mb_move_and_slide_with_snap = godot::api->godot_method_bind_get_method("KinematicBody", "move_and_slide_with_snap");
	___mb.mb_set_axis_lock = godot::api->godot_method_bind_get_method("KinematicBody", "set_axis_lock");
	___mb.mb_set_safe_margin = godot::api->godot_method_bind_get_method("KinematicBody", "set_safe_margin");
	___mb.mb_test_move = godot::api->godot_method_bind_get_method("KinematicBody", "test_move");
}

KinematicBody *KinematicBody::_new()
{
	return (KinematicBody *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"KinematicBody")());
}
bool KinematicBody::get_axis_lock(const int64_t axis) const {
	return ___godot_icall_bool_int(___mb.mb_get_axis_lock, (const Object *) this, axis);
}

Vector3 KinematicBody::get_floor_normal() const {
	return ___godot_icall_Vector3(___mb.mb_get_floor_normal, (const Object *) this);
}

Vector3 KinematicBody::get_floor_velocity() const {
	return ___godot_icall_Vector3(___mb.mb_get_floor_velocity, (const Object *) this);
}

real_t KinematicBody::get_safe_margin() const {
	return ___godot_icall_float(___mb.mb_get_safe_margin, (const Object *) this);
}

Ref<KinematicCollision> KinematicBody::get_slide_collision(const int64_t slide_idx) {
	return Ref<KinematicCollision>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_slide_collision, (const Object *) this, slide_idx));
}

int64_t KinematicBody::get_slide_count() const {
	return ___godot_icall_int(___mb.mb_get_slide_count, (const Object *) this);
}

bool KinematicBody::is_on_ceiling() const {
	return ___godot_icall_bool(___mb.mb_is_on_ceiling, (const Object *) this);
}

bool KinematicBody::is_on_floor() const {
	return ___godot_icall_bool(___mb.mb_is_on_floor, (const Object *) this);
}

bool KinematicBody::is_on_wall() const {
	return ___godot_icall_bool(___mb.mb_is_on_wall, (const Object *) this);
}

Ref<KinematicCollision> KinematicBody::move_and_collide(const Vector3 rel_vec, const bool infinite_inertia, const bool exclude_raycast_shapes, const bool test_only) {
	return Ref<KinematicCollision>::__internal_constructor(___godot_icall_Object_Vector3_bool_bool_bool(___mb.mb_move_and_collide, (const Object *) this, rel_vec, infinite_inertia, exclude_raycast_shapes, test_only));
}

Vector3 KinematicBody::move_and_slide(const Vector3 linear_velocity, const Vector3 up_direction, const bool stop_on_slope, const int64_t max_slides, const real_t floor_max_angle, const bool infinite_inertia) {
	return ___godot_icall_Vector3_Vector3_Vector3_bool_int_float_bool(___mb.mb_move_and_slide, (const Object *) this, linear_velocity, up_direction, stop_on_slope, max_slides, floor_max_angle, infinite_inertia);
}

Vector3 KinematicBody::move_and_slide_with_snap(const Vector3 linear_velocity, const Vector3 snap, const Vector3 up_direction, const bool stop_on_slope, const int64_t max_slides, const real_t floor_max_angle, const bool infinite_inertia) {
	return ___godot_icall_Vector3_Vector3_Vector3_Vector3_bool_int_float_bool(___mb.mb_move_and_slide_with_snap, (const Object *) this, linear_velocity, snap, up_direction, stop_on_slope, max_slides, floor_max_angle, infinite_inertia);
}

void KinematicBody::set_axis_lock(const int64_t axis, const bool lock) {
	___godot_icall_void_int_bool(___mb.mb_set_axis_lock, (const Object *) this, axis, lock);
}

void KinematicBody::set_safe_margin(const real_t pixels) {
	___godot_icall_void_float(___mb.mb_set_safe_margin, (const Object *) this, pixels);
}

bool KinematicBody::test_move(const Transform from, const Vector3 rel_vec, const bool infinite_inertia) {
	return ___godot_icall_bool_Transform_Vector3_bool(___mb.mb_test_move, (const Object *) this, from, rel_vec, infinite_inertia);
}

}