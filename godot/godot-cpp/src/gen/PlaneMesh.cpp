#include "PlaneMesh.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PlaneMesh::___method_bindings PlaneMesh::___mb = {};

void PlaneMesh::___init_method_bindings() {
	___mb.mb_get_size = godot::api->godot_method_bind_get_method("PlaneMesh", "get_size");
	___mb.mb_get_subdivide_depth = godot::api->godot_method_bind_get_method("PlaneMesh", "get_subdivide_depth");
	___mb.mb_get_subdivide_width = godot::api->godot_method_bind_get_method("PlaneMesh", "get_subdivide_width");
	___mb.mb_set_size = godot::api->godot_method_bind_get_method("PlaneMesh", "set_size");
	___mb.mb_set_subdivide_depth = godot::api->godot_method_bind_get_method("PlaneMesh", "set_subdivide_depth");
	___mb.mb_set_subdivide_width = godot::api->godot_method_bind_get_method("PlaneMesh", "set_subdivide_width");
}

PlaneMesh *PlaneMesh::_new()
{
	return (PlaneMesh *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PlaneMesh")());
}
Vector2 PlaneMesh::get_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_size, (const Object *) this);
}

int64_t PlaneMesh::get_subdivide_depth() const {
	return ___godot_icall_int(___mb.mb_get_subdivide_depth, (const Object *) this);
}

int64_t PlaneMesh::get_subdivide_width() const {
	return ___godot_icall_int(___mb.mb_get_subdivide_width, (const Object *) this);
}

void PlaneMesh::set_size(const Vector2 size) {
	___godot_icall_void_Vector2(___mb.mb_set_size, (const Object *) this, size);
}

void PlaneMesh::set_subdivide_depth(const int64_t subdivide) {
	___godot_icall_void_int(___mb.mb_set_subdivide_depth, (const Object *) this, subdivide);
}

void PlaneMesh::set_subdivide_width(const int64_t subdivide) {
	___godot_icall_void_int(___mb.mb_set_subdivide_width, (const Object *) this, subdivide);
}

}