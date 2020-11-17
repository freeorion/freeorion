#include "BakedLightmap.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Node.hpp"
#include "BakedLightmapData.hpp"


namespace godot {


BakedLightmap::___method_bindings BakedLightmap::___mb = {};

void BakedLightmap::___init_method_bindings() {
	___mb.mb_bake = godot::api->godot_method_bind_get_method("BakedLightmap", "bake");
	___mb.mb_debug_bake = godot::api->godot_method_bind_get_method("BakedLightmap", "debug_bake");
	___mb.mb_get_bake_cell_size = godot::api->godot_method_bind_get_method("BakedLightmap", "get_bake_cell_size");
	___mb.mb_get_bake_default_texels_per_unit = godot::api->godot_method_bind_get_method("BakedLightmap", "get_bake_default_texels_per_unit");
	___mb.mb_get_bake_mode = godot::api->godot_method_bind_get_method("BakedLightmap", "get_bake_mode");
	___mb.mb_get_bake_quality = godot::api->godot_method_bind_get_method("BakedLightmap", "get_bake_quality");
	___mb.mb_get_capture_cell_size = godot::api->godot_method_bind_get_method("BakedLightmap", "get_capture_cell_size");
	___mb.mb_get_energy = godot::api->godot_method_bind_get_method("BakedLightmap", "get_energy");
	___mb.mb_get_extents = godot::api->godot_method_bind_get_method("BakedLightmap", "get_extents");
	___mb.mb_get_image_path = godot::api->godot_method_bind_get_method("BakedLightmap", "get_image_path");
	___mb.mb_get_light_data = godot::api->godot_method_bind_get_method("BakedLightmap", "get_light_data");
	___mb.mb_get_propagation = godot::api->godot_method_bind_get_method("BakedLightmap", "get_propagation");
	___mb.mb_is_hdr = godot::api->godot_method_bind_get_method("BakedLightmap", "is_hdr");
	___mb.mb_set_bake_cell_size = godot::api->godot_method_bind_get_method("BakedLightmap", "set_bake_cell_size");
	___mb.mb_set_bake_default_texels_per_unit = godot::api->godot_method_bind_get_method("BakedLightmap", "set_bake_default_texels_per_unit");
	___mb.mb_set_bake_mode = godot::api->godot_method_bind_get_method("BakedLightmap", "set_bake_mode");
	___mb.mb_set_bake_quality = godot::api->godot_method_bind_get_method("BakedLightmap", "set_bake_quality");
	___mb.mb_set_capture_cell_size = godot::api->godot_method_bind_get_method("BakedLightmap", "set_capture_cell_size");
	___mb.mb_set_energy = godot::api->godot_method_bind_get_method("BakedLightmap", "set_energy");
	___mb.mb_set_extents = godot::api->godot_method_bind_get_method("BakedLightmap", "set_extents");
	___mb.mb_set_hdr = godot::api->godot_method_bind_get_method("BakedLightmap", "set_hdr");
	___mb.mb_set_image_path = godot::api->godot_method_bind_get_method("BakedLightmap", "set_image_path");
	___mb.mb_set_light_data = godot::api->godot_method_bind_get_method("BakedLightmap", "set_light_data");
	___mb.mb_set_propagation = godot::api->godot_method_bind_get_method("BakedLightmap", "set_propagation");
}

BakedLightmap *BakedLightmap::_new()
{
	return (BakedLightmap *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"BakedLightmap")());
}
BakedLightmap::BakeError BakedLightmap::bake(const Node *from_node, const bool create_visual_debug) {
	return (BakedLightmap::BakeError) ___godot_icall_int_Object_bool(___mb.mb_bake, (const Object *) this, from_node, create_visual_debug);
}

void BakedLightmap::debug_bake() {
	___godot_icall_void(___mb.mb_debug_bake, (const Object *) this);
}

real_t BakedLightmap::get_bake_cell_size() const {
	return ___godot_icall_float(___mb.mb_get_bake_cell_size, (const Object *) this);
}

real_t BakedLightmap::get_bake_default_texels_per_unit() const {
	return ___godot_icall_float(___mb.mb_get_bake_default_texels_per_unit, (const Object *) this);
}

BakedLightmap::BakeMode BakedLightmap::get_bake_mode() const {
	return (BakedLightmap::BakeMode) ___godot_icall_int(___mb.mb_get_bake_mode, (const Object *) this);
}

BakedLightmap::BakeQuality BakedLightmap::get_bake_quality() const {
	return (BakedLightmap::BakeQuality) ___godot_icall_int(___mb.mb_get_bake_quality, (const Object *) this);
}

real_t BakedLightmap::get_capture_cell_size() const {
	return ___godot_icall_float(___mb.mb_get_capture_cell_size, (const Object *) this);
}

real_t BakedLightmap::get_energy() const {
	return ___godot_icall_float(___mb.mb_get_energy, (const Object *) this);
}

Vector3 BakedLightmap::get_extents() const {
	return ___godot_icall_Vector3(___mb.mb_get_extents, (const Object *) this);
}

String BakedLightmap::get_image_path() const {
	return ___godot_icall_String(___mb.mb_get_image_path, (const Object *) this);
}

Ref<BakedLightmapData> BakedLightmap::get_light_data() const {
	return Ref<BakedLightmapData>::__internal_constructor(___godot_icall_Object(___mb.mb_get_light_data, (const Object *) this));
}

real_t BakedLightmap::get_propagation() const {
	return ___godot_icall_float(___mb.mb_get_propagation, (const Object *) this);
}

bool BakedLightmap::is_hdr() const {
	return ___godot_icall_bool(___mb.mb_is_hdr, (const Object *) this);
}

void BakedLightmap::set_bake_cell_size(const real_t bake_cell_size) {
	___godot_icall_void_float(___mb.mb_set_bake_cell_size, (const Object *) this, bake_cell_size);
}

void BakedLightmap::set_bake_default_texels_per_unit(const real_t texels) {
	___godot_icall_void_float(___mb.mb_set_bake_default_texels_per_unit, (const Object *) this, texels);
}

void BakedLightmap::set_bake_mode(const int64_t bake_mode) {
	___godot_icall_void_int(___mb.mb_set_bake_mode, (const Object *) this, bake_mode);
}

void BakedLightmap::set_bake_quality(const int64_t bake_quality) {
	___godot_icall_void_int(___mb.mb_set_bake_quality, (const Object *) this, bake_quality);
}

void BakedLightmap::set_capture_cell_size(const real_t capture_cell_size) {
	___godot_icall_void_float(___mb.mb_set_capture_cell_size, (const Object *) this, capture_cell_size);
}

void BakedLightmap::set_energy(const real_t energy) {
	___godot_icall_void_float(___mb.mb_set_energy, (const Object *) this, energy);
}

void BakedLightmap::set_extents(const Vector3 extents) {
	___godot_icall_void_Vector3(___mb.mb_set_extents, (const Object *) this, extents);
}

void BakedLightmap::set_hdr(const bool hdr) {
	___godot_icall_void_bool(___mb.mb_set_hdr, (const Object *) this, hdr);
}

void BakedLightmap::set_image_path(const String image_path) {
	___godot_icall_void_String(___mb.mb_set_image_path, (const Object *) this, image_path);
}

void BakedLightmap::set_light_data(const Ref<BakedLightmapData> data) {
	___godot_icall_void_Object(___mb.mb_set_light_data, (const Object *) this, data.ptr());
}

void BakedLightmap::set_propagation(const real_t propagation) {
	___godot_icall_void_float(___mb.mb_set_propagation, (const Object *) this, propagation);
}

}