#ifndef GODOT_CPP_DAMPEDSPRINGJOINT2D_HPP
#define GODOT_CPP_DAMPEDSPRINGJOINT2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Joint2D.hpp"
namespace godot {


class DampedSpringJoint2D : public Joint2D {
	struct ___method_bindings {
		godot_method_bind *mb_get_damping;
		godot_method_bind *mb_get_length;
		godot_method_bind *mb_get_rest_length;
		godot_method_bind *mb_get_stiffness;
		godot_method_bind *mb_set_damping;
		godot_method_bind *mb_set_length;
		godot_method_bind *mb_set_rest_length;
		godot_method_bind *mb_set_stiffness;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "DampedSpringJoint2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static DampedSpringJoint2D *_new();

	// methods
	real_t get_damping() const;
	real_t get_length() const;
	real_t get_rest_length() const;
	real_t get_stiffness() const;
	void set_damping(const real_t damping);
	void set_length(const real_t length);
	void set_rest_length(const real_t rest_length);
	void set_stiffness(const real_t stiffness);

};

}

#endif