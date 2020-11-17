#include "CubeMesh.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


CubeMesh::___method_bindings CubeMesh::___mb = {};

void CubeMesh::___init_method_bindings() {
	___mb.mb_get_size = godot::api->godot_method_bind_get_method("CubeMesh", "get_size");
	___mb.mb_get_subdivide_depth = godot::api->godot_method_bind_get_method("CubeMesh", "get_subdivide_depth");
	___mb.mb_get_subdivide_height = godot::api->godot_method_bind_get_method("CubeMesh", "get_subdivide_height");
	___mb.mb_get_subdivide_width = godot::api->godot_method_bind_get_method("CubeMesh", "get_subdivide_width");
	___mb.mb_set_size = godot::api->godot_method_bind_get_method("CubeMesh", "set_size");
	___mb.mb_set_subdivide_depth = godot::api->godot_method_bind_get_method("CubeMesh", "set_subdivide_depth");
	___mb.mb_set_subdivide_height = godot::api->godot_method_bind_get_method("CubeMesh", "set_subdivide_height");
	___mb.mb_set_subdivide_width = godot::api->godot_method_bind_get_method("CubeMesh", "set_subdivide_width");
}

CubeMesh *CubeMesh::_new()
{
	return (CubeMesh *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CubeMesh")());
}
Vector3 CubeMesh::get_size() const {
	return ___godot_icall_Vector3(___mb.mb_get_size, (const Object *) this);
}

int64_t CubeMesh::get_subdivide_depth() const {
	return ___godot_icall_int(___mb.mb_get_subdivide_depth, (const Object *) this);
}

int64_t CubeMesh::get_subdivide_height() const {
	return ___godot_icall_int(___mb.mb_get_subdivide_height, (const Object *) this);
}

int64_t CubeMesh::get_subdivide_width() const {
	return ___godot_icall_int(___mb.mb_get_subdivide_width, (const Object *) this);
}

void CubeMesh::set_size(const Vector3 size) {
	___godot_icall_void_Vector3(___mb.mb_set_size, (const Object *) this, size);
}

void CubeMesh::set_subdivide_depth(const int64_t divisions) {
	___godot_icall_void_int(___mb.mb_set_subdivide_depth, (const Object *) this, divisions);
}

void CubeMesh::set_subdivide_height(const int64_t divisions) {
	___godot_icall_void_int(___mb.mb_set_subdivide_height, (const Object *) this, divisions);
}

void CubeMesh::set_subdivide_width(const int64_t subdivide) {
	___godot_icall_void_int(___mb.mb_set_subdivide_width, (const Object *) this, subdivide);
}

}