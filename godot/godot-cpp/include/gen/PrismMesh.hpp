#ifndef GODOT_CPP_PRISMMESH_HPP
#define GODOT_CPP_PRISMMESH_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "PrimitiveMesh.hpp"
namespace godot {


class PrismMesh : public PrimitiveMesh {
	struct ___method_bindings {
		godot_method_bind *mb_get_left_to_right;
		godot_method_bind *mb_get_size;
		godot_method_bind *mb_get_subdivide_depth;
		godot_method_bind *mb_get_subdivide_height;
		godot_method_bind *mb_get_subdivide_width;
		godot_method_bind *mb_set_left_to_right;
		godot_method_bind *mb_set_size;
		godot_method_bind *mb_set_subdivide_depth;
		godot_method_bind *mb_set_subdivide_height;
		godot_method_bind *mb_set_subdivide_width;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "PrismMesh"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static PrismMesh *_new();

	// methods
	real_t get_left_to_right() const;
	Vector3 get_size() const;
	int64_t get_subdivide_depth() const;
	int64_t get_subdivide_height() const;
	int64_t get_subdivide_width() const;
	void set_left_to_right(const real_t left_to_right);
	void set_size(const Vector3 size);
	void set_subdivide_depth(const int64_t segments);
	void set_subdivide_height(const int64_t segments);
	void set_subdivide_width(const int64_t segments);

};

}

#endif