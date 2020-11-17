#include "GridMap.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "MeshLibrary.hpp"
#include "Resource.hpp"


namespace godot {


GridMap::___method_bindings GridMap::___mb = {};

void GridMap::___init_method_bindings() {
	___mb.mb__update_octants_callback = godot::api->godot_method_bind_get_method("GridMap", "_update_octants_callback");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("GridMap", "clear");
	___mb.mb_clear_baked_meshes = godot::api->godot_method_bind_get_method("GridMap", "clear_baked_meshes");
	___mb.mb_get_bake_mesh_instance = godot::api->godot_method_bind_get_method("GridMap", "get_bake_mesh_instance");
	___mb.mb_get_bake_meshes = godot::api->godot_method_bind_get_method("GridMap", "get_bake_meshes");
	___mb.mb_get_cell_item = godot::api->godot_method_bind_get_method("GridMap", "get_cell_item");
	___mb.mb_get_cell_item_orientation = godot::api->godot_method_bind_get_method("GridMap", "get_cell_item_orientation");
	___mb.mb_get_cell_scale = godot::api->godot_method_bind_get_method("GridMap", "get_cell_scale");
	___mb.mb_get_cell_size = godot::api->godot_method_bind_get_method("GridMap", "get_cell_size");
	___mb.mb_get_center_x = godot::api->godot_method_bind_get_method("GridMap", "get_center_x");
	___mb.mb_get_center_y = godot::api->godot_method_bind_get_method("GridMap", "get_center_y");
	___mb.mb_get_center_z = godot::api->godot_method_bind_get_method("GridMap", "get_center_z");
	___mb.mb_get_collision_layer = godot::api->godot_method_bind_get_method("GridMap", "get_collision_layer");
	___mb.mb_get_collision_layer_bit = godot::api->godot_method_bind_get_method("GridMap", "get_collision_layer_bit");
	___mb.mb_get_collision_mask = godot::api->godot_method_bind_get_method("GridMap", "get_collision_mask");
	___mb.mb_get_collision_mask_bit = godot::api->godot_method_bind_get_method("GridMap", "get_collision_mask_bit");
	___mb.mb_get_mesh_library = godot::api->godot_method_bind_get_method("GridMap", "get_mesh_library");
	___mb.mb_get_meshes = godot::api->godot_method_bind_get_method("GridMap", "get_meshes");
	___mb.mb_get_octant_size = godot::api->godot_method_bind_get_method("GridMap", "get_octant_size");
	___mb.mb_get_used_cells = godot::api->godot_method_bind_get_method("GridMap", "get_used_cells");
	___mb.mb_make_baked_meshes = godot::api->godot_method_bind_get_method("GridMap", "make_baked_meshes");
	___mb.mb_map_to_world = godot::api->godot_method_bind_get_method("GridMap", "map_to_world");
	___mb.mb_resource_changed = godot::api->godot_method_bind_get_method("GridMap", "resource_changed");
	___mb.mb_set_cell_item = godot::api->godot_method_bind_get_method("GridMap", "set_cell_item");
	___mb.mb_set_cell_scale = godot::api->godot_method_bind_get_method("GridMap", "set_cell_scale");
	___mb.mb_set_cell_size = godot::api->godot_method_bind_get_method("GridMap", "set_cell_size");
	___mb.mb_set_center_x = godot::api->godot_method_bind_get_method("GridMap", "set_center_x");
	___mb.mb_set_center_y = godot::api->godot_method_bind_get_method("GridMap", "set_center_y");
	___mb.mb_set_center_z = godot::api->godot_method_bind_get_method("GridMap", "set_center_z");
	___mb.mb_set_clip = godot::api->godot_method_bind_get_method("GridMap", "set_clip");
	___mb.mb_set_collision_layer = godot::api->godot_method_bind_get_method("GridMap", "set_collision_layer");
	___mb.mb_set_collision_layer_bit = godot::api->godot_method_bind_get_method("GridMap", "set_collision_layer_bit");
	___mb.mb_set_collision_mask = godot::api->godot_method_bind_get_method("GridMap", "set_collision_mask");
	___mb.mb_set_collision_mask_bit = godot::api->godot_method_bind_get_method("GridMap", "set_collision_mask_bit");
	___mb.mb_set_mesh_library = godot::api->godot_method_bind_get_method("GridMap", "set_mesh_library");
	___mb.mb_set_octant_size = godot::api->godot_method_bind_get_method("GridMap", "set_octant_size");
	___mb.mb_world_to_map = godot::api->godot_method_bind_get_method("GridMap", "world_to_map");
}

GridMap *GridMap::_new()
{
	return (GridMap *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"GridMap")());
}
void GridMap::_update_octants_callback() {
	___godot_icall_void(___mb.mb__update_octants_callback, (const Object *) this);
}

void GridMap::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

void GridMap::clear_baked_meshes() {
	___godot_icall_void(___mb.mb_clear_baked_meshes, (const Object *) this);
}

RID GridMap::get_bake_mesh_instance(const int64_t idx) {
	return ___godot_icall_RID_int(___mb.mb_get_bake_mesh_instance, (const Object *) this, idx);
}

Array GridMap::get_bake_meshes() {
	return ___godot_icall_Array(___mb.mb_get_bake_meshes, (const Object *) this);
}

int64_t GridMap::get_cell_item(const int64_t x, const int64_t y, const int64_t z) const {
	return ___godot_icall_int_int_int_int(___mb.mb_get_cell_item, (const Object *) this, x, y, z);
}

int64_t GridMap::get_cell_item_orientation(const int64_t x, const int64_t y, const int64_t z) const {
	return ___godot_icall_int_int_int_int(___mb.mb_get_cell_item_orientation, (const Object *) this, x, y, z);
}

real_t GridMap::get_cell_scale() const {
	return ___godot_icall_float(___mb.mb_get_cell_scale, (const Object *) this);
}

Vector3 GridMap::get_cell_size() const {
	return ___godot_icall_Vector3(___mb.mb_get_cell_size, (const Object *) this);
}

bool GridMap::get_center_x() const {
	return ___godot_icall_bool(___mb.mb_get_center_x, (const Object *) this);
}

bool GridMap::get_center_y() const {
	return ___godot_icall_bool(___mb.mb_get_center_y, (const Object *) this);
}

bool GridMap::get_center_z() const {
	return ___godot_icall_bool(___mb.mb_get_center_z, (const Object *) this);
}

int64_t GridMap::get_collision_layer() const {
	return ___godot_icall_int(___mb.mb_get_collision_layer, (const Object *) this);
}

bool GridMap::get_collision_layer_bit(const int64_t bit) const {
	return ___godot_icall_bool_int(___mb.mb_get_collision_layer_bit, (const Object *) this, bit);
}

int64_t GridMap::get_collision_mask() const {
	return ___godot_icall_int(___mb.mb_get_collision_mask, (const Object *) this);
}

bool GridMap::get_collision_mask_bit(const int64_t bit) const {
	return ___godot_icall_bool_int(___mb.mb_get_collision_mask_bit, (const Object *) this, bit);
}

Ref<MeshLibrary> GridMap::get_mesh_library() const {
	return Ref<MeshLibrary>::__internal_constructor(___godot_icall_Object(___mb.mb_get_mesh_library, (const Object *) this));
}

Array GridMap::get_meshes() {
	return ___godot_icall_Array(___mb.mb_get_meshes, (const Object *) this);
}

int64_t GridMap::get_octant_size() const {
	return ___godot_icall_int(___mb.mb_get_octant_size, (const Object *) this);
}

Array GridMap::get_used_cells() const {
	return ___godot_icall_Array(___mb.mb_get_used_cells, (const Object *) this);
}

void GridMap::make_baked_meshes(const bool gen_lightmap_uv, const real_t lightmap_uv_texel_size) {
	___godot_icall_void_bool_float(___mb.mb_make_baked_meshes, (const Object *) this, gen_lightmap_uv, lightmap_uv_texel_size);
}

Vector3 GridMap::map_to_world(const int64_t x, const int64_t y, const int64_t z) const {
	return ___godot_icall_Vector3_int_int_int(___mb.mb_map_to_world, (const Object *) this, x, y, z);
}

void GridMap::resource_changed(const Ref<Resource> resource) {
	___godot_icall_void_Object(___mb.mb_resource_changed, (const Object *) this, resource.ptr());
}

void GridMap::set_cell_item(const int64_t x, const int64_t y, const int64_t z, const int64_t item, const int64_t orientation) {
	___godot_icall_void_int_int_int_int_int(___mb.mb_set_cell_item, (const Object *) this, x, y, z, item, orientation);
}

void GridMap::set_cell_scale(const real_t scale) {
	___godot_icall_void_float(___mb.mb_set_cell_scale, (const Object *) this, scale);
}

void GridMap::set_cell_size(const Vector3 size) {
	___godot_icall_void_Vector3(___mb.mb_set_cell_size, (const Object *) this, size);
}

void GridMap::set_center_x(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_center_x, (const Object *) this, enable);
}

void GridMap::set_center_y(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_center_y, (const Object *) this, enable);
}

void GridMap::set_center_z(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_center_z, (const Object *) this, enable);
}

void GridMap::set_clip(const bool enabled, const bool clipabove, const int64_t floor, const int64_t axis) {
	___godot_icall_void_bool_bool_int_int(___mb.mb_set_clip, (const Object *) this, enabled, clipabove, floor, axis);
}

void GridMap::set_collision_layer(const int64_t layer) {
	___godot_icall_void_int(___mb.mb_set_collision_layer, (const Object *) this, layer);
}

void GridMap::set_collision_layer_bit(const int64_t bit, const bool value) {
	___godot_icall_void_int_bool(___mb.mb_set_collision_layer_bit, (const Object *) this, bit, value);
}

void GridMap::set_collision_mask(const int64_t mask) {
	___godot_icall_void_int(___mb.mb_set_collision_mask, (const Object *) this, mask);
}

void GridMap::set_collision_mask_bit(const int64_t bit, const bool value) {
	___godot_icall_void_int_bool(___mb.mb_set_collision_mask_bit, (const Object *) this, bit, value);
}

void GridMap::set_mesh_library(const Ref<MeshLibrary> mesh_library) {
	___godot_icall_void_Object(___mb.mb_set_mesh_library, (const Object *) this, mesh_library.ptr());
}

void GridMap::set_octant_size(const int64_t size) {
	___godot_icall_void_int(___mb.mb_set_octant_size, (const Object *) this, size);
}

Vector3 GridMap::world_to_map(const Vector3 pos) const {
	return ___godot_icall_Vector3_Vector3(___mb.mb_world_to_map, (const Object *) this, pos);
}

}