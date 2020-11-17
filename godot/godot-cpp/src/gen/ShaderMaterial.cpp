#include "ShaderMaterial.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Shader.hpp"


namespace godot {


ShaderMaterial::___method_bindings ShaderMaterial::___mb = {};

void ShaderMaterial::___init_method_bindings() {
	___mb.mb__shader_changed = godot::api->godot_method_bind_get_method("ShaderMaterial", "_shader_changed");
	___mb.mb_get_shader = godot::api->godot_method_bind_get_method("ShaderMaterial", "get_shader");
	___mb.mb_get_shader_param = godot::api->godot_method_bind_get_method("ShaderMaterial", "get_shader_param");
	___mb.mb_property_can_revert = godot::api->godot_method_bind_get_method("ShaderMaterial", "property_can_revert");
	___mb.mb_property_get_revert = godot::api->godot_method_bind_get_method("ShaderMaterial", "property_get_revert");
	___mb.mb_set_shader = godot::api->godot_method_bind_get_method("ShaderMaterial", "set_shader");
	___mb.mb_set_shader_param = godot::api->godot_method_bind_get_method("ShaderMaterial", "set_shader_param");
}

ShaderMaterial *ShaderMaterial::_new()
{
	return (ShaderMaterial *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ShaderMaterial")());
}
void ShaderMaterial::_shader_changed() {
	___godot_icall_void(___mb.mb__shader_changed, (const Object *) this);
}

Ref<Shader> ShaderMaterial::get_shader() const {
	return Ref<Shader>::__internal_constructor(___godot_icall_Object(___mb.mb_get_shader, (const Object *) this));
}

Variant ShaderMaterial::get_shader_param(const String param) const {
	return ___godot_icall_Variant_String(___mb.mb_get_shader_param, (const Object *) this, param);
}

bool ShaderMaterial::property_can_revert(const String name) {
	return ___godot_icall_bool_String(___mb.mb_property_can_revert, (const Object *) this, name);
}

Variant ShaderMaterial::property_get_revert(const String name) {
	return ___godot_icall_Variant_String(___mb.mb_property_get_revert, (const Object *) this, name);
}

void ShaderMaterial::set_shader(const Ref<Shader> shader) {
	___godot_icall_void_Object(___mb.mb_set_shader, (const Object *) this, shader.ptr());
}

void ShaderMaterial::set_shader_param(const String param, const Variant value) {
	___godot_icall_void_String_Variant(___mb.mb_set_shader_param, (const Object *) this, param, value);
}

}