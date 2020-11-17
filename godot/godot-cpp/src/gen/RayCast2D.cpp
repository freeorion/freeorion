#include "RayCast2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"


namespace godot {


RayCast2D::___method_bindings RayCast2D::___mb = {};

void RayCast2D::___init_method_bindings() {
	___mb.mb_add_exception = godot::api->godot_method_bind_get_method("RayCast2D", "add_exception");
	___mb.mb_add_exception_rid = godot::api->godot_method_bind_get_method("RayCast2D", "add_exception_rid");
	___mb.mb_clear_exceptions = godot::api->godot_method_bind_get_method("RayCast2D", "clear_exceptions");
	___mb.mb_force_raycast_update = godot::api->godot_method_bind_get_method("RayCast2D", "force_raycast_update");
	___mb.mb_get_cast_to = godot::api->godot_method_bind_get_method("RayCast2D", "get_cast_to");
	___mb.mb_get_collider = godot::api->godot_method_bind_get_method("RayCast2D", "get_collider");
	___mb.mb_get_collider_shape = godot::api->godot_method_bind_get_method("RayCast2D", "get_collider_shape");
	___mb.mb_get_collision_mask = godot::api->godot_method_bind_get_method("RayCast2D", "get_collision_mask");
	___mb.mb_get_collision_mask_bit = godot::api->godot_method_bind_get_method("RayCast2D", "get_collision_mask_bit");
	___mb.mb_get_collision_normal = godot::api->godot_method_bind_get_method("RayCast2D", "get_collision_normal");
	___mb.mb_get_collision_point = godot::api->godot_method_bind_get_method("RayCast2D", "get_collision_point");
	___mb.mb_get_exclude_parent_body = godot::api->godot_method_bind_get_method("RayCast2D", "get_exclude_parent_body");
	___mb.mb_is_collide_with_areas_enabled = godot::api->godot_method_bind_get_method("RayCast2D", "is_collide_with_areas_enabled");
	___mb.mb_is_collide_with_bodies_enabled = godot::api->godot_method_bind_get_method("RayCast2D", "is_collide_with_bodies_enabled");
	___mb.mb_is_colliding = godot::api->godot_method_bind_get_method("RayCast2D", "is_colliding");
	___mb.mb_is_enabled = godot::api->godot_method_bind_get_method("RayCast2D", "is_enabled");
	___mb.mb_remove_exception = godot::api->godot_method_bind_get_method("RayCast2D", "remove_exception");
	___mb.mb_remove_exception_rid = godot::api->godot_method_bind_get_method("RayCast2D", "remove_exception_rid");
	___mb.mb_set_cast_to = godot::api->godot_method_bind_get_method("RayCast2D", "set_cast_to");
	___mb.mb_set_collide_with_areas = godot::api->godot_method_bind_get_method("RayCast2D", "set_collide_with_areas");
	___mb.mb_set_collide_with_bodies = godot::api->godot_method_bind_get_method("RayCast2D", "set_collide_with_bodies");
	___mb.mb_set_collision_mask = godot::api->godot_method_bind_get_method("RayCast2D", "set_collision_mask");
	___mb.mb_set_collision_mask_bit = godot::api->godot_method_bind_get_method("RayCast2D", "set_collision_mask_bit");
	___mb.mb_set_enabled = godot::api->godot_method_bind_get_method("RayCast2D", "set_enabled");
	___mb.mb_set_exclude_parent_body = godot::api->godot_method_bind_get_method("RayCast2D", "set_exclude_parent_body");
}

RayCast2D *RayCast2D::_new()
{
	return (RayCast2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"RayCast2D")());
}
void RayCast2D::add_exception(const Object *node) {
	___godot_icall_void_Object(___mb.mb_add_exception, (const Object *) this, node);
}

void RayCast2D::add_exception_rid(const RID rid) {
	___godot_icall_void_RID(___mb.mb_add_exception_rid, (const Object *) this, rid);
}

void RayCast2D::clear_exceptions() {
	___godot_icall_void(___mb.mb_clear_exceptions, (const Object *) this);
}

void RayCast2D::force_raycast_update() {
	___godot_icall_void(___mb.mb_force_raycast_update, (const Object *) this);
}

Vector2 RayCast2D::get_cast_to() const {
	return ___godot_icall_Vector2(___mb.mb_get_cast_to, (const Object *) this);
}

Object *RayCast2D::get_collider() const {
	return (Object *) ___godot_icall_Object(___mb.mb_get_collider, (const Object *) this);
}

int64_t RayCast2D::get_collider_shape() const {
	return ___godot_icall_int(___mb.mb_get_collider_shape, (const Object *) this);
}

int64_t RayCast2D::get_collision_mask() const {
	return ___godot_icall_int(___mb.mb_get_collision_mask, (const Object *) this);
}

bool RayCast2D::get_collision_mask_bit(const int64_t bit) const {
	return ___godot_icall_bool_int(___mb.mb_get_collision_mask_bit, (const Object *) this, bit);
}

Vector2 RayCast2D::get_collision_normal() const {
	return ___godot_icall_Vector2(___mb.mb_get_collision_normal, (const Object *) this);
}

Vector2 RayCast2D::get_collision_point() const {
	return ___godot_icall_Vector2(___mb.mb_get_collision_point, (const Object *) this);
}

bool RayCast2D::get_exclude_parent_body() const {
	return ___godot_icall_bool(___mb.mb_get_exclude_parent_body, (const Object *) this);
}

bool RayCast2D::is_collide_with_areas_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_collide_with_areas_enabled, (const Object *) this);
}

bool RayCast2D::is_collide_with_bodies_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_collide_with_bodies_enabled, (const Object *) this);
}

bool RayCast2D::is_colliding() const {
	return ___godot_icall_bool(___mb.mb_is_colliding, (const Object *) this);
}

bool RayCast2D::is_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_enabled, (const Object *) this);
}

void RayCast2D::remove_exception(const Object *node) {
	___godot_icall_void_Object(___mb.mb_remove_exception, (const Object *) this, node);
}

void RayCast2D::remove_exception_rid(const RID rid) {
	___godot_icall_void_RID(___mb.mb_remove_exception_rid, (const Object *) this, rid);
}

void RayCast2D::set_cast_to(const Vector2 local_point) {
	___godot_icall_void_Vector2(___mb.mb_set_cast_to, (const Object *) this, local_point);
}

void RayCast2D::set_collide_with_areas(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_collide_with_areas, (const Object *) this, enable);
}

void RayCast2D::set_collide_with_bodies(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_collide_with_bodies, (const Object *) this, enable);
}

void RayCast2D::set_collision_mask(const int64_t mask) {
	___godot_icall_void_int(___mb.mb_set_collision_mask, (const Object *) this, mask);
}

void RayCast2D::set_collision_mask_bit(const int64_t bit, const bool value) {
	___godot_icall_void_int_bool(___mb.mb_set_collision_mask_bit, (const Object *) this, bit, value);
}

void RayCast2D::set_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_enabled, (const Object *) this, enabled);
}

void RayCast2D::set_exclude_parent_body(const bool mask) {
	___godot_icall_void_bool(___mb.mb_set_exclude_parent_body, (const Object *) this, mask);
}

}