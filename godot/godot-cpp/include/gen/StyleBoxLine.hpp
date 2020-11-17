#ifndef GODOT_CPP_STYLEBOXLINE_HPP
#define GODOT_CPP_STYLEBOXLINE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "StyleBox.hpp"
namespace godot {


class StyleBoxLine : public StyleBox {
	struct ___method_bindings {
		godot_method_bind *mb_get_color;
		godot_method_bind *mb_get_grow_begin;
		godot_method_bind *mb_get_grow_end;
		godot_method_bind *mb_get_thickness;
		godot_method_bind *mb_is_vertical;
		godot_method_bind *mb_set_color;
		godot_method_bind *mb_set_grow_begin;
		godot_method_bind *mb_set_grow_end;
		godot_method_bind *mb_set_thickness;
		godot_method_bind *mb_set_vertical;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "StyleBoxLine"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static StyleBoxLine *_new();

	// methods
	Color get_color() const;
	real_t get_grow_begin() const;
	real_t get_grow_end() const;
	int64_t get_thickness() const;
	bool is_vertical() const;
	void set_color(const Color color);
	void set_grow_begin(const real_t offset);
	void set_grow_end(const real_t offset);
	void set_thickness(const int64_t thickness);
	void set_vertical(const bool vertical);

};

}

#endif