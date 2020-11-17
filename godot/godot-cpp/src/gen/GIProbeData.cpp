#include "GIProbeData.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


GIProbeData::___method_bindings GIProbeData::___mb = {};

void GIProbeData::___init_method_bindings() {
	___mb.mb_get_bias = godot::api->godot_method_bind_get_method("GIProbeData", "get_bias");
	___mb.mb_get_bounds = godot::api->godot_method_bind_get_method("GIProbeData", "get_bounds");
	___mb.mb_get_cell_size = godot::api->godot_method_bind_get_method("GIProbeData", "get_cell_size");
	___mb.mb_get_dynamic_data = godot::api->godot_method_bind_get_method("GIProbeData", "get_dynamic_data");
	___mb.mb_get_dynamic_range = godot::api->godot_method_bind_get_method("GIProbeData", "get_dynamic_range");
	___mb.mb_get_energy = godot::api->godot_method_bind_get_method("GIProbeData", "get_energy");
	___mb.mb_get_normal_bias = godot::api->godot_method_bind_get_method("GIProbeData", "get_normal_bias");
	___mb.mb_get_propagation = godot::api->godot_method_bind_get_method("GIProbeData", "get_propagation");
	___mb.mb_get_to_cell_xform = godot::api->godot_method_bind_get_method("GIProbeData", "get_to_cell_xform");
	___mb.mb_is_compressed = godot::api->godot_method_bind_get_method("GIProbeData", "is_compressed");
	___mb.mb_is_interior = godot::api->godot_method_bind_get_method("GIProbeData", "is_interior");
	___mb.mb_set_bias = godot::api->godot_method_bind_get_method("GIProbeData", "set_bias");
	___mb.mb_set_bounds = godot::api->godot_method_bind_get_method("GIProbeData", "set_bounds");
	___mb.mb_set_cell_size = godot::api->godot_method_bind_get_method("GIProbeData", "set_cell_size");
	___mb.mb_set_compress = godot::api->godot_method_bind_get_method("GIProbeData", "set_compress");
	___mb.mb_set_dynamic_data = godot::api->godot_method_bind_get_method("GIProbeData", "set_dynamic_data");
	___mb.mb_set_dynamic_range = godot::api->godot_method_bind_get_method("GIProbeData", "set_dynamic_range");
	___mb.mb_set_energy = godot::api->godot_method_bind_get_method("GIProbeData", "set_energy");
	___mb.mb_set_interior = godot::api->godot_method_bind_get_method("GIProbeData", "set_interior");
	___mb.mb_set_normal_bias = godot::api->godot_method_bind_get_method("GIProbeData", "set_normal_bias");
	___mb.mb_set_propagation = godot::api->godot_method_bind_get_method("GIProbeData", "set_propagation");
	___mb.mb_set_to_cell_xform = godot::api->godot_method_bind_get_method("GIProbeData", "set_to_cell_xform");
}

GIProbeData *GIProbeData::_new()
{
	return (GIProbeData *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"GIProbeData")());
}
real_t GIProbeData::get_bias() const {
	return ___godot_icall_float(___mb.mb_get_bias, (const Object *) this);
}

AABB GIProbeData::get_bounds() const {
	return ___godot_icall_AABB(___mb.mb_get_bounds, (const Object *) this);
}

real_t GIProbeData::get_cell_size() const {
	return ___godot_icall_float(___mb.mb_get_cell_size, (const Object *) this);
}

PoolIntArray GIProbeData::get_dynamic_data() const {
	return ___godot_icall_PoolIntArray(___mb.mb_get_dynamic_data, (const Object *) this);
}

int64_t GIProbeData::get_dynamic_range() const {
	return ___godot_icall_int(___mb.mb_get_dynamic_range, (const Object *) this);
}

real_t GIProbeData::get_energy() const {
	return ___godot_icall_float(___mb.mb_get_energy, (const Object *) this);
}

real_t GIProbeData::get_normal_bias() const {
	return ___godot_icall_float(___mb.mb_get_normal_bias, (const Object *) this);
}

real_t GIProbeData::get_propagation() const {
	return ___godot_icall_float(___mb.mb_get_propagation, (const Object *) this);
}

Transform GIProbeData::get_to_cell_xform() const {
	return ___godot_icall_Transform(___mb.mb_get_to_cell_xform, (const Object *) this);
}

bool GIProbeData::is_compressed() const {
	return ___godot_icall_bool(___mb.mb_is_compressed, (const Object *) this);
}

bool GIProbeData::is_interior() const {
	return ___godot_icall_bool(___mb.mb_is_interior, (const Object *) this);
}

void GIProbeData::set_bias(const real_t bias) {
	___godot_icall_void_float(___mb.mb_set_bias, (const Object *) this, bias);
}

void GIProbeData::set_bounds(const AABB bounds) {
	___godot_icall_void_AABB(___mb.mb_set_bounds, (const Object *) this, bounds);
}

void GIProbeData::set_cell_size(const real_t cell_size) {
	___godot_icall_void_float(___mb.mb_set_cell_size, (const Object *) this, cell_size);
}

void GIProbeData::set_compress(const bool compress) {
	___godot_icall_void_bool(___mb.mb_set_compress, (const Object *) this, compress);
}

void GIProbeData::set_dynamic_data(const PoolIntArray dynamic_data) {
	___godot_icall_void_PoolIntArray(___mb.mb_set_dynamic_data, (const Object *) this, dynamic_data);
}

void GIProbeData::set_dynamic_range(const int64_t dynamic_range) {
	___godot_icall_void_int(___mb.mb_set_dynamic_range, (const Object *) this, dynamic_range);
}

void GIProbeData::set_energy(const real_t energy) {
	___godot_icall_void_float(___mb.mb_set_energy, (const Object *) this, energy);
}

void GIProbeData::set_interior(const bool interior) {
	___godot_icall_void_bool(___mb.mb_set_interior, (const Object *) this, interior);
}

void GIProbeData::set_normal_bias(const real_t bias) {
	___godot_icall_void_float(___mb.mb_set_normal_bias, (const Object *) this, bias);
}

void GIProbeData::set_propagation(const real_t propagation) {
	___godot_icall_void_float(___mb.mb_set_propagation, (const Object *) this, propagation);
}

void GIProbeData::set_to_cell_xform(const Transform to_cell_xform) {
	___godot_icall_void_Transform(___mb.mb_set_to_cell_xform, (const Object *) this, to_cell_xform);
}

}