#include "CSGMesh.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Material.hpp"
#include "Mesh.hpp"


namespace godot {


CSGMesh::___method_bindings CSGMesh::___mb = {};

void CSGMesh::___init_method_bindings() {
	___mb.mb__mesh_changed = godot::api->godot_method_bind_get_method("CSGMesh", "_mesh_changed");
	___mb.mb_get_material = godot::api->godot_method_bind_get_method("CSGMesh", "get_material");
	___mb.mb_get_mesh = godot::api->godot_method_bind_get_method("CSGMesh", "get_mesh");
	___mb.mb_set_material = godot::api->godot_method_bind_get_method("CSGMesh", "set_material");
	___mb.mb_set_mesh = godot::api->godot_method_bind_get_method("CSGMesh", "set_mesh");
}

CSGMesh *CSGMesh::_new()
{
	return (CSGMesh *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CSGMesh")());
}
void CSGMesh::_mesh_changed() {
	___godot_icall_void(___mb.mb__mesh_changed, (const Object *) this);
}

Ref<Material> CSGMesh::get_material() const {
	return Ref<Material>::__internal_constructor(___godot_icall_Object(___mb.mb_get_material, (const Object *) this));
}

Ref<Mesh> CSGMesh::get_mesh() {
	return Ref<Mesh>::__internal_constructor(___godot_icall_Object(___mb.mb_get_mesh, (const Object *) this));
}

void CSGMesh::set_material(const Ref<Material> material) {
	___godot_icall_void_Object(___mb.mb_set_material, (const Object *) this, material.ptr());
}

void CSGMesh::set_mesh(const Ref<Mesh> mesh) {
	___godot_icall_void_Object(___mb.mb_set_mesh, (const Object *) this, mesh.ptr());
}

}