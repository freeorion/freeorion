#include "GeometryInstance.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Material.hpp"


namespace godot {


GeometryInstance::___method_bindings GeometryInstance::___mb = {};

void GeometryInstance::___init_method_bindings() {
	___mb.mb_get_cast_shadows_setting = godot::api->godot_method_bind_get_method("GeometryInstance", "get_cast_shadows_setting");
	___mb.mb_get_extra_cull_margin = godot::api->godot_method_bind_get_method("GeometryInstance", "get_extra_cull_margin");
	___mb.mb_get_flag = godot::api->godot_method_bind_get_method("GeometryInstance", "get_flag");
	___mb.mb_get_lod_max_distance = godot::api->godot_method_bind_get_method("GeometryInstance", "get_lod_max_distance");
	___mb.mb_get_lod_max_hysteresis = godot::api->godot_method_bind_get_method("GeometryInstance", "get_lod_max_hysteresis");
	___mb.mb_get_lod_min_distance = godot::api->godot_method_bind_get_method("GeometryInstance", "get_lod_min_distance");
	___mb.mb_get_lod_min_hysteresis = godot::api->godot_method_bind_get_method("GeometryInstance", "get_lod_min_hysteresis");
	___mb.mb_get_material_override = godot::api->godot_method_bind_get_method("GeometryInstance", "get_material_override");
	___mb.mb_set_cast_shadows_setting = godot::api->godot_method_bind_get_method("GeometryInstance", "set_cast_shadows_setting");
	___mb.mb_set_custom_aabb = godot::api->godot_method_bind_get_method("GeometryInstance", "set_custom_aabb");
	___mb.mb_set_extra_cull_margin = godot::api->godot_method_bind_get_method("GeometryInstance", "set_extra_cull_margin");
	___mb.mb_set_flag = godot::api->godot_method_bind_get_method("GeometryInstance", "set_flag");
	___mb.mb_set_lod_max_distance = godot::api->godot_method_bind_get_method("GeometryInstance", "set_lod_max_distance");
	___mb.mb_set_lod_max_hysteresis = godot::api->godot_method_bind_get_method("GeometryInstance", "set_lod_max_hysteresis");
	___mb.mb_set_lod_min_distance = godot::api->godot_method_bind_get_method("GeometryInstance", "set_lod_min_distance");
	___mb.mb_set_lod_min_hysteresis = godot::api->godot_method_bind_get_method("GeometryInstance", "set_lod_min_hysteresis");
	___mb.mb_set_material_override = godot::api->godot_method_bind_get_method("GeometryInstance", "set_material_override");
}

GeometryInstance::ShadowCastingSetting GeometryInstance::get_cast_shadows_setting() const {
	return (GeometryInstance::ShadowCastingSetting) ___godot_icall_int(___mb.mb_get_cast_shadows_setting, (const Object *) this);
}

real_t GeometryInstance::get_extra_cull_margin() const {
	return ___godot_icall_float(___mb.mb_get_extra_cull_margin, (const Object *) this);
}

bool GeometryInstance::get_flag(const int64_t flag) const {
	return ___godot_icall_bool_int(___mb.mb_get_flag, (const Object *) this, flag);
}

real_t GeometryInstance::get_lod_max_distance() const {
	return ___godot_icall_float(___mb.mb_get_lod_max_distance, (const Object *) this);
}

real_t GeometryInstance::get_lod_max_hysteresis() const {
	return ___godot_icall_float(___mb.mb_get_lod_max_hysteresis, (const Object *) this);
}

real_t GeometryInstance::get_lod_min_distance() const {
	return ___godot_icall_float(___mb.mb_get_lod_min_distance, (const Object *) this);
}

real_t GeometryInstance::get_lod_min_hysteresis() const {
	return ___godot_icall_float(___mb.mb_get_lod_min_hysteresis, (const Object *) this);
}

Ref<Material> GeometryInstance::get_material_override() const {
	return Ref<Material>::__internal_constructor(___godot_icall_Object(___mb.mb_get_material_override, (const Object *) this));
}

void GeometryInstance::set_cast_shadows_setting(const int64_t shadow_casting_setting) {
	___godot_icall_void_int(___mb.mb_set_cast_shadows_setting, (const Object *) this, shadow_casting_setting);
}

void GeometryInstance::set_custom_aabb(const AABB aabb) {
	___godot_icall_void_AABB(___mb.mb_set_custom_aabb, (const Object *) this, aabb);
}

void GeometryInstance::set_extra_cull_margin(const real_t margin) {
	___godot_icall_void_float(___mb.mb_set_extra_cull_margin, (const Object *) this, margin);
}

void GeometryInstance::set_flag(const int64_t flag, const bool value) {
	___godot_icall_void_int_bool(___mb.mb_set_flag, (const Object *) this, flag, value);
}

void GeometryInstance::set_lod_max_distance(const real_t mode) {
	___godot_icall_void_float(___mb.mb_set_lod_max_distance, (const Object *) this, mode);
}

void GeometryInstance::set_lod_max_hysteresis(const real_t mode) {
	___godot_icall_void_float(___mb.mb_set_lod_max_hysteresis, (const Object *) this, mode);
}

void GeometryInstance::set_lod_min_distance(const real_t mode) {
	___godot_icall_void_float(___mb.mb_set_lod_min_distance, (const Object *) this, mode);
}

void GeometryInstance::set_lod_min_hysteresis(const real_t mode) {
	___godot_icall_void_float(___mb.mb_set_lod_min_hysteresis, (const Object *) this, mode);
}

void GeometryInstance::set_material_override(const Ref<Material> material) {
	___godot_icall_void_Object(___mb.mb_set_material_override, (const Object *) this, material.ptr());
}

}