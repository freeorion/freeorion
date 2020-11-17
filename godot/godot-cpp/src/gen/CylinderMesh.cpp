#include "CylinderMesh.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


CylinderMesh::___method_bindings CylinderMesh::___mb = {};

void CylinderMesh::___init_method_bindings() {
	___mb.mb_get_bottom_radius = godot::api->godot_method_bind_get_method("CylinderMesh", "get_bottom_radius");
	___mb.mb_get_height = godot::api->godot_method_bind_get_method("CylinderMesh", "get_height");
	___mb.mb_get_radial_segments = godot::api->godot_method_bind_get_method("CylinderMesh", "get_radial_segments");
	___mb.mb_get_rings = godot::api->godot_method_bind_get_method("CylinderMesh", "get_rings");
	___mb.mb_get_top_radius = godot::api->godot_method_bind_get_method("CylinderMesh", "get_top_radius");
	___mb.mb_set_bottom_radius = godot::api->godot_method_bind_get_method("CylinderMesh", "set_bottom_radius");
	___mb.mb_set_height = godot::api->godot_method_bind_get_method("CylinderMesh", "set_height");
	___mb.mb_set_radial_segments = godot::api->godot_method_bind_get_method("CylinderMesh", "set_radial_segments");
	___mb.mb_set_rings = godot::api->godot_method_bind_get_method("CylinderMesh", "set_rings");
	___mb.mb_set_top_radius = godot::api->godot_method_bind_get_method("CylinderMesh", "set_top_radius");
}

CylinderMesh *CylinderMesh::_new()
{
	return (CylinderMesh *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CylinderMesh")());
}
real_t CylinderMesh::get_bottom_radius() const {
	return ___godot_icall_float(___mb.mb_get_bottom_radius, (const Object *) this);
}

real_t CylinderMesh::get_height() const {
	return ___godot_icall_float(___mb.mb_get_height, (const Object *) this);
}

int64_t CylinderMesh::get_radial_segments() const {
	return ___godot_icall_int(___mb.mb_get_radial_segments, (const Object *) this);
}

int64_t CylinderMesh::get_rings() const {
	return ___godot_icall_int(___mb.mb_get_rings, (const Object *) this);
}

real_t CylinderMesh::get_top_radius() const {
	return ___godot_icall_float(___mb.mb_get_top_radius, (const Object *) this);
}

void CylinderMesh::set_bottom_radius(const real_t radius) {
	___godot_icall_void_float(___mb.mb_set_bottom_radius, (const Object *) this, radius);
}

void CylinderMesh::set_height(const real_t height) {
	___godot_icall_void_float(___mb.mb_set_height, (const Object *) this, height);
}

void CylinderMesh::set_radial_segments(const int64_t segments) {
	___godot_icall_void_int(___mb.mb_set_radial_segments, (const Object *) this, segments);
}

void CylinderMesh::set_rings(const int64_t rings) {
	___godot_icall_void_int(___mb.mb_set_rings, (const Object *) this, rings);
}

void CylinderMesh::set_top_radius(const real_t radius) {
	___godot_icall_void_float(___mb.mb_set_top_radius, (const Object *) this, radius);
}

}