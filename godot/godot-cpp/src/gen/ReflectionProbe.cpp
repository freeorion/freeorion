#include "ReflectionProbe.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ReflectionProbe::___method_bindings ReflectionProbe::___mb = {};

void ReflectionProbe::___init_method_bindings() {
	___mb.mb_are_shadows_enabled = godot::api->godot_method_bind_get_method("ReflectionProbe", "are_shadows_enabled");
	___mb.mb_get_cull_mask = godot::api->godot_method_bind_get_method("ReflectionProbe", "get_cull_mask");
	___mb.mb_get_extents = godot::api->godot_method_bind_get_method("ReflectionProbe", "get_extents");
	___mb.mb_get_intensity = godot::api->godot_method_bind_get_method("ReflectionProbe", "get_intensity");
	___mb.mb_get_interior_ambient = godot::api->godot_method_bind_get_method("ReflectionProbe", "get_interior_ambient");
	___mb.mb_get_interior_ambient_energy = godot::api->godot_method_bind_get_method("ReflectionProbe", "get_interior_ambient_energy");
	___mb.mb_get_interior_ambient_probe_contribution = godot::api->godot_method_bind_get_method("ReflectionProbe", "get_interior_ambient_probe_contribution");
	___mb.mb_get_max_distance = godot::api->godot_method_bind_get_method("ReflectionProbe", "get_max_distance");
	___mb.mb_get_origin_offset = godot::api->godot_method_bind_get_method("ReflectionProbe", "get_origin_offset");
	___mb.mb_get_update_mode = godot::api->godot_method_bind_get_method("ReflectionProbe", "get_update_mode");
	___mb.mb_is_box_projection_enabled = godot::api->godot_method_bind_get_method("ReflectionProbe", "is_box_projection_enabled");
	___mb.mb_is_set_as_interior = godot::api->godot_method_bind_get_method("ReflectionProbe", "is_set_as_interior");
	___mb.mb_set_as_interior = godot::api->godot_method_bind_get_method("ReflectionProbe", "set_as_interior");
	___mb.mb_set_cull_mask = godot::api->godot_method_bind_get_method("ReflectionProbe", "set_cull_mask");
	___mb.mb_set_enable_box_projection = godot::api->godot_method_bind_get_method("ReflectionProbe", "set_enable_box_projection");
	___mb.mb_set_enable_shadows = godot::api->godot_method_bind_get_method("ReflectionProbe", "set_enable_shadows");
	___mb.mb_set_extents = godot::api->godot_method_bind_get_method("ReflectionProbe", "set_extents");
	___mb.mb_set_intensity = godot::api->godot_method_bind_get_method("ReflectionProbe", "set_intensity");
	___mb.mb_set_interior_ambient = godot::api->godot_method_bind_get_method("ReflectionProbe", "set_interior_ambient");
	___mb.mb_set_interior_ambient_energy = godot::api->godot_method_bind_get_method("ReflectionProbe", "set_interior_ambient_energy");
	___mb.mb_set_interior_ambient_probe_contribution = godot::api->godot_method_bind_get_method("ReflectionProbe", "set_interior_ambient_probe_contribution");
	___mb.mb_set_max_distance = godot::api->godot_method_bind_get_method("ReflectionProbe", "set_max_distance");
	___mb.mb_set_origin_offset = godot::api->godot_method_bind_get_method("ReflectionProbe", "set_origin_offset");
	___mb.mb_set_update_mode = godot::api->godot_method_bind_get_method("ReflectionProbe", "set_update_mode");
}

ReflectionProbe *ReflectionProbe::_new()
{
	return (ReflectionProbe *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ReflectionProbe")());
}
bool ReflectionProbe::are_shadows_enabled() const {
	return ___godot_icall_bool(___mb.mb_are_shadows_enabled, (const Object *) this);
}

int64_t ReflectionProbe::get_cull_mask() const {
	return ___godot_icall_int(___mb.mb_get_cull_mask, (const Object *) this);
}

Vector3 ReflectionProbe::get_extents() const {
	return ___godot_icall_Vector3(___mb.mb_get_extents, (const Object *) this);
}

real_t ReflectionProbe::get_intensity() const {
	return ___godot_icall_float(___mb.mb_get_intensity, (const Object *) this);
}

Color ReflectionProbe::get_interior_ambient() const {
	return ___godot_icall_Color(___mb.mb_get_interior_ambient, (const Object *) this);
}

real_t ReflectionProbe::get_interior_ambient_energy() const {
	return ___godot_icall_float(___mb.mb_get_interior_ambient_energy, (const Object *) this);
}

real_t ReflectionProbe::get_interior_ambient_probe_contribution() const {
	return ___godot_icall_float(___mb.mb_get_interior_ambient_probe_contribution, (const Object *) this);
}

real_t ReflectionProbe::get_max_distance() const {
	return ___godot_icall_float(___mb.mb_get_max_distance, (const Object *) this);
}

Vector3 ReflectionProbe::get_origin_offset() const {
	return ___godot_icall_Vector3(___mb.mb_get_origin_offset, (const Object *) this);
}

ReflectionProbe::UpdateMode ReflectionProbe::get_update_mode() const {
	return (ReflectionProbe::UpdateMode) ___godot_icall_int(___mb.mb_get_update_mode, (const Object *) this);
}

bool ReflectionProbe::is_box_projection_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_box_projection_enabled, (const Object *) this);
}

bool ReflectionProbe::is_set_as_interior() const {
	return ___godot_icall_bool(___mb.mb_is_set_as_interior, (const Object *) this);
}

void ReflectionProbe::set_as_interior(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_as_interior, (const Object *) this, enable);
}

void ReflectionProbe::set_cull_mask(const int64_t layers) {
	___godot_icall_void_int(___mb.mb_set_cull_mask, (const Object *) this, layers);
}

void ReflectionProbe::set_enable_box_projection(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_enable_box_projection, (const Object *) this, enable);
}

void ReflectionProbe::set_enable_shadows(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_enable_shadows, (const Object *) this, enable);
}

void ReflectionProbe::set_extents(const Vector3 extents) {
	___godot_icall_void_Vector3(___mb.mb_set_extents, (const Object *) this, extents);
}

void ReflectionProbe::set_intensity(const real_t intensity) {
	___godot_icall_void_float(___mb.mb_set_intensity, (const Object *) this, intensity);
}

void ReflectionProbe::set_interior_ambient(const Color ambient) {
	___godot_icall_void_Color(___mb.mb_set_interior_ambient, (const Object *) this, ambient);
}

void ReflectionProbe::set_interior_ambient_energy(const real_t ambient_energy) {
	___godot_icall_void_float(___mb.mb_set_interior_ambient_energy, (const Object *) this, ambient_energy);
}

void ReflectionProbe::set_interior_ambient_probe_contribution(const real_t ambient_probe_contribution) {
	___godot_icall_void_float(___mb.mb_set_interior_ambient_probe_contribution, (const Object *) this, ambient_probe_contribution);
}

void ReflectionProbe::set_max_distance(const real_t max_distance) {
	___godot_icall_void_float(___mb.mb_set_max_distance, (const Object *) this, max_distance);
}

void ReflectionProbe::set_origin_offset(const Vector3 origin_offset) {
	___godot_icall_void_Vector3(___mb.mb_set_origin_offset, (const Object *) this, origin_offset);
}

void ReflectionProbe::set_update_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_update_mode, (const Object *) this, mode);
}

}