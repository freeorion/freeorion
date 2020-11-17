#include "GIProbe.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Node.hpp"
#include "GIProbeData.hpp"


namespace godot {


GIProbe::___method_bindings GIProbe::___mb = {};

void GIProbe::___init_method_bindings() {
	___mb.mb_bake = godot::api->godot_method_bind_get_method("GIProbe", "bake");
	___mb.mb_debug_bake = godot::api->godot_method_bind_get_method("GIProbe", "debug_bake");
	___mb.mb_get_bias = godot::api->godot_method_bind_get_method("GIProbe", "get_bias");
	___mb.mb_get_dynamic_range = godot::api->godot_method_bind_get_method("GIProbe", "get_dynamic_range");
	___mb.mb_get_energy = godot::api->godot_method_bind_get_method("GIProbe", "get_energy");
	___mb.mb_get_extents = godot::api->godot_method_bind_get_method("GIProbe", "get_extents");
	___mb.mb_get_normal_bias = godot::api->godot_method_bind_get_method("GIProbe", "get_normal_bias");
	___mb.mb_get_probe_data = godot::api->godot_method_bind_get_method("GIProbe", "get_probe_data");
	___mb.mb_get_propagation = godot::api->godot_method_bind_get_method("GIProbe", "get_propagation");
	___mb.mb_get_subdiv = godot::api->godot_method_bind_get_method("GIProbe", "get_subdiv");
	___mb.mb_is_compressed = godot::api->godot_method_bind_get_method("GIProbe", "is_compressed");
	___mb.mb_is_interior = godot::api->godot_method_bind_get_method("GIProbe", "is_interior");
	___mb.mb_set_bias = godot::api->godot_method_bind_get_method("GIProbe", "set_bias");
	___mb.mb_set_compress = godot::api->godot_method_bind_get_method("GIProbe", "set_compress");
	___mb.mb_set_dynamic_range = godot::api->godot_method_bind_get_method("GIProbe", "set_dynamic_range");
	___mb.mb_set_energy = godot::api->godot_method_bind_get_method("GIProbe", "set_energy");
	___mb.mb_set_extents = godot::api->godot_method_bind_get_method("GIProbe", "set_extents");
	___mb.mb_set_interior = godot::api->godot_method_bind_get_method("GIProbe", "set_interior");
	___mb.mb_set_normal_bias = godot::api->godot_method_bind_get_method("GIProbe", "set_normal_bias");
	___mb.mb_set_probe_data = godot::api->godot_method_bind_get_method("GIProbe", "set_probe_data");
	___mb.mb_set_propagation = godot::api->godot_method_bind_get_method("GIProbe", "set_propagation");
	___mb.mb_set_subdiv = godot::api->godot_method_bind_get_method("GIProbe", "set_subdiv");
}

GIProbe *GIProbe::_new()
{
	return (GIProbe *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"GIProbe")());
}
void GIProbe::bake(const Node *from_node, const bool create_visual_debug) {
	___godot_icall_void_Object_bool(___mb.mb_bake, (const Object *) this, from_node, create_visual_debug);
}

void GIProbe::debug_bake() {
	___godot_icall_void(___mb.mb_debug_bake, (const Object *) this);
}

real_t GIProbe::get_bias() const {
	return ___godot_icall_float(___mb.mb_get_bias, (const Object *) this);
}

int64_t GIProbe::get_dynamic_range() const {
	return ___godot_icall_int(___mb.mb_get_dynamic_range, (const Object *) this);
}

real_t GIProbe::get_energy() const {
	return ___godot_icall_float(___mb.mb_get_energy, (const Object *) this);
}

Vector3 GIProbe::get_extents() const {
	return ___godot_icall_Vector3(___mb.mb_get_extents, (const Object *) this);
}

real_t GIProbe::get_normal_bias() const {
	return ___godot_icall_float(___mb.mb_get_normal_bias, (const Object *) this);
}

Ref<GIProbeData> GIProbe::get_probe_data() const {
	return Ref<GIProbeData>::__internal_constructor(___godot_icall_Object(___mb.mb_get_probe_data, (const Object *) this));
}

real_t GIProbe::get_propagation() const {
	return ___godot_icall_float(___mb.mb_get_propagation, (const Object *) this);
}

GIProbe::Subdiv GIProbe::get_subdiv() const {
	return (GIProbe::Subdiv) ___godot_icall_int(___mb.mb_get_subdiv, (const Object *) this);
}

bool GIProbe::is_compressed() const {
	return ___godot_icall_bool(___mb.mb_is_compressed, (const Object *) this);
}

bool GIProbe::is_interior() const {
	return ___godot_icall_bool(___mb.mb_is_interior, (const Object *) this);
}

void GIProbe::set_bias(const real_t max) {
	___godot_icall_void_float(___mb.mb_set_bias, (const Object *) this, max);
}

void GIProbe::set_compress(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_compress, (const Object *) this, enable);
}

void GIProbe::set_dynamic_range(const int64_t max) {
	___godot_icall_void_int(___mb.mb_set_dynamic_range, (const Object *) this, max);
}

void GIProbe::set_energy(const real_t max) {
	___godot_icall_void_float(___mb.mb_set_energy, (const Object *) this, max);
}

void GIProbe::set_extents(const Vector3 extents) {
	___godot_icall_void_Vector3(___mb.mb_set_extents, (const Object *) this, extents);
}

void GIProbe::set_interior(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_interior, (const Object *) this, enable);
}

void GIProbe::set_normal_bias(const real_t max) {
	___godot_icall_void_float(___mb.mb_set_normal_bias, (const Object *) this, max);
}

void GIProbe::set_probe_data(const Ref<GIProbeData> data) {
	___godot_icall_void_Object(___mb.mb_set_probe_data, (const Object *) this, data.ptr());
}

void GIProbe::set_propagation(const real_t max) {
	___godot_icall_void_float(___mb.mb_set_propagation, (const Object *) this, max);
}

void GIProbe::set_subdiv(const int64_t subdiv) {
	___godot_icall_void_int(___mb.mb_set_subdiv, (const Object *) this, subdiv);
}

}