#include "DirectionalLight.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


DirectionalLight::___method_bindings DirectionalLight::___mb = {};

void DirectionalLight::___init_method_bindings() {
	___mb.mb_get_shadow_depth_range = godot::api->godot_method_bind_get_method("DirectionalLight", "get_shadow_depth_range");
	___mb.mb_get_shadow_mode = godot::api->godot_method_bind_get_method("DirectionalLight", "get_shadow_mode");
	___mb.mb_is_blend_splits_enabled = godot::api->godot_method_bind_get_method("DirectionalLight", "is_blend_splits_enabled");
	___mb.mb_set_blend_splits = godot::api->godot_method_bind_get_method("DirectionalLight", "set_blend_splits");
	___mb.mb_set_shadow_depth_range = godot::api->godot_method_bind_get_method("DirectionalLight", "set_shadow_depth_range");
	___mb.mb_set_shadow_mode = godot::api->godot_method_bind_get_method("DirectionalLight", "set_shadow_mode");
}

DirectionalLight *DirectionalLight::_new()
{
	return (DirectionalLight *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"DirectionalLight")());
}
DirectionalLight::ShadowDepthRange DirectionalLight::get_shadow_depth_range() const {
	return (DirectionalLight::ShadowDepthRange) ___godot_icall_int(___mb.mb_get_shadow_depth_range, (const Object *) this);
}

DirectionalLight::ShadowMode DirectionalLight::get_shadow_mode() const {
	return (DirectionalLight::ShadowMode) ___godot_icall_int(___mb.mb_get_shadow_mode, (const Object *) this);
}

bool DirectionalLight::is_blend_splits_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_blend_splits_enabled, (const Object *) this);
}

void DirectionalLight::set_blend_splits(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_blend_splits, (const Object *) this, enabled);
}

void DirectionalLight::set_shadow_depth_range(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_shadow_depth_range, (const Object *) this, mode);
}

void DirectionalLight::set_shadow_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_shadow_mode, (const Object *) this, mode);
}

}