#include "MeshTexture.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"
#include "Mesh.hpp"


namespace godot {


MeshTexture::___method_bindings MeshTexture::___mb = {};

void MeshTexture::___init_method_bindings() {
	___mb.mb_get_base_texture = godot::api->godot_method_bind_get_method("MeshTexture", "get_base_texture");
	___mb.mb_get_image_size = godot::api->godot_method_bind_get_method("MeshTexture", "get_image_size");
	___mb.mb_get_mesh = godot::api->godot_method_bind_get_method("MeshTexture", "get_mesh");
	___mb.mb_set_base_texture = godot::api->godot_method_bind_get_method("MeshTexture", "set_base_texture");
	___mb.mb_set_image_size = godot::api->godot_method_bind_get_method("MeshTexture", "set_image_size");
	___mb.mb_set_mesh = godot::api->godot_method_bind_get_method("MeshTexture", "set_mesh");
}

MeshTexture *MeshTexture::_new()
{
	return (MeshTexture *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"MeshTexture")());
}
Ref<Texture> MeshTexture::get_base_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_base_texture, (const Object *) this));
}

Vector2 MeshTexture::get_image_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_image_size, (const Object *) this);
}

Ref<Mesh> MeshTexture::get_mesh() const {
	return Ref<Mesh>::__internal_constructor(___godot_icall_Object(___mb.mb_get_mesh, (const Object *) this));
}

void MeshTexture::set_base_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_base_texture, (const Object *) this, texture.ptr());
}

void MeshTexture::set_image_size(const Vector2 size) {
	___godot_icall_void_Vector2(___mb.mb_set_image_size, (const Object *) this, size);
}

void MeshTexture::set_mesh(const Ref<Mesh> mesh) {
	___godot_icall_void_Object(___mb.mb_set_mesh, (const Object *) this, mesh.ptr());
}

}