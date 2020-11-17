#include "ViewportTexture.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ViewportTexture::___method_bindings ViewportTexture::___mb = {};

void ViewportTexture::___init_method_bindings() {
	___mb.mb_get_viewport_path_in_scene = godot::api->godot_method_bind_get_method("ViewportTexture", "get_viewport_path_in_scene");
	___mb.mb_set_viewport_path_in_scene = godot::api->godot_method_bind_get_method("ViewportTexture", "set_viewport_path_in_scene");
}

ViewportTexture *ViewportTexture::_new()
{
	return (ViewportTexture *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ViewportTexture")());
}
NodePath ViewportTexture::get_viewport_path_in_scene() const {
	return ___godot_icall_NodePath(___mb.mb_get_viewport_path_in_scene, (const Object *) this);
}

void ViewportTexture::set_viewport_path_in_scene(const NodePath path) {
	___godot_icall_void_NodePath(___mb.mb_set_viewport_path_in_scene, (const Object *) this, path);
}

}