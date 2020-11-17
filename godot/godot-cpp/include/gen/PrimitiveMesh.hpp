#ifndef GODOT_CPP_PRIMITIVEMESH_HPP
#define GODOT_CPP_PRIMITIVEMESH_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Mesh.hpp"
namespace godot {

class Material;

class PrimitiveMesh : public Mesh {
	struct ___method_bindings {
		godot_method_bind *mb__update;
		godot_method_bind *mb_get_custom_aabb;
		godot_method_bind *mb_get_flip_faces;
		godot_method_bind *mb_get_material;
		godot_method_bind *mb_get_mesh_arrays;
		godot_method_bind *mb_set_custom_aabb;
		godot_method_bind *mb_set_flip_faces;
		godot_method_bind *mb_set_material;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "PrimitiveMesh"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void _update() const;
	AABB get_custom_aabb() const;
	bool get_flip_faces() const;
	Ref<Material> get_material() const;
	Array get_mesh_arrays() const;
	void set_custom_aabb(const AABB aabb);
	void set_flip_faces(const bool flip_faces);
	void set_material(const Ref<Material> material);

};

}

#endif