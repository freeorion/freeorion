#include "CollisionPolygon.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


CollisionPolygon::___method_bindings CollisionPolygon::___mb = {};

void CollisionPolygon::___init_method_bindings() {
	___mb.mb__is_editable_3d_polygon = godot::api->godot_method_bind_get_method("CollisionPolygon", "_is_editable_3d_polygon");
	___mb.mb_get_depth = godot::api->godot_method_bind_get_method("CollisionPolygon", "get_depth");
	___mb.mb_get_polygon = godot::api->godot_method_bind_get_method("CollisionPolygon", "get_polygon");
	___mb.mb_is_disabled = godot::api->godot_method_bind_get_method("CollisionPolygon", "is_disabled");
	___mb.mb_set_depth = godot::api->godot_method_bind_get_method("CollisionPolygon", "set_depth");
	___mb.mb_set_disabled = godot::api->godot_method_bind_get_method("CollisionPolygon", "set_disabled");
	___mb.mb_set_polygon = godot::api->godot_method_bind_get_method("CollisionPolygon", "set_polygon");
}

CollisionPolygon *CollisionPolygon::_new()
{
	return (CollisionPolygon *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CollisionPolygon")());
}
bool CollisionPolygon::_is_editable_3d_polygon() const {
	return ___godot_icall_bool(___mb.mb__is_editable_3d_polygon, (const Object *) this);
}

real_t CollisionPolygon::get_depth() const {
	return ___godot_icall_float(___mb.mb_get_depth, (const Object *) this);
}

PoolVector2Array CollisionPolygon::get_polygon() const {
	return ___godot_icall_PoolVector2Array(___mb.mb_get_polygon, (const Object *) this);
}

bool CollisionPolygon::is_disabled() const {
	return ___godot_icall_bool(___mb.mb_is_disabled, (const Object *) this);
}

void CollisionPolygon::set_depth(const real_t depth) {
	___godot_icall_void_float(___mb.mb_set_depth, (const Object *) this, depth);
}

void CollisionPolygon::set_disabled(const bool disabled) {
	___godot_icall_void_bool(___mb.mb_set_disabled, (const Object *) this, disabled);
}

void CollisionPolygon::set_polygon(const PoolVector2Array polygon) {
	___godot_icall_void_PoolVector2Array(___mb.mb_set_polygon, (const Object *) this, polygon);
}

}