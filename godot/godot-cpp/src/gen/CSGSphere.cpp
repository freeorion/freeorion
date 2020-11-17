#include "CSGSphere.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Material.hpp"


namespace godot {


CSGSphere::___method_bindings CSGSphere::___mb = {};

void CSGSphere::___init_method_bindings() {
	___mb.mb_get_material = godot::api->godot_method_bind_get_method("CSGSphere", "get_material");
	___mb.mb_get_radial_segments = godot::api->godot_method_bind_get_method("CSGSphere", "get_radial_segments");
	___mb.mb_get_radius = godot::api->godot_method_bind_get_method("CSGSphere", "get_radius");
	___mb.mb_get_rings = godot::api->godot_method_bind_get_method("CSGSphere", "get_rings");
	___mb.mb_get_smooth_faces = godot::api->godot_method_bind_get_method("CSGSphere", "get_smooth_faces");
	___mb.mb_set_material = godot::api->godot_method_bind_get_method("CSGSphere", "set_material");
	___mb.mb_set_radial_segments = godot::api->godot_method_bind_get_method("CSGSphere", "set_radial_segments");
	___mb.mb_set_radius = godot::api->godot_method_bind_get_method("CSGSphere", "set_radius");
	___mb.mb_set_rings = godot::api->godot_method_bind_get_method("CSGSphere", "set_rings");
	___mb.mb_set_smooth_faces = godot::api->godot_method_bind_get_method("CSGSphere", "set_smooth_faces");
}

CSGSphere *CSGSphere::_new()
{
	return (CSGSphere *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CSGSphere")());
}
Ref<Material> CSGSphere::get_material() const {
	return Ref<Material>::__internal_constructor(___godot_icall_Object(___mb.mb_get_material, (const Object *) this));
}

int64_t CSGSphere::get_radial_segments() const {
	return ___godot_icall_int(___mb.mb_get_radial_segments, (const Object *) this);
}

real_t CSGSphere::get_radius() const {
	return ___godot_icall_float(___mb.mb_get_radius, (const Object *) this);
}

int64_t CSGSphere::get_rings() const {
	return ___godot_icall_int(___mb.mb_get_rings, (const Object *) this);
}

bool CSGSphere::get_smooth_faces() const {
	return ___godot_icall_bool(___mb.mb_get_smooth_faces, (const Object *) this);
}

void CSGSphere::set_material(const Ref<Material> material) {
	___godot_icall_void_Object(___mb.mb_set_material, (const Object *) this, material.ptr());
}

void CSGSphere::set_radial_segments(const int64_t radial_segments) {
	___godot_icall_void_int(___mb.mb_set_radial_segments, (const Object *) this, radial_segments);
}

void CSGSphere::set_radius(const real_t radius) {
	___godot_icall_void_float(___mb.mb_set_radius, (const Object *) this, radius);
}

void CSGSphere::set_rings(const int64_t rings) {
	___godot_icall_void_int(___mb.mb_set_rings, (const Object *) this, rings);
}

void CSGSphere::set_smooth_faces(const bool smooth_faces) {
	___godot_icall_void_bool(___mb.mb_set_smooth_faces, (const Object *) this, smooth_faces);
}

}