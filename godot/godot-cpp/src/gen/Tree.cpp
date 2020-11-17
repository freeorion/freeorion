#include "Tree.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "TreeItem.hpp"
#include "Object.hpp"


namespace godot {


Tree::___method_bindings Tree::___mb = {};

void Tree::___init_method_bindings() {
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("Tree", "_gui_input");
	___mb.mb__popup_select = godot::api->godot_method_bind_get_method("Tree", "_popup_select");
	___mb.mb__range_click_timeout = godot::api->godot_method_bind_get_method("Tree", "_range_click_timeout");
	___mb.mb__scroll_moved = godot::api->godot_method_bind_get_method("Tree", "_scroll_moved");
	___mb.mb__text_editor_enter = godot::api->godot_method_bind_get_method("Tree", "_text_editor_enter");
	___mb.mb__text_editor_modal_close = godot::api->godot_method_bind_get_method("Tree", "_text_editor_modal_close");
	___mb.mb__value_editor_changed = godot::api->godot_method_bind_get_method("Tree", "_value_editor_changed");
	___mb.mb_are_column_titles_visible = godot::api->godot_method_bind_get_method("Tree", "are_column_titles_visible");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("Tree", "clear");
	___mb.mb_create_item = godot::api->godot_method_bind_get_method("Tree", "create_item");
	___mb.mb_ensure_cursor_is_visible = godot::api->godot_method_bind_get_method("Tree", "ensure_cursor_is_visible");
	___mb.mb_get_allow_reselect = godot::api->godot_method_bind_get_method("Tree", "get_allow_reselect");
	___mb.mb_get_allow_rmb_select = godot::api->godot_method_bind_get_method("Tree", "get_allow_rmb_select");
	___mb.mb_get_column_at_position = godot::api->godot_method_bind_get_method("Tree", "get_column_at_position");
	___mb.mb_get_column_title = godot::api->godot_method_bind_get_method("Tree", "get_column_title");
	___mb.mb_get_column_width = godot::api->godot_method_bind_get_method("Tree", "get_column_width");
	___mb.mb_get_columns = godot::api->godot_method_bind_get_method("Tree", "get_columns");
	___mb.mb_get_custom_popup_rect = godot::api->godot_method_bind_get_method("Tree", "get_custom_popup_rect");
	___mb.mb_get_drop_mode_flags = godot::api->godot_method_bind_get_method("Tree", "get_drop_mode_flags");
	___mb.mb_get_drop_section_at_position = godot::api->godot_method_bind_get_method("Tree", "get_drop_section_at_position");
	___mb.mb_get_edited = godot::api->godot_method_bind_get_method("Tree", "get_edited");
	___mb.mb_get_edited_column = godot::api->godot_method_bind_get_method("Tree", "get_edited_column");
	___mb.mb_get_item_area_rect = godot::api->godot_method_bind_get_method("Tree", "get_item_area_rect");
	___mb.mb_get_item_at_position = godot::api->godot_method_bind_get_method("Tree", "get_item_at_position");
	___mb.mb_get_next_selected = godot::api->godot_method_bind_get_method("Tree", "get_next_selected");
	___mb.mb_get_pressed_button = godot::api->godot_method_bind_get_method("Tree", "get_pressed_button");
	___mb.mb_get_root = godot::api->godot_method_bind_get_method("Tree", "get_root");
	___mb.mb_get_scroll = godot::api->godot_method_bind_get_method("Tree", "get_scroll");
	___mb.mb_get_select_mode = godot::api->godot_method_bind_get_method("Tree", "get_select_mode");
	___mb.mb_get_selected = godot::api->godot_method_bind_get_method("Tree", "get_selected");
	___mb.mb_get_selected_column = godot::api->godot_method_bind_get_method("Tree", "get_selected_column");
	___mb.mb_is_folding_hidden = godot::api->godot_method_bind_get_method("Tree", "is_folding_hidden");
	___mb.mb_is_root_hidden = godot::api->godot_method_bind_get_method("Tree", "is_root_hidden");
	___mb.mb_set_allow_reselect = godot::api->godot_method_bind_get_method("Tree", "set_allow_reselect");
	___mb.mb_set_allow_rmb_select = godot::api->godot_method_bind_get_method("Tree", "set_allow_rmb_select");
	___mb.mb_set_column_expand = godot::api->godot_method_bind_get_method("Tree", "set_column_expand");
	___mb.mb_set_column_min_width = godot::api->godot_method_bind_get_method("Tree", "set_column_min_width");
	___mb.mb_set_column_title = godot::api->godot_method_bind_get_method("Tree", "set_column_title");
	___mb.mb_set_column_titles_visible = godot::api->godot_method_bind_get_method("Tree", "set_column_titles_visible");
	___mb.mb_set_columns = godot::api->godot_method_bind_get_method("Tree", "set_columns");
	___mb.mb_set_drop_mode_flags = godot::api->godot_method_bind_get_method("Tree", "set_drop_mode_flags");
	___mb.mb_set_hide_folding = godot::api->godot_method_bind_get_method("Tree", "set_hide_folding");
	___mb.mb_set_hide_root = godot::api->godot_method_bind_get_method("Tree", "set_hide_root");
	___mb.mb_set_select_mode = godot::api->godot_method_bind_get_method("Tree", "set_select_mode");
}

Tree *Tree::_new()
{
	return (Tree *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Tree")());
}
void Tree::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

void Tree::_popup_select(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__popup_select, (const Object *) this, arg0);
}

void Tree::_range_click_timeout() {
	___godot_icall_void(___mb.mb__range_click_timeout, (const Object *) this);
}

void Tree::_scroll_moved(const real_t arg0) {
	___godot_icall_void_float(___mb.mb__scroll_moved, (const Object *) this, arg0);
}

void Tree::_text_editor_enter(const String arg0) {
	___godot_icall_void_String(___mb.mb__text_editor_enter, (const Object *) this, arg0);
}

void Tree::_text_editor_modal_close() {
	___godot_icall_void(___mb.mb__text_editor_modal_close, (const Object *) this);
}

void Tree::_value_editor_changed(const real_t arg0) {
	___godot_icall_void_float(___mb.mb__value_editor_changed, (const Object *) this, arg0);
}

bool Tree::are_column_titles_visible() const {
	return ___godot_icall_bool(___mb.mb_are_column_titles_visible, (const Object *) this);
}

void Tree::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

TreeItem *Tree::create_item(const Object *parent, const int64_t idx) {
	return (TreeItem *) ___godot_icall_Object_Object_int(___mb.mb_create_item, (const Object *) this, parent, idx);
}

void Tree::ensure_cursor_is_visible() {
	___godot_icall_void(___mb.mb_ensure_cursor_is_visible, (const Object *) this);
}

bool Tree::get_allow_reselect() const {
	return ___godot_icall_bool(___mb.mb_get_allow_reselect, (const Object *) this);
}

bool Tree::get_allow_rmb_select() const {
	return ___godot_icall_bool(___mb.mb_get_allow_rmb_select, (const Object *) this);
}

int64_t Tree::get_column_at_position(const Vector2 position) const {
	return ___godot_icall_int_Vector2(___mb.mb_get_column_at_position, (const Object *) this, position);
}

String Tree::get_column_title(const int64_t column) const {
	return ___godot_icall_String_int(___mb.mb_get_column_title, (const Object *) this, column);
}

int64_t Tree::get_column_width(const int64_t column) const {
	return ___godot_icall_int_int(___mb.mb_get_column_width, (const Object *) this, column);
}

int64_t Tree::get_columns() const {
	return ___godot_icall_int(___mb.mb_get_columns, (const Object *) this);
}

Rect2 Tree::get_custom_popup_rect() const {
	return ___godot_icall_Rect2(___mb.mb_get_custom_popup_rect, (const Object *) this);
}

int64_t Tree::get_drop_mode_flags() const {
	return ___godot_icall_int(___mb.mb_get_drop_mode_flags, (const Object *) this);
}

int64_t Tree::get_drop_section_at_position(const Vector2 position) const {
	return ___godot_icall_int_Vector2(___mb.mb_get_drop_section_at_position, (const Object *) this, position);
}

TreeItem *Tree::get_edited() const {
	return (TreeItem *) ___godot_icall_Object(___mb.mb_get_edited, (const Object *) this);
}

int64_t Tree::get_edited_column() const {
	return ___godot_icall_int(___mb.mb_get_edited_column, (const Object *) this);
}

Rect2 Tree::get_item_area_rect(const Object *item, const int64_t column) const {
	return ___godot_icall_Rect2_Object_int(___mb.mb_get_item_area_rect, (const Object *) this, item, column);
}

TreeItem *Tree::get_item_at_position(const Vector2 position) const {
	return (TreeItem *) ___godot_icall_Object_Vector2(___mb.mb_get_item_at_position, (const Object *) this, position);
}

TreeItem *Tree::get_next_selected(const Object *from) {
	return (TreeItem *) ___godot_icall_Object_Object(___mb.mb_get_next_selected, (const Object *) this, from);
}

int64_t Tree::get_pressed_button() const {
	return ___godot_icall_int(___mb.mb_get_pressed_button, (const Object *) this);
}

TreeItem *Tree::get_root() {
	return (TreeItem *) ___godot_icall_Object(___mb.mb_get_root, (const Object *) this);
}

Vector2 Tree::get_scroll() const {
	return ___godot_icall_Vector2(___mb.mb_get_scroll, (const Object *) this);
}

Tree::SelectMode Tree::get_select_mode() const {
	return (Tree::SelectMode) ___godot_icall_int(___mb.mb_get_select_mode, (const Object *) this);
}

TreeItem *Tree::get_selected() const {
	return (TreeItem *) ___godot_icall_Object(___mb.mb_get_selected, (const Object *) this);
}

int64_t Tree::get_selected_column() const {
	return ___godot_icall_int(___mb.mb_get_selected_column, (const Object *) this);
}

bool Tree::is_folding_hidden() const {
	return ___godot_icall_bool(___mb.mb_is_folding_hidden, (const Object *) this);
}

bool Tree::is_root_hidden() const {
	return ___godot_icall_bool(___mb.mb_is_root_hidden, (const Object *) this);
}

void Tree::set_allow_reselect(const bool allow) {
	___godot_icall_void_bool(___mb.mb_set_allow_reselect, (const Object *) this, allow);
}

void Tree::set_allow_rmb_select(const bool allow) {
	___godot_icall_void_bool(___mb.mb_set_allow_rmb_select, (const Object *) this, allow);
}

void Tree::set_column_expand(const int64_t column, const bool expand) {
	___godot_icall_void_int_bool(___mb.mb_set_column_expand, (const Object *) this, column, expand);
}

void Tree::set_column_min_width(const int64_t column, const int64_t min_width) {
	___godot_icall_void_int_int(___mb.mb_set_column_min_width, (const Object *) this, column, min_width);
}

void Tree::set_column_title(const int64_t column, const String title) {
	___godot_icall_void_int_String(___mb.mb_set_column_title, (const Object *) this, column, title);
}

void Tree::set_column_titles_visible(const bool visible) {
	___godot_icall_void_bool(___mb.mb_set_column_titles_visible, (const Object *) this, visible);
}

void Tree::set_columns(const int64_t amount) {
	___godot_icall_void_int(___mb.mb_set_columns, (const Object *) this, amount);
}

void Tree::set_drop_mode_flags(const int64_t flags) {
	___godot_icall_void_int(___mb.mb_set_drop_mode_flags, (const Object *) this, flags);
}

void Tree::set_hide_folding(const bool hide) {
	___godot_icall_void_bool(___mb.mb_set_hide_folding, (const Object *) this, hide);
}

void Tree::set_hide_root(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_hide_root, (const Object *) this, enable);
}

void Tree::set_select_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_select_mode, (const Object *) this, mode);
}

}