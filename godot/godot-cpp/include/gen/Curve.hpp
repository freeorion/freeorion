#ifndef GODOT_CPP_CURVE_HPP
#define GODOT_CPP_CURVE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Curve.hpp"

#include "Resource.hpp"
namespace godot {


class Curve : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb__get_data;
		godot_method_bind *mb__set_data;
		godot_method_bind *mb_add_point;
		godot_method_bind *mb_bake;
		godot_method_bind *mb_clean_dupes;
		godot_method_bind *mb_clear_points;
		godot_method_bind *mb_get_bake_resolution;
		godot_method_bind *mb_get_max_value;
		godot_method_bind *mb_get_min_value;
		godot_method_bind *mb_get_point_count;
		godot_method_bind *mb_get_point_left_mode;
		godot_method_bind *mb_get_point_left_tangent;
		godot_method_bind *mb_get_point_position;
		godot_method_bind *mb_get_point_right_mode;
		godot_method_bind *mb_get_point_right_tangent;
		godot_method_bind *mb_interpolate;
		godot_method_bind *mb_interpolate_baked;
		godot_method_bind *mb_remove_point;
		godot_method_bind *mb_set_bake_resolution;
		godot_method_bind *mb_set_max_value;
		godot_method_bind *mb_set_min_value;
		godot_method_bind *mb_set_point_left_mode;
		godot_method_bind *mb_set_point_left_tangent;
		godot_method_bind *mb_set_point_offset;
		godot_method_bind *mb_set_point_right_mode;
		godot_method_bind *mb_set_point_right_tangent;
		godot_method_bind *mb_set_point_value;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Curve"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum TangentMode {
		TANGENT_FREE = 0,
		TANGENT_LINEAR = 1,
		TANGENT_MODE_COUNT = 2,
	};

	// constants


	static Curve *_new();

	// methods
	Array _get_data() const;
	void _set_data(const Array data);
	int64_t add_point(const Vector2 position, const real_t left_tangent = 0, const real_t right_tangent = 0, const int64_t left_mode = 0, const int64_t right_mode = 0);
	void bake();
	void clean_dupes();
	void clear_points();
	int64_t get_bake_resolution() const;
	real_t get_max_value() const;
	real_t get_min_value() const;
	int64_t get_point_count() const;
	Curve::TangentMode get_point_left_mode(const int64_t index) const;
	real_t get_point_left_tangent(const int64_t index) const;
	Vector2 get_point_position(const int64_t index) const;
	Curve::TangentMode get_point_right_mode(const int64_t index) const;
	real_t get_point_right_tangent(const int64_t index) const;
	real_t interpolate(const real_t offset) const;
	real_t interpolate_baked(const real_t offset);
	void remove_point(const int64_t index);
	void set_bake_resolution(const int64_t resolution);
	void set_max_value(const real_t max);
	void set_min_value(const real_t min);
	void set_point_left_mode(const int64_t index, const int64_t mode);
	void set_point_left_tangent(const int64_t index, const real_t tangent);
	int64_t set_point_offset(const int64_t index, const real_t offset);
	void set_point_right_mode(const int64_t index, const int64_t mode);
	void set_point_right_tangent(const int64_t index, const real_t tangent);
	void set_point_value(const int64_t index, const real_t y);

};

}

#endif