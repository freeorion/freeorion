#include "KinematicBody2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "KinematicCollision2D.hpp"


namespace godot {


KinematicBody2D::___method_bindings KinematicBody2D::___mb = {};

void KinematicBody2D::___init_method_bindings() {
	___mb.mb__direct_state_changed = godot::api->godot_method_bind_get_method("KinematicBody2D", "_direct_state_changed");
	___mb.mb_get_floor_normal = godot::api->godot_method_bind_get_method("KinematicBody2D", "get_floor_normal");
	___mb.mb_get_floor_velocity = godot::api->godot_method_bind_get_method("KinematicBody2D", "get_floor_velocity");
	___mb.mb_get_safe_margin = godot::api->godot_method_bind_get_method("KinematicBody2D", "get_safe_margin");
	___mb.mb_get_slide_collision = godot::api->godot_method_bind_get_method("KinematicBody2D", "get_slide_collision");
	___mb.mb_get_slide_count = godot::api->godot_method_bind_get_method("KinematicBody2D", "get_slide_count");
	___mb.mb_is_on_ceiling = godot::api->godot_method_bind_get_method("KinematicBody2D", "is_on_ceiling");
	___mb.mb_is_on_floor = godot::api->godot_method_bind_get_method("KinematicBody2D", "is_on_floor");
	___mb.mb_is_on_wall = godot::api->godot_method_bind_get_method("KinematicBody2D", "is_on_wall");
	___mb.mb_is_sync_to_physics_enabled = godot::api->godot_method_bind_get_method("KinematicBody2D", "is_sync_to_physics_enabled");
	___mb.mb_move_and_collide = godot::api->godot_method_bind_get_method("KinematicBody2D", "move_and_collide");
	___mb.mb_move_and_slide = godot::api->godot_method_bind_get_method("KinematicBody2D", "move_and_slide");
	___mb.mb_move_and_slide_with_snap = godot::api->godot_method_bind_get_method("KinematicBody2D", "move_and_slide_with_snap");
	___mb.mb_set_safe_margin = godot::api->godot_method_bind_get_method("KinematicBody2D", "set_safe_margin");
	___mb.mb_set_sync_to_physics = godot::api->godot_method_bind_get_method("KinematicBody2D", "set_sync_to_physics");
	___mb.mb_test_move = godot::api->godot_method_bind_get_method("KinematicBody2D", "test_move");
}

KinematicBody2D *KinematicBody2D::_new()
{
	return (KinematicBody2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"KinematicBody2D")());
}
void KinematicBody2D::_direct_state_changed(const Object *arg0) {
	___godot_icall_void_Object(___mb.mb__direct_state_changed, (const Object *) this, arg0);
}

Vector2 KinematicBody2D::get_floor_normal() const {
	return ___godot_icall_Vector2(___mb.mb_get_floor_normal, (const Object *) this);
}

Vector2 KinematicBody2D::get_floor_velocity() const {
	return ___godot_icall_Vector2(___mb.mb_get_floor_velocity, (const Object *) this);
}

real_t KinematicBody2D::get_safe_margin() const {
	return ___godot_icall_float(___mb.mb_get_safe_margin, (const Object *) this);
}

Ref<KinematicCollision2D> KinematicBody2D::get_slide_collision(const int64_t slide_idx) {
	return Ref<KinematicCollision2D>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_slide_collision, (const Object *) this, slide_idx));
}

int64_t KinematicBody2D::get_slide_count() const {
	return ___godot_icall_int(___mb.mb_get_slide_count, (const Object *) this);
}

bool KinematicBody2D::is_on_ceiling() const {
	return ___godot_icall_bool(___mb.mb_is_on_ceiling, (const Object *) this);
}

bool KinematicBody2D::is_on_floor() const {
	return ___godot_icall_bool(___mb.mb_is_on_floor, (const Object *) this);
}

bool KinematicBody2D::is_on_wall() const {
	return ___godot_icall_bool(___mb.mb_is_on_wall, (const Object *) this);
}

bool KinematicBody2D::is_sync_to_physics_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_sync_to_physics_enabled, (const Object *) this);
}

Ref<KinematicCollision2D> KinematicBody2D::move_and_collide(const Vector2 rel_vec, const bool infinite_inertia, const bool exclude_raycast_shapes, const bool test_only) {
	return Ref<KinematicCollision2D>::__internal_constructor(___godot_icall_Object_Vector2_bool_bool_bool(___mb.mb_move_and_collide, (const Object *) this, rel_vec, infinite_inertia, exclude_raycast_shapes, test_only));
}

Vector2 KinematicBody2D::move_and_slide(const Vector2 linear_velocity, const Vector2 up_direction, const bool stop_on_slope, const int64_t max_slides, const real_t floor_max_angle, const bool infinite_inertia) {
	return ___godot_icall_Vector2_Vector2_Vector2_bool_int_float_bool(___mb.mb_move_and_slide, (const Object *) this, linear_velocity, up_direction, stop_on_slope, max_slides, floor_max_angle, infinite_inertia);
}

Vector2 KinematicBody2D::move_and_slide_with_snap(const Vector2 linear_velocity, const Vector2 snap, const Vector2 up_direction, const bool stop_on_slope, const int64_t max_slides, const real_t floor_max_angle, const bool infinite_inertia) {
	return ___godot_icall_Vector2_Vector2_Vector2_Vector2_bool_int_float_bool(___mb.mb_move_and_slide_with_snap, (const Object *) this, linear_velocity, snap, up_direction, stop_on_slope, max_slides, floor_max_angle, infinite_inertia);
}

void KinematicBody2D::set_safe_margin(const real_t pixels) {
	___godot_icall_void_float(___mb.mb_set_safe_margin, (const Object *) this, pixels);
}

void KinematicBody2D::set_sync_to_physics(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_sync_to_physics, (const Object *) this, enable);
}

bool KinematicBody2D::test_move(const Transform2D from, const Vector2 rel_vec, const bool infinite_inertia) {
	return ___godot_icall_bool_Transform2D_Vector2_bool(___mb.mb_test_move, (const Object *) this, from, rel_vec, infinite_inertia);
}

}