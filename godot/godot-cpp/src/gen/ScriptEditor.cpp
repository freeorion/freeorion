#include "ScriptEditor.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "Reference.hpp"
#include "Node.hpp"
#include "Resource.hpp"
#include "Script.hpp"
#include "InputEvent.hpp"
#include "Control.hpp"


namespace godot {


ScriptEditor::___method_bindings ScriptEditor::___mb = {};

void ScriptEditor::___init_method_bindings() {
	___mb.mb__add_callback = godot::api->godot_method_bind_get_method("ScriptEditor", "_add_callback");
	___mb.mb__autosave_scripts = godot::api->godot_method_bind_get_method("ScriptEditor", "_autosave_scripts");
	___mb.mb__breaked = godot::api->godot_method_bind_get_method("ScriptEditor", "_breaked");
	___mb.mb__clear_execution = godot::api->godot_method_bind_get_method("ScriptEditor", "_clear_execution");
	___mb.mb__close_all_tabs = godot::api->godot_method_bind_get_method("ScriptEditor", "_close_all_tabs");
	___mb.mb__close_current_tab = godot::api->godot_method_bind_get_method("ScriptEditor", "_close_current_tab");
	___mb.mb__close_discard_current_tab = godot::api->godot_method_bind_get_method("ScriptEditor", "_close_discard_current_tab");
	___mb.mb__close_docs_tab = godot::api->godot_method_bind_get_method("ScriptEditor", "_close_docs_tab");
	___mb.mb__close_other_tabs = godot::api->godot_method_bind_get_method("ScriptEditor", "_close_other_tabs");
	___mb.mb__copy_script_path = godot::api->godot_method_bind_get_method("ScriptEditor", "_copy_script_path");
	___mb.mb__editor_pause = godot::api->godot_method_bind_get_method("ScriptEditor", "_editor_pause");
	___mb.mb__editor_play = godot::api->godot_method_bind_get_method("ScriptEditor", "_editor_play");
	___mb.mb__editor_settings_changed = godot::api->godot_method_bind_get_method("ScriptEditor", "_editor_settings_changed");
	___mb.mb__editor_stop = godot::api->godot_method_bind_get_method("ScriptEditor", "_editor_stop");
	___mb.mb__file_dialog_action = godot::api->godot_method_bind_get_method("ScriptEditor", "_file_dialog_action");
	___mb.mb__filter_methods_text_changed = godot::api->godot_method_bind_get_method("ScriptEditor", "_filter_methods_text_changed");
	___mb.mb__filter_scripts_text_changed = godot::api->godot_method_bind_get_method("ScriptEditor", "_filter_scripts_text_changed");
	___mb.mb__get_debug_tooltip = godot::api->godot_method_bind_get_method("ScriptEditor", "_get_debug_tooltip");
	___mb.mb__goto_script_line = godot::api->godot_method_bind_get_method("ScriptEditor", "_goto_script_line");
	___mb.mb__goto_script_line2 = godot::api->godot_method_bind_get_method("ScriptEditor", "_goto_script_line2");
	___mb.mb__help_class_goto = godot::api->godot_method_bind_get_method("ScriptEditor", "_help_class_goto");
	___mb.mb__help_class_open = godot::api->godot_method_bind_get_method("ScriptEditor", "_help_class_open");
	___mb.mb__help_overview_selected = godot::api->godot_method_bind_get_method("ScriptEditor", "_help_overview_selected");
	___mb.mb__help_search = godot::api->godot_method_bind_get_method("ScriptEditor", "_help_search");
	___mb.mb__history_back = godot::api->godot_method_bind_get_method("ScriptEditor", "_history_back");
	___mb.mb__history_forward = godot::api->godot_method_bind_get_method("ScriptEditor", "_history_forward");
	___mb.mb__live_auto_reload_running_scripts = godot::api->godot_method_bind_get_method("ScriptEditor", "_live_auto_reload_running_scripts");
	___mb.mb__members_overview_selected = godot::api->godot_method_bind_get_method("ScriptEditor", "_members_overview_selected");
	___mb.mb__menu_option = godot::api->godot_method_bind_get_method("ScriptEditor", "_menu_option");
	___mb.mb__on_find_in_files_modified_files = godot::api->godot_method_bind_get_method("ScriptEditor", "_on_find_in_files_modified_files");
	___mb.mb__on_find_in_files_requested = godot::api->godot_method_bind_get_method("ScriptEditor", "_on_find_in_files_requested");
	___mb.mb__on_find_in_files_result_selected = godot::api->godot_method_bind_get_method("ScriptEditor", "_on_find_in_files_result_selected");
	___mb.mb__open_recent_script = godot::api->godot_method_bind_get_method("ScriptEditor", "_open_recent_script");
	___mb.mb__reload_scripts = godot::api->godot_method_bind_get_method("ScriptEditor", "_reload_scripts");
	___mb.mb__request_help = godot::api->godot_method_bind_get_method("ScriptEditor", "_request_help");
	___mb.mb__res_saved_callback = godot::api->godot_method_bind_get_method("ScriptEditor", "_res_saved_callback");
	___mb.mb__resave_scripts = godot::api->godot_method_bind_get_method("ScriptEditor", "_resave_scripts");
	___mb.mb__save_history = godot::api->godot_method_bind_get_method("ScriptEditor", "_save_history");
	___mb.mb__script_changed = godot::api->godot_method_bind_get_method("ScriptEditor", "_script_changed");
	___mb.mb__script_created = godot::api->godot_method_bind_get_method("ScriptEditor", "_script_created");
	___mb.mb__script_list_gui_input = godot::api->godot_method_bind_get_method("ScriptEditor", "_script_list_gui_input");
	___mb.mb__script_selected = godot::api->godot_method_bind_get_method("ScriptEditor", "_script_selected");
	___mb.mb__script_split_dragged = godot::api->godot_method_bind_get_method("ScriptEditor", "_script_split_dragged");
	___mb.mb__set_execution = godot::api->godot_method_bind_get_method("ScriptEditor", "_set_execution");
	___mb.mb__show_debugger = godot::api->godot_method_bind_get_method("ScriptEditor", "_show_debugger");
	___mb.mb__start_find_in_files = godot::api->godot_method_bind_get_method("ScriptEditor", "_start_find_in_files");
	___mb.mb__tab_changed = godot::api->godot_method_bind_get_method("ScriptEditor", "_tab_changed");
	___mb.mb__theme_option = godot::api->godot_method_bind_get_method("ScriptEditor", "_theme_option");
	___mb.mb__toggle_members_overview_alpha_sort = godot::api->godot_method_bind_get_method("ScriptEditor", "_toggle_members_overview_alpha_sort");
	___mb.mb__tree_changed = godot::api->godot_method_bind_get_method("ScriptEditor", "_tree_changed");
	___mb.mb__unhandled_input = godot::api->godot_method_bind_get_method("ScriptEditor", "_unhandled_input");
	___mb.mb__update_autosave_timer = godot::api->godot_method_bind_get_method("ScriptEditor", "_update_autosave_timer");
	___mb.mb__update_members_overview = godot::api->godot_method_bind_get_method("ScriptEditor", "_update_members_overview");
	___mb.mb__update_recent_scripts = godot::api->godot_method_bind_get_method("ScriptEditor", "_update_recent_scripts");
	___mb.mb__update_script_connections = godot::api->godot_method_bind_get_method("ScriptEditor", "_update_script_connections");
	___mb.mb__update_script_names = godot::api->godot_method_bind_get_method("ScriptEditor", "_update_script_names");
	___mb.mb_can_drop_data_fw = godot::api->godot_method_bind_get_method("ScriptEditor", "can_drop_data_fw");
	___mb.mb_drop_data_fw = godot::api->godot_method_bind_get_method("ScriptEditor", "drop_data_fw");
	___mb.mb_get_current_script = godot::api->godot_method_bind_get_method("ScriptEditor", "get_current_script");
	___mb.mb_get_drag_data_fw = godot::api->godot_method_bind_get_method("ScriptEditor", "get_drag_data_fw");
	___mb.mb_get_open_scripts = godot::api->godot_method_bind_get_method("ScriptEditor", "get_open_scripts");
	___mb.mb_goto_line = godot::api->godot_method_bind_get_method("ScriptEditor", "goto_line");
	___mb.mb_open_script_create_dialog = godot::api->godot_method_bind_get_method("ScriptEditor", "open_script_create_dialog");
}

void ScriptEditor::_add_callback(const Object *arg0, const String arg1, const PoolStringArray arg2) {
	___godot_icall_void_Object_String_PoolStringArray(___mb.mb__add_callback, (const Object *) this, arg0, arg1, arg2);
}

void ScriptEditor::_autosave_scripts() {
	___godot_icall_void(___mb.mb__autosave_scripts, (const Object *) this);
}

void ScriptEditor::_breaked(const bool arg0, const bool arg1) {
	___godot_icall_void_bool_bool(___mb.mb__breaked, (const Object *) this, arg0, arg1);
}

void ScriptEditor::_clear_execution(const Reference *arg0) {
	___godot_icall_void_Object(___mb.mb__clear_execution, (const Object *) this, arg0);
}

void ScriptEditor::_close_all_tabs() {
	___godot_icall_void(___mb.mb__close_all_tabs, (const Object *) this);
}

void ScriptEditor::_close_current_tab() {
	___godot_icall_void(___mb.mb__close_current_tab, (const Object *) this);
}

void ScriptEditor::_close_discard_current_tab(const String arg0) {
	___godot_icall_void_String(___mb.mb__close_discard_current_tab, (const Object *) this, arg0);
}

void ScriptEditor::_close_docs_tab() {
	___godot_icall_void(___mb.mb__close_docs_tab, (const Object *) this);
}

void ScriptEditor::_close_other_tabs() {
	___godot_icall_void(___mb.mb__close_other_tabs, (const Object *) this);
}

void ScriptEditor::_copy_script_path() {
	___godot_icall_void(___mb.mb__copy_script_path, (const Object *) this);
}

void ScriptEditor::_editor_pause() {
	___godot_icall_void(___mb.mb__editor_pause, (const Object *) this);
}

void ScriptEditor::_editor_play() {
	___godot_icall_void(___mb.mb__editor_play, (const Object *) this);
}

void ScriptEditor::_editor_settings_changed() {
	___godot_icall_void(___mb.mb__editor_settings_changed, (const Object *) this);
}

void ScriptEditor::_editor_stop() {
	___godot_icall_void(___mb.mb__editor_stop, (const Object *) this);
}

void ScriptEditor::_file_dialog_action(const String arg0) {
	___godot_icall_void_String(___mb.mb__file_dialog_action, (const Object *) this, arg0);
}

void ScriptEditor::_filter_methods_text_changed(const String arg0) {
	___godot_icall_void_String(___mb.mb__filter_methods_text_changed, (const Object *) this, arg0);
}

void ScriptEditor::_filter_scripts_text_changed(const String arg0) {
	___godot_icall_void_String(___mb.mb__filter_scripts_text_changed, (const Object *) this, arg0);
}

String ScriptEditor::_get_debug_tooltip(const String arg0, const Node *arg1) {
	return ___godot_icall_String_String_Object(___mb.mb__get_debug_tooltip, (const Object *) this, arg0, arg1);
}

void ScriptEditor::_goto_script_line(const Reference *arg0, const int64_t arg1) {
	___godot_icall_void_Object_int(___mb.mb__goto_script_line, (const Object *) this, arg0, arg1);
}

void ScriptEditor::_goto_script_line2(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__goto_script_line2, (const Object *) this, arg0);
}

void ScriptEditor::_help_class_goto(const String arg0) {
	___godot_icall_void_String(___mb.mb__help_class_goto, (const Object *) this, arg0);
}

void ScriptEditor::_help_class_open(const String arg0) {
	___godot_icall_void_String(___mb.mb__help_class_open, (const Object *) this, arg0);
}

void ScriptEditor::_help_overview_selected(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__help_overview_selected, (const Object *) this, arg0);
}

void ScriptEditor::_help_search(const String arg0) {
	___godot_icall_void_String(___mb.mb__help_search, (const Object *) this, arg0);
}

void ScriptEditor::_history_back() {
	___godot_icall_void(___mb.mb__history_back, (const Object *) this);
}

void ScriptEditor::_history_forward() {
	___godot_icall_void(___mb.mb__history_forward, (const Object *) this);
}

void ScriptEditor::_live_auto_reload_running_scripts() {
	___godot_icall_void(___mb.mb__live_auto_reload_running_scripts, (const Object *) this);
}

void ScriptEditor::_members_overview_selected(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__members_overview_selected, (const Object *) this, arg0);
}

void ScriptEditor::_menu_option(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__menu_option, (const Object *) this, arg0);
}

void ScriptEditor::_on_find_in_files_modified_files(const PoolStringArray arg0) {
	___godot_icall_void_PoolStringArray(___mb.mb__on_find_in_files_modified_files, (const Object *) this, arg0);
}

void ScriptEditor::_on_find_in_files_requested(const String arg0) {
	___godot_icall_void_String(___mb.mb__on_find_in_files_requested, (const Object *) this, arg0);
}

void ScriptEditor::_on_find_in_files_result_selected(const String arg0, const int64_t arg1, const int64_t arg2, const int64_t arg3) {
	___godot_icall_void_String_int_int_int(___mb.mb__on_find_in_files_result_selected, (const Object *) this, arg0, arg1, arg2, arg3);
}

void ScriptEditor::_open_recent_script(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__open_recent_script, (const Object *) this, arg0);
}

void ScriptEditor::_reload_scripts() {
	___godot_icall_void(___mb.mb__reload_scripts, (const Object *) this);
}

void ScriptEditor::_request_help(const String arg0) {
	___godot_icall_void_String(___mb.mb__request_help, (const Object *) this, arg0);
}

void ScriptEditor::_res_saved_callback(const Ref<Resource> arg0) {
	___godot_icall_void_Object(___mb.mb__res_saved_callback, (const Object *) this, arg0.ptr());
}

void ScriptEditor::_resave_scripts(const String arg0) {
	___godot_icall_void_String(___mb.mb__resave_scripts, (const Object *) this, arg0);
}

void ScriptEditor::_save_history() {
	___godot_icall_void(___mb.mb__save_history, (const Object *) this);
}

void ScriptEditor::_script_changed() {
	___godot_icall_void(___mb.mb__script_changed, (const Object *) this);
}

void ScriptEditor::_script_created(const Ref<Script> arg0) {
	___godot_icall_void_Object(___mb.mb__script_created, (const Object *) this, arg0.ptr());
}

void ScriptEditor::_script_list_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__script_list_gui_input, (const Object *) this, arg0.ptr());
}

void ScriptEditor::_script_selected(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__script_selected, (const Object *) this, arg0);
}

void ScriptEditor::_script_split_dragged(const real_t arg0) {
	___godot_icall_void_float(___mb.mb__script_split_dragged, (const Object *) this, arg0);
}

void ScriptEditor::_set_execution(const Reference *arg0, const int64_t arg1) {
	___godot_icall_void_Object_int(___mb.mb__set_execution, (const Object *) this, arg0, arg1);
}

void ScriptEditor::_show_debugger(const bool arg0) {
	___godot_icall_void_bool(___mb.mb__show_debugger, (const Object *) this, arg0);
}

void ScriptEditor::_start_find_in_files(const bool arg0) {
	___godot_icall_void_bool(___mb.mb__start_find_in_files, (const Object *) this, arg0);
}

void ScriptEditor::_tab_changed(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__tab_changed, (const Object *) this, arg0);
}

void ScriptEditor::_theme_option(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__theme_option, (const Object *) this, arg0);
}

void ScriptEditor::_toggle_members_overview_alpha_sort(const bool arg0) {
	___godot_icall_void_bool(___mb.mb__toggle_members_overview_alpha_sort, (const Object *) this, arg0);
}

void ScriptEditor::_tree_changed() {
	___godot_icall_void(___mb.mb__tree_changed, (const Object *) this);
}

void ScriptEditor::_unhandled_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__unhandled_input, (const Object *) this, arg0.ptr());
}

void ScriptEditor::_update_autosave_timer() {
	___godot_icall_void(___mb.mb__update_autosave_timer, (const Object *) this);
}

void ScriptEditor::_update_members_overview() {
	___godot_icall_void(___mb.mb__update_members_overview, (const Object *) this);
}

void ScriptEditor::_update_recent_scripts() {
	___godot_icall_void(___mb.mb__update_recent_scripts, (const Object *) this);
}

void ScriptEditor::_update_script_connections() {
	___godot_icall_void(___mb.mb__update_script_connections, (const Object *) this);
}

void ScriptEditor::_update_script_names() {
	___godot_icall_void(___mb.mb__update_script_names, (const Object *) this);
}

bool ScriptEditor::can_drop_data_fw(const Vector2 point, const Variant data, const Control *from) const {
	return ___godot_icall_bool_Vector2_Variant_Object(___mb.mb_can_drop_data_fw, (const Object *) this, point, data, from);
}

void ScriptEditor::drop_data_fw(const Vector2 point, const Variant data, const Control *from) {
	___godot_icall_void_Vector2_Variant_Object(___mb.mb_drop_data_fw, (const Object *) this, point, data, from);
}

Ref<Script> ScriptEditor::get_current_script() {
	return Ref<Script>::__internal_constructor(___godot_icall_Object(___mb.mb_get_current_script, (const Object *) this));
}

Variant ScriptEditor::get_drag_data_fw(const Vector2 point, const Control *from) {
	return ___godot_icall_Variant_Vector2_Object(___mb.mb_get_drag_data_fw, (const Object *) this, point, from);
}

Array ScriptEditor::get_open_scripts() const {
	return ___godot_icall_Array(___mb.mb_get_open_scripts, (const Object *) this);
}

void ScriptEditor::goto_line(const int64_t line_number) {
	___godot_icall_void_int(___mb.mb_goto_line, (const Object *) this, line_number);
}

void ScriptEditor::open_script_create_dialog(const String base_name, const String base_path) {
	___godot_icall_void_String_String(___mb.mb_open_script_create_dialog, (const Object *) this, base_name, base_path);
}

}