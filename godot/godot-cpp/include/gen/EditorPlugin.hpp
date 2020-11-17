#ifndef GODOT_CPP_EDITORPLUGIN_HPP
#define GODOT_CPP_EDITORPLUGIN_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node.hpp"
namespace godot {

class ToolButton;
class Control;
class Script;
class Texture;
class EditorExportPlugin;
class EditorImportPlugin;
class EditorInspectorPlugin;
class EditorSceneImporter;
class EditorSpatialGizmoPlugin;
class Object;
class InputEvent;
class Camera;
class EditorInterface;
class ScriptCreateDialog;
class UndoRedo;
class ConfigFile;

class EditorPlugin : public Node {
	struct ___method_bindings {
		godot_method_bind *mb_add_autoload_singleton;
		godot_method_bind *mb_add_control_to_bottom_panel;
		godot_method_bind *mb_add_control_to_container;
		godot_method_bind *mb_add_control_to_dock;
		godot_method_bind *mb_add_custom_type;
		godot_method_bind *mb_add_export_plugin;
		godot_method_bind *mb_add_import_plugin;
		godot_method_bind *mb_add_inspector_plugin;
		godot_method_bind *mb_add_scene_import_plugin;
		godot_method_bind *mb_add_spatial_gizmo_plugin;
		godot_method_bind *mb_add_tool_menu_item;
		godot_method_bind *mb_add_tool_submenu_item;
		godot_method_bind *mb_apply_changes;
		godot_method_bind *mb_build;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_disable_plugin;
		godot_method_bind *mb_edit;
		godot_method_bind *mb_enable_plugin;
		godot_method_bind *mb_forward_canvas_draw_over_viewport;
		godot_method_bind *mb_forward_canvas_force_draw_over_viewport;
		godot_method_bind *mb_forward_canvas_gui_input;
		godot_method_bind *mb_forward_spatial_gui_input;
		godot_method_bind *mb_get_breakpoints;
		godot_method_bind *mb_get_editor_interface;
		godot_method_bind *mb_get_plugin_icon;
		godot_method_bind *mb_get_plugin_name;
		godot_method_bind *mb_get_script_create_dialog;
		godot_method_bind *mb_get_state;
		godot_method_bind *mb_get_undo_redo;
		godot_method_bind *mb_get_window_layout;
		godot_method_bind *mb_handles;
		godot_method_bind *mb_has_main_screen;
		godot_method_bind *mb_hide_bottom_panel;
		godot_method_bind *mb_make_bottom_panel_item_visible;
		godot_method_bind *mb_make_visible;
		godot_method_bind *mb_queue_save_layout;
		godot_method_bind *mb_remove_autoload_singleton;
		godot_method_bind *mb_remove_control_from_bottom_panel;
		godot_method_bind *mb_remove_control_from_container;
		godot_method_bind *mb_remove_control_from_docks;
		godot_method_bind *mb_remove_custom_type;
		godot_method_bind *mb_remove_export_plugin;
		godot_method_bind *mb_remove_import_plugin;
		godot_method_bind *mb_remove_inspector_plugin;
		godot_method_bind *mb_remove_scene_import_plugin;
		godot_method_bind *mb_remove_spatial_gizmo_plugin;
		godot_method_bind *mb_remove_tool_menu_item;
		godot_method_bind *mb_save_external_data;
		godot_method_bind *mb_set_force_draw_over_forwarding_enabled;
		godot_method_bind *mb_set_input_event_forwarding_always_enabled;
		godot_method_bind *mb_set_state;
		godot_method_bind *mb_set_window_layout;
		godot_method_bind *mb_update_overlays;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorPlugin"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum DockSlot {
		DOCK_SLOT_LEFT_UL = 0,
		DOCK_SLOT_LEFT_BL = 1,
		DOCK_SLOT_LEFT_UR = 2,
		DOCK_SLOT_LEFT_BR = 3,
		DOCK_SLOT_RIGHT_UL = 4,
		DOCK_SLOT_RIGHT_BL = 5,
		DOCK_SLOT_RIGHT_UR = 6,
		DOCK_SLOT_RIGHT_BR = 7,
		DOCK_SLOT_MAX = 8,
	};
	enum CustomControlContainer {
		CONTAINER_TOOLBAR = 0,
		CONTAINER_SPATIAL_EDITOR_MENU = 1,
		CONTAINER_SPATIAL_EDITOR_SIDE_LEFT = 2,
		CONTAINER_SPATIAL_EDITOR_SIDE_RIGHT = 3,
		CONTAINER_SPATIAL_EDITOR_BOTTOM = 4,
		CONTAINER_CANVAS_EDITOR_MENU = 5,
		CONTAINER_CANVAS_EDITOR_SIDE_LEFT = 6,
		CONTAINER_CANVAS_EDITOR_SIDE_RIGHT = 7,
		CONTAINER_CANVAS_EDITOR_BOTTOM = 8,
		CONTAINER_PROPERTY_EDITOR_BOTTOM = 9,
		CONTAINER_PROJECT_SETTING_TAB_LEFT = 10,
		CONTAINER_PROJECT_SETTING_TAB_RIGHT = 11,
	};

	// constants

	// methods
	void add_autoload_singleton(const String name, const String path);
	ToolButton *add_control_to_bottom_panel(const Control *control, const String title);
	void add_control_to_container(const int64_t container, const Control *control);
	void add_control_to_dock(const int64_t slot, const Control *control);
	void add_custom_type(const String type, const String base, const Ref<Script> script, const Ref<Texture> icon);
	void add_export_plugin(const Ref<EditorExportPlugin> plugin);
	void add_import_plugin(const Ref<EditorImportPlugin> importer);
	void add_inspector_plugin(const Ref<EditorInspectorPlugin> plugin);
	void add_scene_import_plugin(const Ref<EditorSceneImporter> scene_importer);
	void add_spatial_gizmo_plugin(const Ref<EditorSpatialGizmoPlugin> plugin);
	void add_tool_menu_item(const String name, const Object *handler, const String callback, const Variant ud = Variant());
	void add_tool_submenu_item(const String name, const Object *submenu);
	void apply_changes();
	bool build();
	void clear();
	void disable_plugin();
	void edit(const Object *object);
	void enable_plugin();
	void forward_canvas_draw_over_viewport(const Control *overlay);
	void forward_canvas_force_draw_over_viewport(const Control *overlay);
	bool forward_canvas_gui_input(const Ref<InputEvent> event);
	bool forward_spatial_gui_input(const Camera *camera, const Ref<InputEvent> event);
	PoolStringArray get_breakpoints();
	EditorInterface *get_editor_interface();
	Object *get_plugin_icon();
	String get_plugin_name();
	ScriptCreateDialog *get_script_create_dialog();
	Dictionary get_state();
	UndoRedo *get_undo_redo();
	void get_window_layout(const Ref<ConfigFile> layout);
	bool handles(const Object *object);
	bool has_main_screen();
	void hide_bottom_panel();
	void make_bottom_panel_item_visible(const Control *item);
	void make_visible(const bool visible);
	void queue_save_layout() const;
	void remove_autoload_singleton(const String name);
	void remove_control_from_bottom_panel(const Control *control);
	void remove_control_from_container(const int64_t container, const Control *control);
	void remove_control_from_docks(const Control *control);
	void remove_custom_type(const String type);
	void remove_export_plugin(const Ref<EditorExportPlugin> plugin);
	void remove_import_plugin(const Ref<EditorImportPlugin> importer);
	void remove_inspector_plugin(const Ref<EditorInspectorPlugin> plugin);
	void remove_scene_import_plugin(const Ref<EditorSceneImporter> scene_importer);
	void remove_spatial_gizmo_plugin(const Ref<EditorSpatialGizmoPlugin> plugin);
	void remove_tool_menu_item(const String name);
	void save_external_data();
	void set_force_draw_over_forwarding_enabled();
	void set_input_event_forwarding_always_enabled();
	void set_state(const Dictionary state);
	void set_window_layout(const Ref<ConfigFile> layout);
	int64_t update_overlays() const;

};

}

#endif