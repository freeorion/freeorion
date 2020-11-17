#ifndef GODOT_CPP_BOXSHAPE_HPP
#define GODOT_CPP_BOXSHAPE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Shape.hpp"
namespace godot {


class BoxShape : public Shape {
	struct ___method_bindings {
		godot_method_bind *mb_get_extents;
		godot_method_bind *mb_set_extents;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "BoxShape"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static BoxShape *_new();

	// methods
	Vector3 get_extents() const;
	void set_extents(const Vector3 extents);

};

}

#endif