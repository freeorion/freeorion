#include "CapsuleMesh.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


CapsuleMesh::___method_bindings CapsuleMesh::___mb = {};

void CapsuleMesh::___init_method_bindings() {
	___mb.mb_get_mid_height = godot::api->godot_method_bind_get_method("CapsuleMesh", "get_mid_height");
	___mb.mb_get_radial_segments = godot::api->godot_method_bind_get_method("CapsuleMesh", "get_radial_segments");
	___mb.mb_get_radius = godot::api->godot_method_bind_get_method("CapsuleMesh", "get_radius");
	___mb.mb_get_rings = godot::api->godot_method_bind_get_method("CapsuleMesh", "get_rings");
	___mb.mb_set_mid_height = godot::api->godot_method_bind_get_method("CapsuleMesh", "set_mid_height");
	___mb.mb_set_radial_segments = godot::api->godot_method_bind_get_method("CapsuleMesh", "set_radial_segments");
	___mb.mb_set_radius = godot::api->godot_method_bind_get_method("CapsuleMesh", "set_radius");
	___mb.mb_set_rings = godot::api->godot_method_bind_get_method("CapsuleMesh", "set_rings");
}

CapsuleMesh *CapsuleMesh::_new()
{
	return (CapsuleMesh *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CapsuleMesh")());
}
real_t CapsuleMesh::get_mid_height() const {
	return ___godot_icall_float(___mb.mb_get_mid_height, (const Object *) this);
}

int64_t CapsuleMesh::get_radial_segments() const {
	return ___godot_icall_int(___mb.mb_get_radial_segments, (const Object *) this);
}

real_t CapsuleMesh::get_radius() const {
	return ___godot_icall_float(___mb.mb_get_radius, (const Object *) this);
}

int64_t CapsuleMesh::get_rings() const {
	return ___godot_icall_int(___mb.mb_get_rings, (const Object *) this);
}

void CapsuleMesh::set_mid_height(const real_t mid_height) {
	___godot_icall_void_float(___mb.mb_set_mid_height, (const Object *) this, mid_height);
}

void CapsuleMesh::set_radial_segments(const int64_t segments) {
	___godot_icall_void_int(___mb.mb_set_radial_segments, (const Object *) this, segments);
}

void CapsuleMesh::set_radius(const real_t radius) {
	___godot_icall_void_float(___mb.mb_set_radius, (const Object *) this, radius);
}

void CapsuleMesh::set_rings(const int64_t rings) {
	___godot_icall_void_int(___mb.mb_set_rings, (const Object *) this, rings);
}

}