#include "EditorSpatialGizmo.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "TriangleMesh.hpp"
#include "Material.hpp"
#include "ArrayMesh.hpp"
#include "SkinReference.hpp"
#include "EditorSpatialGizmoPlugin.hpp"
#include "Spatial.hpp"
#include "Camera.hpp"
#include "Node.hpp"


namespace godot {


EditorSpatialGizmo::___method_bindings EditorSpatialGizmo::___mb = {};

void EditorSpatialGizmo::___init_method_bindings() {
	___mb.mb_add_collision_segments = godot::api->godot_method_bind_get_method("EditorSpatialGizmo", "add_collision_segments");
	___mb.mb_add_collision_triangles = godot::api->godot_method_bind_get_method("EditorSpatialGizmo", "add_collision_triangles");
	___mb.mb_add_handles = godot::api->godot_method_bind_get_method("EditorSpatialGizmo", "add_handles");
	___mb.mb_add_lines = godot::api->godot_method_bind_get_method("EditorSpatialGizmo", "add_lines");
	___mb.mb_add_mesh = godot::api->godot_method_bind_get_method("EditorSpatialGizmo", "add_mesh");
	___mb.mb_add_unscaled_billboard = godot::api->godot_method_bind_get_method("EditorSpatialGizmo", "add_unscaled_billboard");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("EditorSpatialGizmo", "clear");
	___mb.mb_commit_handle = godot::api->godot_method_bind_get_method("EditorSpatialGizmo", "commit_handle");
	___mb.mb_get_handle_name = godot::api->godot_method_bind_get_method("EditorSpatialGizmo", "get_handle_name");
	___mb.mb_get_handle_value = godot::api->godot_method_bind_get_method("EditorSpatialGizmo", "get_handle_value");
	___mb.mb_get_plugin = godot::api->godot_method_bind_get_method("EditorSpatialGizmo", "get_plugin");
	___mb.mb_get_spatial_node = godot::api->godot_method_bind_get_method("EditorSpatialGizmo", "get_spatial_node");
	___mb.mb_is_handle_highlighted = godot::api->godot_method_bind_get_method("EditorSpatialGizmo", "is_handle_highlighted");
	___mb.mb_redraw = godot::api->godot_method_bind_get_method("EditorSpatialGizmo", "redraw");
	___mb.mb_set_handle = godot::api->godot_method_bind_get_method("EditorSpatialGizmo", "set_handle");
	___mb.mb_set_hidden = godot::api->godot_method_bind_get_method("EditorSpatialGizmo", "set_hidden");
	___mb.mb_set_spatial_node = godot::api->godot_method_bind_get_method("EditorSpatialGizmo", "set_spatial_node");
}

void EditorSpatialGizmo::add_collision_segments(const PoolVector3Array segments) {
	___godot_icall_void_PoolVector3Array(___mb.mb_add_collision_segments, (const Object *) this, segments);
}

void EditorSpatialGizmo::add_collision_triangles(const Ref<TriangleMesh> triangles) {
	___godot_icall_void_Object(___mb.mb_add_collision_triangles, (const Object *) this, triangles.ptr());
}

void EditorSpatialGizmo::add_handles(const PoolVector3Array handles, const Ref<Material> material, const bool billboard, const bool secondary) {
	___godot_icall_void_PoolVector3Array_Object_bool_bool(___mb.mb_add_handles, (const Object *) this, handles, material.ptr(), billboard, secondary);
}

void EditorSpatialGizmo::add_lines(const PoolVector3Array lines, const Ref<Material> material, const bool billboard, const Color modulate) {
	___godot_icall_void_PoolVector3Array_Object_bool_Color(___mb.mb_add_lines, (const Object *) this, lines, material.ptr(), billboard, modulate);
}

void EditorSpatialGizmo::add_mesh(const Ref<ArrayMesh> mesh, const bool billboard, const Ref<SkinReference> skeleton, const Ref<Material> material) {
	___godot_icall_void_Object_bool_Object_Object(___mb.mb_add_mesh, (const Object *) this, mesh.ptr(), billboard, skeleton.ptr(), material.ptr());
}

void EditorSpatialGizmo::add_unscaled_billboard(const Ref<Material> material, const real_t default_scale, const Color modulate) {
	___godot_icall_void_Object_float_Color(___mb.mb_add_unscaled_billboard, (const Object *) this, material.ptr(), default_scale, modulate);
}

void EditorSpatialGizmo::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

void EditorSpatialGizmo::commit_handle(const int64_t index, const Variant restore, const bool cancel) {
	___godot_icall_void_int_Variant_bool(___mb.mb_commit_handle, (const Object *) this, index, restore, cancel);
}

String EditorSpatialGizmo::get_handle_name(const int64_t index) {
	return ___godot_icall_String_int(___mb.mb_get_handle_name, (const Object *) this, index);
}

Variant EditorSpatialGizmo::get_handle_value(const int64_t index) {
	return ___godot_icall_Variant_int(___mb.mb_get_handle_value, (const Object *) this, index);
}

Ref<EditorSpatialGizmoPlugin> EditorSpatialGizmo::get_plugin() const {
	return Ref<EditorSpatialGizmoPlugin>::__internal_constructor(___godot_icall_Object(___mb.mb_get_plugin, (const Object *) this));
}

Spatial *EditorSpatialGizmo::get_spatial_node() const {
	return (Spatial *) ___godot_icall_Object(___mb.mb_get_spatial_node, (const Object *) this);
}

bool EditorSpatialGizmo::is_handle_highlighted(const int64_t index) {
	return ___godot_icall_bool_int(___mb.mb_is_handle_highlighted, (const Object *) this, index);
}

void EditorSpatialGizmo::redraw() {
	___godot_icall_void(___mb.mb_redraw, (const Object *) this);
}

void EditorSpatialGizmo::set_handle(const int64_t index, const Camera *camera, const Vector2 point) {
	___godot_icall_void_int_Object_Vector2(___mb.mb_set_handle, (const Object *) this, index, camera, point);
}

void EditorSpatialGizmo::set_hidden(const bool hidden) {
	___godot_icall_void_bool(___mb.mb_set_hidden, (const Object *) this, hidden);
}

void EditorSpatialGizmo::set_spatial_node(const Node *node) {
	___godot_icall_void_Object(___mb.mb_set_spatial_node, (const Object *) this, node);
}

}