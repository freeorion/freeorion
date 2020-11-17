#ifndef GODOT_CPP_CAPSULEMESH_HPP
#define GODOT_CPP_CAPSULEMESH_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "PrimitiveMesh.hpp"
namespace godot {


class CapsuleMesh : public PrimitiveMesh {
	struct ___method_bindings {
		godot_method_bind *mb_get_mid_height;
		godot_method_bind *mb_get_radial_segments;
		godot_method_bind *mb_get_radius;
		godot_method_bind *mb_get_rings;
		godot_method_bind *mb_set_mid_height;
		godot_method_bind *mb_set_radial_segments;
		godot_method_bind *mb_set_radius;
		godot_method_bind *mb_set_rings;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CapsuleMesh"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static CapsuleMesh *_new();

	// methods
	real_t get_mid_height() const;
	int64_t get_radial_segments() const;
	real_t get_radius() const;
	int64_t get_rings() const;
	void set_mid_height(const real_t mid_height);
	void set_radial_segments(const int64_t segments);
	void set_radius(const real_t radius);
	void set_rings(const int64_t rings);

};

}

#endif