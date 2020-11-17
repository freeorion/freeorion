#ifndef GODOT_CPP_NAVIGATIONMESHINSTANCE_HPP
#define GODOT_CPP_NAVIGATIONMESHINSTANCE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Spatial.hpp"
namespace godot {

class NavigationMesh;

class NavigationMeshInstance : public Spatial {
	struct ___method_bindings {
		godot_method_bind *mb_get_navigation_mesh;
		godot_method_bind *mb_is_enabled;
		godot_method_bind *mb_set_enabled;
		godot_method_bind *mb_set_navigation_mesh;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "NavigationMeshInstance"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static NavigationMeshInstance *_new();

	// methods
	Ref<NavigationMesh> get_navigation_mesh() const;
	bool is_enabled() const;
	void set_enabled(const bool enabled);
	void set_navigation_mesh(const Ref<NavigationMesh> navmesh);

};

}

#endif