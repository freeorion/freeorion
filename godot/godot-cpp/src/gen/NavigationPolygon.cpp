#include "NavigationPolygon.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


NavigationPolygon::___method_bindings NavigationPolygon::___mb = {};

void NavigationPolygon::___init_method_bindings() {
	___mb.mb__get_outlines = godot::api->godot_method_bind_get_method("NavigationPolygon", "_get_outlines");
	___mb.mb__get_polygons = godot::api->godot_method_bind_get_method("NavigationPolygon", "_get_polygons");
	___mb.mb__set_outlines = godot::api->godot_method_bind_get_method("NavigationPolygon", "_set_outlines");
	___mb.mb__set_polygons = godot::api->godot_method_bind_get_method("NavigationPolygon", "_set_polygons");
	___mb.mb_add_outline = godot::api->godot_method_bind_get_method("NavigationPolygon", "add_outline");
	___mb.mb_add_outline_at_index = godot::api->godot_method_bind_get_method("NavigationPolygon", "add_outline_at_index");
	___mb.mb_add_polygon = godot::api->godot_method_bind_get_method("NavigationPolygon", "add_polygon");
	___mb.mb_clear_outlines = godot::api->godot_method_bind_get_method("NavigationPolygon", "clear_outlines");
	___mb.mb_clear_polygons = godot::api->godot_method_bind_get_method("NavigationPolygon", "clear_polygons");
	___mb.mb_get_outline = godot::api->godot_method_bind_get_method("NavigationPolygon", "get_outline");
	___mb.mb_get_outline_count = godot::api->godot_method_bind_get_method("NavigationPolygon", "get_outline_count");
	___mb.mb_get_polygon = godot::api->godot_method_bind_get_method("NavigationPolygon", "get_polygon");
	___mb.mb_get_polygon_count = godot::api->godot_method_bind_get_method("NavigationPolygon", "get_polygon_count");
	___mb.mb_get_vertices = godot::api->godot_method_bind_get_method("NavigationPolygon", "get_vertices");
	___mb.mb_make_polygons_from_outlines = godot::api->godot_method_bind_get_method("NavigationPolygon", "make_polygons_from_outlines");
	___mb.mb_remove_outline = godot::api->godot_method_bind_get_method("NavigationPolygon", "remove_outline");
	___mb.mb_set_outline = godot::api->godot_method_bind_get_method("NavigationPolygon", "set_outline");
	___mb.mb_set_vertices = godot::api->godot_method_bind_get_method("NavigationPolygon", "set_vertices");
}

NavigationPolygon *NavigationPolygon::_new()
{
	return (NavigationPolygon *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"NavigationPolygon")());
}
Array NavigationPolygon::_get_outlines() const {
	return ___godot_icall_Array(___mb.mb__get_outlines, (const Object *) this);
}

Array NavigationPolygon::_get_polygons() const {
	return ___godot_icall_Array(___mb.mb__get_polygons, (const Object *) this);
}

void NavigationPolygon::_set_outlines(const Array outlines) {
	___godot_icall_void_Array(___mb.mb__set_outlines, (const Object *) this, outlines);
}

void NavigationPolygon::_set_polygons(const Array polygons) {
	___godot_icall_void_Array(___mb.mb__set_polygons, (const Object *) this, polygons);
}

void NavigationPolygon::add_outline(const PoolVector2Array outline) {
	___godot_icall_void_PoolVector2Array(___mb.mb_add_outline, (const Object *) this, outline);
}

void NavigationPolygon::add_outline_at_index(const PoolVector2Array outline, const int64_t index) {
	___godot_icall_void_PoolVector2Array_int(___mb.mb_add_outline_at_index, (const Object *) this, outline, index);
}

void NavigationPolygon::add_polygon(const PoolIntArray polygon) {
	___godot_icall_void_PoolIntArray(___mb.mb_add_polygon, (const Object *) this, polygon);
}

void NavigationPolygon::clear_outlines() {
	___godot_icall_void(___mb.mb_clear_outlines, (const Object *) this);
}

void NavigationPolygon::clear_polygons() {
	___godot_icall_void(___mb.mb_clear_polygons, (const Object *) this);
}

PoolVector2Array NavigationPolygon::get_outline(const int64_t idx) const {
	return ___godot_icall_PoolVector2Array_int(___mb.mb_get_outline, (const Object *) this, idx);
}

int64_t NavigationPolygon::get_outline_count() const {
	return ___godot_icall_int(___mb.mb_get_outline_count, (const Object *) this);
}

PoolIntArray NavigationPolygon::get_polygon(const int64_t idx) {
	return ___godot_icall_PoolIntArray_int(___mb.mb_get_polygon, (const Object *) this, idx);
}

int64_t NavigationPolygon::get_polygon_count() const {
	return ___godot_icall_int(___mb.mb_get_polygon_count, (const Object *) this);
}

PoolVector2Array NavigationPolygon::get_vertices() const {
	return ___godot_icall_PoolVector2Array(___mb.mb_get_vertices, (const Object *) this);
}

void NavigationPolygon::make_polygons_from_outlines() {
	___godot_icall_void(___mb.mb_make_polygons_from_outlines, (const Object *) this);
}

void NavigationPolygon::remove_outline(const int64_t idx) {
	___godot_icall_void_int(___mb.mb_remove_outline, (const Object *) this, idx);
}

void NavigationPolygon::set_outline(const int64_t idx, const PoolVector2Array outline) {
	___godot_icall_void_int_PoolVector2Array(___mb.mb_set_outline, (const Object *) this, idx, outline);
}

void NavigationPolygon::set_vertices(const PoolVector2Array vertices) {
	___godot_icall_void_PoolVector2Array(___mb.mb_set_vertices, (const Object *) this, vertices);
}

}