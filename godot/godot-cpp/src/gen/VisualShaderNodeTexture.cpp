#include "VisualShaderNodeTexture.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


VisualShaderNodeTexture::___method_bindings VisualShaderNodeTexture::___mb = {};

void VisualShaderNodeTexture::___init_method_bindings() {
	___mb.mb_get_source = godot::api->godot_method_bind_get_method("VisualShaderNodeTexture", "get_source");
	___mb.mb_get_texture = godot::api->godot_method_bind_get_method("VisualShaderNodeTexture", "get_texture");
	___mb.mb_get_texture_type = godot::api->godot_method_bind_get_method("VisualShaderNodeTexture", "get_texture_type");
	___mb.mb_set_source = godot::api->godot_method_bind_get_method("VisualShaderNodeTexture", "set_source");
	___mb.mb_set_texture = godot::api->godot_method_bind_get_method("VisualShaderNodeTexture", "set_texture");
	___mb.mb_set_texture_type = godot::api->godot_method_bind_get_method("VisualShaderNodeTexture", "set_texture_type");
}

VisualShaderNodeTexture *VisualShaderNodeTexture::_new()
{
	return (VisualShaderNodeTexture *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeTexture")());
}
VisualShaderNodeTexture::Source VisualShaderNodeTexture::get_source() const {
	return (VisualShaderNodeTexture::Source) ___godot_icall_int(___mb.mb_get_source, (const Object *) this);
}

Ref<Texture> VisualShaderNodeTexture::get_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_texture, (const Object *) this));
}

VisualShaderNodeTexture::TextureType VisualShaderNodeTexture::get_texture_type() const {
	return (VisualShaderNodeTexture::TextureType) ___godot_icall_int(___mb.mb_get_texture_type, (const Object *) this);
}

void VisualShaderNodeTexture::set_source(const int64_t value) {
	___godot_icall_void_int(___mb.mb_set_source, (const Object *) this, value);
}

void VisualShaderNodeTexture::set_texture(const Ref<Texture> value) {
	___godot_icall_void_Object(___mb.mb_set_texture, (const Object *) this, value.ptr());
}

void VisualShaderNodeTexture::set_texture_type(const int64_t value) {
	___godot_icall_void_int(___mb.mb_set_texture_type, (const Object *) this, value);
}

}