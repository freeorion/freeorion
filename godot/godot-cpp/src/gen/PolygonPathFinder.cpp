#include "PolygonPathFinder.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PolygonPathFinder::___method_bindings PolygonPathFinder::___mb = {};

void PolygonPathFinder::___init_method_bindings() {
	___mb.mb__get_data = godot::api->godot_method_bind_get_method("PolygonPathFinder", "_get_data");
	___mb.mb__set_data = godot::api->godot_method_bind_get_method("PolygonPathFinder", "_set_data");
	___mb.mb_find_path = godot::api->godot_method_bind_get_method("PolygonPathFinder", "find_path");
	___mb.mb_get_bounds = godot::api->godot_method_bind_get_method("PolygonPathFinder", "get_bounds");
	___mb.mb_get_closest_point = godot::api->godot_method_bind_get_method("PolygonPathFinder", "get_closest_point");
	___mb.mb_get_intersections = godot::api->godot_method_bind_get_method("PolygonPathFinder", "get_intersections");
	___mb.mb_get_point_penalty = godot::api->godot_method_bind_get_method("PolygonPathFinder", "get_point_penalty");
	___mb.mb_is_point_inside = godot::api->godot_method_bind_get_method("PolygonPathFinder", "is_point_inside");
	___mb.mb_set_point_penalty = godot::api->godot_method_bind_get_method("PolygonPathFinder", "set_point_penalty");
	___mb.mb_setup = godot::api->godot_method_bind_get_method("PolygonPathFinder", "setup");
}

PolygonPathFinder *PolygonPathFinder::_new()
{
	return (PolygonPathFinder *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PolygonPathFinder")());
}
Dictionary PolygonPathFinder::_get_data() const {
	return ___godot_icall_Dictionary(___mb.mb__get_data, (const Object *) this);
}

void PolygonPathFinder::_set_data(const Dictionary arg0) {
	___godot_icall_void_Dictionary(___mb.mb__set_data, (const Object *) this, arg0);
}

PoolVector2Array PolygonPathFinder::find_path(const Vector2 from, const Vector2 to) {
	return ___godot_icall_PoolVector2Array_Vector2_Vector2(___mb.mb_find_path, (const Object *) this, from, to);
}

Rect2 PolygonPathFinder::get_bounds() const {
	return ___godot_icall_Rect2(___mb.mb_get_bounds, (const Object *) this);
}

Vector2 PolygonPathFinder::get_closest_point(const Vector2 point) const {
	return ___godot_icall_Vector2_Vector2(___mb.mb_get_closest_point, (const Object *) this, point);
}

PoolVector2Array PolygonPathFinder::get_intersections(const Vector2 from, const Vector2 to) const {
	return ___godot_icall_PoolVector2Array_Vector2_Vector2(___mb.mb_get_intersections, (const Object *) this, from, to);
}

real_t PolygonPathFinder::get_point_penalty(const int64_t idx) const {
	return ___godot_icall_float_int(___mb.mb_get_point_penalty, (const Object *) this, idx);
}

bool PolygonPathFinder::is_point_inside(const Vector2 point) const {
	return ___godot_icall_bool_Vector2(___mb.mb_is_point_inside, (const Object *) this, point);
}

void PolygonPathFinder::set_point_penalty(const int64_t idx, const real_t penalty) {
	___godot_icall_void_int_float(___mb.mb_set_point_penalty, (const Object *) this, idx, penalty);
}

void PolygonPathFinder::setup(const PoolVector2Array points, const PoolIntArray connections) {
	___godot_icall_void_PoolVector2Array_PoolIntArray(___mb.mb_setup, (const Object *) this, points, connections);
}

}