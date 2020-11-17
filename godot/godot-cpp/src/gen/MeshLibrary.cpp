#include "MeshLibrary.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Mesh.hpp"
#include "NavigationMesh.hpp"
#include "Texture.hpp"


namespace godot {


MeshLibrary::___method_bindings MeshLibrary::___mb = {};

void MeshLibrary::___init_method_bindings() {
	___mb.mb_clear = godot::api->godot_method_bind_get_method("MeshLibrary", "clear");
	___mb.mb_create_item = godot::api->godot_method_bind_get_method("MeshLibrary", "create_item");
	___mb.mb_find_item_by_name = godot::api->godot_method_bind_get_method("MeshLibrary", "find_item_by_name");
	___mb.mb_get_item_list = godot::api->godot_method_bind_get_method("MeshLibrary", "get_item_list");
	___mb.mb_get_item_mesh = godot::api->godot_method_bind_get_method("MeshLibrary", "get_item_mesh");
	___mb.mb_get_item_name = godot::api->godot_method_bind_get_method("MeshLibrary", "get_item_name");
	___mb.mb_get_item_navmesh = godot::api->godot_method_bind_get_method("MeshLibrary", "get_item_navmesh");
	___mb.mb_get_item_navmesh_transform = godot::api->godot_method_bind_get_method("MeshLibrary", "get_item_navmesh_transform");
	___mb.mb_get_item_preview = godot::api->godot_method_bind_get_method("MeshLibrary", "get_item_preview");
	___mb.mb_get_item_shapes = godot::api->godot_method_bind_get_method("MeshLibrary", "get_item_shapes");
	___mb.mb_get_last_unused_item_id = godot::api->godot_method_bind_get_method("MeshLibrary", "get_last_unused_item_id");
	___mb.mb_remove_item = godot::api->godot_method_bind_get_method("MeshLibrary", "remove_item");
	___mb.mb_set_item_mesh = godot::api->godot_method_bind_get_method("MeshLibrary", "set_item_mesh");
	___mb.mb_set_item_name = godot::api->godot_method_bind_get_method("MeshLibrary", "set_item_name");
	___mb.mb_set_item_navmesh = godot::api->godot_method_bind_get_method("MeshLibrary", "set_item_navmesh");
	___mb.mb_set_item_navmesh_transform = godot::api->godot_method_bind_get_method("MeshLibrary", "set_item_navmesh_transform");
	___mb.mb_set_item_preview = godot::api->godot_method_bind_get_method("MeshLibrary", "set_item_preview");
	___mb.mb_set_item_shapes = godot::api->godot_method_bind_get_method("MeshLibrary", "set_item_shapes");
}

MeshLibrary *MeshLibrary::_new()
{
	return (MeshLibrary *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"MeshLibrary")());
}
void MeshLibrary::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

void MeshLibrary::create_item(const int64_t id) {
	___godot_icall_void_int(___mb.mb_create_item, (const Object *) this, id);
}

int64_t MeshLibrary::find_item_by_name(const String name) const {
	return ___godot_icall_int_String(___mb.mb_find_item_by_name, (const Object *) this, name);
}

PoolIntArray MeshLibrary::get_item_list() const {
	return ___godot_icall_PoolIntArray(___mb.mb_get_item_list, (const Object *) this);
}

Ref<Mesh> MeshLibrary::get_item_mesh(const int64_t id) const {
	return Ref<Mesh>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_item_mesh, (const Object *) this, id));
}

String MeshLibrary::get_item_name(const int64_t id) const {
	return ___godot_icall_String_int(___mb.mb_get_item_name, (const Object *) this, id);
}

Ref<NavigationMesh> MeshLibrary::get_item_navmesh(const int64_t id) const {
	return Ref<NavigationMesh>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_item_navmesh, (const Object *) this, id));
}

Transform MeshLibrary::get_item_navmesh_transform(const int64_t id) const {
	return ___godot_icall_Transform_int(___mb.mb_get_item_navmesh_transform, (const Object *) this, id);
}

Ref<Texture> MeshLibrary::get_item_preview(const int64_t id) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_item_preview, (const Object *) this, id));
}

Array MeshLibrary::get_item_shapes(const int64_t id) const {
	return ___godot_icall_Array_int(___mb.mb_get_item_shapes, (const Object *) this, id);
}

int64_t MeshLibrary::get_last_unused_item_id() const {
	return ___godot_icall_int(___mb.mb_get_last_unused_item_id, (const Object *) this);
}

void MeshLibrary::remove_item(const int64_t id) {
	___godot_icall_void_int(___mb.mb_remove_item, (const Object *) this, id);
}

void MeshLibrary::set_item_mesh(const int64_t id, const Ref<Mesh> mesh) {
	___godot_icall_void_int_Object(___mb.mb_set_item_mesh, (const Object *) this, id, mesh.ptr());
}

void MeshLibrary::set_item_name(const int64_t id, const String name) {
	___godot_icall_void_int_String(___mb.mb_set_item_name, (const Object *) this, id, name);
}

void MeshLibrary::set_item_navmesh(const int64_t id, const Ref<NavigationMesh> navmesh) {
	___godot_icall_void_int_Object(___mb.mb_set_item_navmesh, (const Object *) this, id, navmesh.ptr());
}

void MeshLibrary::set_item_navmesh_transform(const int64_t id, const Transform navmesh) {
	___godot_icall_void_int_Transform(___mb.mb_set_item_navmesh_transform, (const Object *) this, id, navmesh);
}

void MeshLibrary::set_item_preview(const int64_t id, const Ref<Texture> texture) {
	___godot_icall_void_int_Object(___mb.mb_set_item_preview, (const Object *) this, id, texture.ptr());
}

void MeshLibrary::set_item_shapes(const int64_t id, const Array shapes) {
	___godot_icall_void_int_Array(___mb.mb_set_item_shapes, (const Object *) this, id, shapes);
}

}