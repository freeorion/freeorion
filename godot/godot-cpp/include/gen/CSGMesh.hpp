#ifndef GODOT_CPP_CSGMESH_HPP
#define GODOT_CPP_CSGMESH_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "CSGPrimitive.hpp"
namespace godot {

class Material;
class Mesh;

class CSGMesh : public CSGPrimitive {
	struct ___method_bindings {
		godot_method_bind *mb__mesh_changed;
		godot_method_bind *mb_get_material;
		godot_method_bind *mb_get_mesh;
		godot_method_bind *mb_set_material;
		godot_method_bind *mb_set_mesh;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CSGMesh"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static CSGMesh *_new();

	// methods
	void _mesh_changed();
	Ref<Material> get_material() const;
	Ref<Mesh> get_mesh();
	void set_material(const Ref<Material> material);
	void set_mesh(const Ref<Mesh> mesh);

};

}

#endif