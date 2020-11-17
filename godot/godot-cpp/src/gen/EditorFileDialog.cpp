#include "EditorFileDialog.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"
#include "InputEvent.hpp"
#include "VBoxContainer.hpp"


namespace godot {


EditorFileDialog::___method_bindings EditorFileDialog::___mb = {};

void EditorFileDialog::___init_method_bindings() {
	___mb.mb__action_pressed = godot::api->godot_method_bind_get_method("EditorFileDialog", "_action_pressed");
	___mb.mb__cancel_pressed = godot::api->godot_method_bind_get_method("EditorFileDialog", "_cancel_pressed");
	___mb.mb__dir_entered = godot::api->godot_method_bind_get_method("EditorFileDialog", "_dir_entered");
	___mb.mb__favorite_move_down = godot::api->godot_method_bind_get_method("EditorFileDialog", "_favorite_move_down");
	___mb.mb__favorite_move_up = godot::api->godot_method_bind_get_method("EditorFileDialog", "_favorite_move_up");
	___mb.mb__favorite_pressed = godot::api->godot_method_bind_get_method("EditorFileDialog", "_favorite_pressed");
	___mb.mb__favorite_selected = godot::api->godot_method_bind_get_method("EditorFileDialog", "_favorite_selected");
	___mb.mb__file_entered = godot::api->godot_method_bind_get_method("EditorFileDialog", "_file_entered");
	___mb.mb__filter_selected = godot::api->godot_method_bind_get_method("EditorFileDialog", "_filter_selected");
	___mb.mb__go_back = godot::api->godot_method_bind_get_method("EditorFileDialog", "_go_back");
	___mb.mb__go_forward = godot::api->godot_method_bind_get_method("EditorFileDialog", "_go_forward");
	___mb.mb__go_up = godot::api->godot_method_bind_get_method("EditorFileDialog", "_go_up");
	___mb.mb__item_db_selected = godot::api->godot_method_bind_get_method("EditorFileDialog", "_item_db_selected");
	___mb.mb__item_list_item_rmb_selected = godot::api->godot_method_bind_get_method("EditorFileDialog", "_item_list_item_rmb_selected");
	___mb.mb__item_list_rmb_clicked = godot::api->godot_method_bind_get_method("EditorFileDialog", "_item_list_rmb_clicked");
	___mb.mb__item_menu_id_pressed = godot::api->godot_method_bind_get_method("EditorFileDialog", "_item_menu_id_pressed");
	___mb.mb__item_selected = godot::api->godot_method_bind_get_method("EditorFileDialog", "_item_selected");
	___mb.mb__items_clear_selection = godot::api->godot_method_bind_get_method("EditorFileDialog", "_items_clear_selection");
	___mb.mb__make_dir = godot::api->godot_method_bind_get_method("EditorFileDialog", "_make_dir");
	___mb.mb__make_dir_confirm = godot::api->godot_method_bind_get_method("EditorFileDialog", "_make_dir_confirm");
	___mb.mb__multi_selected = godot::api->godot_method_bind_get_method("EditorFileDialog", "_multi_selected");
	___mb.mb__recent_selected = godot::api->godot_method_bind_get_method("EditorFileDialog", "_recent_selected");
	___mb.mb__save_confirm_pressed = godot::api->godot_method_bind_get_method("EditorFileDialog", "_save_confirm_pressed");
	___mb.mb__select_drive = godot::api->godot_method_bind_get_method("EditorFileDialog", "_select_drive");
	___mb.mb__thumbnail_done = godot::api->godot_method_bind_get_method("EditorFileDialog", "_thumbnail_done");
	___mb.mb__thumbnail_result = godot::api->godot_method_bind_get_method("EditorFileDialog", "_thumbnail_result");
	___mb.mb__unhandled_input = godot::api->godot_method_bind_get_method("EditorFileDialog", "_unhandled_input");
	___mb.mb__update_dir = godot::api->godot_method_bind_get_method("EditorFileDialog", "_update_dir");
	___mb.mb__update_file_list = godot::api->godot_method_bind_get_method("EditorFileDialog", "_update_file_list");
	___mb.mb__update_file_name = godot::api->godot_method_bind_get_method("EditorFileDialog", "_update_file_name");
	___mb.mb_add_filter = godot::api->godot_method_bind_get_method("EditorFileDialog", "add_filter");
	___mb.mb_clear_filters = godot::api->godot_method_bind_get_method("EditorFileDialog", "clear_filters");
	___mb.mb_get_access = godot::api->godot_method_bind_get_method("EditorFileDialog", "get_access");
	___mb.mb_get_current_dir = godot::api->godot_method_bind_get_method("EditorFileDialog", "get_current_dir");
	___mb.mb_get_current_file = godot::api->godot_method_bind_get_method("EditorFileDialog", "get_current_file");
	___mb.mb_get_current_path = godot::api->godot_method_bind_get_method("EditorFileDialog", "get_current_path");
	___mb.mb_get_display_mode = godot::api->godot_method_bind_get_method("EditorFileDialog", "get_display_mode");
	___mb.mb_get_mode = godot::api->godot_method_bind_get_method("EditorFileDialog", "get_mode");
	___mb.mb_get_vbox = godot::api->godot_method_bind_get_method("EditorFileDialog", "get_vbox");
	___mb.mb_invalidate = godot::api->godot_method_bind_get_method("EditorFileDialog", "invalidate");
	___mb.mb_is_overwrite_warning_disabled = godot::api->godot_method_bind_get_method("EditorFileDialog", "is_overwrite_warning_disabled");
	___mb.mb_is_showing_hidden_files = godot::api->godot_method_bind_get_method("EditorFileDialog", "is_showing_hidden_files");
	___mb.mb_set_access = godot::api->godot_method_bind_get_method("EditorFileDialog", "set_access");
	___mb.mb_set_current_dir = godot::api->godot_method_bind_get_method("EditorFileDialog", "set_current_dir");
	___mb.mb_set_current_file = godot::api->godot_method_bind_get_method("EditorFileDialog", "set_current_file");
	___mb.mb_set_current_path = godot::api->godot_method_bind_get_method("EditorFileDialog", "set_current_path");
	___mb.mb_set_disable_overwrite_warning = godot::api->godot_method_bind_get_method("EditorFileDialog", "set_disable_overwrite_warning");
	___mb.mb_set_display_mode = godot::api->godot_method_bind_get_method("EditorFileDialog", "set_display_mode");
	___mb.mb_set_mode = godot::api->godot_method_bind_get_method("EditorFileDialog", "set_mode");
	___mb.mb_set_show_hidden_files = godot::api->godot_method_bind_get_method("EditorFileDialog", "set_show_hidden_files");
}

void EditorFileDialog::_action_pressed() {
	___godot_icall_void(___mb.mb__action_pressed, (const Object *) this);
}

void EditorFileDialog::_cancel_pressed() {
	___godot_icall_void(___mb.mb__cancel_pressed, (const Object *) this);
}

void EditorFileDialog::_dir_entered(const String arg0) {
	___godot_icall_void_String(___mb.mb__dir_entered, (const Object *) this, arg0);
}

void EditorFileDialog::_favorite_move_down() {
	___godot_icall_void(___mb.mb__favorite_move_down, (const Object *) this);
}

void EditorFileDialog::_favorite_move_up() {
	___godot_icall_void(___mb.mb__favorite_move_up, (const Object *) this);
}

void EditorFileDialog::_favorite_pressed() {
	___godot_icall_void(___mb.mb__favorite_pressed, (const Object *) this);
}

void EditorFileDialog::_favorite_selected(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__favorite_selected, (const Object *) this, arg0);
}

void EditorFileDialog::_file_entered(const String arg0) {
	___godot_icall_void_String(___mb.mb__file_entered, (const Object *) this, arg0);
}

void EditorFileDialog::_filter_selected(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__filter_selected, (const Object *) this, arg0);
}

void EditorFileDialog::_go_back() {
	___godot_icall_void(___mb.mb__go_back, (const Object *) this);
}

void EditorFileDialog::_go_forward() {
	___godot_icall_void(___mb.mb__go_forward, (const Object *) this);
}

void EditorFileDialog::_go_up() {
	___godot_icall_void(___mb.mb__go_up, (const Object *) this);
}

void EditorFileDialog::_item_db_selected(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__item_db_selected, (const Object *) this, arg0);
}

void EditorFileDialog::_item_list_item_rmb_selected(const int64_t arg0, const Vector2 arg1) {
	___godot_icall_void_int_Vector2(___mb.mb__item_list_item_rmb_selected, (const Object *) this, arg0, arg1);
}

void EditorFileDialog::_item_list_rmb_clicked(const Vector2 arg0) {
	___godot_icall_void_Vector2(___mb.mb__item_list_rmb_clicked, (const Object *) this, arg0);
}

void EditorFileDialog::_item_menu_id_pressed(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__item_menu_id_pressed, (const Object *) this, arg0);
}

void EditorFileDialog::_item_selected(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__item_selected, (const Object *) this, arg0);
}

void EditorFileDialog::_items_clear_selection() {
	___godot_icall_void(___mb.mb__items_clear_selection, (const Object *) this);
}

void EditorFileDialog::_make_dir() {
	___godot_icall_void(___mb.mb__make_dir, (const Object *) this);
}

void EditorFileDialog::_make_dir_confirm() {
	___godot_icall_void(___mb.mb__make_dir_confirm, (const Object *) this);
}

void EditorFileDialog::_multi_selected(const int64_t arg0, const bool arg1) {
	___godot_icall_void_int_bool(___mb.mb__multi_selected, (const Object *) this, arg0, arg1);
}

void EditorFileDialog::_recent_selected(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__recent_selected, (const Object *) this, arg0);
}

void EditorFileDialog::_save_confirm_pressed() {
	___godot_icall_void(___mb.mb__save_confirm_pressed, (const Object *) this);
}

void EditorFileDialog::_select_drive(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__select_drive, (const Object *) this, arg0);
}

void EditorFileDialog::_thumbnail_done(const String arg0, const Ref<Texture> arg1, const Ref<Texture> arg2, const Variant arg3) {
	___godot_icall_void_String_Object_Object_Variant(___mb.mb__thumbnail_done, (const Object *) this, arg0, arg1.ptr(), arg2.ptr(), arg3);
}

void EditorFileDialog::_thumbnail_result(const String arg0, const Ref<Texture> arg1, const Ref<Texture> arg2, const Variant arg3) {
	___godot_icall_void_String_Object_Object_Variant(___mb.mb__thumbnail_result, (const Object *) this, arg0, arg1.ptr(), arg2.ptr(), arg3);
}

void EditorFileDialog::_unhandled_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__unhandled_input, (const Object *) this, arg0.ptr());
}

void EditorFileDialog::_update_dir() {
	___godot_icall_void(___mb.mb__update_dir, (const Object *) this);
}

void EditorFileDialog::_update_file_list() {
	___godot_icall_void(___mb.mb__update_file_list, (const Object *) this);
}

void EditorFileDialog::_update_file_name() {
	___godot_icall_void(___mb.mb__update_file_name, (const Object *) this);
}

void EditorFileDialog::add_filter(const String filter) {
	___godot_icall_void_String(___mb.mb_add_filter, (const Object *) this, filter);
}

void EditorFileDialog::clear_filters() {
	___godot_icall_void(___mb.mb_clear_filters, (const Object *) this);
}

EditorFileDialog::Access EditorFileDialog::get_access() const {
	return (EditorFileDialog::Access) ___godot_icall_int(___mb.mb_get_access, (const Object *) this);
}

String EditorFileDialog::get_current_dir() const {
	return ___godot_icall_String(___mb.mb_get_current_dir, (const Object *) this);
}

String EditorFileDialog::get_current_file() const {
	return ___godot_icall_String(___mb.mb_get_current_file, (const Object *) this);
}

String EditorFileDialog::get_current_path() const {
	return ___godot_icall_String(___mb.mb_get_current_path, (const Object *) this);
}

EditorFileDialog::DisplayMode EditorFileDialog::get_display_mode() const {
	return (EditorFileDialog::DisplayMode) ___godot_icall_int(___mb.mb_get_display_mode, (const Object *) this);
}

EditorFileDialog::Mode EditorFileDialog::get_mode() const {
	return (EditorFileDialog::Mode) ___godot_icall_int(___mb.mb_get_mode, (const Object *) this);
}

VBoxContainer *EditorFileDialog::get_vbox() {
	return (VBoxContainer *) ___godot_icall_Object(___mb.mb_get_vbox, (const Object *) this);
}

void EditorFileDialog::invalidate() {
	___godot_icall_void(___mb.mb_invalidate, (const Object *) this);
}

bool EditorFileDialog::is_overwrite_warning_disabled() const {
	return ___godot_icall_bool(___mb.mb_is_overwrite_warning_disabled, (const Object *) this);
}

bool EditorFileDialog::is_showing_hidden_files() const {
	return ___godot_icall_bool(___mb.mb_is_showing_hidden_files, (const Object *) this);
}

void EditorFileDialog::set_access(const int64_t access) {
	___godot_icall_void_int(___mb.mb_set_access, (const Object *) this, access);
}

void EditorFileDialog::set_current_dir(const String dir) {
	___godot_icall_void_String(___mb.mb_set_current_dir, (const Object *) this, dir);
}

void EditorFileDialog::set_current_file(const String file) {
	___godot_icall_void_String(___mb.mb_set_current_file, (const Object *) this, file);
}

void EditorFileDialog::set_current_path(const String path) {
	___godot_icall_void_String(___mb.mb_set_current_path, (const Object *) this, path);
}

void EditorFileDialog::set_disable_overwrite_warning(const bool disable) {
	___godot_icall_void_bool(___mb.mb_set_disable_overwrite_warning, (const Object *) this, disable);
}

void EditorFileDialog::set_display_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_display_mode, (const Object *) this, mode);
}

void EditorFileDialog::set_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_mode, (const Object *) this, mode);
}

void EditorFileDialog::set_show_hidden_files(const bool show) {
	___godot_icall_void_bool(___mb.mb_set_show_hidden_files, (const Object *) this, show);
}

}