#include "OmniLight.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


OmniLight::___method_bindings OmniLight::___mb = {};

void OmniLight::___init_method_bindings() {
	___mb.mb_get_shadow_detail = godot::api->godot_method_bind_get_method("OmniLight", "get_shadow_detail");
	___mb.mb_get_shadow_mode = godot::api->godot_method_bind_get_method("OmniLight", "get_shadow_mode");
	___mb.mb_set_shadow_detail = godot::api->godot_method_bind_get_method("OmniLight", "set_shadow_detail");
	___mb.mb_set_shadow_mode = godot::api->godot_method_bind_get_method("OmniLight", "set_shadow_mode");
}

OmniLight *OmniLight::_new()
{
	return (OmniLight *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"OmniLight")());
}
OmniLight::ShadowDetail OmniLight::get_shadow_detail() const {
	return (OmniLight::ShadowDetail) ___godot_icall_int(___mb.mb_get_shadow_detail, (const Object *) this);
}

OmniLight::ShadowMode OmniLight::get_shadow_mode() const {
	return (OmniLight::ShadowMode) ___godot_icall_int(___mb.mb_get_shadow_mode, (const Object *) this);
}

void OmniLight::set_shadow_detail(const int64_t detail) {
	___godot_icall_void_int(___mb.mb_set_shadow_detail, (const Object *) this, detail);
}

void OmniLight::set_shadow_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_shadow_mode, (const Object *) this, mode);
}

}