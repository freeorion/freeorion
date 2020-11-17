#ifndef GODOT_CPP_TILEMAP_HPP
#define GODOT_CPP_TILEMAP_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "TileMap.hpp"

#include "Node2D.hpp"
namespace godot {

class TileSet;

class TileMap : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb__clear_quadrants;
		godot_method_bind *mb__get_old_cell_size;
		godot_method_bind *mb__get_tile_data;
		godot_method_bind *mb__recreate_quadrants;
		godot_method_bind *mb__set_celld;
		godot_method_bind *mb__set_old_cell_size;
		godot_method_bind *mb__set_tile_data;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_fix_invalid_tiles;
		godot_method_bind *mb_get_cell;
		godot_method_bind *mb_get_cell_autotile_coord;
		godot_method_bind *mb_get_cell_size;
		godot_method_bind *mb_get_cellv;
		godot_method_bind *mb_get_clip_uv;
		godot_method_bind *mb_get_collision_bounce;
		godot_method_bind *mb_get_collision_friction;
		godot_method_bind *mb_get_collision_layer;
		godot_method_bind *mb_get_collision_layer_bit;
		godot_method_bind *mb_get_collision_mask;
		godot_method_bind *mb_get_collision_mask_bit;
		godot_method_bind *mb_get_collision_use_kinematic;
		godot_method_bind *mb_get_collision_use_parent;
		godot_method_bind *mb_get_custom_transform;
		godot_method_bind *mb_get_half_offset;
		godot_method_bind *mb_get_mode;
		godot_method_bind *mb_get_occluder_light_mask;
		godot_method_bind *mb_get_quadrant_size;
		godot_method_bind *mb_get_tile_origin;
		godot_method_bind *mb_get_tileset;
		godot_method_bind *mb_get_used_cells;
		godot_method_bind *mb_get_used_cells_by_id;
		godot_method_bind *mb_get_used_rect;
		godot_method_bind *mb_is_cell_transposed;
		godot_method_bind *mb_is_cell_x_flipped;
		godot_method_bind *mb_is_cell_y_flipped;
		godot_method_bind *mb_is_centered_textures_enabled;
		godot_method_bind *mb_is_compatibility_mode_enabled;
		godot_method_bind *mb_is_y_sort_mode_enabled;
		godot_method_bind *mb_map_to_world;
		godot_method_bind *mb_set_cell;
		godot_method_bind *mb_set_cell_size;
		godot_method_bind *mb_set_cellv;
		godot_method_bind *mb_set_centered_textures;
		godot_method_bind *mb_set_clip_uv;
		godot_method_bind *mb_set_collision_bounce;
		godot_method_bind *mb_set_collision_friction;
		godot_method_bind *mb_set_collision_layer;
		godot_method_bind *mb_set_collision_layer_bit;
		godot_method_bind *mb_set_collision_mask;
		godot_method_bind *mb_set_collision_mask_bit;
		godot_method_bind *mb_set_collision_use_kinematic;
		godot_method_bind *mb_set_collision_use_parent;
		godot_method_bind *mb_set_compatibility_mode;
		godot_method_bind *mb_set_custom_transform;
		godot_method_bind *mb_set_half_offset;
		godot_method_bind *mb_set_mode;
		godot_method_bind *mb_set_occluder_light_mask;
		godot_method_bind *mb_set_quadrant_size;
		godot_method_bind *mb_set_tile_origin;
		godot_method_bind *mb_set_tileset;
		godot_method_bind *mb_set_y_sort_mode;
		godot_method_bind *mb_update_bitmask_area;
		godot_method_bind *mb_update_bitmask_region;
		godot_method_bind *mb_update_dirty_quadrants;
		godot_method_bind *mb_world_to_map;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "TileMap"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Mode {
		MODE_SQUARE = 0,
		MODE_ISOMETRIC = 1,
		MODE_CUSTOM = 2,
	};
	enum TileOrigin {
		TILE_ORIGIN_TOP_LEFT = 0,
		TILE_ORIGIN_CENTER = 1,
		TILE_ORIGIN_BOTTOM_LEFT = 2,
	};
	enum HalfOffset {
		HALF_OFFSET_X = 0,
		HALF_OFFSET_Y = 1,
		HALF_OFFSET_DISABLED = 2,
		HALF_OFFSET_NEGATIVE_X = 3,
		HALF_OFFSET_NEGATIVE_Y = 4,
	};

	// constants
	const static int INVALID_CELL = -1;


	static TileMap *_new();

	// methods
	void _clear_quadrants();
	int64_t _get_old_cell_size() const;
	PoolIntArray _get_tile_data() const;
	void _recreate_quadrants();
	void _set_celld(const Vector2 position, const Dictionary data);
	void _set_old_cell_size(const int64_t size);
	void _set_tile_data(const PoolIntArray arg0);
	void clear();
	void fix_invalid_tiles();
	int64_t get_cell(const int64_t x, const int64_t y) const;
	Vector2 get_cell_autotile_coord(const int64_t x, const int64_t y) const;
	Vector2 get_cell_size() const;
	int64_t get_cellv(const Vector2 position) const;
	bool get_clip_uv() const;
	real_t get_collision_bounce() const;
	real_t get_collision_friction() const;
	int64_t get_collision_layer() const;
	bool get_collision_layer_bit(const int64_t bit) const;
	int64_t get_collision_mask() const;
	bool get_collision_mask_bit(const int64_t bit) const;
	bool get_collision_use_kinematic() const;
	bool get_collision_use_parent() const;
	Transform2D get_custom_transform() const;
	TileMap::HalfOffset get_half_offset() const;
	TileMap::Mode get_mode() const;
	int64_t get_occluder_light_mask() const;
	int64_t get_quadrant_size() const;
	TileMap::TileOrigin get_tile_origin() const;
	Ref<TileSet> get_tileset() const;
	Array get_used_cells() const;
	Array get_used_cells_by_id(const int64_t id) const;
	Rect2 get_used_rect();
	bool is_cell_transposed(const int64_t x, const int64_t y) const;
	bool is_cell_x_flipped(const int64_t x, const int64_t y) const;
	bool is_cell_y_flipped(const int64_t x, const int64_t y) const;
	bool is_centered_textures_enabled() const;
	bool is_compatibility_mode_enabled() const;
	bool is_y_sort_mode_enabled() const;
	Vector2 map_to_world(const Vector2 map_position, const bool ignore_half_ofs = false) const;
	void set_cell(const int64_t x, const int64_t y, const int64_t tile, const bool flip_x = false, const bool flip_y = false, const bool transpose = false, const Vector2 autotile_coord = Vector2(0, 0));
	void set_cell_size(const Vector2 size);
	void set_cellv(const Vector2 position, const int64_t tile, const bool flip_x = false, const bool flip_y = false, const bool transpose = false);
	void set_centered_textures(const bool enable);
	void set_clip_uv(const bool enable);
	void set_collision_bounce(const real_t value);
	void set_collision_friction(const real_t value);
	void set_collision_layer(const int64_t layer);
	void set_collision_layer_bit(const int64_t bit, const bool value);
	void set_collision_mask(const int64_t mask);
	void set_collision_mask_bit(const int64_t bit, const bool value);
	void set_collision_use_kinematic(const bool use_kinematic);
	void set_collision_use_parent(const bool use_parent);
	void set_compatibility_mode(const bool enable);
	void set_custom_transform(const Transform2D custom_transform);
	void set_half_offset(const int64_t half_offset);
	void set_mode(const int64_t mode);
	void set_occluder_light_mask(const int64_t mask);
	void set_quadrant_size(const int64_t size);
	void set_tile_origin(const int64_t origin);
	void set_tileset(const Ref<TileSet> tileset);
	void set_y_sort_mode(const bool enable);
	void update_bitmask_area(const Vector2 position);
	void update_bitmask_region(const Vector2 start = Vector2(0, 0), const Vector2 end = Vector2(0, 0));
	void update_dirty_quadrants();
	Vector2 world_to_map(const Vector2 world_position) const;

};

}

#endif