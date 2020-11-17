#ifndef GODOT_CPP_EDITORNAVIGATIONMESHGENERATOR_HPP
#define GODOT_CPP_EDITORNAVIGATIONMESHGENERATOR_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {

class NavigationMesh;
class Node;

class EditorNavigationMeshGenerator : public Object {
	struct ___method_bindings {
		godot_method_bind *mb_bake;
		godot_method_bind *mb_clear;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorNavigationMeshGenerator"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void bake(const Ref<NavigationMesh> nav_mesh, const Node *root_node);
	void clear(const Ref<NavigationMesh> nav_mesh);

};

}

#endif