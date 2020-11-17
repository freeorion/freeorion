#include "EditorNavigationMeshGenerator.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "NavigationMesh.hpp"
#include "Node.hpp"


namespace godot {


EditorNavigationMeshGenerator::___method_bindings EditorNavigationMeshGenerator::___mb = {};

void EditorNavigationMeshGenerator::___init_method_bindings() {
	___mb.mb_bake = godot::api->godot_method_bind_get_method("EditorNavigationMeshGenerator", "bake");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("EditorNavigationMeshGenerator", "clear");
}

void EditorNavigationMeshGenerator::bake(const Ref<NavigationMesh> nav_mesh, const Node *root_node) {
	___godot_icall_void_Object_Object(___mb.mb_bake, (const Object *) this, nav_mesh.ptr(), root_node);
}

void EditorNavigationMeshGenerator::clear(const Ref<NavigationMesh> nav_mesh) {
	___godot_icall_void_Object(___mb.mb_clear, (const Object *) this, nav_mesh.ptr());
}

}