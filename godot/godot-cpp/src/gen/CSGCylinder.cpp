#include "CSGCylinder.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Material.hpp"


namespace godot {


CSGCylinder::___method_bindings CSGCylinder::___mb = {};

void CSGCylinder::___init_method_bindings() {
	___mb.mb_get_height = godot::api->godot_method_bind_get_method("CSGCylinder", "get_height");
	___mb.mb_get_material = godot::api->godot_method_bind_get_method("CSGCylinder", "get_material");
	___mb.mb_get_radius = godot::api->godot_method_bind_get_method("CSGCylinder", "get_radius");
	___mb.mb_get_sides = godot::api->godot_method_bind_get_method("CSGCylinder", "get_sides");
	___mb.mb_get_smooth_faces = godot::api->godot_method_bind_get_method("CSGCylinder", "get_smooth_faces");
	___mb.mb_is_cone = godot::api->godot_method_bind_get_method("CSGCylinder", "is_cone");
	___mb.mb_set_cone = godot::api->godot_method_bind_get_method("CSGCylinder", "set_cone");
	___mb.mb_set_height = godot::api->godot_method_bind_get_method("CSGCylinder", "set_height");
	___mb.mb_set_material = godot::api->godot_method_bind_get_method("CSGCylinder", "set_material");
	___mb.mb_set_radius = godot::api->godot_method_bind_get_method("CSGCylinder", "set_radius");
	___mb.mb_set_sides = godot::api->godot_method_bind_get_method("CSGCylinder", "set_sides");
	___mb.mb_set_smooth_faces = godot::api->godot_method_bind_get_method("CSGCylinder", "set_smooth_faces");
}

CSGCylinder *CSGCylinder::_new()
{
	return (CSGCylinder *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CSGCylinder")());
}
real_t CSGCylinder::get_height() const {
	return ___godot_icall_float(___mb.mb_get_height, (const Object *) this);
}

Ref<Material> CSGCylinder::get_material() const {
	return Ref<Material>::__internal_constructor(___godot_icall_Object(___mb.mb_get_material, (const Object *) this));
}

real_t CSGCylinder::get_radius() const {
	return ___godot_icall_float(___mb.mb_get_radius, (const Object *) this);
}

int64_t CSGCylinder::get_sides() const {
	return ___godot_icall_int(___mb.mb_get_sides, (const Object *) this);
}

bool CSGCylinder::get_smooth_faces() const {
	return ___godot_icall_bool(___mb.mb_get_smooth_faces, (const Object *) this);
}

bool CSGCylinder::is_cone() const {
	return ___godot_icall_bool(___mb.mb_is_cone, (const Object *) this);
}

void CSGCylinder::set_cone(const bool cone) {
	___godot_icall_void_bool(___mb.mb_set_cone, (const Object *) this, cone);
}

void CSGCylinder::set_height(const real_t height) {
	___godot_icall_void_float(___mb.mb_set_height, (const Object *) this, height);
}

void CSGCylinder::set_material(const Ref<Material> material) {
	___godot_icall_void_Object(___mb.mb_set_material, (const Object *) this, material.ptr());
}

void CSGCylinder::set_radius(const real_t radius) {
	___godot_icall_void_float(___mb.mb_set_radius, (const Object *) this, radius);
}

void CSGCylinder::set_sides(const int64_t sides) {
	___godot_icall_void_int(___mb.mb_set_sides, (const Object *) this, sides);
}

void CSGCylinder::set_smooth_faces(const bool smooth_faces) {
	___godot_icall_void_bool(___mb.mb_set_smooth_faces, (const Object *) this, smooth_faces);
}

}