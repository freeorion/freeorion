#include "EditorPlugin.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "ToolButton.hpp"
#include "Control.hpp"
#include "Script.hpp"
#include "Texture.hpp"
#include "EditorExportPlugin.hpp"
#include "EditorImportPlugin.hpp"
#include "EditorInspectorPlugin.hpp"
#include "EditorSceneImporter.hpp"
#include "EditorSpatialGizmoPlugin.hpp"
#include "Object.hpp"
#include "InputEvent.hpp"
#include "Camera.hpp"
#include "EditorInterface.hpp"
#include "ScriptCreateDialog.hpp"
#include "UndoRedo.hpp"
#include "ConfigFile.hpp"


namespace godot {


EditorPlugin::___method_bindings EditorPlugin::___mb = {};

void EditorPlugin::___init_method_bindings() {
	___mb.mb_add_autoload_singleton = godot::api->godot_method_bind_get_method("EditorPlugin", "add_autoload_singleton");
	___mb.mb_add_control_to_bottom_panel = godot::api->godot_method_bind_get_method("EditorPlugin", "add_control_to_bottom_panel");
	___mb.mb_add_control_to_container = godot::api->godot_method_bind_get_method("EditorPlugin", "add_control_to_container");
	___mb.mb_add_control_to_dock = godot::api->godot_method_bind_get_method("EditorPlugin", "add_control_to_dock");
	___mb.mb_add_custom_type = godot::api->godot_method_bind_get_method("EditorPlugin", "add_custom_type");
	___mb.mb_add_export_plugin = godot::api->godot_method_bind_get_method("EditorPlugin", "add_export_plugin");
	___mb.mb_add_import_plugin = godot::api->godot_method_bind_get_method("EditorPlugin", "add_import_plugin");
	___mb.mb_add_inspector_plugin = godot::api->godot_method_bind_get_method("EditorPlugin", "add_inspector_plugin");
	___mb.mb_add_scene_import_plugin = godot::api->godot_method_bind_get_method("EditorPlugin", "add_scene_import_plugin");
	___mb.mb_add_spatial_gizmo_plugin = godot::api->godot_method_bind_get_method("EditorPlugin", "add_spatial_gizmo_plugin");
	___mb.mb_add_tool_menu_item = godot::api->godot_method_bind_get_method("EditorPlugin", "add_tool_menu_item");
	___mb.mb_add_tool_submenu_item = godot::api->godot_method_bind_get_method("EditorPlugin", "add_tool_submenu_item");
	___mb.mb_apply_changes = godot::api->godot_method_bind_get_method("EditorPlugin", "apply_changes");
	___mb.mb_build = godot::api->godot_method_bind_get_method("EditorPlugin", "build");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("EditorPlugin", "clear");
	___mb.mb_disable_plugin = godot::api->godot_method_bind_get_method("EditorPlugin", "disable_plugin");
	___mb.mb_edit = godot::api->godot_method_bind_get_method("EditorPlugin", "edit");
	___mb.mb_enable_plugin = godot::api->godot_method_bind_get_method("EditorPlugin", "enable_plugin");
	___mb.mb_forward_canvas_draw_over_viewport = godot::api->godot_method_bind_get_method("EditorPlugin", "forward_canvas_draw_over_viewport");
	___mb.mb_forward_canvas_force_draw_over_viewport = godot::api->godot_method_bind_get_method("EditorPlugin", "forward_canvas_force_draw_over_viewport");
	___mb.mb_forward_canvas_gui_input = godot::api->godot_method_bind_get_method("EditorPlugin", "forward_canvas_gui_input");
	___mb.mb_forward_spatial_gui_input = godot::api->godot_method_bind_get_method("EditorPlugin", "forward_spatial_gui_input");
	___mb.mb_get_breakpoints = godot::api->godot_method_bind_get_method("EditorPlugin", "get_breakpoints");
	___mb.mb_get_editor_interface = godot::api->godot_method_bind_get_method("EditorPlugin", "get_editor_interface");
	___mb.mb_get_plugin_icon = godot::api->godot_method_bind_get_method("EditorPlugin", "get_plugin_icon");
	___mb.mb_get_plugin_name = godot::api->godot_method_bind_get_method("EditorPlugin", "get_plugin_name");
	___mb.mb_get_script_create_dialog = godot::api->godot_method_bind_get_method("EditorPlugin", "get_script_create_dialog");
	___mb.mb_get_state = godot::api->godot_method_bind_get_method("EditorPlugin", "get_state");
	___mb.mb_get_undo_redo = godot::api->godot_method_bind_get_method("EditorPlugin", "get_undo_redo");
	___mb.mb_get_window_layout = godot::api->godot_method_bind_get_method("EditorPlugin", "get_window_layout");
	___mb.mb_handles = godot::api->godot_method_bind_get_method("EditorPlugin", "handles");
	___mb.mb_has_main_screen = godot::api->godot_method_bind_get_method("EditorPlugin", "has_main_screen");
	___mb.mb_hide_bottom_panel = godot::api->godot_method_bind_get_method("EditorPlugin", "hide_bottom_panel");
	___mb.mb_make_bottom_panel_item_visible = godot::api->godot_method_bind_get_method("EditorPlugin", "make_bottom_panel_item_visible");
	___mb.mb_make_visible = godot::api->godot_method_bind_get_method("EditorPlugin", "make_visible");
	___mb.mb_queue_save_layout = godot::api->godot_method_bind_get_method("EditorPlugin", "queue_save_layout");
	___mb.mb_remove_autoload_singleton = godot::api->godot_method_bind_get_method("EditorPlugin", "remove_autoload_singleton");
	___mb.mb_remove_control_from_bottom_panel = godot::api->godot_method_bind_get_method("EditorPlugin", "remove_control_from_bottom_panel");
	___mb.mb_remove_control_from_container = godot::api->godot_method_bind_get_method("EditorPlugin", "remove_control_from_container");
	___mb.mb_remove_control_from_docks = godot::api->godot_method_bind_get_method("EditorPlugin", "remove_control_from_docks");
	___mb.mb_remove_custom_type = godot::api->godot_method_bind_get_method("EditorPlugin", "remove_custom_type");
	___mb.mb_remove_export_plugin = godot::api->godot_method_bind_get_method("EditorPlugin", "remove_export_plugin");
	___mb.mb_remove_import_plugin = godot::api->godot_method_bind_get_method("EditorPlugin", "remove_import_plugin");
	___mb.mb_remove_inspector_plugin = godot::api->godot_method_bind_get_method("EditorPlugin", "remove_inspector_plugin");
	___mb.mb_remove_scene_import_plugin = godot::api->godot_method_bind_get_method("EditorPlugin", "remove_scene_import_plugin");
	___mb.mb_remove_spatial_gizmo_plugin = godot::api->godot_method_bind_get_method("EditorPlugin", "remove_spatial_gizmo_plugin");
	___mb.mb_remove_tool_menu_item = godot::api->godot_method_bind_get_method("EditorPlugin", "remove_tool_menu_item");
	___mb.mb_save_external_data = godot::api->godot_method_bind_get_method("EditorPlugin", "save_external_data");
	___mb.mb_set_force_draw_over_forwarding_enabled = godot::api->godot_method_bind_get_method("EditorPlugin", "set_force_draw_over_forwarding_enabled");
	___mb.mb_set_input_event_forwarding_always_enabled = godot::api->godot_method_bind_get_method("EditorPlugin", "set_input_event_forwarding_always_enabled");
	___mb.mb_set_state = godot::api->godot_method_bind_get_method("EditorPlugin", "set_state");
	___mb.mb_set_window_layout = godot::api->godot_method_bind_get_method("EditorPlugin", "set_window_layout");
	___mb.mb_update_overlays = godot::api->godot_method_bind_get_method("EditorPlugin", "update_overlays");
}

void EditorPlugin::add_autoload_singleton(const String name, const String path) {
	___godot_icall_void_String_String(___mb.mb_add_autoload_singleton, (const Object *) this, name, path);
}

ToolButton *EditorPlugin::add_control_to_bottom_panel(const Control *control, const String title) {
	return (ToolButton *) ___godot_icall_Object_Object_String(___mb.mb_add_control_to_bottom_panel, (const Object *) this, control, title);
}

void EditorPlugin::add_control_to_container(const int64_t container, const Control *control) {
	___godot_icall_void_int_Object(___mb.mb_add_control_to_container, (const Object *) this, container, control);
}

void EditorPlugin::add_control_to_dock(const int64_t slot, const Control *control) {
	___godot_icall_void_int_Object(___mb.mb_add_control_to_dock, (const Object *) this, slot, control);
}

void EditorPlugin::add_custom_type(const String type, const String base, const Ref<Script> script, const Ref<Texture> icon) {
	___godot_icall_void_String_String_Object_Object(___mb.mb_add_custom_type, (const Object *) this, type, base, script.ptr(), icon.ptr());
}

void EditorPlugin::add_export_plugin(const Ref<EditorExportPlugin> plugin) {
	___godot_icall_void_Object(___mb.mb_add_export_plugin, (const Object *) this, plugin.ptr());
}

void EditorPlugin::add_import_plugin(const Ref<EditorImportPlugin> importer) {
	___godot_icall_void_Object(___mb.mb_add_import_plugin, (const Object *) this, importer.ptr());
}

void EditorPlugin::add_inspector_plugin(const Ref<EditorInspectorPlugin> plugin) {
	___godot_icall_void_Object(___mb.mb_add_inspector_plugin, (const Object *) this, plugin.ptr());
}

void EditorPlugin::add_scene_import_plugin(const Ref<EditorSceneImporter> scene_importer) {
	___godot_icall_void_Object(___mb.mb_add_scene_import_plugin, (const Object *) this, scene_importer.ptr());
}

void EditorPlugin::add_spatial_gizmo_plugin(const Ref<EditorSpatialGizmoPlugin> plugin) {
	___godot_icall_void_Object(___mb.mb_add_spatial_gizmo_plugin, (const Object *) this, plugin.ptr());
}

void EditorPlugin::add_tool_menu_item(const String name, const Object *handler, const String callback, const Variant ud) {
	___godot_icall_void_String_Object_String_Variant(___mb.mb_add_tool_menu_item, (const Object *) this, name, handler, callback, ud);
}

void EditorPlugin::add_tool_submenu_item(const String name, const Object *submenu) {
	___godot_icall_void_String_Object(___mb.mb_add_tool_submenu_item, (const Object *) this, name, submenu);
}

void EditorPlugin::apply_changes() {
	___godot_icall_void(___mb.mb_apply_changes, (const Object *) this);
}

bool EditorPlugin::build() {
	return ___godot_icall_bool(___mb.mb_build, (const Object *) this);
}

void EditorPlugin::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

void EditorPlugin::disable_plugin() {
	___godot_icall_void(___mb.mb_disable_plugin, (const Object *) this);
}

void EditorPlugin::edit(const Object *object) {
	___godot_icall_void_Object(___mb.mb_edit, (const Object *) this, object);
}

void EditorPlugin::enable_plugin() {
	___godot_icall_void(___mb.mb_enable_plugin, (const Object *) this);
}

void EditorPlugin::forward_canvas_draw_over_viewport(const Control *overlay) {
	___godot_icall_void_Object(___mb.mb_forward_canvas_draw_over_viewport, (const Object *) this, overlay);
}

void EditorPlugin::forward_canvas_force_draw_over_viewport(const Control *overlay) {
	___godot_icall_void_Object(___mb.mb_forward_canvas_force_draw_over_viewport, (const Object *) this, overlay);
}

bool EditorPlugin::forward_canvas_gui_input(const Ref<InputEvent> event) {
	return ___godot_icall_bool_Object(___mb.mb_forward_canvas_gui_input, (const Object *) this, event.ptr());
}

bool EditorPlugin::forward_spatial_gui_input(const Camera *camera, const Ref<InputEvent> event) {
	return ___godot_icall_bool_Object_Object(___mb.mb_forward_spatial_gui_input, (const Object *) this, camera, event.ptr());
}

PoolStringArray EditorPlugin::get_breakpoints() {
	return ___godot_icall_PoolStringArray(___mb.mb_get_breakpoints, (const Object *) this);
}

EditorInterface *EditorPlugin::get_editor_interface() {
	return (EditorInterface *) ___godot_icall_Object(___mb.mb_get_editor_interface, (const Object *) this);
}

Object *EditorPlugin::get_plugin_icon() {
	return (Object *) ___godot_icall_Object(___mb.mb_get_plugin_icon, (const Object *) this);
}

String EditorPlugin::get_plugin_name() {
	return ___godot_icall_String(___mb.mb_get_plugin_name, (const Object *) this);
}

ScriptCreateDialog *EditorPlugin::get_script_create_dialog() {
	return (ScriptCreateDialog *) ___godot_icall_Object(___mb.mb_get_script_create_dialog, (const Object *) this);
}

Dictionary EditorPlugin::get_state() {
	return ___godot_icall_Dictionary(___mb.mb_get_state, (const Object *) this);
}

UndoRedo *EditorPlugin::get_undo_redo() {
	return (UndoRedo *) ___godot_icall_Object(___mb.mb_get_undo_redo, (const Object *) this);
}

void EditorPlugin::get_window_layout(const Ref<ConfigFile> layout) {
	___godot_icall_void_Object(___mb.mb_get_window_layout, (const Object *) this, layout.ptr());
}

bool EditorPlugin::handles(const Object *object) {
	return ___godot_icall_bool_Object(___mb.mb_handles, (const Object *) this, object);
}

bool EditorPlugin::has_main_screen() {
	return ___godot_icall_bool(___mb.mb_has_main_screen, (const Object *) this);
}

void EditorPlugin::hide_bottom_panel() {
	___godot_icall_void(___mb.mb_hide_bottom_panel, (const Object *) this);
}

void EditorPlugin::make_bottom_panel_item_visible(const Control *item) {
	___godot_icall_void_Object(___mb.mb_make_bottom_panel_item_visible, (const Object *) this, item);
}

void EditorPlugin::make_visible(const bool visible) {
	___godot_icall_void_bool(___mb.mb_make_visible, (const Object *) this, visible);
}

void EditorPlugin::queue_save_layout() const {
	___godot_icall_void(___mb.mb_queue_save_layout, (const Object *) this);
}

void EditorPlugin::remove_autoload_singleton(const String name) {
	___godot_icall_void_String(___mb.mb_remove_autoload_singleton, (const Object *) this, name);
}

void EditorPlugin::remove_control_from_bottom_panel(const Control *control) {
	___godot_icall_void_Object(___mb.mb_remove_control_from_bottom_panel, (const Object *) this, control);
}

void EditorPlugin::remove_control_from_container(const int64_t container, const Control *control) {
	___godot_icall_void_int_Object(___mb.mb_remove_control_from_container, (const Object *) this, container, control);
}

void EditorPlugin::remove_control_from_docks(const Control *control) {
	___godot_icall_void_Object(___mb.mb_remove_control_from_docks, (const Object *) this, control);
}

void EditorPlugin::remove_custom_type(const String type) {
	___godot_icall_void_String(___mb.mb_remove_custom_type, (const Object *) this, type);
}

void EditorPlugin::remove_export_plugin(const Ref<EditorExportPlugin> plugin) {
	___godot_icall_void_Object(___mb.mb_remove_export_plugin, (const Object *) this, plugin.ptr());
}

void EditorPlugin::remove_import_plugin(const Ref<EditorImportPlugin> importer) {
	___godot_icall_void_Object(___mb.mb_remove_import_plugin, (const Object *) this, importer.ptr());
}

void EditorPlugin::remove_inspector_plugin(const Ref<EditorInspectorPlugin> plugin) {
	___godot_icall_void_Object(___mb.mb_remove_inspector_plugin, (const Object *) this, plugin.ptr());
}

void EditorPlugin::remove_scene_import_plugin(const Ref<EditorSceneImporter> scene_importer) {
	___godot_icall_void_Object(___mb.mb_remove_scene_import_plugin, (const Object *) this, scene_importer.ptr());
}

void EditorPlugin::remove_spatial_gizmo_plugin(const Ref<EditorSpatialGizmoPlugin> plugin) {
	___godot_icall_void_Object(___mb.mb_remove_spatial_gizmo_plugin, (const Object *) this, plugin.ptr());
}

void EditorPlugin::remove_tool_menu_item(const String name) {
	___godot_icall_void_String(___mb.mb_remove_tool_menu_item, (const Object *) this, name);
}

void EditorPlugin::save_external_data() {
	___godot_icall_void(___mb.mb_save_external_data, (const Object *) this);
}

void EditorPlugin::set_force_draw_over_forwarding_enabled() {
	___godot_icall_void(___mb.mb_set_force_draw_over_forwarding_enabled, (const Object *) this);
}

void EditorPlugin::set_input_event_forwarding_always_enabled() {
	___godot_icall_void(___mb.mb_set_input_event_forwarding_always_enabled, (const Object *) this);
}

void EditorPlugin::set_state(const Dictionary state) {
	___godot_icall_void_Dictionary(___mb.mb_set_state, (const Object *) this, state);
}

void EditorPlugin::set_window_layout(const Ref<ConfigFile> layout) {
	___godot_icall_void_Object(___mb.mb_set_window_layout, (const Object *) this, layout.ptr());
}

int64_t EditorPlugin::update_overlays() const {
	return ___godot_icall_int(___mb.mb_update_overlays, (const Object *) this);
}

}