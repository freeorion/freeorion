#include "PrismMesh.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PrismMesh::___method_bindings PrismMesh::___mb = {};

void PrismMesh::___init_method_bindings() {
	___mb.mb_get_left_to_right = godot::api->godot_method_bind_get_method("PrismMesh", "get_left_to_right");
	___mb.mb_get_size = godot::api->godot_method_bind_get_method("PrismMesh", "get_size");
	___mb.mb_get_subdivide_depth = godot::api->godot_method_bind_get_method("PrismMesh", "get_subdivide_depth");
	___mb.mb_get_subdivide_height = godot::api->godot_method_bind_get_method("PrismMesh", "get_subdivide_height");
	___mb.mb_get_subdivide_width = godot::api->godot_method_bind_get_method("PrismMesh", "get_subdivide_width");
	___mb.mb_set_left_to_right = godot::api->godot_method_bind_get_method("PrismMesh", "set_left_to_right");
	___mb.mb_set_size = godot::api->godot_method_bind_get_method("PrismMesh", "set_size");
	___mb.mb_set_subdivide_depth = godot::api->godot_method_bind_get_method("PrismMesh", "set_subdivide_depth");
	___mb.mb_set_subdivide_height = godot::api->godot_method_bind_get_method("PrismMesh", "set_subdivide_height");
	___mb.mb_set_subdivide_width = godot::api->godot_method_bind_get_method("PrismMesh", "set_subdivide_width");
}

PrismMesh *PrismMesh::_new()
{
	return (PrismMesh *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PrismMesh")());
}
real_t PrismMesh::get_left_to_right() const {
	return ___godot_icall_float(___mb.mb_get_left_to_right, (const Object *) this);
}

Vector3 PrismMesh::get_size() const {
	return ___godot_icall_Vector3(___mb.mb_get_size, (const Object *) this);
}

int64_t PrismMesh::get_subdivide_depth() const {
	return ___godot_icall_int(___mb.mb_get_subdivide_depth, (const Object *) this);
}

int64_t PrismMesh::get_subdivide_height() const {
	return ___godot_icall_int(___mb.mb_get_subdivide_height, (const Object *) this);
}

int64_t PrismMesh::get_subdivide_width() const {
	return ___godot_icall_int(___mb.mb_get_subdivide_width, (const Object *) this);
}

void PrismMesh::set_left_to_right(const real_t left_to_right) {
	___godot_icall_void_float(___mb.mb_set_left_to_right, (const Object *) this, left_to_right);
}

void PrismMesh::set_size(const Vector3 size) {
	___godot_icall_void_Vector3(___mb.mb_set_size, (const Object *) this, size);
}

void PrismMesh::set_subdivide_depth(const int64_t segments) {
	___godot_icall_void_int(___mb.mb_set_subdivide_depth, (const Object *) this, segments);
}

void PrismMesh::set_subdivide_height(const int64_t segments) {
	___godot_icall_void_int(___mb.mb_set_subdivide_height, (const Object *) this, segments);
}

void PrismMesh::set_subdivide_width(const int64_t segments) {
	___godot_icall_void_int(___mb.mb_set_subdivide_width, (const Object *) this, segments);
}

}