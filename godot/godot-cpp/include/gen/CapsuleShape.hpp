#ifndef GODOT_CPP_CAPSULESHAPE_HPP
#define GODOT_CPP_CAPSULESHAPE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Shape.hpp"
namespace godot {


class CapsuleShape : public Shape {
	struct ___method_bindings {
		godot_method_bind *mb_get_height;
		godot_method_bind *mb_get_radius;
		godot_method_bind *mb_set_height;
		godot_method_bind *mb_set_radius;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CapsuleShape"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static CapsuleShape *_new();

	// methods
	real_t get_height() const;
	real_t get_radius() const;
	void set_height(const real_t height);
	void set_radius(const real_t radius);

};

}

#endif