#ifndef GODOT_CPP_EDITORINTERFACE_HPP
#define GODOT_CPP_EDITORINTERFACE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node.hpp"
namespace godot {

class Resource;
class Control;
class Node;
class EditorSettings;
class EditorInspector;
class EditorFileSystem;
class EditorResourcePreview;
class ScriptEditor;
class EditorSelection;
class Object;

class EditorInterface : public Node {
	struct ___method_bindings {
		godot_method_bind *mb_edit_resource;
		godot_method_bind *mb_get_base_control;
		godot_method_bind *mb_get_current_path;
		godot_method_bind *mb_get_edited_scene_root;
		godot_method_bind *mb_get_editor_settings;
		godot_method_bind *mb_get_editor_viewport;
		godot_method_bind *mb_get_inspector;
		godot_method_bind *mb_get_open_scenes;
		godot_method_bind *mb_get_resource_filesystem;
		godot_method_bind *mb_get_resource_previewer;
		godot_method_bind *mb_get_script_editor;
		godot_method_bind *mb_get_selected_path;
		godot_method_bind *mb_get_selection;
		godot_method_bind *mb_inspect_object;
		godot_method_bind *mb_is_plugin_enabled;
		godot_method_bind *mb_make_mesh_previews;
		godot_method_bind *mb_open_scene_from_path;
		godot_method_bind *mb_reload_scene_from_path;
		godot_method_bind *mb_save_scene;
		godot_method_bind *mb_save_scene_as;
		godot_method_bind *mb_select_file;
		godot_method_bind *mb_set_distraction_free_mode;
		godot_method_bind *mb_set_main_screen_editor;
		godot_method_bind *mb_set_plugin_enabled;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorInterface"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void edit_resource(const Ref<Resource> resource);
	Control *get_base_control();
	String get_current_path() const;
	Node *get_edited_scene_root();
	Ref<EditorSettings> get_editor_settings();
	Control *get_editor_viewport();
	EditorInspector *get_inspector() const;
	Array get_open_scenes() const;
	EditorFileSystem *get_resource_filesystem();
	EditorResourcePreview *get_resource_previewer();
	ScriptEditor *get_script_editor();
	String get_selected_path() const;
	EditorSelection *get_selection();
	void inspect_object(const Object *object, const String for_property = "");
	bool is_plugin_enabled(const String plugin) const;
	Array make_mesh_previews(const Array meshes, const int64_t preview_size);
	void open_scene_from_path(const String scene_filepath);
	void reload_scene_from_path(const String scene_filepath);
	Error save_scene();
	void save_scene_as(const String path, const bool with_preview = true);
	void select_file(const String file);
	void set_distraction_free_mode(const bool enter);
	void set_main_screen_editor(const String name);
	void set_plugin_enabled(const String plugin, const bool enabled);

};

}

#endif