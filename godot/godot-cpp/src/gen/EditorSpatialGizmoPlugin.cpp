#include "EditorSpatialGizmoPlugin.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "SpatialMaterial.hpp"
#include "EditorSpatialGizmo.hpp"
#include "Spatial.hpp"
#include "Texture.hpp"
#include "Camera.hpp"


namespace godot {


EditorSpatialGizmoPlugin::___method_bindings EditorSpatialGizmoPlugin::___mb = {};

void EditorSpatialGizmoPlugin::___init_method_bindings() {
	___mb.mb_add_material = godot::api->godot_method_bind_get_method("EditorSpatialGizmoPlugin", "add_material");
	___mb.mb_can_be_hidden = godot::api->godot_method_bind_get_method("EditorSpatialGizmoPlugin", "can_be_hidden");
	___mb.mb_commit_handle = godot::api->godot_method_bind_get_method("EditorSpatialGizmoPlugin", "commit_handle");
	___mb.mb_create_gizmo = godot::api->godot_method_bind_get_method("EditorSpatialGizmoPlugin", "create_gizmo");
	___mb.mb_create_handle_material = godot::api->godot_method_bind_get_method("EditorSpatialGizmoPlugin", "create_handle_material");
	___mb.mb_create_icon_material = godot::api->godot_method_bind_get_method("EditorSpatialGizmoPlugin", "create_icon_material");
	___mb.mb_create_material = godot::api->godot_method_bind_get_method("EditorSpatialGizmoPlugin", "create_material");
	___mb.mb_get_handle_name = godot::api->godot_method_bind_get_method("EditorSpatialGizmoPlugin", "get_handle_name");
	___mb.mb_get_handle_value = godot::api->godot_method_bind_get_method("EditorSpatialGizmoPlugin", "get_handle_value");
	___mb.mb_get_material = godot::api->godot_method_bind_get_method("EditorSpatialGizmoPlugin", "get_material");
	___mb.mb_get_name = godot::api->godot_method_bind_get_method("EditorSpatialGizmoPlugin", "get_name");
	___mb.mb_get_priority = godot::api->godot_method_bind_get_method("EditorSpatialGizmoPlugin", "get_priority");
	___mb.mb_has_gizmo = godot::api->godot_method_bind_get_method("EditorSpatialGizmoPlugin", "has_gizmo");
	___mb.mb_is_handle_highlighted = godot::api->godot_method_bind_get_method("EditorSpatialGizmoPlugin", "is_handle_highlighted");
	___mb.mb_is_selectable_when_hidden = godot::api->godot_method_bind_get_method("EditorSpatialGizmoPlugin", "is_selectable_when_hidden");
	___mb.mb_redraw = godot::api->godot_method_bind_get_method("EditorSpatialGizmoPlugin", "redraw");
	___mb.mb_set_handle = godot::api->godot_method_bind_get_method("EditorSpatialGizmoPlugin", "set_handle");
}

void EditorSpatialGizmoPlugin::add_material(const String name, const Ref<SpatialMaterial> material) {
	___godot_icall_void_String_Object(___mb.mb_add_material, (const Object *) this, name, material.ptr());
}

bool EditorSpatialGizmoPlugin::can_be_hidden() {
	return ___godot_icall_bool(___mb.mb_can_be_hidden, (const Object *) this);
}

void EditorSpatialGizmoPlugin::commit_handle(const Ref<EditorSpatialGizmo> gizmo, const int64_t index, const Variant restore, const bool cancel) {
	___godot_icall_void_Object_int_Variant_bool(___mb.mb_commit_handle, (const Object *) this, gizmo.ptr(), index, restore, cancel);
}

Ref<EditorSpatialGizmo> EditorSpatialGizmoPlugin::create_gizmo(const Spatial *spatial) {
	return Ref<EditorSpatialGizmo>::__internal_constructor(___godot_icall_Object_Object(___mb.mb_create_gizmo, (const Object *) this, spatial));
}

void EditorSpatialGizmoPlugin::create_handle_material(const String name, const bool billboard) {
	___godot_icall_void_String_bool(___mb.mb_create_handle_material, (const Object *) this, name, billboard);
}

void EditorSpatialGizmoPlugin::create_icon_material(const String name, const Ref<Texture> texture, const bool on_top, const Color color) {
	___godot_icall_void_String_Object_bool_Color(___mb.mb_create_icon_material, (const Object *) this, name, texture.ptr(), on_top, color);
}

void EditorSpatialGizmoPlugin::create_material(const String name, const Color color, const bool billboard, const bool on_top, const bool use_vertex_color) {
	___godot_icall_void_String_Color_bool_bool_bool(___mb.mb_create_material, (const Object *) this, name, color, billboard, on_top, use_vertex_color);
}

String EditorSpatialGizmoPlugin::get_handle_name(const Ref<EditorSpatialGizmo> gizmo, const int64_t index) {
	return ___godot_icall_String_Object_int(___mb.mb_get_handle_name, (const Object *) this, gizmo.ptr(), index);
}

Variant EditorSpatialGizmoPlugin::get_handle_value(const Ref<EditorSpatialGizmo> gizmo, const int64_t index) {
	return ___godot_icall_Variant_Object_int(___mb.mb_get_handle_value, (const Object *) this, gizmo.ptr(), index);
}

Ref<SpatialMaterial> EditorSpatialGizmoPlugin::get_material(const String name, const Ref<EditorSpatialGizmo> gizmo) {
	return Ref<SpatialMaterial>::__internal_constructor(___godot_icall_Object_String_Object(___mb.mb_get_material, (const Object *) this, name, gizmo.ptr()));
}

String EditorSpatialGizmoPlugin::get_name() {
	return ___godot_icall_String(___mb.mb_get_name, (const Object *) this);
}

String EditorSpatialGizmoPlugin::get_priority() {
	return ___godot_icall_String(___mb.mb_get_priority, (const Object *) this);
}

bool EditorSpatialGizmoPlugin::has_gizmo(const Spatial *spatial) {
	return ___godot_icall_bool_Object(___mb.mb_has_gizmo, (const Object *) this, spatial);
}

bool EditorSpatialGizmoPlugin::is_handle_highlighted(const Ref<EditorSpatialGizmo> gizmo, const int64_t index) {
	return ___godot_icall_bool_Object_int(___mb.mb_is_handle_highlighted, (const Object *) this, gizmo.ptr(), index);
}

bool EditorSpatialGizmoPlugin::is_selectable_when_hidden() {
	return ___godot_icall_bool(___mb.mb_is_selectable_when_hidden, (const Object *) this);
}

void EditorSpatialGizmoPlugin::redraw(const Ref<EditorSpatialGizmo> gizmo) {
	___godot_icall_void_Object(___mb.mb_redraw, (const Object *) this, gizmo.ptr());
}

void EditorSpatialGizmoPlugin::set_handle(const Ref<EditorSpatialGizmo> gizmo, const int64_t index, const Camera *camera, const Vector2 point) {
	___godot_icall_void_Object_int_Object_Vector2(___mb.mb_set_handle, (const Object *) this, gizmo.ptr(), index, camera, point);
}

}