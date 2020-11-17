#include "SurfaceTool.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Mesh.hpp"
#include "ArrayMesh.hpp"
#include "Material.hpp"


namespace godot {


SurfaceTool::___method_bindings SurfaceTool::___mb = {};

void SurfaceTool::___init_method_bindings() {
	___mb.mb_add_bones = godot::api->godot_method_bind_get_method("SurfaceTool", "add_bones");
	___mb.mb_add_color = godot::api->godot_method_bind_get_method("SurfaceTool", "add_color");
	___mb.mb_add_index = godot::api->godot_method_bind_get_method("SurfaceTool", "add_index");
	___mb.mb_add_normal = godot::api->godot_method_bind_get_method("SurfaceTool", "add_normal");
	___mb.mb_add_smooth_group = godot::api->godot_method_bind_get_method("SurfaceTool", "add_smooth_group");
	___mb.mb_add_tangent = godot::api->godot_method_bind_get_method("SurfaceTool", "add_tangent");
	___mb.mb_add_triangle_fan = godot::api->godot_method_bind_get_method("SurfaceTool", "add_triangle_fan");
	___mb.mb_add_uv = godot::api->godot_method_bind_get_method("SurfaceTool", "add_uv");
	___mb.mb_add_uv2 = godot::api->godot_method_bind_get_method("SurfaceTool", "add_uv2");
	___mb.mb_add_vertex = godot::api->godot_method_bind_get_method("SurfaceTool", "add_vertex");
	___mb.mb_add_weights = godot::api->godot_method_bind_get_method("SurfaceTool", "add_weights");
	___mb.mb_append_from = godot::api->godot_method_bind_get_method("SurfaceTool", "append_from");
	___mb.mb_begin = godot::api->godot_method_bind_get_method("SurfaceTool", "begin");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("SurfaceTool", "clear");
	___mb.mb_commit = godot::api->godot_method_bind_get_method("SurfaceTool", "commit");
	___mb.mb_commit_to_arrays = godot::api->godot_method_bind_get_method("SurfaceTool", "commit_to_arrays");
	___mb.mb_create_from = godot::api->godot_method_bind_get_method("SurfaceTool", "create_from");
	___mb.mb_create_from_blend_shape = godot::api->godot_method_bind_get_method("SurfaceTool", "create_from_blend_shape");
	___mb.mb_deindex = godot::api->godot_method_bind_get_method("SurfaceTool", "deindex");
	___mb.mb_generate_normals = godot::api->godot_method_bind_get_method("SurfaceTool", "generate_normals");
	___mb.mb_generate_tangents = godot::api->godot_method_bind_get_method("SurfaceTool", "generate_tangents");
	___mb.mb_index = godot::api->godot_method_bind_get_method("SurfaceTool", "index");
	___mb.mb_set_material = godot::api->godot_method_bind_get_method("SurfaceTool", "set_material");
}

SurfaceTool *SurfaceTool::_new()
{
	return (SurfaceTool *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"SurfaceTool")());
}
void SurfaceTool::add_bones(const PoolIntArray bones) {
	___godot_icall_void_PoolIntArray(___mb.mb_add_bones, (const Object *) this, bones);
}

void SurfaceTool::add_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_add_color, (const Object *) this, color);
}

void SurfaceTool::add_index(const int64_t index) {
	___godot_icall_void_int(___mb.mb_add_index, (const Object *) this, index);
}

void SurfaceTool::add_normal(const Vector3 normal) {
	___godot_icall_void_Vector3(___mb.mb_add_normal, (const Object *) this, normal);
}

void SurfaceTool::add_smooth_group(const bool smooth) {
	___godot_icall_void_bool(___mb.mb_add_smooth_group, (const Object *) this, smooth);
}

void SurfaceTool::add_tangent(const Plane tangent) {
	___godot_icall_void_Plane(___mb.mb_add_tangent, (const Object *) this, tangent);
}

void SurfaceTool::add_triangle_fan(const PoolVector3Array vertices, const PoolVector2Array uvs, const PoolColorArray colors, const PoolVector2Array uv2s, const PoolVector3Array normals, const Array tangents) {
	___godot_icall_void_PoolVector3Array_PoolVector2Array_PoolColorArray_PoolVector2Array_PoolVector3Array_Array(___mb.mb_add_triangle_fan, (const Object *) this, vertices, uvs, colors, uv2s, normals, tangents);
}

void SurfaceTool::add_uv(const Vector2 uv) {
	___godot_icall_void_Vector2(___mb.mb_add_uv, (const Object *) this, uv);
}

void SurfaceTool::add_uv2(const Vector2 uv2) {
	___godot_icall_void_Vector2(___mb.mb_add_uv2, (const Object *) this, uv2);
}

void SurfaceTool::add_vertex(const Vector3 vertex) {
	___godot_icall_void_Vector3(___mb.mb_add_vertex, (const Object *) this, vertex);
}

void SurfaceTool::add_weights(const PoolRealArray weights) {
	___godot_icall_void_PoolRealArray(___mb.mb_add_weights, (const Object *) this, weights);
}

void SurfaceTool::append_from(const Ref<Mesh> existing, const int64_t surface, const Transform transform) {
	___godot_icall_void_Object_int_Transform(___mb.mb_append_from, (const Object *) this, existing.ptr(), surface, transform);
}

void SurfaceTool::begin(const int64_t primitive) {
	___godot_icall_void_int(___mb.mb_begin, (const Object *) this, primitive);
}

void SurfaceTool::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

Ref<ArrayMesh> SurfaceTool::commit(const Ref<ArrayMesh> existing, const int64_t flags) {
	return Ref<ArrayMesh>::__internal_constructor(___godot_icall_Object_Object_int(___mb.mb_commit, (const Object *) this, existing.ptr(), flags));
}

Array SurfaceTool::commit_to_arrays() {
	return ___godot_icall_Array(___mb.mb_commit_to_arrays, (const Object *) this);
}

void SurfaceTool::create_from(const Ref<Mesh> existing, const int64_t surface) {
	___godot_icall_void_Object_int(___mb.mb_create_from, (const Object *) this, existing.ptr(), surface);
}

void SurfaceTool::create_from_blend_shape(const Ref<Mesh> existing, const int64_t surface, const String blend_shape) {
	___godot_icall_void_Object_int_String(___mb.mb_create_from_blend_shape, (const Object *) this, existing.ptr(), surface, blend_shape);
}

void SurfaceTool::deindex() {
	___godot_icall_void(___mb.mb_deindex, (const Object *) this);
}

void SurfaceTool::generate_normals(const bool flip) {
	___godot_icall_void_bool(___mb.mb_generate_normals, (const Object *) this, flip);
}

void SurfaceTool::generate_tangents() {
	___godot_icall_void(___mb.mb_generate_tangents, (const Object *) this);
}

void SurfaceTool::index() {
	___godot_icall_void(___mb.mb_index, (const Object *) this);
}

void SurfaceTool::set_material(const Ref<Material> material) {
	___godot_icall_void_Object(___mb.mb_set_material, (const Object *) this, material.ptr());
}

}