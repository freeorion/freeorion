#ifndef GODOT_CPP_SCRIPTEDITOR_HPP
#define GODOT_CPP_SCRIPTEDITOR_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "PanelContainer.hpp"
namespace godot {

class Object;
class Reference;
class Node;
class Resource;
class Script;
class InputEvent;
class Control;

class ScriptEditor : public PanelContainer {
	struct ___method_bindings {
		godot_method_bind *mb__add_callback;
		godot_method_bind *mb__autosave_scripts;
		godot_method_bind *mb__breaked;
		godot_method_bind *mb__clear_execution;
		godot_method_bind *mb__close_all_tabs;
		godot_method_bind *mb__close_current_tab;
		godot_method_bind *mb__close_discard_current_tab;
		godot_method_bind *mb__close_docs_tab;
		godot_method_bind *mb__close_other_tabs;
		godot_method_bind *mb__copy_script_path;
		godot_method_bind *mb__editor_pause;
		godot_method_bind *mb__editor_play;
		godot_method_bind *mb__editor_settings_changed;
		godot_method_bind *mb__editor_stop;
		godot_method_bind *mb__file_dialog_action;
		godot_method_bind *mb__filter_methods_text_changed;
		godot_method_bind *mb__filter_scripts_text_changed;
		godot_method_bind *mb__get_debug_tooltip;
		godot_method_bind *mb__goto_script_line;
		godot_method_bind *mb__goto_script_line2;
		godot_method_bind *mb__help_class_goto;
		godot_method_bind *mb__help_class_open;
		godot_method_bind *mb__help_overview_selected;
		godot_method_bind *mb__help_search;
		godot_method_bind *mb__history_back;
		godot_method_bind *mb__history_forward;
		godot_method_bind *mb__live_auto_reload_running_scripts;
		godot_method_bind *mb__members_overview_selected;
		godot_method_bind *mb__menu_option;
		godot_method_bind *mb__on_find_in_files_modified_files;
		godot_method_bind *mb__on_find_in_files_requested;
		godot_method_bind *mb__on_find_in_files_result_selected;
		godot_method_bind *mb__open_recent_script;
		godot_method_bind *mb__reload_scripts;
		godot_method_bind *mb__request_help;
		godot_method_bind *mb__res_saved_callback;
		godot_method_bind *mb__resave_scripts;
		godot_method_bind *mb__save_history;
		godot_method_bind *mb__script_changed;
		godot_method_bind *mb__script_created;
		godot_method_bind *mb__script_list_gui_input;
		godot_method_bind *mb__script_selected;
		godot_method_bind *mb__script_split_dragged;
		godot_method_bind *mb__set_execution;
		godot_method_bind *mb__show_debugger;
		godot_method_bind *mb__start_find_in_files;
		godot_method_bind *mb__tab_changed;
		godot_method_bind *mb__theme_option;
		godot_method_bind *mb__toggle_members_overview_alpha_sort;
		godot_method_bind *mb__tree_changed;
		godot_method_bind *mb__unhandled_input;
		godot_method_bind *mb__update_autosave_timer;
		godot_method_bind *mb__update_members_overview;
		godot_method_bind *mb__update_recent_scripts;
		godot_method_bind *mb__update_script_connections;
		godot_method_bind *mb__update_script_names;
		godot_method_bind *mb_can_drop_data_fw;
		godot_method_bind *mb_drop_data_fw;
		godot_method_bind *mb_get_current_script;
		godot_method_bind *mb_get_drag_data_fw;
		godot_method_bind *mb_get_open_scripts;
		godot_method_bind *mb_goto_line;
		godot_method_bind *mb_open_script_create_dialog;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ScriptEditor"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void _add_callback(const Object *arg0, const String arg1, const PoolStringArray arg2);
	void _autosave_scripts();
	void _breaked(const bool arg0, const bool arg1);
	void _clear_execution(const Reference *arg0);
	void _close_all_tabs();
	void _close_current_tab();
	void _close_discard_current_tab(const String arg0);
	void _close_docs_tab();
	void _close_other_tabs();
	void _copy_script_path();
	void _editor_pause();
	void _editor_play();
	void _editor_settings_changed();
	void _editor_stop();
	void _file_dialog_action(const String arg0);
	void _filter_methods_text_changed(const String arg0);
	void _filter_scripts_text_changed(const String arg0);
	String _get_debug_tooltip(const String arg0, const Node *arg1);
	void _goto_script_line(const Reference *arg0, const int64_t arg1);
	void _goto_script_line2(const int64_t arg0);
	void _help_class_goto(const String arg0);
	void _help_class_open(const String arg0);
	void _help_overview_selected(const int64_t arg0);
	void _help_search(const String arg0);
	void _history_back();
	void _history_forward();
	void _live_auto_reload_running_scripts();
	void _members_overview_selected(const int64_t arg0);
	void _menu_option(const int64_t arg0);
	void _on_find_in_files_modified_files(const PoolStringArray arg0);
	void _on_find_in_files_requested(const String arg0);
	void _on_find_in_files_result_selected(const String arg0, const int64_t arg1, const int64_t arg2, const int64_t arg3);
	void _open_recent_script(const int64_t arg0);
	void _reload_scripts();
	void _request_help(const String arg0);
	void _res_saved_callback(const Ref<Resource> arg0);
	void _resave_scripts(const String arg0);
	void _save_history();
	void _script_changed();
	void _script_created(const Ref<Script> arg0);
	void _script_list_gui_input(const Ref<InputEvent> arg0);
	void _script_selected(const int64_t arg0);
	void _script_split_dragged(const real_t arg0);
	void _set_execution(const Reference *arg0, const int64_t arg1);
	void _show_debugger(const bool arg0);
	void _start_find_in_files(const bool arg0);
	void _tab_changed(const int64_t arg0);
	void _theme_option(const int64_t arg0);
	void _toggle_members_overview_alpha_sort(const bool arg0);
	void _tree_changed();
	void _unhandled_input(const Ref<InputEvent> arg0);
	void _update_autosave_timer();
	void _update_members_overview();
	void _update_recent_scripts();
	void _update_script_connections();
	void _update_script_names();
	bool can_drop_data_fw(const Vector2 point, const Variant data, const Control *from) const;
	void drop_data_fw(const Vector2 point, const Variant data, const Control *from);
	Ref<Script> get_current_script();
	Variant get_drag_data_fw(const Vector2 point, const Control *from);
	Array get_open_scripts() const;
	void goto_line(const int64_t line_number);
	void open_script_create_dialog(const String base_name, const String base_path);

};

}

#endif