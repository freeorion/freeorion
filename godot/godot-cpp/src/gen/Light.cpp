#include "Light.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Light::___method_bindings Light::___mb = {};

void Light::___init_method_bindings() {
	___mb.mb_get_bake_mode = godot::api->godot_method_bind_get_method("Light", "get_bake_mode");
	___mb.mb_get_color = godot::api->godot_method_bind_get_method("Light", "get_color");
	___mb.mb_get_cull_mask = godot::api->godot_method_bind_get_method("Light", "get_cull_mask");
	___mb.mb_get_param = godot::api->godot_method_bind_get_method("Light", "get_param");
	___mb.mb_get_shadow_color = godot::api->godot_method_bind_get_method("Light", "get_shadow_color");
	___mb.mb_get_shadow_reverse_cull_face = godot::api->godot_method_bind_get_method("Light", "get_shadow_reverse_cull_face");
	___mb.mb_has_shadow = godot::api->godot_method_bind_get_method("Light", "has_shadow");
	___mb.mb_is_editor_only = godot::api->godot_method_bind_get_method("Light", "is_editor_only");
	___mb.mb_is_negative = godot::api->godot_method_bind_get_method("Light", "is_negative");
	___mb.mb_set_bake_mode = godot::api->godot_method_bind_get_method("Light", "set_bake_mode");
	___mb.mb_set_color = godot::api->godot_method_bind_get_method("Light", "set_color");
	___mb.mb_set_cull_mask = godot::api->godot_method_bind_get_method("Light", "set_cull_mask");
	___mb.mb_set_editor_only = godot::api->godot_method_bind_get_method("Light", "set_editor_only");
	___mb.mb_set_negative = godot::api->godot_method_bind_get_method("Light", "set_negative");
	___mb.mb_set_param = godot::api->godot_method_bind_get_method("Light", "set_param");
	___mb.mb_set_shadow = godot::api->godot_method_bind_get_method("Light", "set_shadow");
	___mb.mb_set_shadow_color = godot::api->godot_method_bind_get_method("Light", "set_shadow_color");
	___mb.mb_set_shadow_reverse_cull_face = godot::api->godot_method_bind_get_method("Light", "set_shadow_reverse_cull_face");
}

Light::BakeMode Light::get_bake_mode() const {
	return (Light::BakeMode) ___godot_icall_int(___mb.mb_get_bake_mode, (const Object *) this);
}

Color Light::get_color() const {
	return ___godot_icall_Color(___mb.mb_get_color, (const Object *) this);
}

int64_t Light::get_cull_mask() const {
	return ___godot_icall_int(___mb.mb_get_cull_mask, (const Object *) this);
}

real_t Light::get_param(const int64_t param) const {
	return ___godot_icall_float_int(___mb.mb_get_param, (const Object *) this, param);
}

Color Light::get_shadow_color() const {
	return ___godot_icall_Color(___mb.mb_get_shadow_color, (const Object *) this);
}

bool Light::get_shadow_reverse_cull_face() const {
	return ___godot_icall_bool(___mb.mb_get_shadow_reverse_cull_face, (const Object *) this);
}

bool Light::has_shadow() const {
	return ___godot_icall_bool(___mb.mb_has_shadow, (const Object *) this);
}

bool Light::is_editor_only() const {
	return ___godot_icall_bool(___mb.mb_is_editor_only, (const Object *) this);
}

bool Light::is_negative() const {
	return ___godot_icall_bool(___mb.mb_is_negative, (const Object *) this);
}

void Light::set_bake_mode(const int64_t bake_mode) {
	___godot_icall_void_int(___mb.mb_set_bake_mode, (const Object *) this, bake_mode);
}

void Light::set_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_color, (const Object *) this, color);
}

void Light::set_cull_mask(const int64_t cull_mask) {
	___godot_icall_void_int(___mb.mb_set_cull_mask, (const Object *) this, cull_mask);
}

void Light::set_editor_only(const bool editor_only) {
	___godot_icall_void_bool(___mb.mb_set_editor_only, (const Object *) this, editor_only);
}

void Light::set_negative(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_negative, (const Object *) this, enabled);
}

void Light::set_param(const int64_t param, const real_t value) {
	___godot_icall_void_int_float(___mb.mb_set_param, (const Object *) this, param, value);
}

void Light::set_shadow(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_shadow, (const Object *) this, enabled);
}

void Light::set_shadow_color(const Color shadow_color) {
	___godot_icall_void_Color(___mb.mb_set_shadow_color, (const Object *) this, shadow_color);
}

void Light::set_shadow_reverse_cull_face(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_shadow_reverse_cull_face, (const Object *) this, enable);
}

}