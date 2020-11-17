#ifndef GODOT_CPP_CURVE2D_HPP
#define GODOT_CPP_CURVE2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {


class Curve2D : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb__get_data;
		godot_method_bind *mb__set_data;
		godot_method_bind *mb_add_point;
		godot_method_bind *mb_clear_points;
		godot_method_bind *mb_get_bake_interval;
		godot_method_bind *mb_get_baked_length;
		godot_method_bind *mb_get_baked_points;
		godot_method_bind *mb_get_closest_offset;
		godot_method_bind *mb_get_closest_point;
		godot_method_bind *mb_get_point_count;
		godot_method_bind *mb_get_point_in;
		godot_method_bind *mb_get_point_out;
		godot_method_bind *mb_get_point_position;
		godot_method_bind *mb_interpolate;
		godot_method_bind *mb_interpolate_baked;
		godot_method_bind *mb_interpolatef;
		godot_method_bind *mb_remove_point;
		godot_method_bind *mb_set_bake_interval;
		godot_method_bind *mb_set_point_in;
		godot_method_bind *mb_set_point_out;
		godot_method_bind *mb_set_point_position;
		godot_method_bind *mb_tessellate;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Curve2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Curve2D *_new();

	// methods
	Dictionary _get_data() const;
	void _set_data(const Dictionary arg0);
	void add_point(const Vector2 position, const Vector2 in = Vector2(0, 0), const Vector2 out = Vector2(0, 0), const int64_t at_position = -1);
	void clear_points();
	real_t get_bake_interval() const;
	real_t get_baked_length() const;
	PoolVector2Array get_baked_points() const;
	real_t get_closest_offset(const Vector2 to_point) const;
	Vector2 get_closest_point(const Vector2 to_point) const;
	int64_t get_point_count() const;
	Vector2 get_point_in(const int64_t idx) const;
	Vector2 get_point_out(const int64_t idx) const;
	Vector2 get_point_position(const int64_t idx) const;
	Vector2 interpolate(const int64_t idx, const real_t t) const;
	Vector2 interpolate_baked(const real_t offset, const bool cubic = false) const;
	Vector2 interpolatef(const real_t fofs) const;
	void remove_point(const int64_t idx);
	void set_bake_interval(const real_t distance);
	void set_point_in(const int64_t idx, const Vector2 position);
	void set_point_out(const int64_t idx, const Vector2 position);
	void set_point_position(const int64_t idx, const Vector2 position);
	PoolVector2Array tessellate(const int64_t max_stages = 5, const real_t tolerance_degrees = 4) const;

};

}

#endif