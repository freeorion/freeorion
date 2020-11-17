#include "VisualShaderNodeTextureUniform.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNodeTextureUniform::___method_bindings VisualShaderNodeTextureUniform::___mb = {};

void VisualShaderNodeTextureUniform::___init_method_bindings() {
	___mb.mb_get_color_default = godot::api->godot_method_bind_get_method("VisualShaderNodeTextureUniform", "get_color_default");
	___mb.mb_get_texture_type = godot::api->godot_method_bind_get_method("VisualShaderNodeTextureUniform", "get_texture_type");
	___mb.mb_set_color_default = godot::api->godot_method_bind_get_method("VisualShaderNodeTextureUniform", "set_color_default");
	___mb.mb_set_texture_type = godot::api->godot_method_bind_get_method("VisualShaderNodeTextureUniform", "set_texture_type");
}

VisualShaderNodeTextureUniform *VisualShaderNodeTextureUniform::_new()
{
	return (VisualShaderNodeTextureUniform *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeTextureUniform")());
}
VisualShaderNodeTextureUniform::ColorDefault VisualShaderNodeTextureUniform::get_color_default() const {
	return (VisualShaderNodeTextureUniform::ColorDefault) ___godot_icall_int(___mb.mb_get_color_default, (const Object *) this);
}

VisualShaderNodeTextureUniform::TextureType VisualShaderNodeTextureUniform::get_texture_type() const {
	return (VisualShaderNodeTextureUniform::TextureType) ___godot_icall_int(___mb.mb_get_texture_type, (const Object *) this);
}

void VisualShaderNodeTextureUniform::set_color_default(const int64_t type) {
	___godot_icall_void_int(___mb.mb_set_color_default, (const Object *) this, type);
}

void VisualShaderNodeTextureUniform::set_texture_type(const int64_t type) {
	___godot_icall_void_int(___mb.mb_set_texture_type, (const Object *) this, type);
}

}