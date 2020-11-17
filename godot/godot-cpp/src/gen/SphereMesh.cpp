#include "SphereMesh.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


SphereMesh::___method_bindings SphereMesh::___mb = {};

void SphereMesh::___init_method_bindings() {
	___mb.mb_get_height = godot::api->godot_method_bind_get_method("SphereMesh", "get_height");
	___mb.mb_get_is_hemisphere = godot::api->godot_method_bind_get_method("SphereMesh", "get_is_hemisphere");
	___mb.mb_get_radial_segments = godot::api->godot_method_bind_get_method("SphereMesh", "get_radial_segments");
	___mb.mb_get_radius = godot::api->godot_method_bind_get_method("SphereMesh", "get_radius");
	___mb.mb_get_rings = godot::api->godot_method_bind_get_method("SphereMesh", "get_rings");
	___mb.mb_set_height = godot::api->godot_method_bind_get_method("SphereMesh", "set_height");
	___mb.mb_set_is_hemisphere = godot::api->godot_method_bind_get_method("SphereMesh", "set_is_hemisphere");
	___mb.mb_set_radial_segments = godot::api->godot_method_bind_get_method("SphereMesh", "set_radial_segments");
	___mb.mb_set_radius = godot::api->godot_method_bind_get_method("SphereMesh", "set_radius");
	___mb.mb_set_rings = godot::api->godot_method_bind_get_method("SphereMesh", "set_rings");
}

SphereMesh *SphereMesh::_new()
{
	return (SphereMesh *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"SphereMesh")());
}
real_t SphereMesh::get_height() const {
	return ___godot_icall_float(___mb.mb_get_height, (const Object *) this);
}

bool SphereMesh::get_is_hemisphere() const {
	return ___godot_icall_bool(___mb.mb_get_is_hemisphere, (const Object *) this);
}

int64_t SphereMesh::get_radial_segments() const {
	return ___godot_icall_int(___mb.mb_get_radial_segments, (const Object *) this);
}

real_t SphereMesh::get_radius() const {
	return ___godot_icall_float(___mb.mb_get_radius, (const Object *) this);
}

int64_t SphereMesh::get_rings() const {
	return ___godot_icall_int(___mb.mb_get_rings, (const Object *) this);
}

void SphereMesh::set_height(const real_t height) {
	___godot_icall_void_float(___mb.mb_set_height, (const Object *) this, height);
}

void SphereMesh::set_is_hemisphere(const bool is_hemisphere) {
	___godot_icall_void_bool(___mb.mb_set_is_hemisphere, (const Object *) this, is_hemisphere);
}

void SphereMesh::set_radial_segments(const int64_t radial_segments) {
	___godot_icall_void_int(___mb.mb_set_radial_segments, (const Object *) this, radial_segments);
}

void SphereMesh::set_radius(const real_t radius) {
	___godot_icall_void_float(___mb.mb_set_radius, (const Object *) this, radius);
}

void SphereMesh::set_rings(const int64_t rings) {
	___godot_icall_void_int(___mb.mb_set_rings, (const Object *) this, rings);
}

}