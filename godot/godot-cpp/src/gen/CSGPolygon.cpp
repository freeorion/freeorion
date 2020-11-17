#include "CSGPolygon.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Material.hpp"


namespace godot {


CSGPolygon::___method_bindings CSGPolygon::___mb = {};

void CSGPolygon::___init_method_bindings() {
	___mb.mb__has_editable_3d_polygon_no_depth = godot::api->godot_method_bind_get_method("CSGPolygon", "_has_editable_3d_polygon_no_depth");
	___mb.mb__is_editable_3d_polygon = godot::api->godot_method_bind_get_method("CSGPolygon", "_is_editable_3d_polygon");
	___mb.mb__path_changed = godot::api->godot_method_bind_get_method("CSGPolygon", "_path_changed");
	___mb.mb__path_exited = godot::api->godot_method_bind_get_method("CSGPolygon", "_path_exited");
	___mb.mb_get_depth = godot::api->godot_method_bind_get_method("CSGPolygon", "get_depth");
	___mb.mb_get_material = godot::api->godot_method_bind_get_method("CSGPolygon", "get_material");
	___mb.mb_get_mode = godot::api->godot_method_bind_get_method("CSGPolygon", "get_mode");
	___mb.mb_get_path_interval = godot::api->godot_method_bind_get_method("CSGPolygon", "get_path_interval");
	___mb.mb_get_path_node = godot::api->godot_method_bind_get_method("CSGPolygon", "get_path_node");
	___mb.mb_get_path_rotation = godot::api->godot_method_bind_get_method("CSGPolygon", "get_path_rotation");
	___mb.mb_get_polygon = godot::api->godot_method_bind_get_method("CSGPolygon", "get_polygon");
	___mb.mb_get_smooth_faces = godot::api->godot_method_bind_get_method("CSGPolygon", "get_smooth_faces");
	___mb.mb_get_spin_degrees = godot::api->godot_method_bind_get_method("CSGPolygon", "get_spin_degrees");
	___mb.mb_get_spin_sides = godot::api->godot_method_bind_get_method("CSGPolygon", "get_spin_sides");
	___mb.mb_is_path_continuous_u = godot::api->godot_method_bind_get_method("CSGPolygon", "is_path_continuous_u");
	___mb.mb_is_path_joined = godot::api->godot_method_bind_get_method("CSGPolygon", "is_path_joined");
	___mb.mb_is_path_local = godot::api->godot_method_bind_get_method("CSGPolygon", "is_path_local");
	___mb.mb_set_depth = godot::api->godot_method_bind_get_method("CSGPolygon", "set_depth");
	___mb.mb_set_material = godot::api->godot_method_bind_get_method("CSGPolygon", "set_material");
	___mb.mb_set_mode = godot::api->godot_method_bind_get_method("CSGPolygon", "set_mode");
	___mb.mb_set_path_continuous_u = godot::api->godot_method_bind_get_method("CSGPolygon", "set_path_continuous_u");
	___mb.mb_set_path_interval = godot::api->godot_method_bind_get_method("CSGPolygon", "set_path_interval");
	___mb.mb_set_path_joined = godot::api->godot_method_bind_get_method("CSGPolygon", "set_path_joined");
	___mb.mb_set_path_local = godot::api->godot_method_bind_get_method("CSGPolygon", "set_path_local");
	___mb.mb_set_path_node = godot::api->godot_method_bind_get_method("CSGPolygon", "set_path_node");
	___mb.mb_set_path_rotation = godot::api->godot_method_bind_get_method("CSGPolygon", "set_path_rotation");
	___mb.mb_set_polygon = godot::api->godot_method_bind_get_method("CSGPolygon", "set_polygon");
	___mb.mb_set_smooth_faces = godot::api->godot_method_bind_get_method("CSGPolygon", "set_smooth_faces");
	___mb.mb_set_spin_degrees = godot::api->godot_method_bind_get_method("CSGPolygon", "set_spin_degrees");
	___mb.mb_set_spin_sides = godot::api->godot_method_bind_get_method("CSGPolygon", "set_spin_sides");
}

CSGPolygon *CSGPolygon::_new()
{
	return (CSGPolygon *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CSGPolygon")());
}
bool CSGPolygon::_has_editable_3d_polygon_no_depth() const {
	return ___godot_icall_bool(___mb.mb__has_editable_3d_polygon_no_depth, (const Object *) this);
}

bool CSGPolygon::_is_editable_3d_polygon() const {
	return ___godot_icall_bool(___mb.mb__is_editable_3d_polygon, (const Object *) this);
}

void CSGPolygon::_path_changed() {
	___godot_icall_void(___mb.mb__path_changed, (const Object *) this);
}

void CSGPolygon::_path_exited() {
	___godot_icall_void(___mb.mb__path_exited, (const Object *) this);
}

real_t CSGPolygon::get_depth() const {
	return ___godot_icall_float(___mb.mb_get_depth, (const Object *) this);
}

Ref<Material> CSGPolygon::get_material() const {
	return Ref<Material>::__internal_constructor(___godot_icall_Object(___mb.mb_get_material, (const Object *) this));
}

CSGPolygon::Mode CSGPolygon::get_mode() const {
	return (CSGPolygon::Mode) ___godot_icall_int(___mb.mb_get_mode, (const Object *) this);
}

real_t CSGPolygon::get_path_interval() const {
	return ___godot_icall_float(___mb.mb_get_path_interval, (const Object *) this);
}

NodePath CSGPolygon::get_path_node() const {
	return ___godot_icall_NodePath(___mb.mb_get_path_node, (const Object *) this);
}

CSGPolygon::PathRotation CSGPolygon::get_path_rotation() const {
	return (CSGPolygon::PathRotation) ___godot_icall_int(___mb.mb_get_path_rotation, (const Object *) this);
}

PoolVector2Array CSGPolygon::get_polygon() const {
	return ___godot_icall_PoolVector2Array(___mb.mb_get_polygon, (const Object *) this);
}

bool CSGPolygon::get_smooth_faces() const {
	return ___godot_icall_bool(___mb.mb_get_smooth_faces, (const Object *) this);
}

real_t CSGPolygon::get_spin_degrees() const {
	return ___godot_icall_float(___mb.mb_get_spin_degrees, (const Object *) this);
}

int64_t CSGPolygon::get_spin_sides() const {
	return ___godot_icall_int(___mb.mb_get_spin_sides, (const Object *) this);
}

bool CSGPolygon::is_path_continuous_u() const {
	return ___godot_icall_bool(___mb.mb_is_path_continuous_u, (const Object *) this);
}

bool CSGPolygon::is_path_joined() const {
	return ___godot_icall_bool(___mb.mb_is_path_joined, (const Object *) this);
}

bool CSGPolygon::is_path_local() const {
	return ___godot_icall_bool(___mb.mb_is_path_local, (const Object *) this);
}

void CSGPolygon::set_depth(const real_t depth) {
	___godot_icall_void_float(___mb.mb_set_depth, (const Object *) this, depth);
}

void CSGPolygon::set_material(const Ref<Material> material) {
	___godot_icall_void_Object(___mb.mb_set_material, (const Object *) this, material.ptr());
}

void CSGPolygon::set_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_mode, (const Object *) this, mode);
}

void CSGPolygon::set_path_continuous_u(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_path_continuous_u, (const Object *) this, enable);
}

void CSGPolygon::set_path_interval(const real_t distance) {
	___godot_icall_void_float(___mb.mb_set_path_interval, (const Object *) this, distance);
}

void CSGPolygon::set_path_joined(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_path_joined, (const Object *) this, enable);
}

void CSGPolygon::set_path_local(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_path_local, (const Object *) this, enable);
}

void CSGPolygon::set_path_node(const NodePath path) {
	___godot_icall_void_NodePath(___mb.mb_set_path_node, (const Object *) this, path);
}

void CSGPolygon::set_path_rotation(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_path_rotation, (const Object *) this, mode);
}

void CSGPolygon::set_polygon(const PoolVector2Array polygon) {
	___godot_icall_void_PoolVector2Array(___mb.mb_set_polygon, (const Object *) this, polygon);
}

void CSGPolygon::set_smooth_faces(const bool smooth_faces) {
	___godot_icall_void_bool(___mb.mb_set_smooth_faces, (const Object *) this, smooth_faces);
}

void CSGPolygon::set_spin_degrees(const real_t degrees) {
	___godot_icall_void_float(___mb.mb_set_spin_degrees, (const Object *) this, degrees);
}

void CSGPolygon::set_spin_sides(const int64_t spin_sides) {
	___godot_icall_void_int(___mb.mb_set_spin_sides, (const Object *) this, spin_sides);
}

}