#include "PrimitiveMesh.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Material.hpp"


namespace godot {


PrimitiveMesh::___method_bindings PrimitiveMesh::___mb = {};

void PrimitiveMesh::___init_method_bindings() {
	___mb.mb__update = godot::api->godot_method_bind_get_method("PrimitiveMesh", "_update");
	___mb.mb_get_custom_aabb = godot::api->godot_method_bind_get_method("PrimitiveMesh", "get_custom_aabb");
	___mb.mb_get_flip_faces = godot::api->godot_method_bind_get_method("PrimitiveMesh", "get_flip_faces");
	___mb.mb_get_material = godot::api->godot_method_bind_get_method("PrimitiveMesh", "get_material");
	___mb.mb_get_mesh_arrays = godot::api->godot_method_bind_get_method("PrimitiveMesh", "get_mesh_arrays");
	___mb.mb_set_custom_aabb = godot::api->godot_method_bind_get_method("PrimitiveMesh", "set_custom_aabb");
	___mb.mb_set_flip_faces = godot::api->godot_method_bind_get_method("PrimitiveMesh", "set_flip_faces");
	___mb.mb_set_material = godot::api->godot_method_bind_get_method("PrimitiveMesh", "set_material");
}

void PrimitiveMesh::_update() const {
	___godot_icall_void(___mb.mb__update, (const Object *) this);
}

AABB PrimitiveMesh::get_custom_aabb() const {
	return ___godot_icall_AABB(___mb.mb_get_custom_aabb, (const Object *) this);
}

bool PrimitiveMesh::get_flip_faces() const {
	return ___godot_icall_bool(___mb.mb_get_flip_faces, (const Object *) this);
}

Ref<Material> PrimitiveMesh::get_material() const {
	return Ref<Material>::__internal_constructor(___godot_icall_Object(___mb.mb_get_material, (const Object *) this));
}

Array PrimitiveMesh::get_mesh_arrays() const {
	return ___godot_icall_Array(___mb.mb_get_mesh_arrays, (const Object *) this);
}

void PrimitiveMesh::set_custom_aabb(const AABB aabb) {
	___godot_icall_void_AABB(___mb.mb_set_custom_aabb, (const Object *) this, aabb);
}

void PrimitiveMesh::set_flip_faces(const bool flip_faces) {
	___godot_icall_void_bool(___mb.mb_set_flip_faces, (const Object *) this, flip_faces);
}

void PrimitiveMesh::set_material(const Ref<Material> material) {
	___godot_icall_void_Object(___mb.mb_set_material, (const Object *) this, material.ptr());
}

}