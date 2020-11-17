#include "Geometry.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Geometry *Geometry::_singleton = NULL;


Geometry::Geometry() {
	_owner = godot::api->godot_global_get_singleton((char *) "Geometry");
}


Geometry::___method_bindings Geometry::___mb = {};

void Geometry::___init_method_bindings() {
	___mb.mb_build_box_planes = godot::api->godot_method_bind_get_method("_Geometry", "build_box_planes");
	___mb.mb_build_capsule_planes = godot::api->godot_method_bind_get_method("_Geometry", "build_capsule_planes");
	___mb.mb_build_cylinder_planes = godot::api->godot_method_bind_get_method("_Geometry", "build_cylinder_planes");
	___mb.mb_clip_polygon = godot::api->godot_method_bind_get_method("_Geometry", "clip_polygon");
	___mb.mb_clip_polygons_2d = godot::api->godot_method_bind_get_method("_Geometry", "clip_polygons_2d");
	___mb.mb_clip_polyline_with_polygon_2d = godot::api->godot_method_bind_get_method("_Geometry", "clip_polyline_with_polygon_2d");
	___mb.mb_convex_hull_2d = godot::api->godot_method_bind_get_method("_Geometry", "convex_hull_2d");
	___mb.mb_exclude_polygons_2d = godot::api->godot_method_bind_get_method("_Geometry", "exclude_polygons_2d");
	___mb.mb_get_closest_point_to_segment = godot::api->godot_method_bind_get_method("_Geometry", "get_closest_point_to_segment");
	___mb.mb_get_closest_point_to_segment_2d = godot::api->godot_method_bind_get_method("_Geometry", "get_closest_point_to_segment_2d");
	___mb.mb_get_closest_point_to_segment_uncapped = godot::api->godot_method_bind_get_method("_Geometry", "get_closest_point_to_segment_uncapped");
	___mb.mb_get_closest_point_to_segment_uncapped_2d = godot::api->godot_method_bind_get_method("_Geometry", "get_closest_point_to_segment_uncapped_2d");
	___mb.mb_get_closest_points_between_segments = godot::api->godot_method_bind_get_method("_Geometry", "get_closest_points_between_segments");
	___mb.mb_get_closest_points_between_segments_2d = godot::api->godot_method_bind_get_method("_Geometry", "get_closest_points_between_segments_2d");
	___mb.mb_get_uv84_normal_bit = godot::api->godot_method_bind_get_method("_Geometry", "get_uv84_normal_bit");
	___mb.mb_intersect_polygons_2d = godot::api->godot_method_bind_get_method("_Geometry", "intersect_polygons_2d");
	___mb.mb_intersect_polyline_with_polygon_2d = godot::api->godot_method_bind_get_method("_Geometry", "intersect_polyline_with_polygon_2d");
	___mb.mb_is_point_in_circle = godot::api->godot_method_bind_get_method("_Geometry", "is_point_in_circle");
	___mb.mb_is_point_in_polygon = godot::api->godot_method_bind_get_method("_Geometry", "is_point_in_polygon");
	___mb.mb_is_polygon_clockwise = godot::api->godot_method_bind_get_method("_Geometry", "is_polygon_clockwise");
	___mb.mb_line_intersects_line_2d = godot::api->godot_method_bind_get_method("_Geometry", "line_intersects_line_2d");
	___mb.mb_make_atlas = godot::api->godot_method_bind_get_method("_Geometry", "make_atlas");
	___mb.mb_merge_polygons_2d = godot::api->godot_method_bind_get_method("_Geometry", "merge_polygons_2d");
	___mb.mb_offset_polygon_2d = godot::api->godot_method_bind_get_method("_Geometry", "offset_polygon_2d");
	___mb.mb_offset_polyline_2d = godot::api->godot_method_bind_get_method("_Geometry", "offset_polyline_2d");
	___mb.mb_point_is_inside_triangle = godot::api->godot_method_bind_get_method("_Geometry", "point_is_inside_triangle");
	___mb.mb_ray_intersects_triangle = godot::api->godot_method_bind_get_method("_Geometry", "ray_intersects_triangle");
	___mb.mb_segment_intersects_circle = godot::api->godot_method_bind_get_method("_Geometry", "segment_intersects_circle");
	___mb.mb_segment_intersects_convex = godot::api->godot_method_bind_get_method("_Geometry", "segment_intersects_convex");
	___mb.mb_segment_intersects_cylinder = godot::api->godot_method_bind_get_method("_Geometry", "segment_intersects_cylinder");
	___mb.mb_segment_intersects_segment_2d = godot::api->godot_method_bind_get_method("_Geometry", "segment_intersects_segment_2d");
	___mb.mb_segment_intersects_sphere = godot::api->godot_method_bind_get_method("_Geometry", "segment_intersects_sphere");
	___mb.mb_segment_intersects_triangle = godot::api->godot_method_bind_get_method("_Geometry", "segment_intersects_triangle");
	___mb.mb_triangulate_delaunay_2d = godot::api->godot_method_bind_get_method("_Geometry", "triangulate_delaunay_2d");
	___mb.mb_triangulate_polygon = godot::api->godot_method_bind_get_method("_Geometry", "triangulate_polygon");
}

Array Geometry::build_box_planes(const Vector3 extents) {
	return ___godot_icall_Array_Vector3(___mb.mb_build_box_planes, (const Object *) this, extents);
}

Array Geometry::build_capsule_planes(const real_t radius, const real_t height, const int64_t sides, const int64_t lats, const int64_t axis) {
	return ___godot_icall_Array_float_float_int_int_int(___mb.mb_build_capsule_planes, (const Object *) this, radius, height, sides, lats, axis);
}

Array Geometry::build_cylinder_planes(const real_t radius, const real_t height, const int64_t sides, const int64_t axis) {
	return ___godot_icall_Array_float_float_int_int(___mb.mb_build_cylinder_planes, (const Object *) this, radius, height, sides, axis);
}

PoolVector3Array Geometry::clip_polygon(const PoolVector3Array points, const Plane plane) {
	return ___godot_icall_PoolVector3Array_PoolVector3Array_Plane(___mb.mb_clip_polygon, (const Object *) this, points, plane);
}

Array Geometry::clip_polygons_2d(const PoolVector2Array polygon_a, const PoolVector2Array polygon_b) {
	return ___godot_icall_Array_PoolVector2Array_PoolVector2Array(___mb.mb_clip_polygons_2d, (const Object *) this, polygon_a, polygon_b);
}

Array Geometry::clip_polyline_with_polygon_2d(const PoolVector2Array polyline, const PoolVector2Array polygon) {
	return ___godot_icall_Array_PoolVector2Array_PoolVector2Array(___mb.mb_clip_polyline_with_polygon_2d, (const Object *) this, polyline, polygon);
}

PoolVector2Array Geometry::convex_hull_2d(const PoolVector2Array points) {
	return ___godot_icall_PoolVector2Array_PoolVector2Array(___mb.mb_convex_hull_2d, (const Object *) this, points);
}

Array Geometry::exclude_polygons_2d(const PoolVector2Array polygon_a, const PoolVector2Array polygon_b) {
	return ___godot_icall_Array_PoolVector2Array_PoolVector2Array(___mb.mb_exclude_polygons_2d, (const Object *) this, polygon_a, polygon_b);
}

Vector3 Geometry::get_closest_point_to_segment(const Vector3 point, const Vector3 s1, const Vector3 s2) {
	return ___godot_icall_Vector3_Vector3_Vector3_Vector3(___mb.mb_get_closest_point_to_segment, (const Object *) this, point, s1, s2);
}

Vector2 Geometry::get_closest_point_to_segment_2d(const Vector2 point, const Vector2 s1, const Vector2 s2) {
	return ___godot_icall_Vector2_Vector2_Vector2_Vector2(___mb.mb_get_closest_point_to_segment_2d, (const Object *) this, point, s1, s2);
}

Vector3 Geometry::get_closest_point_to_segment_uncapped(const Vector3 point, const Vector3 s1, const Vector3 s2) {
	return ___godot_icall_Vector3_Vector3_Vector3_Vector3(___mb.mb_get_closest_point_to_segment_uncapped, (const Object *) this, point, s1, s2);
}

Vector2 Geometry::get_closest_point_to_segment_uncapped_2d(const Vector2 point, const Vector2 s1, const Vector2 s2) {
	return ___godot_icall_Vector2_Vector2_Vector2_Vector2(___mb.mb_get_closest_point_to_segment_uncapped_2d, (const Object *) this, point, s1, s2);
}

PoolVector3Array Geometry::get_closest_points_between_segments(const Vector3 p1, const Vector3 p2, const Vector3 q1, const Vector3 q2) {
	return ___godot_icall_PoolVector3Array_Vector3_Vector3_Vector3_Vector3(___mb.mb_get_closest_points_between_segments, (const Object *) this, p1, p2, q1, q2);
}

PoolVector2Array Geometry::get_closest_points_between_segments_2d(const Vector2 p1, const Vector2 q1, const Vector2 p2, const Vector2 q2) {
	return ___godot_icall_PoolVector2Array_Vector2_Vector2_Vector2_Vector2(___mb.mb_get_closest_points_between_segments_2d, (const Object *) this, p1, q1, p2, q2);
}

int64_t Geometry::get_uv84_normal_bit(const Vector3 normal) {
	return ___godot_icall_int_Vector3(___mb.mb_get_uv84_normal_bit, (const Object *) this, normal);
}

Array Geometry::intersect_polygons_2d(const PoolVector2Array polygon_a, const PoolVector2Array polygon_b) {
	return ___godot_icall_Array_PoolVector2Array_PoolVector2Array(___mb.mb_intersect_polygons_2d, (const Object *) this, polygon_a, polygon_b);
}

Array Geometry::intersect_polyline_with_polygon_2d(const PoolVector2Array polyline, const PoolVector2Array polygon) {
	return ___godot_icall_Array_PoolVector2Array_PoolVector2Array(___mb.mb_intersect_polyline_with_polygon_2d, (const Object *) this, polyline, polygon);
}

bool Geometry::is_point_in_circle(const Vector2 point, const Vector2 circle_position, const real_t circle_radius) {
	return ___godot_icall_bool_Vector2_Vector2_float(___mb.mb_is_point_in_circle, (const Object *) this, point, circle_position, circle_radius);
}

bool Geometry::is_point_in_polygon(const Vector2 point, const PoolVector2Array polygon) {
	return ___godot_icall_bool_Vector2_PoolVector2Array(___mb.mb_is_point_in_polygon, (const Object *) this, point, polygon);
}

bool Geometry::is_polygon_clockwise(const PoolVector2Array polygon) {
	return ___godot_icall_bool_PoolVector2Array(___mb.mb_is_polygon_clockwise, (const Object *) this, polygon);
}

Variant Geometry::line_intersects_line_2d(const Vector2 from_a, const Vector2 dir_a, const Vector2 from_b, const Vector2 dir_b) {
	return ___godot_icall_Variant_Vector2_Vector2_Vector2_Vector2(___mb.mb_line_intersects_line_2d, (const Object *) this, from_a, dir_a, from_b, dir_b);
}

Dictionary Geometry::make_atlas(const PoolVector2Array sizes) {
	return ___godot_icall_Dictionary_PoolVector2Array(___mb.mb_make_atlas, (const Object *) this, sizes);
}

Array Geometry::merge_polygons_2d(const PoolVector2Array polygon_a, const PoolVector2Array polygon_b) {
	return ___godot_icall_Array_PoolVector2Array_PoolVector2Array(___mb.mb_merge_polygons_2d, (const Object *) this, polygon_a, polygon_b);
}

Array Geometry::offset_polygon_2d(const PoolVector2Array polygon, const real_t delta, const int64_t join_type) {
	return ___godot_icall_Array_PoolVector2Array_float_int(___mb.mb_offset_polygon_2d, (const Object *) this, polygon, delta, join_type);
}

Array Geometry::offset_polyline_2d(const PoolVector2Array polyline, const real_t delta, const int64_t join_type, const int64_t end_type) {
	return ___godot_icall_Array_PoolVector2Array_float_int_int(___mb.mb_offset_polyline_2d, (const Object *) this, polyline, delta, join_type, end_type);
}

bool Geometry::point_is_inside_triangle(const Vector2 point, const Vector2 a, const Vector2 b, const Vector2 c) const {
	return ___godot_icall_bool_Vector2_Vector2_Vector2_Vector2(___mb.mb_point_is_inside_triangle, (const Object *) this, point, a, b, c);
}

Variant Geometry::ray_intersects_triangle(const Vector3 from, const Vector3 dir, const Vector3 a, const Vector3 b, const Vector3 c) {
	return ___godot_icall_Variant_Vector3_Vector3_Vector3_Vector3_Vector3(___mb.mb_ray_intersects_triangle, (const Object *) this, from, dir, a, b, c);
}

real_t Geometry::segment_intersects_circle(const Vector2 segment_from, const Vector2 segment_to, const Vector2 circle_position, const real_t circle_radius) {
	return ___godot_icall_float_Vector2_Vector2_Vector2_float(___mb.mb_segment_intersects_circle, (const Object *) this, segment_from, segment_to, circle_position, circle_radius);
}

PoolVector3Array Geometry::segment_intersects_convex(const Vector3 from, const Vector3 to, const Array planes) {
	return ___godot_icall_PoolVector3Array_Vector3_Vector3_Array(___mb.mb_segment_intersects_convex, (const Object *) this, from, to, planes);
}

PoolVector3Array Geometry::segment_intersects_cylinder(const Vector3 from, const Vector3 to, const real_t height, const real_t radius) {
	return ___godot_icall_PoolVector3Array_Vector3_Vector3_float_float(___mb.mb_segment_intersects_cylinder, (const Object *) this, from, to, height, radius);
}

Variant Geometry::segment_intersects_segment_2d(const Vector2 from_a, const Vector2 to_a, const Vector2 from_b, const Vector2 to_b) {
	return ___godot_icall_Variant_Vector2_Vector2_Vector2_Vector2(___mb.mb_segment_intersects_segment_2d, (const Object *) this, from_a, to_a, from_b, to_b);
}

PoolVector3Array Geometry::segment_intersects_sphere(const Vector3 from, const Vector3 to, const Vector3 sphere_position, const real_t sphere_radius) {
	return ___godot_icall_PoolVector3Array_Vector3_Vector3_Vector3_float(___mb.mb_segment_intersects_sphere, (const Object *) this, from, to, sphere_position, sphere_radius);
}

Variant Geometry::segment_intersects_triangle(const Vector3 from, const Vector3 to, const Vector3 a, const Vector3 b, const Vector3 c) {
	return ___godot_icall_Variant_Vector3_Vector3_Vector3_Vector3_Vector3(___mb.mb_segment_intersects_triangle, (const Object *) this, from, to, a, b, c);
}

PoolIntArray Geometry::triangulate_delaunay_2d(const PoolVector2Array points) {
	return ___godot_icall_PoolIntArray_PoolVector2Array(___mb.mb_triangulate_delaunay_2d, (const Object *) this, points);
}

PoolIntArray Geometry::triangulate_polygon(const PoolVector2Array polygon) {
	return ___godot_icall_PoolIntArray_PoolVector2Array(___mb.mb_triangulate_polygon, (const Object *) this, polygon);
}

}