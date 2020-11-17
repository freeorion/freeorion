#include "MultiMeshInstance2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "MultiMesh.hpp"
#include "Texture.hpp"


namespace godot {


MultiMeshInstance2D::___method_bindings MultiMeshInstance2D::___mb = {};

void MultiMeshInstance2D::___init_method_bindings() {
	___mb.mb_get_multimesh = godot::api->godot_method_bind_get_method("MultiMeshInstance2D", "get_multimesh");
	___mb.mb_get_normal_map = godot::api->godot_method_bind_get_method("MultiMeshInstance2D", "get_normal_map");
	___mb.mb_get_texture = godot::api->godot_method_bind_get_method("MultiMeshInstance2D", "get_texture");
	___mb.mb_set_multimesh = godot::api->godot_method_bind_get_method("MultiMeshInstance2D", "set_multimesh");
	___mb.mb_set_normal_map = godot::api->godot_method_bind_get_method("MultiMeshInstance2D", "set_normal_map");
	___mb.mb_set_texture = godot::api->godot_method_bind_get_method("MultiMeshInstance2D", "set_texture");
}

MultiMeshInstance2D *MultiMeshInstance2D::_new()
{
	return (MultiMeshInstance2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"MultiMeshInstance2D")());
}
Ref<MultiMesh> MultiMeshInstance2D::get_multimesh() const {
	return Ref<MultiMesh>::__internal_constructor(___godot_icall_Object(___mb.mb_get_multimesh, (const Object *) this));
}

Ref<Texture> MultiMeshInstance2D::get_normal_map() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_normal_map, (const Object *) this));
}

Ref<Texture> MultiMeshInstance2D::get_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_texture, (const Object *) this));
}

void MultiMeshInstance2D::set_multimesh(const Ref<MultiMesh> multimesh) {
	___godot_icall_void_Object(___mb.mb_set_multimesh, (const Object *) this, multimesh.ptr());
}

void MultiMeshInstance2D::set_normal_map(const Ref<Texture> normal_map) {
	___godot_icall_void_Object(___mb.mb_set_normal_map, (const Object *) this, normal_map.ptr());
}

void MultiMeshInstance2D::set_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_texture, (const Object *) this, texture.ptr());
}

}