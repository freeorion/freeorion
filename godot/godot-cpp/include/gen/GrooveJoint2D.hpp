#ifndef GODOT_CPP_GROOVEJOINT2D_HPP
#define GODOT_CPP_GROOVEJOINT2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Joint2D.hpp"
namespace godot {


class GrooveJoint2D : public Joint2D {
	struct ___method_bindings {
		godot_method_bind *mb_get_initial_offset;
		godot_method_bind *mb_get_length;
		godot_method_bind *mb_set_initial_offset;
		godot_method_bind *mb_set_length;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "GrooveJoint2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static GrooveJoint2D *_new();

	// methods
	real_t get_initial_offset() const;
	real_t get_length() const;
	void set_initial_offset(const real_t offset);
	void set_length(const real_t length);

};

}

#endif