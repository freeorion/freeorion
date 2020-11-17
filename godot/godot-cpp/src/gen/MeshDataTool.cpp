#include "MeshDataTool.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "ArrayMesh.hpp"
#include "Material.hpp"


namespace godot {


MeshDataTool::___method_bindings MeshDataTool::___mb = {};

void MeshDataTool::___init_method_bindings() {
	___mb.mb_clear = godot::api->godot_method_bind_get_method("MeshDataTool", "clear");
	___mb.mb_commit_to_surface = godot::api->godot_method_bind_get_method("MeshDataTool", "commit_to_surface");
	___mb.mb_create_from_surface = godot::api->godot_method_bind_get_method("MeshDataTool", "create_from_surface");
	___mb.mb_get_edge_count = godot::api->godot_method_bind_get_method("MeshDataTool", "get_edge_count");
	___mb.mb_get_edge_faces = godot::api->godot_method_bind_get_method("MeshDataTool", "get_edge_faces");
	___mb.mb_get_edge_meta = godot::api->godot_method_bind_get_method("MeshDataTool", "get_edge_meta");
	___mb.mb_get_edge_vertex = godot::api->godot_method_bind_get_method("MeshDataTool", "get_edge_vertex");
	___mb.mb_get_face_count = godot::api->godot_method_bind_get_method("MeshDataTool", "get_face_count");
	___mb.mb_get_face_edge = godot::api->godot_method_bind_get_method("MeshDataTool", "get_face_edge");
	___mb.mb_get_face_meta = godot::api->godot_method_bind_get_method("MeshDataTool", "get_face_meta");
	___mb.mb_get_face_normal = godot::api->godot_method_bind_get_method("MeshDataTool", "get_face_normal");
	___mb.mb_get_face_vertex = godot::api->godot_method_bind_get_method("MeshDataTool", "get_face_vertex");
	___mb.mb_get_format = godot::api->godot_method_bind_get_method("MeshDataTool", "get_format");
	___mb.mb_get_material = godot::api->godot_method_bind_get_method("MeshDataTool", "get_material");
	___mb.mb_get_vertex = godot::api->godot_method_bind_get_method("MeshDataTool", "get_vertex");
	___mb.mb_get_vertex_bones = godot::api->godot_method_bind_get_method("MeshDataTool", "get_vertex_bones");
	___mb.mb_get_vertex_color = godot::api->godot_method_bind_get_method("MeshDataTool", "get_vertex_color");
	___mb.mb_get_vertex_count = godot::api->godot_method_bind_get_method("MeshDataTool", "get_vertex_count");
	___mb.mb_get_vertex_edges = godot::api->godot_method_bind_get_method("MeshDataTool", "get_vertex_edges");
	___mb.mb_get_vertex_faces = godot::api->godot_method_bind_get_method("MeshDataTool", "get_vertex_faces");
	___mb.mb_get_vertex_meta = godot::api->godot_method_bind_get_method("MeshDataTool", "get_vertex_meta");
	___mb.mb_get_vertex_normal = godot::api->godot_method_bind_get_method("MeshDataTool", "get_vertex_normal");
	___mb.mb_get_vertex_tangent = godot::api->godot_method_bind_get_method("MeshDataTool", "get_vertex_tangent");
	___mb.mb_get_vertex_uv = godot::api->godot_method_bind_get_method("MeshDataTool", "get_vertex_uv");
	___mb.mb_get_vertex_uv2 = godot::api->godot_method_bind_get_method("MeshDataTool", "get_vertex_uv2");
	___mb.mb_get_vertex_weights = godot::api->godot_method_bind_get_method("MeshDataTool", "get_vertex_weights");
	___mb.mb_set_edge_meta = godot::api->godot_method_bind_get_method("MeshDataTool", "set_edge_meta");
	___mb.mb_set_face_meta = godot::api->godot_method_bind_get_method("MeshDataTool", "set_face_meta");
	___mb.mb_set_material = godot::api->godot_method_bind_get_method("MeshDataTool", "set_material");
	___mb.mb_set_vertex = godot::api->godot_method_bind_get_method("MeshDataTool", "set_vertex");
	___mb.mb_set_vertex_bones = godot::api->godot_method_bind_get_method("MeshDataTool", "set_vertex_bones");
	___mb.mb_set_vertex_color = godot::api->godot_method_bind_get_method("MeshDataTool", "set_vertex_color");
	___mb.mb_set_vertex_meta = godot::api->godot_method_bind_get_method("MeshDataTool", "set_vertex_meta");
	___mb.mb_set_vertex_normal = godot::api->godot_method_bind_get_method("MeshDataTool", "set_vertex_normal");
	___mb.mb_set_vertex_tangent = godot::api->godot_method_bind_get_method("MeshDataTool", "set_vertex_tangent");
	___mb.mb_set_vertex_uv = godot::api->godot_method_bind_get_method("MeshDataTool", "set_vertex_uv");
	___mb.mb_set_vertex_uv2 = godot::api->godot_method_bind_get_method("MeshDataTool", "set_vertex_uv2");
	___mb.mb_set_vertex_weights = godot::api->godot_method_bind_get_method("MeshDataTool", "set_vertex_weights");
}

MeshDataTool *MeshDataTool::_new()
{
	return (MeshDataTool *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"MeshDataTool")());
}
void MeshDataTool::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

Error MeshDataTool::commit_to_surface(const Ref<ArrayMesh> mesh) {
	return (Error) ___godot_icall_int_Object(___mb.mb_commit_to_surface, (const Object *) this, mesh.ptr());
}

Error MeshDataTool::create_from_surface(const Ref<ArrayMesh> mesh, const int64_t surface) {
	return (Error) ___godot_icall_int_Object_int(___mb.mb_create_from_surface, (const Object *) this, mesh.ptr(), surface);
}

int64_t MeshDataTool::get_edge_count() const {
	return ___godot_icall_int(___mb.mb_get_edge_count, (const Object *) this);
}

PoolIntArray MeshDataTool::get_edge_faces(const int64_t idx) const {
	return ___godot_icall_PoolIntArray_int(___mb.mb_get_edge_faces, (const Object *) this, idx);
}

Variant MeshDataTool::get_edge_meta(const int64_t idx) const {
	return ___godot_icall_Variant_int(___mb.mb_get_edge_meta, (const Object *) this, idx);
}

int64_t MeshDataTool::get_edge_vertex(const int64_t idx, const int64_t vertex) const {
	return ___godot_icall_int_int_int(___mb.mb_get_edge_vertex, (const Object *) this, idx, vertex);
}

int64_t MeshDataTool::get_face_count() const {
	return ___godot_icall_int(___mb.mb_get_face_count, (const Object *) this);
}

int64_t MeshDataTool::get_face_edge(const int64_t idx, const int64_t edge) const {
	return ___godot_icall_int_int_int(___mb.mb_get_face_edge, (const Object *) this, idx, edge);
}

Variant MeshDataTool::get_face_meta(const int64_t idx) const {
	return ___godot_icall_Variant_int(___mb.mb_get_face_meta, (const Object *) this, idx);
}

Vector3 MeshDataTool::get_face_normal(const int64_t idx) const {
	return ___godot_icall_Vector3_int(___mb.mb_get_face_normal, (const Object *) this, idx);
}

int64_t MeshDataTool::get_face_vertex(const int64_t idx, const int64_t vertex) const {
	return ___godot_icall_int_int_int(___mb.mb_get_face_vertex, (const Object *) this, idx, vertex);
}

int64_t MeshDataTool::get_format() const {
	return ___godot_icall_int(___mb.mb_get_format, (const Object *) this);
}

Ref<Material> MeshDataTool::get_material() const {
	return Ref<Material>::__internal_constructor(___godot_icall_Object(___mb.mb_get_material, (const Object *) this));
}

Vector3 MeshDataTool::get_vertex(const int64_t idx) const {
	return ___godot_icall_Vector3_int(___mb.mb_get_vertex, (const Object *) this, idx);
}

PoolIntArray MeshDataTool::get_vertex_bones(const int64_t idx) const {
	return ___godot_icall_PoolIntArray_int(___mb.mb_get_vertex_bones, (const Object *) this, idx);
}

Color MeshDataTool::get_vertex_color(const int64_t idx) const {
	return ___godot_icall_Color_int(___mb.mb_get_vertex_color, (const Object *) this, idx);
}

int64_t MeshDataTool::get_vertex_count() const {
	return ___godot_icall_int(___mb.mb_get_vertex_count, (const Object *) this);
}

PoolIntArray MeshDataTool::get_vertex_edges(const int64_t idx) const {
	return ___godot_icall_PoolIntArray_int(___mb.mb_get_vertex_edges, (const Object *) this, idx);
}

PoolIntArray MeshDataTool::get_vertex_faces(const int64_t idx) const {
	return ___godot_icall_PoolIntArray_int(___mb.mb_get_vertex_faces, (const Object *) this, idx);
}

Variant MeshDataTool::get_vertex_meta(const int64_t idx) const {
	return ___godot_icall_Variant_int(___mb.mb_get_vertex_meta, (const Object *) this, idx);
}

Vector3 MeshDataTool::get_vertex_normal(const int64_t idx) const {
	return ___godot_icall_Vector3_int(___mb.mb_get_vertex_normal, (const Object *) this, idx);
}

Plane MeshDataTool::get_vertex_tangent(const int64_t idx) const {
	return ___godot_icall_Plane_int(___mb.mb_get_vertex_tangent, (const Object *) this, idx);
}

Vector2 MeshDataTool::get_vertex_uv(const int64_t idx) const {
	return ___godot_icall_Vector2_int(___mb.mb_get_vertex_uv, (const Object *) this, idx);
}

Vector2 MeshDataTool::get_vertex_uv2(const int64_t idx) const {
	return ___godot_icall_Vector2_int(___mb.mb_get_vertex_uv2, (const Object *) this, idx);
}

PoolRealArray MeshDataTool::get_vertex_weights(const int64_t idx) const {
	return ___godot_icall_PoolRealArray_int(___mb.mb_get_vertex_weights, (const Object *) this, idx);
}

void MeshDataTool::set_edge_meta(const int64_t idx, const Variant meta) {
	___godot_icall_void_int_Variant(___mb.mb_set_edge_meta, (const Object *) this, idx, meta);
}

void MeshDataTool::set_face_meta(const int64_t idx, const Variant meta) {
	___godot_icall_void_int_Variant(___mb.mb_set_face_meta, (const Object *) this, idx, meta);
}

void MeshDataTool::set_material(const Ref<Material> material) {
	___godot_icall_void_Object(___mb.mb_set_material, (const Object *) this, material.ptr());
}

void MeshDataTool::set_vertex(const int64_t idx, const Vector3 vertex) {
	___godot_icall_void_int_Vector3(___mb.mb_set_vertex, (const Object *) this, idx, vertex);
}

void MeshDataTool::set_vertex_bones(const int64_t idx, const PoolIntArray bones) {
	___godot_icall_void_int_PoolIntArray(___mb.mb_set_vertex_bones, (const Object *) this, idx, bones);
}

void MeshDataTool::set_vertex_color(const int64_t idx, const Color color) {
	___godot_icall_void_int_Color(___mb.mb_set_vertex_color, (const Object *) this, idx, color);
}

void MeshDataTool::set_vertex_meta(const int64_t idx, const Variant meta) {
	___godot_icall_void_int_Variant(___mb.mb_set_vertex_meta, (const Object *) this, idx, meta);
}

void MeshDataTool::set_vertex_normal(const int64_t idx, const Vector3 normal) {
	___godot_icall_void_int_Vector3(___mb.mb_set_vertex_normal, (const Object *) this, idx, normal);
}

void MeshDataTool::set_vertex_tangent(const int64_t idx, const Plane tangent) {
	___godot_icall_void_int_Plane(___mb.mb_set_vertex_tangent, (const Object *) this, idx, tangent);
}

void MeshDataTool::set_vertex_uv(const int64_t idx, const Vector2 uv) {
	___godot_icall_void_int_Vector2(___mb.mb_set_vertex_uv, (const Object *) this, idx, uv);
}

void MeshDataTool::set_vertex_uv2(const int64_t idx, const Vector2 uv2) {
	___godot_icall_void_int_Vector2(___mb.mb_set_vertex_uv2, (const Object *) this, idx, uv2);
}

void MeshDataTool::set_vertex_weights(const int64_t idx, const PoolRealArray weights) {
	___godot_icall_void_int_PoolRealArray(___mb.mb_set_vertex_weights, (const Object *) this, idx, weights);
}

}