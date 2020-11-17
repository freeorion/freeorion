#include "TileSet.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "OccluderPolygon2D.hpp"
#include "NavigationPolygon.hpp"
#include "Shape2D.hpp"
#include "ShaderMaterial.hpp"
#include "Texture.hpp"


namespace godot {


TileSet::___method_bindings TileSet::___mb = {};

void TileSet::___init_method_bindings() {
	___mb.mb__forward_atlas_subtile_selection = godot::api->godot_method_bind_get_method("TileSet", "_forward_atlas_subtile_selection");
	___mb.mb__forward_subtile_selection = godot::api->godot_method_bind_get_method("TileSet", "_forward_subtile_selection");
	___mb.mb__is_tile_bound = godot::api->godot_method_bind_get_method("TileSet", "_is_tile_bound");
	___mb.mb_autotile_clear_bitmask_map = godot::api->godot_method_bind_get_method("TileSet", "autotile_clear_bitmask_map");
	___mb.mb_autotile_get_bitmask = godot::api->godot_method_bind_get_method("TileSet", "autotile_get_bitmask");
	___mb.mb_autotile_get_bitmask_mode = godot::api->godot_method_bind_get_method("TileSet", "autotile_get_bitmask_mode");
	___mb.mb_autotile_get_icon_coordinate = godot::api->godot_method_bind_get_method("TileSet", "autotile_get_icon_coordinate");
	___mb.mb_autotile_get_light_occluder = godot::api->godot_method_bind_get_method("TileSet", "autotile_get_light_occluder");
	___mb.mb_autotile_get_navigation_polygon = godot::api->godot_method_bind_get_method("TileSet", "autotile_get_navigation_polygon");
	___mb.mb_autotile_get_size = godot::api->godot_method_bind_get_method("TileSet", "autotile_get_size");
	___mb.mb_autotile_get_spacing = godot::api->godot_method_bind_get_method("TileSet", "autotile_get_spacing");
	___mb.mb_autotile_get_subtile_priority = godot::api->godot_method_bind_get_method("TileSet", "autotile_get_subtile_priority");
	___mb.mb_autotile_get_z_index = godot::api->godot_method_bind_get_method("TileSet", "autotile_get_z_index");
	___mb.mb_autotile_set_bitmask = godot::api->godot_method_bind_get_method("TileSet", "autotile_set_bitmask");
	___mb.mb_autotile_set_bitmask_mode = godot::api->godot_method_bind_get_method("TileSet", "autotile_set_bitmask_mode");
	___mb.mb_autotile_set_icon_coordinate = godot::api->godot_method_bind_get_method("TileSet", "autotile_set_icon_coordinate");
	___mb.mb_autotile_set_light_occluder = godot::api->godot_method_bind_get_method("TileSet", "autotile_set_light_occluder");
	___mb.mb_autotile_set_navigation_polygon = godot::api->godot_method_bind_get_method("TileSet", "autotile_set_navigation_polygon");
	___mb.mb_autotile_set_size = godot::api->godot_method_bind_get_method("TileSet", "autotile_set_size");
	___mb.mb_autotile_set_spacing = godot::api->godot_method_bind_get_method("TileSet", "autotile_set_spacing");
	___mb.mb_autotile_set_subtile_priority = godot::api->godot_method_bind_get_method("TileSet", "autotile_set_subtile_priority");
	___mb.mb_autotile_set_z_index = godot::api->godot_method_bind_get_method("TileSet", "autotile_set_z_index");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("TileSet", "clear");
	___mb.mb_create_tile = godot::api->godot_method_bind_get_method("TileSet", "create_tile");
	___mb.mb_find_tile_by_name = godot::api->godot_method_bind_get_method("TileSet", "find_tile_by_name");
	___mb.mb_get_last_unused_tile_id = godot::api->godot_method_bind_get_method("TileSet", "get_last_unused_tile_id");
	___mb.mb_get_tiles_ids = godot::api->godot_method_bind_get_method("TileSet", "get_tiles_ids");
	___mb.mb_remove_tile = godot::api->godot_method_bind_get_method("TileSet", "remove_tile");
	___mb.mb_tile_add_shape = godot::api->godot_method_bind_get_method("TileSet", "tile_add_shape");
	___mb.mb_tile_get_light_occluder = godot::api->godot_method_bind_get_method("TileSet", "tile_get_light_occluder");
	___mb.mb_tile_get_material = godot::api->godot_method_bind_get_method("TileSet", "tile_get_material");
	___mb.mb_tile_get_modulate = godot::api->godot_method_bind_get_method("TileSet", "tile_get_modulate");
	___mb.mb_tile_get_name = godot::api->godot_method_bind_get_method("TileSet", "tile_get_name");
	___mb.mb_tile_get_navigation_polygon = godot::api->godot_method_bind_get_method("TileSet", "tile_get_navigation_polygon");
	___mb.mb_tile_get_navigation_polygon_offset = godot::api->godot_method_bind_get_method("TileSet", "tile_get_navigation_polygon_offset");
	___mb.mb_tile_get_normal_map = godot::api->godot_method_bind_get_method("TileSet", "tile_get_normal_map");
	___mb.mb_tile_get_occluder_offset = godot::api->godot_method_bind_get_method("TileSet", "tile_get_occluder_offset");
	___mb.mb_tile_get_region = godot::api->godot_method_bind_get_method("TileSet", "tile_get_region");
	___mb.mb_tile_get_shape = godot::api->godot_method_bind_get_method("TileSet", "tile_get_shape");
	___mb.mb_tile_get_shape_count = godot::api->godot_method_bind_get_method("TileSet", "tile_get_shape_count");
	___mb.mb_tile_get_shape_offset = godot::api->godot_method_bind_get_method("TileSet", "tile_get_shape_offset");
	___mb.mb_tile_get_shape_one_way = godot::api->godot_method_bind_get_method("TileSet", "tile_get_shape_one_way");
	___mb.mb_tile_get_shape_one_way_margin = godot::api->godot_method_bind_get_method("TileSet", "tile_get_shape_one_way_margin");
	___mb.mb_tile_get_shape_transform = godot::api->godot_method_bind_get_method("TileSet", "tile_get_shape_transform");
	___mb.mb_tile_get_shapes = godot::api->godot_method_bind_get_method("TileSet", "tile_get_shapes");
	___mb.mb_tile_get_texture = godot::api->godot_method_bind_get_method("TileSet", "tile_get_texture");
	___mb.mb_tile_get_texture_offset = godot::api->godot_method_bind_get_method("TileSet", "tile_get_texture_offset");
	___mb.mb_tile_get_tile_mode = godot::api->godot_method_bind_get_method("TileSet", "tile_get_tile_mode");
	___mb.mb_tile_get_z_index = godot::api->godot_method_bind_get_method("TileSet", "tile_get_z_index");
	___mb.mb_tile_set_light_occluder = godot::api->godot_method_bind_get_method("TileSet", "tile_set_light_occluder");
	___mb.mb_tile_set_material = godot::api->godot_method_bind_get_method("TileSet", "tile_set_material");
	___mb.mb_tile_set_modulate = godot::api->godot_method_bind_get_method("TileSet", "tile_set_modulate");
	___mb.mb_tile_set_name = godot::api->godot_method_bind_get_method("TileSet", "tile_set_name");
	___mb.mb_tile_set_navigation_polygon = godot::api->godot_method_bind_get_method("TileSet", "tile_set_navigation_polygon");
	___mb.mb_tile_set_navigation_polygon_offset = godot::api->godot_method_bind_get_method("TileSet", "tile_set_navigation_polygon_offset");
	___mb.mb_tile_set_normal_map = godot::api->godot_method_bind_get_method("TileSet", "tile_set_normal_map");
	___mb.mb_tile_set_occluder_offset = godot::api->godot_method_bind_get_method("TileSet", "tile_set_occluder_offset");
	___mb.mb_tile_set_region = godot::api->godot_method_bind_get_method("TileSet", "tile_set_region");
	___mb.mb_tile_set_shape = godot::api->godot_method_bind_get_method("TileSet", "tile_set_shape");
	___mb.mb_tile_set_shape_offset = godot::api->godot_method_bind_get_method("TileSet", "tile_set_shape_offset");
	___mb.mb_tile_set_shape_one_way = godot::api->godot_method_bind_get_method("TileSet", "tile_set_shape_one_way");
	___mb.mb_tile_set_shape_one_way_margin = godot::api->godot_method_bind_get_method("TileSet", "tile_set_shape_one_way_margin");
	___mb.mb_tile_set_shape_transform = godot::api->godot_method_bind_get_method("TileSet", "tile_set_shape_transform");
	___mb.mb_tile_set_shapes = godot::api->godot_method_bind_get_method("TileSet", "tile_set_shapes");
	___mb.mb_tile_set_texture = godot::api->godot_method_bind_get_method("TileSet", "tile_set_texture");
	___mb.mb_tile_set_texture_offset = godot::api->godot_method_bind_get_method("TileSet", "tile_set_texture_offset");
	___mb.mb_tile_set_tile_mode = godot::api->godot_method_bind_get_method("TileSet", "tile_set_tile_mode");
	___mb.mb_tile_set_z_index = godot::api->godot_method_bind_get_method("TileSet", "tile_set_z_index");
}

TileSet *TileSet::_new()
{
	return (TileSet *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"TileSet")());
}
Vector2 TileSet::_forward_atlas_subtile_selection(const int64_t atlastile_id, const Object *tilemap, const Vector2 tile_location) {
	return ___godot_icall_Vector2_int_Object_Vector2(___mb.mb__forward_atlas_subtile_selection, (const Object *) this, atlastile_id, tilemap, tile_location);
}

Vector2 TileSet::_forward_subtile_selection(const int64_t autotile_id, const int64_t bitmask, const Object *tilemap, const Vector2 tile_location) {
	return ___godot_icall_Vector2_int_int_Object_Vector2(___mb.mb__forward_subtile_selection, (const Object *) this, autotile_id, bitmask, tilemap, tile_location);
}

bool TileSet::_is_tile_bound(const int64_t drawn_id, const int64_t neighbor_id) {
	return ___godot_icall_bool_int_int(___mb.mb__is_tile_bound, (const Object *) this, drawn_id, neighbor_id);
}

void TileSet::autotile_clear_bitmask_map(const int64_t id) {
	___godot_icall_void_int(___mb.mb_autotile_clear_bitmask_map, (const Object *) this, id);
}

int64_t TileSet::autotile_get_bitmask(const int64_t id, const Vector2 coord) {
	return ___godot_icall_int_int_Vector2(___mb.mb_autotile_get_bitmask, (const Object *) this, id, coord);
}

TileSet::BitmaskMode TileSet::autotile_get_bitmask_mode(const int64_t id) const {
	return (TileSet::BitmaskMode) ___godot_icall_int_int(___mb.mb_autotile_get_bitmask_mode, (const Object *) this, id);
}

Vector2 TileSet::autotile_get_icon_coordinate(const int64_t id) const {
	return ___godot_icall_Vector2_int(___mb.mb_autotile_get_icon_coordinate, (const Object *) this, id);
}

Ref<OccluderPolygon2D> TileSet::autotile_get_light_occluder(const int64_t id, const Vector2 coord) const {
	return Ref<OccluderPolygon2D>::__internal_constructor(___godot_icall_Object_int_Vector2(___mb.mb_autotile_get_light_occluder, (const Object *) this, id, coord));
}

Ref<NavigationPolygon> TileSet::autotile_get_navigation_polygon(const int64_t id, const Vector2 coord) const {
	return Ref<NavigationPolygon>::__internal_constructor(___godot_icall_Object_int_Vector2(___mb.mb_autotile_get_navigation_polygon, (const Object *) this, id, coord));
}

Vector2 TileSet::autotile_get_size(const int64_t id) const {
	return ___godot_icall_Vector2_int(___mb.mb_autotile_get_size, (const Object *) this, id);
}

int64_t TileSet::autotile_get_spacing(const int64_t id) const {
	return ___godot_icall_int_int(___mb.mb_autotile_get_spacing, (const Object *) this, id);
}

int64_t TileSet::autotile_get_subtile_priority(const int64_t id, const Vector2 coord) {
	return ___godot_icall_int_int_Vector2(___mb.mb_autotile_get_subtile_priority, (const Object *) this, id, coord);
}

int64_t TileSet::autotile_get_z_index(const int64_t id, const Vector2 coord) {
	return ___godot_icall_int_int_Vector2(___mb.mb_autotile_get_z_index, (const Object *) this, id, coord);
}

void TileSet::autotile_set_bitmask(const int64_t id, const Vector2 bitmask, const int64_t flag) {
	___godot_icall_void_int_Vector2_int(___mb.mb_autotile_set_bitmask, (const Object *) this, id, bitmask, flag);
}

void TileSet::autotile_set_bitmask_mode(const int64_t id, const int64_t mode) {
	___godot_icall_void_int_int(___mb.mb_autotile_set_bitmask_mode, (const Object *) this, id, mode);
}

void TileSet::autotile_set_icon_coordinate(const int64_t id, const Vector2 coord) {
	___godot_icall_void_int_Vector2(___mb.mb_autotile_set_icon_coordinate, (const Object *) this, id, coord);
}

void TileSet::autotile_set_light_occluder(const int64_t id, const Ref<OccluderPolygon2D> light_occluder, const Vector2 coord) {
	___godot_icall_void_int_Object_Vector2(___mb.mb_autotile_set_light_occluder, (const Object *) this, id, light_occluder.ptr(), coord);
}

void TileSet::autotile_set_navigation_polygon(const int64_t id, const Ref<NavigationPolygon> navigation_polygon, const Vector2 coord) {
	___godot_icall_void_int_Object_Vector2(___mb.mb_autotile_set_navigation_polygon, (const Object *) this, id, navigation_polygon.ptr(), coord);
}

void TileSet::autotile_set_size(const int64_t id, const Vector2 size) {
	___godot_icall_void_int_Vector2(___mb.mb_autotile_set_size, (const Object *) this, id, size);
}

void TileSet::autotile_set_spacing(const int64_t id, const int64_t spacing) {
	___godot_icall_void_int_int(___mb.mb_autotile_set_spacing, (const Object *) this, id, spacing);
}

void TileSet::autotile_set_subtile_priority(const int64_t id, const Vector2 coord, const int64_t priority) {
	___godot_icall_void_int_Vector2_int(___mb.mb_autotile_set_subtile_priority, (const Object *) this, id, coord, priority);
}

void TileSet::autotile_set_z_index(const int64_t id, const Vector2 coord, const int64_t z_index) {
	___godot_icall_void_int_Vector2_int(___mb.mb_autotile_set_z_index, (const Object *) this, id, coord, z_index);
}

void TileSet::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

void TileSet::create_tile(const int64_t id) {
	___godot_icall_void_int(___mb.mb_create_tile, (const Object *) this, id);
}

int64_t TileSet::find_tile_by_name(const String name) const {
	return ___godot_icall_int_String(___mb.mb_find_tile_by_name, (const Object *) this, name);
}

int64_t TileSet::get_last_unused_tile_id() const {
	return ___godot_icall_int(___mb.mb_get_last_unused_tile_id, (const Object *) this);
}

Array TileSet::get_tiles_ids() const {
	return ___godot_icall_Array(___mb.mb_get_tiles_ids, (const Object *) this);
}

void TileSet::remove_tile(const int64_t id) {
	___godot_icall_void_int(___mb.mb_remove_tile, (const Object *) this, id);
}

void TileSet::tile_add_shape(const int64_t id, const Ref<Shape2D> shape, const Transform2D shape_transform, const bool one_way, const Vector2 autotile_coord) {
	___godot_icall_void_int_Object_Transform2D_bool_Vector2(___mb.mb_tile_add_shape, (const Object *) this, id, shape.ptr(), shape_transform, one_way, autotile_coord);
}

Ref<OccluderPolygon2D> TileSet::tile_get_light_occluder(const int64_t id) const {
	return Ref<OccluderPolygon2D>::__internal_constructor(___godot_icall_Object_int(___mb.mb_tile_get_light_occluder, (const Object *) this, id));
}

Ref<ShaderMaterial> TileSet::tile_get_material(const int64_t id) const {
	return Ref<ShaderMaterial>::__internal_constructor(___godot_icall_Object_int(___mb.mb_tile_get_material, (const Object *) this, id));
}

Color TileSet::tile_get_modulate(const int64_t id) const {
	return ___godot_icall_Color_int(___mb.mb_tile_get_modulate, (const Object *) this, id);
}

String TileSet::tile_get_name(const int64_t id) const {
	return ___godot_icall_String_int(___mb.mb_tile_get_name, (const Object *) this, id);
}

Ref<NavigationPolygon> TileSet::tile_get_navigation_polygon(const int64_t id) const {
	return Ref<NavigationPolygon>::__internal_constructor(___godot_icall_Object_int(___mb.mb_tile_get_navigation_polygon, (const Object *) this, id));
}

Vector2 TileSet::tile_get_navigation_polygon_offset(const int64_t id) const {
	return ___godot_icall_Vector2_int(___mb.mb_tile_get_navigation_polygon_offset, (const Object *) this, id);
}

Ref<Texture> TileSet::tile_get_normal_map(const int64_t id) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_int(___mb.mb_tile_get_normal_map, (const Object *) this, id));
}

Vector2 TileSet::tile_get_occluder_offset(const int64_t id) const {
	return ___godot_icall_Vector2_int(___mb.mb_tile_get_occluder_offset, (const Object *) this, id);
}

Rect2 TileSet::tile_get_region(const int64_t id) const {
	return ___godot_icall_Rect2_int(___mb.mb_tile_get_region, (const Object *) this, id);
}

Ref<Shape2D> TileSet::tile_get_shape(const int64_t id, const int64_t shape_id) const {
	return Ref<Shape2D>::__internal_constructor(___godot_icall_Object_int_int(___mb.mb_tile_get_shape, (const Object *) this, id, shape_id));
}

int64_t TileSet::tile_get_shape_count(const int64_t id) const {
	return ___godot_icall_int_int(___mb.mb_tile_get_shape_count, (const Object *) this, id);
}

Vector2 TileSet::tile_get_shape_offset(const int64_t id, const int64_t shape_id) const {
	return ___godot_icall_Vector2_int_int(___mb.mb_tile_get_shape_offset, (const Object *) this, id, shape_id);
}

bool TileSet::tile_get_shape_one_way(const int64_t id, const int64_t shape_id) const {
	return ___godot_icall_bool_int_int(___mb.mb_tile_get_shape_one_way, (const Object *) this, id, shape_id);
}

real_t TileSet::tile_get_shape_one_way_margin(const int64_t id, const int64_t shape_id) const {
	return ___godot_icall_float_int_int(___mb.mb_tile_get_shape_one_way_margin, (const Object *) this, id, shape_id);
}

Transform2D TileSet::tile_get_shape_transform(const int64_t id, const int64_t shape_id) const {
	return ___godot_icall_Transform2D_int_int(___mb.mb_tile_get_shape_transform, (const Object *) this, id, shape_id);
}

Array TileSet::tile_get_shapes(const int64_t id) const {
	return ___godot_icall_Array_int(___mb.mb_tile_get_shapes, (const Object *) this, id);
}

Ref<Texture> TileSet::tile_get_texture(const int64_t id) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_int(___mb.mb_tile_get_texture, (const Object *) this, id));
}

Vector2 TileSet::tile_get_texture_offset(const int64_t id) const {
	return ___godot_icall_Vector2_int(___mb.mb_tile_get_texture_offset, (const Object *) this, id);
}

TileSet::TileMode TileSet::tile_get_tile_mode(const int64_t id) const {
	return (TileSet::TileMode) ___godot_icall_int_int(___mb.mb_tile_get_tile_mode, (const Object *) this, id);
}

int64_t TileSet::tile_get_z_index(const int64_t id) const {
	return ___godot_icall_int_int(___mb.mb_tile_get_z_index, (const Object *) this, id);
}

void TileSet::tile_set_light_occluder(const int64_t id, const Ref<OccluderPolygon2D> light_occluder) {
	___godot_icall_void_int_Object(___mb.mb_tile_set_light_occluder, (const Object *) this, id, light_occluder.ptr());
}

void TileSet::tile_set_material(const int64_t id, const Ref<ShaderMaterial> material) {
	___godot_icall_void_int_Object(___mb.mb_tile_set_material, (const Object *) this, id, material.ptr());
}

void TileSet::tile_set_modulate(const int64_t id, const Color color) {
	___godot_icall_void_int_Color(___mb.mb_tile_set_modulate, (const Object *) this, id, color);
}

void TileSet::tile_set_name(const int64_t id, const String name) {
	___godot_icall_void_int_String(___mb.mb_tile_set_name, (const Object *) this, id, name);
}

void TileSet::tile_set_navigation_polygon(const int64_t id, const Ref<NavigationPolygon> navigation_polygon) {
	___godot_icall_void_int_Object(___mb.mb_tile_set_navigation_polygon, (const Object *) this, id, navigation_polygon.ptr());
}

void TileSet::tile_set_navigation_polygon_offset(const int64_t id, const Vector2 navigation_polygon_offset) {
	___godot_icall_void_int_Vector2(___mb.mb_tile_set_navigation_polygon_offset, (const Object *) this, id, navigation_polygon_offset);
}

void TileSet::tile_set_normal_map(const int64_t id, const Ref<Texture> normal_map) {
	___godot_icall_void_int_Object(___mb.mb_tile_set_normal_map, (const Object *) this, id, normal_map.ptr());
}

void TileSet::tile_set_occluder_offset(const int64_t id, const Vector2 occluder_offset) {
	___godot_icall_void_int_Vector2(___mb.mb_tile_set_occluder_offset, (const Object *) this, id, occluder_offset);
}

void TileSet::tile_set_region(const int64_t id, const Rect2 region) {
	___godot_icall_void_int_Rect2(___mb.mb_tile_set_region, (const Object *) this, id, region);
}

void TileSet::tile_set_shape(const int64_t id, const int64_t shape_id, const Ref<Shape2D> shape) {
	___godot_icall_void_int_int_Object(___mb.mb_tile_set_shape, (const Object *) this, id, shape_id, shape.ptr());
}

void TileSet::tile_set_shape_offset(const int64_t id, const int64_t shape_id, const Vector2 shape_offset) {
	___godot_icall_void_int_int_Vector2(___mb.mb_tile_set_shape_offset, (const Object *) this, id, shape_id, shape_offset);
}

void TileSet::tile_set_shape_one_way(const int64_t id, const int64_t shape_id, const bool one_way) {
	___godot_icall_void_int_int_bool(___mb.mb_tile_set_shape_one_way, (const Object *) this, id, shape_id, one_way);
}

void TileSet::tile_set_shape_one_way_margin(const int64_t id, const int64_t shape_id, const real_t one_way) {
	___godot_icall_void_int_int_float(___mb.mb_tile_set_shape_one_way_margin, (const Object *) this, id, shape_id, one_way);
}

void TileSet::tile_set_shape_transform(const int64_t id, const int64_t shape_id, const Transform2D shape_transform) {
	___godot_icall_void_int_int_Transform2D(___mb.mb_tile_set_shape_transform, (const Object *) this, id, shape_id, shape_transform);
}

void TileSet::tile_set_shapes(const int64_t id, const Array shapes) {
	___godot_icall_void_int_Array(___mb.mb_tile_set_shapes, (const Object *) this, id, shapes);
}

void TileSet::tile_set_texture(const int64_t id, const Ref<Texture> texture) {
	___godot_icall_void_int_Object(___mb.mb_tile_set_texture, (const Object *) this, id, texture.ptr());
}

void TileSet::tile_set_texture_offset(const int64_t id, const Vector2 texture_offset) {
	___godot_icall_void_int_Vector2(___mb.mb_tile_set_texture_offset, (const Object *) this, id, texture_offset);
}

void TileSet::tile_set_tile_mode(const int64_t id, const int64_t tilemode) {
	___godot_icall_void_int_int(___mb.mb_tile_set_tile_mode, (const Object *) this, id, tilemode);
}

void TileSet::tile_set_z_index(const int64_t id, const int64_t z_index) {
	___godot_icall_void_int_int(___mb.mb_tile_set_z_index, (const Object *) this, id, z_index);
}

}