#include "CSGTorus.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Material.hpp"


namespace godot {


CSGTorus::___method_bindings CSGTorus::___mb = {};

void CSGTorus::___init_method_bindings() {
	___mb.mb_get_inner_radius = godot::api->godot_method_bind_get_method("CSGTorus", "get_inner_radius");
	___mb.mb_get_material = godot::api->godot_method_bind_get_method("CSGTorus", "get_material");
	___mb.mb_get_outer_radius = godot::api->godot_method_bind_get_method("CSGTorus", "get_outer_radius");
	___mb.mb_get_ring_sides = godot::api->godot_method_bind_get_method("CSGTorus", "get_ring_sides");
	___mb.mb_get_sides = godot::api->godot_method_bind_get_method("CSGTorus", "get_sides");
	___mb.mb_get_smooth_faces = godot::api->godot_method_bind_get_method("CSGTorus", "get_smooth_faces");
	___mb.mb_set_inner_radius = godot::api->godot_method_bind_get_method("CSGTorus", "set_inner_radius");
	___mb.mb_set_material = godot::api->godot_method_bind_get_method("CSGTorus", "set_material");
	___mb.mb_set_outer_radius = godot::api->godot_method_bind_get_method("CSGTorus", "set_outer_radius");
	___mb.mb_set_ring_sides = godot::api->godot_method_bind_get_method("CSGTorus", "set_ring_sides");
	___mb.mb_set_sides = godot::api->godot_method_bind_get_method("CSGTorus", "set_sides");
	___mb.mb_set_smooth_faces = godot::api->godot_method_bind_get_method("CSGTorus", "set_smooth_faces");
}

CSGTorus *CSGTorus::_new()
{
	return (CSGTorus *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CSGTorus")());
}
real_t CSGTorus::get_inner_radius() const {
	return ___godot_icall_float(___mb.mb_get_inner_radius, (const Object *) this);
}

Ref<Material> CSGTorus::get_material() const {
	return Ref<Material>::__internal_constructor(___godot_icall_Object(___mb.mb_get_material, (const Object *) this));
}

real_t CSGTorus::get_outer_radius() const {
	return ___godot_icall_float(___mb.mb_get_outer_radius, (const Object *) this);
}

int64_t CSGTorus::get_ring_sides() const {
	return ___godot_icall_int(___mb.mb_get_ring_sides, (const Object *) this);
}

int64_t CSGTorus::get_sides() const {
	return ___godot_icall_int(___mb.mb_get_sides, (const Object *) this);
}

bool CSGTorus::get_smooth_faces() const {
	return ___godot_icall_bool(___mb.mb_get_smooth_faces, (const Object *) this);
}

void CSGTorus::set_inner_radius(const real_t radius) {
	___godot_icall_void_float(___mb.mb_set_inner_radius, (const Object *) this, radius);
}

void CSGTorus::set_material(const Ref<Material> material) {
	___godot_icall_void_Object(___mb.mb_set_material, (const Object *) this, material.ptr());
}

void CSGTorus::set_outer_radius(const real_t radius) {
	___godot_icall_void_float(___mb.mb_set_outer_radius, (const Object *) this, radius);
}

void CSGTorus::set_ring_sides(const int64_t sides) {
	___godot_icall_void_int(___mb.mb_set_ring_sides, (const Object *) this, sides);
}

void CSGTorus::set_sides(const int64_t sides) {
	___godot_icall_void_int(___mb.mb_set_sides, (const Object *) this, sides);
}

void CSGTorus::set_smooth_faces(const bool smooth_faces) {
	___godot_icall_void_bool(___mb.mb_set_smooth_faces, (const Object *) this, smooth_faces);
}

}