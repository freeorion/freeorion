#include "FileDialog.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "InputEvent.hpp"
#include "LineEdit.hpp"
#include "VBoxContainer.hpp"


namespace godot {


FileDialog::___method_bindings FileDialog::___mb = {};

void FileDialog::___init_method_bindings() {
	___mb.mb__action_pressed = godot::api->godot_method_bind_get_method("FileDialog", "_action_pressed");
	___mb.mb__cancel_pressed = godot::api->godot_method_bind_get_method("FileDialog", "_cancel_pressed");
	___mb.mb__dir_entered = godot::api->godot_method_bind_get_method("FileDialog", "_dir_entered");
	___mb.mb__file_entered = godot::api->godot_method_bind_get_method("FileDialog", "_file_entered");
	___mb.mb__filter_selected = godot::api->godot_method_bind_get_method("FileDialog", "_filter_selected");
	___mb.mb__go_up = godot::api->godot_method_bind_get_method("FileDialog", "_go_up");
	___mb.mb__make_dir = godot::api->godot_method_bind_get_method("FileDialog", "_make_dir");
	___mb.mb__make_dir_confirm = godot::api->godot_method_bind_get_method("FileDialog", "_make_dir_confirm");
	___mb.mb__save_confirm_pressed = godot::api->godot_method_bind_get_method("FileDialog", "_save_confirm_pressed");
	___mb.mb__select_drive = godot::api->godot_method_bind_get_method("FileDialog", "_select_drive");
	___mb.mb__tree_item_activated = godot::api->godot_method_bind_get_method("FileDialog", "_tree_item_activated");
	___mb.mb__tree_multi_selected = godot::api->godot_method_bind_get_method("FileDialog", "_tree_multi_selected");
	___mb.mb__tree_selected = godot::api->godot_method_bind_get_method("FileDialog", "_tree_selected");
	___mb.mb__unhandled_input = godot::api->godot_method_bind_get_method("FileDialog", "_unhandled_input");
	___mb.mb__update_dir = godot::api->godot_method_bind_get_method("FileDialog", "_update_dir");
	___mb.mb__update_file_list = godot::api->godot_method_bind_get_method("FileDialog", "_update_file_list");
	___mb.mb__update_file_name = godot::api->godot_method_bind_get_method("FileDialog", "_update_file_name");
	___mb.mb_add_filter = godot::api->godot_method_bind_get_method("FileDialog", "add_filter");
	___mb.mb_clear_filters = godot::api->godot_method_bind_get_method("FileDialog", "clear_filters");
	___mb.mb_deselect_items = godot::api->godot_method_bind_get_method("FileDialog", "deselect_items");
	___mb.mb_get_access = godot::api->godot_method_bind_get_method("FileDialog", "get_access");
	___mb.mb_get_current_dir = godot::api->godot_method_bind_get_method("FileDialog", "get_current_dir");
	___mb.mb_get_current_file = godot::api->godot_method_bind_get_method("FileDialog", "get_current_file");
	___mb.mb_get_current_path = godot::api->godot_method_bind_get_method("FileDialog", "get_current_path");
	___mb.mb_get_filters = godot::api->godot_method_bind_get_method("FileDialog", "get_filters");
	___mb.mb_get_line_edit = godot::api->godot_method_bind_get_method("FileDialog", "get_line_edit");
	___mb.mb_get_mode = godot::api->godot_method_bind_get_method("FileDialog", "get_mode");
	___mb.mb_get_vbox = godot::api->godot_method_bind_get_method("FileDialog", "get_vbox");
	___mb.mb_invalidate = godot::api->godot_method_bind_get_method("FileDialog", "invalidate");
	___mb.mb_is_mode_overriding_title = godot::api->godot_method_bind_get_method("FileDialog", "is_mode_overriding_title");
	___mb.mb_is_showing_hidden_files = godot::api->godot_method_bind_get_method("FileDialog", "is_showing_hidden_files");
	___mb.mb_set_access = godot::api->godot_method_bind_get_method("FileDialog", "set_access");
	___mb.mb_set_current_dir = godot::api->godot_method_bind_get_method("FileDialog", "set_current_dir");
	___mb.mb_set_current_file = godot::api->godot_method_bind_get_method("FileDialog", "set_current_file");
	___mb.mb_set_current_path = godot::api->godot_method_bind_get_method("FileDialog", "set_current_path");
	___mb.mb_set_filters = godot::api->godot_method_bind_get_method("FileDialog", "set_filters");
	___mb.mb_set_mode = godot::api->godot_method_bind_get_method("FileDialog", "set_mode");
	___mb.mb_set_mode_overrides_title = godot::api->godot_method_bind_get_method("FileDialog", "set_mode_overrides_title");
	___mb.mb_set_show_hidden_files = godot::api->godot_method_bind_get_method("FileDialog", "set_show_hidden_files");
}

FileDialog *FileDialog::_new()
{
	return (FileDialog *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"FileDialog")());
}
void FileDialog::_action_pressed() {
	___godot_icall_void(___mb.mb__action_pressed, (const Object *) this);
}

void FileDialog::_cancel_pressed() {
	___godot_icall_void(___mb.mb__cancel_pressed, (const Object *) this);
}

void FileDialog::_dir_entered(const String arg0) {
	___godot_icall_void_String(___mb.mb__dir_entered, (const Object *) this, arg0);
}

void FileDialog::_file_entered(const String arg0) {
	___godot_icall_void_String(___mb.mb__file_entered, (const Object *) this, arg0);
}

void FileDialog::_filter_selected(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__filter_selected, (const Object *) this, arg0);
}

void FileDialog::_go_up() {
	___godot_icall_void(___mb.mb__go_up, (const Object *) this);
}

void FileDialog::_make_dir() {
	___godot_icall_void(___mb.mb__make_dir, (const Object *) this);
}

void FileDialog::_make_dir_confirm() {
	___godot_icall_void(___mb.mb__make_dir_confirm, (const Object *) this);
}

void FileDialog::_save_confirm_pressed() {
	___godot_icall_void(___mb.mb__save_confirm_pressed, (const Object *) this);
}

void FileDialog::_select_drive(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__select_drive, (const Object *) this, arg0);
}

void FileDialog::_tree_item_activated() {
	___godot_icall_void(___mb.mb__tree_item_activated, (const Object *) this);
}

void FileDialog::_tree_multi_selected(const Object *arg0, const int64_t arg1, const bool arg2) {
	___godot_icall_void_Object_int_bool(___mb.mb__tree_multi_selected, (const Object *) this, arg0, arg1, arg2);
}

void FileDialog::_tree_selected() {
	___godot_icall_void(___mb.mb__tree_selected, (const Object *) this);
}

void FileDialog::_unhandled_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__unhandled_input, (const Object *) this, arg0.ptr());
}

void FileDialog::_update_dir() {
	___godot_icall_void(___mb.mb__update_dir, (const Object *) this);
}

void FileDialog::_update_file_list() {
	___godot_icall_void(___mb.mb__update_file_list, (const Object *) this);
}

void FileDialog::_update_file_name() {
	___godot_icall_void(___mb.mb__update_file_name, (const Object *) this);
}

void FileDialog::add_filter(const String filter) {
	___godot_icall_void_String(___mb.mb_add_filter, (const Object *) this, filter);
}

void FileDialog::clear_filters() {
	___godot_icall_void(___mb.mb_clear_filters, (const Object *) this);
}

void FileDialog::deselect_items() {
	___godot_icall_void(___mb.mb_deselect_items, (const Object *) this);
}

FileDialog::Access FileDialog::get_access() const {
	return (FileDialog::Access) ___godot_icall_int(___mb.mb_get_access, (const Object *) this);
}

String FileDialog::get_current_dir() const {
	return ___godot_icall_String(___mb.mb_get_current_dir, (const Object *) this);
}

String FileDialog::get_current_file() const {
	return ___godot_icall_String(___mb.mb_get_current_file, (const Object *) this);
}

String FileDialog::get_current_path() const {
	return ___godot_icall_String(___mb.mb_get_current_path, (const Object *) this);
}

PoolStringArray FileDialog::get_filters() const {
	return ___godot_icall_PoolStringArray(___mb.mb_get_filters, (const Object *) this);
}

LineEdit *FileDialog::get_line_edit() {
	return (LineEdit *) ___godot_icall_Object(___mb.mb_get_line_edit, (const Object *) this);
}

FileDialog::Mode FileDialog::get_mode() const {
	return (FileDialog::Mode) ___godot_icall_int(___mb.mb_get_mode, (const Object *) this);
}

VBoxContainer *FileDialog::get_vbox() {
	return (VBoxContainer *) ___godot_icall_Object(___mb.mb_get_vbox, (const Object *) this);
}

void FileDialog::invalidate() {
	___godot_icall_void(___mb.mb_invalidate, (const Object *) this);
}

bool FileDialog::is_mode_overriding_title() const {
	return ___godot_icall_bool(___mb.mb_is_mode_overriding_title, (const Object *) this);
}

bool FileDialog::is_showing_hidden_files() const {
	return ___godot_icall_bool(___mb.mb_is_showing_hidden_files, (const Object *) this);
}

void FileDialog::set_access(const int64_t access) {
	___godot_icall_void_int(___mb.mb_set_access, (const Object *) this, access);
}

void FileDialog::set_current_dir(const String dir) {
	___godot_icall_void_String(___mb.mb_set_current_dir, (const Object *) this, dir);
}

void FileDialog::set_current_file(const String file) {
	___godot_icall_void_String(___mb.mb_set_current_file, (const Object *) this, file);
}

void FileDialog::set_current_path(const String path) {
	___godot_icall_void_String(___mb.mb_set_current_path, (const Object *) this, path);
}

void FileDialog::set_filters(const PoolStringArray filters) {
	___godot_icall_void_PoolStringArray(___mb.mb_set_filters, (const Object *) this, filters);
}

void FileDialog::set_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_mode, (const Object *) this, mode);
}

void FileDialog::set_mode_overrides_title(const bool override) {
	___godot_icall_void_bool(___mb.mb_set_mode_overrides_title, (const Object *) this, override);
}

void FileDialog::set_show_hidden_files(const bool show) {
	___godot_icall_void_bool(___mb.mb_set_show_hidden_files, (const Object *) this, show);
}

}