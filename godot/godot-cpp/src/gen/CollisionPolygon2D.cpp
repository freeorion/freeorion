#include "CollisionPolygon2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


CollisionPolygon2D::___method_bindings CollisionPolygon2D::___mb = {};

void CollisionPolygon2D::___init_method_bindings() {
	___mb.mb_get_build_mode = godot::api->godot_method_bind_get_method("CollisionPolygon2D", "get_build_mode");
	___mb.mb_get_one_way_collision_margin = godot::api->godot_method_bind_get_method("CollisionPolygon2D", "get_one_way_collision_margin");
	___mb.mb_get_polygon = godot::api->godot_method_bind_get_method("CollisionPolygon2D", "get_polygon");
	___mb.mb_is_disabled = godot::api->godot_method_bind_get_method("CollisionPolygon2D", "is_disabled");
	___mb.mb_is_one_way_collision_enabled = godot::api->godot_method_bind_get_method("CollisionPolygon2D", "is_one_way_collision_enabled");
	___mb.mb_set_build_mode = godot::api->godot_method_bind_get_method("CollisionPolygon2D", "set_build_mode");
	___mb.mb_set_disabled = godot::api->godot_method_bind_get_method("CollisionPolygon2D", "set_disabled");
	___mb.mb_set_one_way_collision = godot::api->godot_method_bind_get_method("CollisionPolygon2D", "set_one_way_collision");
	___mb.mb_set_one_way_collision_margin = godot::api->godot_method_bind_get_method("CollisionPolygon2D", "set_one_way_collision_margin");
	___mb.mb_set_polygon = godot::api->godot_method_bind_get_method("CollisionPolygon2D", "set_polygon");
}

CollisionPolygon2D *CollisionPolygon2D::_new()
{
	return (CollisionPolygon2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CollisionPolygon2D")());
}
CollisionPolygon2D::BuildMode CollisionPolygon2D::get_build_mode() const {
	return (CollisionPolygon2D::BuildMode) ___godot_icall_int(___mb.mb_get_build_mode, (const Object *) this);
}

real_t CollisionPolygon2D::get_one_way_collision_margin() const {
	return ___godot_icall_float(___mb.mb_get_one_way_collision_margin, (const Object *) this);
}

PoolVector2Array CollisionPolygon2D::get_polygon() const {
	return ___godot_icall_PoolVector2Array(___mb.mb_get_polygon, (const Object *) this);
}

bool CollisionPolygon2D::is_disabled() const {
	return ___godot_icall_bool(___mb.mb_is_disabled, (const Object *) this);
}

bool CollisionPolygon2D::is_one_way_collision_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_one_way_collision_enabled, (const Object *) this);
}

void CollisionPolygon2D::set_build_mode(const int64_t build_mode) {
	___godot_icall_void_int(___mb.mb_set_build_mode, (const Object *) this, build_mode);
}

void CollisionPolygon2D::set_disabled(const bool disabled) {
	___godot_icall_void_bool(___mb.mb_set_disabled, (const Object *) this, disabled);
}

void CollisionPolygon2D::set_one_way_collision(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_one_way_collision, (const Object *) this, enabled);
}

void CollisionPolygon2D::set_one_way_collision_margin(const real_t margin) {
	___godot_icall_void_float(___mb.mb_set_one_way_collision_margin, (const Object *) this, margin);
}

void CollisionPolygon2D::set_polygon(const PoolVector2Array polygon) {
	___godot_icall_void_PoolVector2Array(___mb.mb_set_polygon, (const Object *) this, polygon);
}

}