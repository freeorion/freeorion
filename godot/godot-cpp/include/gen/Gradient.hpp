#ifndef GODOT_CPP_GRADIENT_HPP
#define GODOT_CPP_GRADIENT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {


class Gradient : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_add_point;
		godot_method_bind *mb_get_color;
		godot_method_bind *mb_get_colors;
		godot_method_bind *mb_get_offset;
		godot_method_bind *mb_get_offsets;
		godot_method_bind *mb_get_point_count;
		godot_method_bind *mb_interpolate;
		godot_method_bind *mb_remove_point;
		godot_method_bind *mb_set_color;
		godot_method_bind *mb_set_colors;
		godot_method_bind *mb_set_offset;
		godot_method_bind *mb_set_offsets;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Gradient"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Gradient *_new();

	// methods
	void add_point(const real_t offset, const Color color);
	Color get_color(const int64_t point) const;
	PoolColorArray get_colors() const;
	real_t get_offset(const int64_t point) const;
	PoolRealArray get_offsets() const;
	int64_t get_point_count() const;
	Color interpolate(const real_t offset);
	void remove_point(const int64_t offset);
	void set_color(const int64_t point, const Color color);
	void set_colors(const PoolColorArray colors);
	void set_offset(const int64_t point, const real_t offset);
	void set_offsets(const PoolRealArray offsets);

};

}

#endif