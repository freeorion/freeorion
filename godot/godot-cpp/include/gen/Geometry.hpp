#ifndef GODOT_CPP_GEOMETRY_HPP
#define GODOT_CPP_GEOMETRY_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {


class Geometry : public Object {
	static Geometry *_singleton;

	Geometry();

	struct ___method_bindings {
		godot_method_bind *mb_build_box_planes;
		godot_method_bind *mb_build_capsule_planes;
		godot_method_bind *mb_build_cylinder_planes;
		godot_method_bind *mb_clip_polygon;
		godot_method_bind *mb_clip_polygons_2d;
		godot_method_bind *mb_clip_polyline_with_polygon_2d;
		godot_method_bind *mb_convex_hull_2d;
		godot_method_bind *mb_exclude_polygons_2d;
		godot_method_bind *mb_get_closest_point_to_segment;
		godot_method_bind *mb_get_closest_point_to_segment_2d;
		godot_method_bind *mb_get_closest_point_to_segment_uncapped;
		godot_method_bind *mb_get_closest_point_to_segment_uncapped_2d;
		godot_method_bind *mb_get_closest_points_between_segments;
		godot_method_bind *mb_get_closest_points_between_segments_2d;
		godot_method_bind *mb_get_uv84_normal_bit;
		godot_method_bind *mb_intersect_polygons_2d;
		godot_method_bind *mb_intersect_polyline_with_polygon_2d;
		godot_method_bind *mb_is_point_in_circle;
		godot_method_bind *mb_is_point_in_polygon;
		godot_method_bind *mb_is_polygon_clockwise;
		godot_method_bind *mb_line_intersects_line_2d;
		godot_method_bind *mb_make_atlas;
		godot_method_bind *mb_merge_polygons_2d;
		godot_method_bind *mb_offset_polygon_2d;
		godot_method_bind *mb_offset_polyline_2d;
		godot_method_bind *mb_point_is_inside_triangle;
		godot_method_bind *mb_ray_intersects_triangle;
		godot_method_bind *mb_segment_intersects_circle;
		godot_method_bind *mb_segment_intersects_convex;
		godot_method_bind *mb_segment_intersects_cylinder;
		godot_method_bind *mb_segment_intersects_segment_2d;
		godot_method_bind *mb_segment_intersects_sphere;
		godot_method_bind *mb_segment_intersects_triangle;
		godot_method_bind *mb_triangulate_delaunay_2d;
		godot_method_bind *mb_triangulate_polygon;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline Geometry *get_singleton()
	{
		if (!Geometry::_singleton) {
			Geometry::_singleton = new Geometry;
		}
		return Geometry::_singleton;
	}

	static inline const char *___get_class_name() { return (const char *) "Geometry"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum PolyEndType {
		END_POLYGON = 0,
		END_JOINED = 1,
		END_BUTT = 2,
		END_SQUARE = 3,
		END_ROUND = 4,
	};
	enum PolyBooleanOperation {
		OPERATION_UNION = 0,
		OPERATION_DIFFERENCE = 1,
		OPERATION_INTERSECTION = 2,
		OPERATION_XOR = 3,
	};
	enum PolyJoinType {
		JOIN_SQUARE = 0,
		JOIN_ROUND = 1,
		JOIN_MITER = 2,
	};

	// constants

	// methods
	Array build_box_planes(const Vector3 extents);
	Array build_capsule_planes(const real_t radius, const real_t height, const int64_t sides, const int64_t lats, const int64_t axis = 2);
	Array build_cylinder_planes(const real_t radius, const real_t height, const int64_t sides, const int64_t axis = 2);
	PoolVector3Array clip_polygon(const PoolVector3Array points, const Plane plane);
	Array clip_polygons_2d(const PoolVector2Array polygon_a, const PoolVector2Array polygon_b);
	Array clip_polyline_with_polygon_2d(const PoolVector2Array polyline, const PoolVector2Array polygon);
	PoolVector2Array convex_hull_2d(const PoolVector2Array points);
	Array exclude_polygons_2d(const PoolVector2Array polygon_a, const PoolVector2Array polygon_b);
	Vector3 get_closest_point_to_segment(const Vector3 point, const Vector3 s1, const Vector3 s2);
	Vector2 get_closest_point_to_segment_2d(const Vector2 point, const Vector2 s1, const Vector2 s2);
	Vector3 get_closest_point_to_segment_uncapped(const Vector3 point, const Vector3 s1, const Vector3 s2);
	Vector2 get_closest_point_to_segment_uncapped_2d(const Vector2 point, const Vector2 s1, const Vector2 s2);
	PoolVector3Array get_closest_points_between_segments(const Vector3 p1, const Vector3 p2, const Vector3 q1, const Vector3 q2);
	PoolVector2Array get_closest_points_between_segments_2d(const Vector2 p1, const Vector2 q1, const Vector2 p2, const Vector2 q2);
	int64_t get_uv84_normal_bit(const Vector3 normal);
	Array intersect_polygons_2d(const PoolVector2Array polygon_a, const PoolVector2Array polygon_b);
	Array intersect_polyline_with_polygon_2d(const PoolVector2Array polyline, const PoolVector2Array polygon);
	bool is_point_in_circle(const Vector2 point, const Vector2 circle_position, const real_t circle_radius);
	bool is_point_in_polygon(const Vector2 point, const PoolVector2Array polygon);
	bool is_polygon_clockwise(const PoolVector2Array polygon);
	Variant line_intersects_line_2d(const Vector2 from_a, const Vector2 dir_a, const Vector2 from_b, const Vector2 dir_b);
	Dictionary make_atlas(const PoolVector2Array sizes);
	Array merge_polygons_2d(const PoolVector2Array polygon_a, const PoolVector2Array polygon_b);
	Array offset_polygon_2d(const PoolVector2Array polygon, const real_t delta, const int64_t join_type = 0);
	Array offset_polyline_2d(const PoolVector2Array polyline, const real_t delta, const int64_t join_type = 0, const int64_t end_type = 3);
	bool point_is_inside_triangle(const Vector2 point, const Vector2 a, const Vector2 b, const Vector2 c) const;
	Variant ray_intersects_triangle(const Vector3 from, const Vector3 dir, const Vector3 a, const Vector3 b, const Vector3 c);
	real_t segment_intersects_circle(const Vector2 segment_from, const Vector2 segment_to, const Vector2 circle_position, const real_t circle_radius);
	PoolVector3Array segment_intersects_convex(const Vector3 from, const Vector3 to, const Array planes);
	PoolVector3Array segment_intersects_cylinder(const Vector3 from, const Vector3 to, const real_t height, const real_t radius);
	Variant segment_intersects_segment_2d(const Vector2 from_a, const Vector2 to_a, const Vector2 from_b, const Vector2 to_b);
	PoolVector3Array segment_intersects_sphere(const Vector3 from, const Vector3 to, const Vector3 sphere_position, const real_t sphere_radius);
	Variant segment_intersects_triangle(const Vector3 from, const Vector3 to, const Vector3 a, const Vector3 b, const Vector3 c);
	PoolIntArray triangulate_delaunay_2d(const PoolVector2Array points);
	PoolIntArray triangulate_polygon(const PoolVector2Array polygon);

};

}

#endif