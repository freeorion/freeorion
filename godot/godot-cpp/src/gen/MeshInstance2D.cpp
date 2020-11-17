#include "MeshInstance2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Mesh.hpp"
#include "Texture.hpp"


namespace godot {


MeshInstance2D::___method_bindings MeshInstance2D::___mb = {};

void MeshInstance2D::___init_method_bindings() {
	___mb.mb_get_mesh = godot::api->godot_method_bind_get_method("MeshInstance2D", "get_mesh");
	___mb.mb_get_normal_map = godot::api->godot_method_bind_get_method("MeshInstance2D", "get_normal_map");
	___mb.mb_get_texture = godot::api->godot_method_bind_get_method("MeshInstance2D", "get_texture");
	___mb.mb_set_mesh = godot::api->godot_method_bind_get_method("MeshInstance2D", "set_mesh");
	___mb.mb_set_normal_map = godot::api->godot_method_bind_get_method("MeshInstance2D", "set_normal_map");
	___mb.mb_set_texture = godot::api->godot_method_bind_get_method("MeshInstance2D", "set_texture");
}

MeshInstance2D *MeshInstance2D::_new()
{
	return (MeshInstance2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"MeshInstance2D")());
}
Ref<Mesh> MeshInstance2D::get_mesh() const {
	return Ref<Mesh>::__internal_constructor(___godot_icall_Object(___mb.mb_get_mesh, (const Object *) this));
}

Ref<Texture> MeshInstance2D::get_normal_map() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_normal_map, (const Object *) this));
}

Ref<Texture> MeshInstance2D::get_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_texture, (const Object *) this));
}

void MeshInstance2D::set_mesh(const Ref<Mesh> mesh) {
	___godot_icall_void_Object(___mb.mb_set_mesh, (const Object *) this, mesh.ptr());
}

void MeshInstance2D::set_normal_map(const Ref<Texture> normal_map) {
	___godot_icall_void_Object(___mb.mb_set_normal_map, (const Object *) this, normal_map.ptr());
}

void MeshInstance2D::set_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_texture, (const Object *) this, texture.ptr());
}

}