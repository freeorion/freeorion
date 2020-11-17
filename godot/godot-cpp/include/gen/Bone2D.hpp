#ifndef GODOT_CPP_BONE2D_HPP
#define GODOT_CPP_BONE2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node2D.hpp"
namespace godot {


class Bone2D : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb_apply_rest;
		godot_method_bind *mb_get_default_length;
		godot_method_bind *mb_get_index_in_skeleton;
		godot_method_bind *mb_get_rest;
		godot_method_bind *mb_get_skeleton_rest;
		godot_method_bind *mb_set_default_length;
		godot_method_bind *mb_set_rest;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Bone2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Bone2D *_new();

	// methods
	void apply_rest();
	real_t get_default_length() const;
	int64_t get_index_in_skeleton() const;
	Transform2D get_rest() const;
	Transform2D get_skeleton_rest() const;
	void set_default_length(const real_t default_length);
	void set_rest(const Transform2D rest);

};

}

#endif