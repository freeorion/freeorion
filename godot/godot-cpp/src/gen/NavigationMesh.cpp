#include "NavigationMesh.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Mesh.hpp"


namespace godot {


NavigationMesh::___method_bindings NavigationMesh::___mb = {};

void NavigationMesh::___init_method_bindings() {
	___mb.mb__get_polygons = godot::api->godot_method_bind_get_method("NavigationMesh", "_get_polygons");
	___mb.mb__set_polygons = godot::api->godot_method_bind_get_method("NavigationMesh", "_set_polygons");
	___mb.mb_add_polygon = godot::api->godot_method_bind_get_method("NavigationMesh", "add_polygon");
	___mb.mb_clear_polygons = godot::api->godot_method_bind_get_method("NavigationMesh", "clear_polygons");
	___mb.mb_create_from_mesh = godot::api->godot_method_bind_get_method("NavigationMesh", "create_from_mesh");
	___mb.mb_get_agent_height = godot::api->godot_method_bind_get_method("NavigationMesh", "get_agent_height");
	___mb.mb_get_agent_max_climb = godot::api->godot_method_bind_get_method("NavigationMesh", "get_agent_max_climb");
	___mb.mb_get_agent_max_slope = godot::api->godot_method_bind_get_method("NavigationMesh", "get_agent_max_slope");
	___mb.mb_get_agent_radius = godot::api->godot_method_bind_get_method("NavigationMesh", "get_agent_radius");
	___mb.mb_get_cell_height = godot::api->godot_method_bind_get_method("NavigationMesh", "get_cell_height");
	___mb.mb_get_cell_size = godot::api->godot_method_bind_get_method("NavigationMesh", "get_cell_size");
	___mb.mb_get_collision_mask = godot::api->godot_method_bind_get_method("NavigationMesh", "get_collision_mask");
	___mb.mb_get_collision_mask_bit = godot::api->godot_method_bind_get_method("NavigationMesh", "get_collision_mask_bit");
	___mb.mb_get_detail_sample_distance = godot::api->godot_method_bind_get_method("NavigationMesh", "get_detail_sample_distance");
	___mb.mb_get_detail_sample_max_error = godot::api->godot_method_bind_get_method("NavigationMesh", "get_detail_sample_max_error");
	___mb.mb_get_edge_max_error = godot::api->godot_method_bind_get_method("NavigationMesh", "get_edge_max_error");
	___mb.mb_get_edge_max_length = godot::api->godot_method_bind_get_method("NavigationMesh", "get_edge_max_length");
	___mb.mb_get_filter_ledge_spans = godot::api->godot_method_bind_get_method("NavigationMesh", "get_filter_ledge_spans");
	___mb.mb_get_filter_low_hanging_obstacles = godot::api->godot_method_bind_get_method("NavigationMesh", "get_filter_low_hanging_obstacles");
	___mb.mb_get_filter_walkable_low_height_spans = godot::api->godot_method_bind_get_method("NavigationMesh", "get_filter_walkable_low_height_spans");
	___mb.mb_get_parsed_geometry_type = godot::api->godot_method_bind_get_method("NavigationMesh", "get_parsed_geometry_type");
	___mb.mb_get_polygon = godot::api->godot_method_bind_get_method("NavigationMesh", "get_polygon");
	___mb.mb_get_polygon_count = godot::api->godot_method_bind_get_method("NavigationMesh", "get_polygon_count");
	___mb.mb_get_region_merge_size = godot::api->godot_method_bind_get_method("NavigationMesh", "get_region_merge_size");
	___mb.mb_get_region_min_size = godot::api->godot_method_bind_get_method("NavigationMesh", "get_region_min_size");
	___mb.mb_get_sample_partition_type = godot::api->godot_method_bind_get_method("NavigationMesh", "get_sample_partition_type");
	___mb.mb_get_source_geometry_mode = godot::api->godot_method_bind_get_method("NavigationMesh", "get_source_geometry_mode");
	___mb.mb_get_source_group_name = godot::api->godot_method_bind_get_method("NavigationMesh", "get_source_group_name");
	___mb.mb_get_vertices = godot::api->godot_method_bind_get_method("NavigationMesh", "get_vertices");
	___mb.mb_get_verts_per_poly = godot::api->godot_method_bind_get_method("NavigationMesh", "get_verts_per_poly");
	___mb.mb_set_agent_height = godot::api->godot_method_bind_get_method("NavigationMesh", "set_agent_height");
	___mb.mb_set_agent_max_climb = godot::api->godot_method_bind_get_method("NavigationMesh", "set_agent_max_climb");
	___mb.mb_set_agent_max_slope = godot::api->godot_method_bind_get_method("NavigationMesh", "set_agent_max_slope");
	___mb.mb_set_agent_radius = godot::api->godot_method_bind_get_method("NavigationMesh", "set_agent_radius");
	___mb.mb_set_cell_height = godot::api->godot_method_bind_get_method("NavigationMesh", "set_cell_height");
	___mb.mb_set_cell_size = godot::api->godot_method_bind_get_method("NavigationMesh", "set_cell_size");
	___mb.mb_set_collision_mask = godot::api->godot_method_bind_get_method("NavigationMesh", "set_collision_mask");
	___mb.mb_set_collision_mask_bit = godot::api->godot_method_bind_get_method("NavigationMesh", "set_collision_mask_bit");
	___mb.mb_set_detail_sample_distance = godot::api->godot_method_bind_get_method("NavigationMesh", "set_detail_sample_distance");
	___mb.mb_set_detail_sample_max_error = godot::api->godot_method_bind_get_method("NavigationMesh", "set_detail_sample_max_error");
	___mb.mb_set_edge_max_error = godot::api->godot_method_bind_get_method("NavigationMesh", "set_edge_max_error");
	___mb.mb_set_edge_max_length = godot::api->godot_method_bind_get_method("NavigationMesh", "set_edge_max_length");
	___mb.mb_set_filter_ledge_spans = godot::api->godot_method_bind_get_method("NavigationMesh", "set_filter_ledge_spans");
	___mb.mb_set_filter_low_hanging_obstacles = godot::api->godot_method_bind_get_method("NavigationMesh", "set_filter_low_hanging_obstacles");
	___mb.mb_set_filter_walkable_low_height_spans = godot::api->godot_method_bind_get_method("NavigationMesh", "set_filter_walkable_low_height_spans");
	___mb.mb_set_parsed_geometry_type = godot::api->godot_method_bind_get_method("NavigationMesh", "set_parsed_geometry_type");
	___mb.mb_set_region_merge_size = godot::api->godot_method_bind_get_method("NavigationMesh", "set_region_merge_size");
	___mb.mb_set_region_min_size = godot::api->godot_method_bind_get_method("NavigationMesh", "set_region_min_size");
	___mb.mb_set_sample_partition_type = godot::api->godot_method_bind_get_method("NavigationMesh", "set_sample_partition_type");
	___mb.mb_set_source_geometry_mode = godot::api->godot_method_bind_get_method("NavigationMesh", "set_source_geometry_mode");
	___mb.mb_set_source_group_name = godot::api->godot_method_bind_get_method("NavigationMesh", "set_source_group_name");
	___mb.mb_set_vertices = godot::api->godot_method_bind_get_method("NavigationMesh", "set_vertices");
	___mb.mb_set_verts_per_poly = godot::api->godot_method_bind_get_method("NavigationMesh", "set_verts_per_poly");
}

NavigationMesh *NavigationMesh::_new()
{
	return (NavigationMesh *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"NavigationMesh")());
}
Array NavigationMesh::_get_polygons() const {
	return ___godot_icall_Array(___mb.mb__get_polygons, (const Object *) this);
}

void NavigationMesh::_set_polygons(const Array polygons) {
	___godot_icall_void_Array(___mb.mb__set_polygons, (const Object *) this, polygons);
}

void NavigationMesh::add_polygon(const PoolIntArray polygon) {
	___godot_icall_void_PoolIntArray(___mb.mb_add_polygon, (const Object *) this, polygon);
}

void NavigationMesh::clear_polygons() {
	___godot_icall_void(___mb.mb_clear_polygons, (const Object *) this);
}

void NavigationMesh::create_from_mesh(const Ref<Mesh> mesh) {
	___godot_icall_void_Object(___mb.mb_create_from_mesh, (const Object *) this, mesh.ptr());
}

real_t NavigationMesh::get_agent_height() const {
	return ___godot_icall_float(___mb.mb_get_agent_height, (const Object *) this);
}

real_t NavigationMesh::get_agent_max_climb() const {
	return ___godot_icall_float(___mb.mb_get_agent_max_climb, (const Object *) this);
}

real_t NavigationMesh::get_agent_max_slope() const {
	return ___godot_icall_float(___mb.mb_get_agent_max_slope, (const Object *) this);
}

real_t NavigationMesh::get_agent_radius() {
	return ___godot_icall_float(___mb.mb_get_agent_radius, (const Object *) this);
}

real_t NavigationMesh::get_cell_height() const {
	return ___godot_icall_float(___mb.mb_get_cell_height, (const Object *) this);
}

real_t NavigationMesh::get_cell_size() const {
	return ___godot_icall_float(___mb.mb_get_cell_size, (const Object *) this);
}

int64_t NavigationMesh::get_collision_mask() const {
	return ___godot_icall_int(___mb.mb_get_collision_mask, (const Object *) this);
}

bool NavigationMesh::get_collision_mask_bit(const int64_t bit) const {
	return ___godot_icall_bool_int(___mb.mb_get_collision_mask_bit, (const Object *) this, bit);
}

real_t NavigationMesh::get_detail_sample_distance() const {
	return ___godot_icall_float(___mb.mb_get_detail_sample_distance, (const Object *) this);
}

real_t NavigationMesh::get_detail_sample_max_error() const {
	return ___godot_icall_float(___mb.mb_get_detail_sample_max_error, (const Object *) this);
}

real_t NavigationMesh::get_edge_max_error() const {
	return ___godot_icall_float(___mb.mb_get_edge_max_error, (const Object *) this);
}

real_t NavigationMesh::get_edge_max_length() const {
	return ___godot_icall_float(___mb.mb_get_edge_max_length, (const Object *) this);
}

bool NavigationMesh::get_filter_ledge_spans() const {
	return ___godot_icall_bool(___mb.mb_get_filter_ledge_spans, (const Object *) this);
}

bool NavigationMesh::get_filter_low_hanging_obstacles() const {
	return ___godot_icall_bool(___mb.mb_get_filter_low_hanging_obstacles, (const Object *) this);
}

bool NavigationMesh::get_filter_walkable_low_height_spans() const {
	return ___godot_icall_bool(___mb.mb_get_filter_walkable_low_height_spans, (const Object *) this);
}

int64_t NavigationMesh::get_parsed_geometry_type() const {
	return ___godot_icall_int(___mb.mb_get_parsed_geometry_type, (const Object *) this);
}

PoolIntArray NavigationMesh::get_polygon(const int64_t idx) {
	return ___godot_icall_PoolIntArray_int(___mb.mb_get_polygon, (const Object *) this, idx);
}

int64_t NavigationMesh::get_polygon_count() const {
	return ___godot_icall_int(___mb.mb_get_polygon_count, (const Object *) this);
}

real_t NavigationMesh::get_region_merge_size() const {
	return ___godot_icall_float(___mb.mb_get_region_merge_size, (const Object *) this);
}

real_t NavigationMesh::get_region_min_size() const {
	return ___godot_icall_float(___mb.mb_get_region_min_size, (const Object *) this);
}

int64_t NavigationMesh::get_sample_partition_type() const {
	return ___godot_icall_int(___mb.mb_get_sample_partition_type, (const Object *) this);
}

int64_t NavigationMesh::get_source_geometry_mode() const {
	return ___godot_icall_int(___mb.mb_get_source_geometry_mode, (const Object *) this);
}

String NavigationMesh::get_source_group_name() const {
	return ___godot_icall_String(___mb.mb_get_source_group_name, (const Object *) this);
}

PoolVector3Array NavigationMesh::get_vertices() const {
	return ___godot_icall_PoolVector3Array(___mb.mb_get_vertices, (const Object *) this);
}

real_t NavigationMesh::get_verts_per_poly() const {
	return ___godot_icall_float(___mb.mb_get_verts_per_poly, (const Object *) this);
}

void NavigationMesh::set_agent_height(const real_t agent_height) {
	___godot_icall_void_float(___mb.mb_set_agent_height, (const Object *) this, agent_height);
}

void NavigationMesh::set_agent_max_climb(const real_t agent_max_climb) {
	___godot_icall_void_float(___mb.mb_set_agent_max_climb, (const Object *) this, agent_max_climb);
}

void NavigationMesh::set_agent_max_slope(const real_t agent_max_slope) {
	___godot_icall_void_float(___mb.mb_set_agent_max_slope, (const Object *) this, agent_max_slope);
}

void NavigationMesh::set_agent_radius(const real_t agent_radius) {
	___godot_icall_void_float(___mb.mb_set_agent_radius, (const Object *) this, agent_radius);
}

void NavigationMesh::set_cell_height(const real_t cell_height) {
	___godot_icall_void_float(___mb.mb_set_cell_height, (const Object *) this, cell_height);
}

void NavigationMesh::set_cell_size(const real_t cell_size) {
	___godot_icall_void_float(___mb.mb_set_cell_size, (const Object *) this, cell_size);
}

void NavigationMesh::set_collision_mask(const int64_t mask) {
	___godot_icall_void_int(___mb.mb_set_collision_mask, (const Object *) this, mask);
}

void NavigationMesh::set_collision_mask_bit(const int64_t bit, const bool value) {
	___godot_icall_void_int_bool(___mb.mb_set_collision_mask_bit, (const Object *) this, bit, value);
}

void NavigationMesh::set_detail_sample_distance(const real_t detail_sample_dist) {
	___godot_icall_void_float(___mb.mb_set_detail_sample_distance, (const Object *) this, detail_sample_dist);
}

void NavigationMesh::set_detail_sample_max_error(const real_t detail_sample_max_error) {
	___godot_icall_void_float(___mb.mb_set_detail_sample_max_error, (const Object *) this, detail_sample_max_error);
}

void NavigationMesh::set_edge_max_error(const real_t edge_max_error) {
	___godot_icall_void_float(___mb.mb_set_edge_max_error, (const Object *) this, edge_max_error);
}

void NavigationMesh::set_edge_max_length(const real_t edge_max_length) {
	___godot_icall_void_float(___mb.mb_set_edge_max_length, (const Object *) this, edge_max_length);
}

void NavigationMesh::set_filter_ledge_spans(const bool filter_ledge_spans) {
	___godot_icall_void_bool(___mb.mb_set_filter_ledge_spans, (const Object *) this, filter_ledge_spans);
}

void NavigationMesh::set_filter_low_hanging_obstacles(const bool filter_low_hanging_obstacles) {
	___godot_icall_void_bool(___mb.mb_set_filter_low_hanging_obstacles, (const Object *) this, filter_low_hanging_obstacles);
}

void NavigationMesh::set_filter_walkable_low_height_spans(const bool filter_walkable_low_height_spans) {
	___godot_icall_void_bool(___mb.mb_set_filter_walkable_low_height_spans, (const Object *) this, filter_walkable_low_height_spans);
}

void NavigationMesh::set_parsed_geometry_type(const int64_t geometry_type) {
	___godot_icall_void_int(___mb.mb_set_parsed_geometry_type, (const Object *) this, geometry_type);
}

void NavigationMesh::set_region_merge_size(const real_t region_merge_size) {
	___godot_icall_void_float(___mb.mb_set_region_merge_size, (const Object *) this, region_merge_size);
}

void NavigationMesh::set_region_min_size(const real_t region_min_size) {
	___godot_icall_void_float(___mb.mb_set_region_min_size, (const Object *) this, region_min_size);
}

void NavigationMesh::set_sample_partition_type(const int64_t sample_partition_type) {
	___godot_icall_void_int(___mb.mb_set_sample_partition_type, (const Object *) this, sample_partition_type);
}

void NavigationMesh::set_source_geometry_mode(const int64_t mask) {
	___godot_icall_void_int(___mb.mb_set_source_geometry_mode, (const Object *) this, mask);
}

void NavigationMesh::set_source_group_name(const String mask) {
	___godot_icall_void_String(___mb.mb_set_source_group_name, (const Object *) this, mask);
}

void NavigationMesh::set_vertices(const PoolVector3Array vertices) {
	___godot_icall_void_PoolVector3Array(___mb.mb_set_vertices, (const Object *) this, vertices);
}

void NavigationMesh::set_verts_per_poly(const real_t verts_per_poly) {
	___godot_icall_void_float(___mb.mb_set_verts_per_poly, (const Object *) this, verts_per_poly);
}

}