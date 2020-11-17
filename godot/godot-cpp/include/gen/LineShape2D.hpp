#ifndef GODOT_CPP_LINESHAPE2D_HPP
#define GODOT_CPP_LINESHAPE2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Shape2D.hpp"
namespace godot {


class LineShape2D : public Shape2D {
	struct ___method_bindings {
		godot_method_bind *mb_get_d;
		godot_method_bind *mb_get_normal;
		godot_method_bind *mb_set_d;
		godot_method_bind *mb_set_normal;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "LineShape2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static LineShape2D *_new();

	// methods
	real_t get_d() const;
	Vector2 get_normal() const;
	void set_d(const real_t d);
	void set_normal(const Vector2 normal);

};

}

#endif