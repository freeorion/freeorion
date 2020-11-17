#include "TileMap.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "TileSet.hpp"


namespace godot {


TileMap::___method_bindings TileMap::___mb = {};

void TileMap::___init_method_bindings() {
	___mb.mb__clear_quadrants = godot::api->godot_method_bind_get_method("TileMap", "_clear_quadrants");
	___mb.mb__get_old_cell_size = godot::api->godot_method_bind_get_method("TileMap", "_get_old_cell_size");
	___mb.mb__get_tile_data = godot::api->godot_method_bind_get_method("TileMap", "_get_tile_data");
	___mb.mb__recreate_quadrants = godot::api->godot_method_bind_get_method("TileMap", "_recreate_quadrants");
	___mb.mb__set_celld = godot::api->godot_method_bind_get_method("TileMap", "_set_celld");
	___mb.mb__set_old_cell_size = godot::api->godot_method_bind_get_method("TileMap", "_set_old_cell_size");
	___mb.mb__set_tile_data = godot::api->godot_method_bind_get_method("TileMap", "_set_tile_data");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("TileMap", "clear");
	___mb.mb_fix_invalid_tiles = godot::api->godot_method_bind_get_method("TileMap", "fix_invalid_tiles");
	___mb.mb_get_cell = godot::api->godot_method_bind_get_method("TileMap", "get_cell");
	___mb.mb_get_cell_autotile_coord = godot::api->godot_method_bind_get_method("TileMap", "get_cell_autotile_coord");
	___mb.mb_get_cell_size = godot::api->godot_method_bind_get_method("TileMap", "get_cell_size");
	___mb.mb_get_cellv = godot::api->godot_method_bind_get_method("TileMap", "get_cellv");
	___mb.mb_get_clip_uv = godot::api->godot_method_bind_get_method("TileMap", "get_clip_uv");
	___mb.mb_get_collision_bounce = godot::api->godot_method_bind_get_method("TileMap", "get_collision_bounce");
	___mb.mb_get_collision_friction = godot::api->godot_method_bind_get_method("TileMap", "get_collision_friction");
	___mb.mb_get_collision_layer = godot::api->godot_method_bind_get_method("TileMap", "get_collision_layer");
	___mb.mb_get_collision_layer_bit = godot::api->godot_method_bind_get_method("TileMap", "get_collision_layer_bit");
	___mb.mb_get_collision_mask = godot::api->godot_method_bind_get_method("TileMap", "get_collision_mask");
	___mb.mb_get_collision_mask_bit = godot::api->godot_method_bind_get_method("TileMap", "get_collision_mask_bit");
	___mb.mb_get_collision_use_kinematic = godot::api->godot_method_bind_get_method("TileMap", "get_collision_use_kinematic");
	___mb.mb_get_collision_use_parent = godot::api->godot_method_bind_get_method("TileMap", "get_collision_use_parent");
	___mb.mb_get_custom_transform = godot::api->godot_method_bind_get_method("TileMap", "get_custom_transform");
	___mb.mb_get_half_offset = godot::api->godot_method_bind_get_method("TileMap", "get_half_offset");
	___mb.mb_get_mode = godot::api->godot_method_bind_get_method("TileMap", "get_mode");
	___mb.mb_get_occluder_light_mask = godot::api->godot_method_bind_get_method("TileMap", "get_occluder_light_mask");
	___mb.mb_get_quadrant_size = godot::api->godot_method_bind_get_method("TileMap", "get_quadrant_size");
	___mb.mb_get_tile_origin = godot::api->godot_method_bind_get_method("TileMap", "get_tile_origin");
	___mb.mb_get_tileset = godot::api->godot_method_bind_get_method("TileMap", "get_tileset");
	___mb.mb_get_used_cells = godot::api->godot_method_bind_get_method("TileMap", "get_used_cells");
	___mb.mb_get_used_cells_by_id = godot::api->godot_method_bind_get_method("TileMap", "get_used_cells_by_id");
	___mb.mb_get_used_rect = godot::api->godot_method_bind_get_method("TileMap", "get_used_rect");
	___mb.mb_is_cell_transposed = godot::api->godot_method_bind_get_method("TileMap", "is_cell_transposed");
	___mb.mb_is_cell_x_flipped = godot::api->godot_method_bind_get_method("TileMap", "is_cell_x_flipped");
	___mb.mb_is_cell_y_flipped = godot::api->godot_method_bind_get_method("TileMap", "is_cell_y_flipped");
	___mb.mb_is_centered_textures_enabled = godot::api->godot_method_bind_get_method("TileMap", "is_centered_textures_enabled");
	___mb.mb_is_compatibility_mode_enabled = godot::api->godot_method_bind_get_method("TileMap", "is_compatibility_mode_enabled");
	___mb.mb_is_y_sort_mode_enabled = godot::api->godot_method_bind_get_method("TileMap", "is_y_sort_mode_enabled");
	___mb.mb_map_to_world = godot::api->godot_method_bind_get_method("TileMap", "map_to_world");
	___mb.mb_set_cell = godot::api->godot_method_bind_get_method("TileMap", "set_cell");
	___mb.mb_set_cell_size = godot::api->godot_method_bind_get_method("TileMap", "set_cell_size");
	___mb.mb_set_cellv = godot::api->godot_method_bind_get_method("TileMap", "set_cellv");
	___mb.mb_set_centered_textures = godot::api->godot_method_bind_get_method("TileMap", "set_centered_textures");
	___mb.mb_set_clip_uv = godot::api->godot_method_bind_get_method("TileMap", "set_clip_uv");
	___mb.mb_set_collision_bounce = godot::api->godot_method_bind_get_method("TileMap", "set_collision_bounce");
	___mb.mb_set_collision_friction = godot::api->godot_method_bind_get_method("TileMap", "set_collision_friction");
	___mb.mb_set_collision_layer = godot::api->godot_method_bind_get_method("TileMap", "set_collision_layer");
	___mb.mb_set_collision_layer_bit = godot::api->godot_method_bind_get_method("TileMap", "set_collision_layer_bit");
	___mb.mb_set_collision_mask = godot::api->godot_method_bind_get_method("TileMap", "set_collision_mask");
	___mb.mb_set_collision_mask_bit = godot::api->godot_method_bind_get_method("TileMap", "set_collision_mask_bit");
	___mb.mb_set_collision_use_kinematic = godot::api->godot_method_bind_get_method("TileMap", "set_collision_use_kinematic");
	___mb.mb_set_collision_use_parent = godot::api->godot_method_bind_get_method("TileMap", "set_collision_use_parent");
	___mb.mb_set_compatibility_mode = godot::api->godot_method_bind_get_method("TileMap", "set_compatibility_mode");
	___mb.mb_set_custom_transform = godot::api->godot_method_bind_get_method("TileMap", "set_custom_transform");
	___mb.mb_set_half_offset = godot::api->godot_method_bind_get_method("TileMap", "set_half_offset");
	___mb.mb_set_mode = godot::api->godot_method_bind_get_method("TileMap", "set_mode");
	___mb.mb_set_occluder_light_mask = godot::api->godot_method_bind_get_method("TileMap", "set_occluder_light_mask");
	___mb.mb_set_quadrant_size = godot::api->godot_method_bind_get_method("TileMap", "set_quadrant_size");
	___mb.mb_set_tile_origin = godot::api->godot_method_bind_get_method("TileMap", "set_tile_origin");
	___mb.mb_set_tileset = godot::api->godot_method_bind_get_method("TileMap", "set_tileset");
	___mb.mb_set_y_sort_mode = godot::api->godot_method_bind_get_method("TileMap", "set_y_sort_mode");
	___mb.mb_update_bitmask_area = godot::api->godot_method_bind_get_method("TileMap", "update_bitmask_area");
	___mb.mb_update_bitmask_region = godot::api->godot_method_bind_get_method("TileMap", "update_bitmask_region");
	___mb.mb_update_dirty_quadrants = godot::api->godot_method_bind_get_method("TileMap", "update_dirty_quadrants");
	___mb.mb_world_to_map = godot::api->godot_method_bind_get_method("TileMap", "world_to_map");
}

TileMap *TileMap::_new()
{
	return (TileMap *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"TileMap")());
}
void TileMap::_clear_quadrants() {
	___godot_icall_void(___mb.mb__clear_quadrants, (const Object *) this);
}

int64_t TileMap::_get_old_cell_size() const {
	return ___godot_icall_int(___mb.mb__get_old_cell_size, (const Object *) this);
}

PoolIntArray TileMap::_get_tile_data() const {
	return ___godot_icall_PoolIntArray(___mb.mb__get_tile_data, (const Object *) this);
}

void TileMap::_recreate_quadrants() {
	___godot_icall_void(___mb.mb__recreate_quadrants, (const Object *) this);
}

void TileMap::_set_celld(const Vector2 position, const Dictionary data) {
	___godot_icall_void_Vector2_Dictionary(___mb.mb__set_celld, (const Object *) this, position, data);
}

void TileMap::_set_old_cell_size(const int64_t size) {
	___godot_icall_void_int(___mb.mb__set_old_cell_size, (const Object *) this, size);
}

void TileMap::_set_tile_data(const PoolIntArray arg0) {
	___godot_icall_void_PoolIntArray(___mb.mb__set_tile_data, (const Object *) this, arg0);
}

void TileMap::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

void TileMap::fix_invalid_tiles() {
	___godot_icall_void(___mb.mb_fix_invalid_tiles, (const Object *) this);
}

int64_t TileMap::get_cell(const int64_t x, const int64_t y) const {
	return ___godot_icall_int_int_int(___mb.mb_get_cell, (const Object *) this, x, y);
}

Vector2 TileMap::get_cell_autotile_coord(const int64_t x, const int64_t y) const {
	return ___godot_icall_Vector2_int_int(___mb.mb_get_cell_autotile_coord, (const Object *) this, x, y);
}

Vector2 TileMap::get_cell_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_cell_size, (const Object *) this);
}

int64_t TileMap::get_cellv(const Vector2 position) const {
	return ___godot_icall_int_Vector2(___mb.mb_get_cellv, (const Object *) this, position);
}

bool TileMap::get_clip_uv() const {
	return ___godot_icall_bool(___mb.mb_get_clip_uv, (const Object *) this);
}

real_t TileMap::get_collision_bounce() const {
	return ___godot_icall_float(___mb.mb_get_collision_bounce, (const Object *) this);
}

real_t TileMap::get_collision_friction() const {
	return ___godot_icall_float(___mb.mb_get_collision_friction, (const Object *) this);
}

int64_t TileMap::get_collision_layer() const {
	return ___godot_icall_int(___mb.mb_get_collision_layer, (const Object *) this);
}

bool TileMap::get_collision_layer_bit(const int64_t bit) const {
	return ___godot_icall_bool_int(___mb.mb_get_collision_layer_bit, (const Object *) this, bit);
}

int64_t TileMap::get_collision_mask() const {
	return ___godot_icall_int(___mb.mb_get_collision_mask, (const Object *) this);
}

bool TileMap::get_collision_mask_bit(const int64_t bit) const {
	return ___godot_icall_bool_int(___mb.mb_get_collision_mask_bit, (const Object *) this, bit);
}

bool TileMap::get_collision_use_kinematic() const {
	return ___godot_icall_bool(___mb.mb_get_collision_use_kinematic, (const Object *) this);
}

bool TileMap::get_collision_use_parent() const {
	return ___godot_icall_bool(___mb.mb_get_collision_use_parent, (const Object *) this);
}

Transform2D TileMap::get_custom_transform() const {
	return ___godot_icall_Transform2D(___mb.mb_get_custom_transform, (const Object *) this);
}

TileMap::HalfOffset TileMap::get_half_offset() const {
	return (TileMap::HalfOffset) ___godot_icall_int(___mb.mb_get_half_offset, (const Object *) this);
}

TileMap::Mode TileMap::get_mode() const {
	return (TileMap::Mode) ___godot_icall_int(___mb.mb_get_mode, (const Object *) this);
}

int64_t TileMap::get_occluder_light_mask() const {
	return ___godot_icall_int(___mb.mb_get_occluder_light_mask, (const Object *) this);
}

int64_t TileMap::get_quadrant_size() const {
	return ___godot_icall_int(___mb.mb_get_quadrant_size, (const Object *) this);
}

TileMap::TileOrigin TileMap::get_tile_origin() const {
	return (TileMap::TileOrigin) ___godot_icall_int(___mb.mb_get_tile_origin, (const Object *) this);
}

Ref<TileSet> TileMap::get_tileset() const {
	return Ref<TileSet>::__internal_constructor(___godot_icall_Object(___mb.mb_get_tileset, (const Object *) this));
}

Array TileMap::get_used_cells() const {
	return ___godot_icall_Array(___mb.mb_get_used_cells, (const Object *) this);
}

Array TileMap::get_used_cells_by_id(const int64_t id) const {
	return ___godot_icall_Array_int(___mb.mb_get_used_cells_by_id, (const Object *) this, id);
}

Rect2 TileMap::get_used_rect() {
	return ___godot_icall_Rect2(___mb.mb_get_used_rect, (const Object *) this);
}

bool TileMap::is_cell_transposed(const int64_t x, const int64_t y) const {
	return ___godot_icall_bool_int_int(___mb.mb_is_cell_transposed, (const Object *) this, x, y);
}

bool TileMap::is_cell_x_flipped(const int64_t x, const int64_t y) const {
	return ___godot_icall_bool_int_int(___mb.mb_is_cell_x_flipped, (const Object *) this, x, y);
}

bool TileMap::is_cell_y_flipped(const int64_t x, const int64_t y) const {
	return ___godot_icall_bool_int_int(___mb.mb_is_cell_y_flipped, (const Object *) this, x, y);
}

bool TileMap::is_centered_textures_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_centered_textures_enabled, (const Object *) this);
}

bool TileMap::is_compatibility_mode_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_compatibility_mode_enabled, (const Object *) this);
}

bool TileMap::is_y_sort_mode_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_y_sort_mode_enabled, (const Object *) this);
}

Vector2 TileMap::map_to_world(const Vector2 map_position, const bool ignore_half_ofs) const {
	return ___godot_icall_Vector2_Vector2_bool(___mb.mb_map_to_world, (const Object *) this, map_position, ignore_half_ofs);
}

void TileMap::set_cell(const int64_t x, const int64_t y, const int64_t tile, const bool flip_x, const bool flip_y, const bool transpose, const Vector2 autotile_coord) {
	___godot_icall_void_int_int_int_bool_bool_bool_Vector2(___mb.mb_set_cell, (const Object *) this, x, y, tile, flip_x, flip_y, transpose, autotile_coord);
}

void TileMap::set_cell_size(const Vector2 size) {
	___godot_icall_void_Vector2(___mb.mb_set_cell_size, (const Object *) this, size);
}

void TileMap::set_cellv(const Vector2 position, const int64_t tile, const bool flip_x, const bool flip_y, const bool transpose) {
	___godot_icall_void_Vector2_int_bool_bool_bool(___mb.mb_set_cellv, (const Object *) this, position, tile, flip_x, flip_y, transpose);
}

void TileMap::set_centered_textures(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_centered_textures, (const Object *) this, enable);
}

void TileMap::set_clip_uv(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_clip_uv, (const Object *) this, enable);
}

void TileMap::set_collision_bounce(const real_t value) {
	___godot_icall_void_float(___mb.mb_set_collision_bounce, (const Object *) this, value);
}

void TileMap::set_collision_friction(const real_t value) {
	___godot_icall_void_float(___mb.mb_set_collision_friction, (const Object *) this, value);
}

void TileMap::set_collision_layer(const int64_t layer) {
	___godot_icall_void_int(___mb.mb_set_collision_layer, (const Object *) this, layer);
}

void TileMap::set_collision_layer_bit(const int64_t bit, const bool value) {
	___godot_icall_void_int_bool(___mb.mb_set_collision_layer_bit, (const Object *) this, bit, value);
}

void TileMap::set_collision_mask(const int64_t mask) {
	___godot_icall_void_int(___mb.mb_set_collision_mask, (const Object *) this, mask);
}

void TileMap::set_collision_mask_bit(const int64_t bit, const bool value) {
	___godot_icall_void_int_bool(___mb.mb_set_collision_mask_bit, (const Object *) this, bit, value);
}

void TileMap::set_collision_use_kinematic(const bool use_kinematic) {
	___godot_icall_void_bool(___mb.mb_set_collision_use_kinematic, (const Object *) this, use_kinematic);
}

void TileMap::set_collision_use_parent(const bool use_parent) {
	___godot_icall_void_bool(___mb.mb_set_collision_use_parent, (const Object *) this, use_parent);
}

void TileMap::set_compatibility_mode(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_compatibility_mode, (const Object *) this, enable);
}

void TileMap::set_custom_transform(const Transform2D custom_transform) {
	___godot_icall_void_Transform2D(___mb.mb_set_custom_transform, (const Object *) this, custom_transform);
}

void TileMap::set_half_offset(const int64_t half_offset) {
	___godot_icall_void_int(___mb.mb_set_half_offset, (const Object *) this, half_offset);
}

void TileMap::set_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_mode, (const Object *) this, mode);
}

void TileMap::set_occluder_light_mask(const int64_t mask) {
	___godot_icall_void_int(___mb.mb_set_occluder_light_mask, (const Object *) this, mask);
}

void TileMap::set_quadrant_size(const int64_t size) {
	___godot_icall_void_int(___mb.mb_set_quadrant_size, (const Object *) this, size);
}

void TileMap::set_tile_origin(const int64_t origin) {
	___godot_icall_void_int(___mb.mb_set_tile_origin, (const Object *) this, origin);
}

void TileMap::set_tileset(const Ref<TileSet> tileset) {
	___godot_icall_void_Object(___mb.mb_set_tileset, (const Object *) this, tileset.ptr());
}

void TileMap::set_y_sort_mode(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_y_sort_mode, (const Object *) this, enable);
}

void TileMap::update_bitmask_area(const Vector2 position) {
	___godot_icall_void_Vector2(___mb.mb_update_bitmask_area, (const Object *) this, position);
}

void TileMap::update_bitmask_region(const Vector2 start, const Vector2 end) {
	___godot_icall_void_Vector2_Vector2(___mb.mb_update_bitmask_region, (const Object *) this, start, end);
}

void TileMap::update_dirty_quadrants() {
	___godot_icall_void(___mb.mb_update_dirty_quadrants, (const Object *) this);
}

Vector2 TileMap::world_to_map(const Vector2 world_position) const {
	return ___godot_icall_Vector2_Vector2(___mb.mb_world_to_map, (const Object *) this, world_position);
}

}