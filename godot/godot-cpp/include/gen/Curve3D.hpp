#ifndef GODOT_CPP_CURVE3D_HPP
#define GODOT_CPP_CURVE3D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {


class Curve3D : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb__get_data;
		godot_method_bind *mb__set_data;
		godot_method_bind *mb_add_point;
		godot_method_bind *mb_clear_points;
		godot_method_bind *mb_get_bake_interval;
		godot_method_bind *mb_get_baked_length;
		godot_method_bind *mb_get_baked_points;
		godot_method_bind *mb_get_baked_tilts;
		godot_method_bind *mb_get_baked_up_vectors;
		godot_method_bind *mb_get_closest_offset;
		godot_method_bind *mb_get_closest_point;
		godot_method_bind *mb_get_point_count;
		godot_method_bind *mb_get_point_in;
		godot_method_bind *mb_get_point_out;
		godot_method_bind *mb_get_point_position;
		godot_method_bind *mb_get_point_tilt;
		godot_method_bind *mb_interpolate;
		godot_method_bind *mb_interpolate_baked;
		godot_method_bind *mb_interpolate_baked_up_vector;
		godot_method_bind *mb_interpolatef;
		godot_method_bind *mb_is_up_vector_enabled;
		godot_method_bind *mb_remove_point;
		godot_method_bind *mb_set_bake_interval;
		godot_method_bind *mb_set_point_in;
		godot_method_bind *mb_set_point_out;
		godot_method_bind *mb_set_point_position;
		godot_method_bind *mb_set_point_tilt;
		godot_method_bind *mb_set_up_vector_enabled;
		godot_method_bind *mb_tessellate;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Curve3D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Curve3D *_new();

	// methods
	Dictionary _get_data() const;
	void _set_data(const Dictionary arg0);
	void add_point(const Vector3 position, const Vector3 in = Vector3(0, 0, 0), const Vector3 out = Vector3(0, 0, 0), const int64_t at_position = -1);
	void clear_points();
	real_t get_bake_interval() const;
	real_t get_baked_length() const;
	PoolVector3Array get_baked_points() const;
	PoolRealArray get_baked_tilts() const;
	PoolVector3Array get_baked_up_vectors() const;
	real_t get_closest_offset(const Vector3 to_point) const;
	Vector3 get_closest_point(const Vector3 to_point) const;
	int64_t get_point_count() const;
	Vector3 get_point_in(const int64_t idx) const;
	Vector3 get_point_out(const int64_t idx) const;
	Vector3 get_point_position(const int64_t idx) const;
	real_t get_point_tilt(const int64_t idx) const;
	Vector3 interpolate(const int64_t idx, const real_t t) const;
	Vector3 interpolate_baked(const real_t offset, const bool cubic = false) const;
	Vector3 interpolate_baked_up_vector(const real_t offset, const bool apply_tilt = false) const;
	Vector3 interpolatef(const real_t fofs) const;
	bool is_up_vector_enabled() const;
	void remove_point(const int64_t idx);
	void set_bake_interval(const real_t distance);
	void set_point_in(const int64_t idx, const Vector3 position);
	void set_point_out(const int64_t idx, const Vector3 position);
	void set_point_position(const int64_t idx, const Vector3 position);
	void set_point_tilt(const int64_t idx, const real_t tilt);
	void set_up_vector_enabled(const bool enable);
	PoolVector3Array tessellate(const int64_t max_stages = 5, const real_t tolerance_degrees = 4) const;

};

}

#endif