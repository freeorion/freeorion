#include "ItemList.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "Texture.hpp"
#include "VScrollBar.hpp"


namespace godot {


ItemList::___method_bindings ItemList::___mb = {};

void ItemList::___init_method_bindings() {
	___mb.mb__get_items = godot::api->godot_method_bind_get_method("ItemList", "_get_items");
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("ItemList", "_gui_input");
	___mb.mb__scroll_changed = godot::api->godot_method_bind_get_method("ItemList", "_scroll_changed");
	___mb.mb__set_items = godot::api->godot_method_bind_get_method("ItemList", "_set_items");
	___mb.mb_add_icon_item = godot::api->godot_method_bind_get_method("ItemList", "add_icon_item");
	___mb.mb_add_item = godot::api->godot_method_bind_get_method("ItemList", "add_item");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("ItemList", "clear");
	___mb.mb_ensure_current_is_visible = godot::api->godot_method_bind_get_method("ItemList", "ensure_current_is_visible");
	___mb.mb_get_allow_reselect = godot::api->godot_method_bind_get_method("ItemList", "get_allow_reselect");
	___mb.mb_get_allow_rmb_select = godot::api->godot_method_bind_get_method("ItemList", "get_allow_rmb_select");
	___mb.mb_get_fixed_column_width = godot::api->godot_method_bind_get_method("ItemList", "get_fixed_column_width");
	___mb.mb_get_fixed_icon_size = godot::api->godot_method_bind_get_method("ItemList", "get_fixed_icon_size");
	___mb.mb_get_icon_mode = godot::api->godot_method_bind_get_method("ItemList", "get_icon_mode");
	___mb.mb_get_icon_scale = godot::api->godot_method_bind_get_method("ItemList", "get_icon_scale");
	___mb.mb_get_item_at_position = godot::api->godot_method_bind_get_method("ItemList", "get_item_at_position");
	___mb.mb_get_item_count = godot::api->godot_method_bind_get_method("ItemList", "get_item_count");
	___mb.mb_get_item_custom_bg_color = godot::api->godot_method_bind_get_method("ItemList", "get_item_custom_bg_color");
	___mb.mb_get_item_custom_fg_color = godot::api->godot_method_bind_get_method("ItemList", "get_item_custom_fg_color");
	___mb.mb_get_item_icon = godot::api->godot_method_bind_get_method("ItemList", "get_item_icon");
	___mb.mb_get_item_icon_modulate = godot::api->godot_method_bind_get_method("ItemList", "get_item_icon_modulate");
	___mb.mb_get_item_icon_region = godot::api->godot_method_bind_get_method("ItemList", "get_item_icon_region");
	___mb.mb_get_item_metadata = godot::api->godot_method_bind_get_method("ItemList", "get_item_metadata");
	___mb.mb_get_item_text = godot::api->godot_method_bind_get_method("ItemList", "get_item_text");
	___mb.mb_get_item_tooltip = godot::api->godot_method_bind_get_method("ItemList", "get_item_tooltip");
	___mb.mb_get_max_columns = godot::api->godot_method_bind_get_method("ItemList", "get_max_columns");
	___mb.mb_get_max_text_lines = godot::api->godot_method_bind_get_method("ItemList", "get_max_text_lines");
	___mb.mb_get_select_mode = godot::api->godot_method_bind_get_method("ItemList", "get_select_mode");
	___mb.mb_get_selected_items = godot::api->godot_method_bind_get_method("ItemList", "get_selected_items");
	___mb.mb_get_v_scroll = godot::api->godot_method_bind_get_method("ItemList", "get_v_scroll");
	___mb.mb_has_auto_height = godot::api->godot_method_bind_get_method("ItemList", "has_auto_height");
	___mb.mb_is_anything_selected = godot::api->godot_method_bind_get_method("ItemList", "is_anything_selected");
	___mb.mb_is_item_disabled = godot::api->godot_method_bind_get_method("ItemList", "is_item_disabled");
	___mb.mb_is_item_icon_transposed = godot::api->godot_method_bind_get_method("ItemList", "is_item_icon_transposed");
	___mb.mb_is_item_selectable = godot::api->godot_method_bind_get_method("ItemList", "is_item_selectable");
	___mb.mb_is_item_tooltip_enabled = godot::api->godot_method_bind_get_method("ItemList", "is_item_tooltip_enabled");
	___mb.mb_is_same_column_width = godot::api->godot_method_bind_get_method("ItemList", "is_same_column_width");
	___mb.mb_is_selected = godot::api->godot_method_bind_get_method("ItemList", "is_selected");
	___mb.mb_move_item = godot::api->godot_method_bind_get_method("ItemList", "move_item");
	___mb.mb_remove_item = godot::api->godot_method_bind_get_method("ItemList", "remove_item");
	___mb.mb_select = godot::api->godot_method_bind_get_method("ItemList", "select");
	___mb.mb_set_allow_reselect = godot::api->godot_method_bind_get_method("ItemList", "set_allow_reselect");
	___mb.mb_set_allow_rmb_select = godot::api->godot_method_bind_get_method("ItemList", "set_allow_rmb_select");
	___mb.mb_set_auto_height = godot::api->godot_method_bind_get_method("ItemList", "set_auto_height");
	___mb.mb_set_fixed_column_width = godot::api->godot_method_bind_get_method("ItemList", "set_fixed_column_width");
	___mb.mb_set_fixed_icon_size = godot::api->godot_method_bind_get_method("ItemList", "set_fixed_icon_size");
	___mb.mb_set_icon_mode = godot::api->godot_method_bind_get_method("ItemList", "set_icon_mode");
	___mb.mb_set_icon_scale = godot::api->godot_method_bind_get_method("ItemList", "set_icon_scale");
	___mb.mb_set_item_custom_bg_color = godot::api->godot_method_bind_get_method("ItemList", "set_item_custom_bg_color");
	___mb.mb_set_item_custom_fg_color = godot::api->godot_method_bind_get_method("ItemList", "set_item_custom_fg_color");
	___mb.mb_set_item_disabled = godot::api->godot_method_bind_get_method("ItemList", "set_item_disabled");
	___mb.mb_set_item_icon = godot::api->godot_method_bind_get_method("ItemList", "set_item_icon");
	___mb.mb_set_item_icon_modulate = godot::api->godot_method_bind_get_method("ItemList", "set_item_icon_modulate");
	___mb.mb_set_item_icon_region = godot::api->godot_method_bind_get_method("ItemList", "set_item_icon_region");
	___mb.mb_set_item_icon_transposed = godot::api->godot_method_bind_get_method("ItemList", "set_item_icon_transposed");
	___mb.mb_set_item_metadata = godot::api->godot_method_bind_get_method("ItemList", "set_item_metadata");
	___mb.mb_set_item_selectable = godot::api->godot_method_bind_get_method("ItemList", "set_item_selectable");
	___mb.mb_set_item_text = godot::api->godot_method_bind_get_method("ItemList", "set_item_text");
	___mb.mb_set_item_tooltip = godot::api->godot_method_bind_get_method("ItemList", "set_item_tooltip");
	___mb.mb_set_item_tooltip_enabled = godot::api->godot_method_bind_get_method("ItemList", "set_item_tooltip_enabled");
	___mb.mb_set_max_columns = godot::api->godot_method_bind_get_method("ItemList", "set_max_columns");
	___mb.mb_set_max_text_lines = godot::api->godot_method_bind_get_method("ItemList", "set_max_text_lines");
	___mb.mb_set_same_column_width = godot::api->godot_method_bind_get_method("ItemList", "set_same_column_width");
	___mb.mb_set_select_mode = godot::api->godot_method_bind_get_method("ItemList", "set_select_mode");
	___mb.mb_sort_items_by_text = godot::api->godot_method_bind_get_method("ItemList", "sort_items_by_text");
	___mb.mb_unselect = godot::api->godot_method_bind_get_method("ItemList", "unselect");
	___mb.mb_unselect_all = godot::api->godot_method_bind_get_method("ItemList", "unselect_all");
}

ItemList *ItemList::_new()
{
	return (ItemList *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ItemList")());
}
Array ItemList::_get_items() const {
	return ___godot_icall_Array(___mb.mb__get_items, (const Object *) this);
}

void ItemList::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

void ItemList::_scroll_changed(const real_t arg0) {
	___godot_icall_void_float(___mb.mb__scroll_changed, (const Object *) this, arg0);
}

void ItemList::_set_items(const Array arg0) {
	___godot_icall_void_Array(___mb.mb__set_items, (const Object *) this, arg0);
}

void ItemList::add_icon_item(const Ref<Texture> icon, const bool selectable) {
	___godot_icall_void_Object_bool(___mb.mb_add_icon_item, (const Object *) this, icon.ptr(), selectable);
}

void ItemList::add_item(const String text, const Ref<Texture> icon, const bool selectable) {
	___godot_icall_void_String_Object_bool(___mb.mb_add_item, (const Object *) this, text, icon.ptr(), selectable);
}

void ItemList::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

void ItemList::ensure_current_is_visible() {
	___godot_icall_void(___mb.mb_ensure_current_is_visible, (const Object *) this);
}

bool ItemList::get_allow_reselect() const {
	return ___godot_icall_bool(___mb.mb_get_allow_reselect, (const Object *) this);
}

bool ItemList::get_allow_rmb_select() const {
	return ___godot_icall_bool(___mb.mb_get_allow_rmb_select, (const Object *) this);
}

int64_t ItemList::get_fixed_column_width() const {
	return ___godot_icall_int(___mb.mb_get_fixed_column_width, (const Object *) this);
}

Vector2 ItemList::get_fixed_icon_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_fixed_icon_size, (const Object *) this);
}

ItemList::IconMode ItemList::get_icon_mode() const {
	return (ItemList::IconMode) ___godot_icall_int(___mb.mb_get_icon_mode, (const Object *) this);
}

real_t ItemList::get_icon_scale() const {
	return ___godot_icall_float(___mb.mb_get_icon_scale, (const Object *) this);
}

int64_t ItemList::get_item_at_position(const Vector2 position, const bool exact) const {
	return ___godot_icall_int_Vector2_bool(___mb.mb_get_item_at_position, (const Object *) this, position, exact);
}

int64_t ItemList::get_item_count() const {
	return ___godot_icall_int(___mb.mb_get_item_count, (const Object *) this);
}

Color ItemList::get_item_custom_bg_color(const int64_t idx) const {
	return ___godot_icall_Color_int(___mb.mb_get_item_custom_bg_color, (const Object *) this, idx);
}

Color ItemList::get_item_custom_fg_color(const int64_t idx) const {
	return ___godot_icall_Color_int(___mb.mb_get_item_custom_fg_color, (const Object *) this, idx);
}

Ref<Texture> ItemList::get_item_icon(const int64_t idx) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_item_icon, (const Object *) this, idx));
}

Color ItemList::get_item_icon_modulate(const int64_t idx) const {
	return ___godot_icall_Color_int(___mb.mb_get_item_icon_modulate, (const Object *) this, idx);
}

Rect2 ItemList::get_item_icon_region(const int64_t idx) const {
	return ___godot_icall_Rect2_int(___mb.mb_get_item_icon_region, (const Object *) this, idx);
}

Variant ItemList::get_item_metadata(const int64_t idx) const {
	return ___godot_icall_Variant_int(___mb.mb_get_item_metadata, (const Object *) this, idx);
}

String ItemList::get_item_text(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_item_text, (const Object *) this, idx);
}

String ItemList::get_item_tooltip(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_item_tooltip, (const Object *) this, idx);
}

int64_t ItemList::get_max_columns() const {
	return ___godot_icall_int(___mb.mb_get_max_columns, (const Object *) this);
}

int64_t ItemList::get_max_text_lines() const {
	return ___godot_icall_int(___mb.mb_get_max_text_lines, (const Object *) this);
}

ItemList::SelectMode ItemList::get_select_mode() const {
	return (ItemList::SelectMode) ___godot_icall_int(___mb.mb_get_select_mode, (const Object *) this);
}

PoolIntArray ItemList::get_selected_items() {
	return ___godot_icall_PoolIntArray(___mb.mb_get_selected_items, (const Object *) this);
}

VScrollBar *ItemList::get_v_scroll() {
	return (VScrollBar *) ___godot_icall_Object(___mb.mb_get_v_scroll, (const Object *) this);
}

bool ItemList::has_auto_height() const {
	return ___godot_icall_bool(___mb.mb_has_auto_height, (const Object *) this);
}

bool ItemList::is_anything_selected() {
	return ___godot_icall_bool(___mb.mb_is_anything_selected, (const Object *) this);
}

bool ItemList::is_item_disabled(const int64_t idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_item_disabled, (const Object *) this, idx);
}

bool ItemList::is_item_icon_transposed(const int64_t idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_item_icon_transposed, (const Object *) this, idx);
}

bool ItemList::is_item_selectable(const int64_t idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_item_selectable, (const Object *) this, idx);
}

bool ItemList::is_item_tooltip_enabled(const int64_t idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_item_tooltip_enabled, (const Object *) this, idx);
}

bool ItemList::is_same_column_width() const {
	return ___godot_icall_bool(___mb.mb_is_same_column_width, (const Object *) this);
}

bool ItemList::is_selected(const int64_t idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_selected, (const Object *) this, idx);
}

void ItemList::move_item(const int64_t from_idx, const int64_t to_idx) {
	___godot_icall_void_int_int(___mb.mb_move_item, (const Object *) this, from_idx, to_idx);
}

void ItemList::remove_item(const int64_t idx) {
	___godot_icall_void_int(___mb.mb_remove_item, (const Object *) this, idx);
}

void ItemList::select(const int64_t idx, const bool single) {
	___godot_icall_void_int_bool(___mb.mb_select, (const Object *) this, idx, single);
}

void ItemList::set_allow_reselect(const bool allow) {
	___godot_icall_void_bool(___mb.mb_set_allow_reselect, (const Object *) this, allow);
}

void ItemList::set_allow_rmb_select(const bool allow) {
	___godot_icall_void_bool(___mb.mb_set_allow_rmb_select, (const Object *) this, allow);
}

void ItemList::set_auto_height(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_auto_height, (const Object *) this, enable);
}

void ItemList::set_fixed_column_width(const int64_t width) {
	___godot_icall_void_int(___mb.mb_set_fixed_column_width, (const Object *) this, width);
}

void ItemList::set_fixed_icon_size(const Vector2 size) {
	___godot_icall_void_Vector2(___mb.mb_set_fixed_icon_size, (const Object *) this, size);
}

void ItemList::set_icon_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_icon_mode, (const Object *) this, mode);
}

void ItemList::set_icon_scale(const real_t scale) {
	___godot_icall_void_float(___mb.mb_set_icon_scale, (const Object *) this, scale);
}

void ItemList::set_item_custom_bg_color(const int64_t idx, const Color custom_bg_color) {
	___godot_icall_void_int_Color(___mb.mb_set_item_custom_bg_color, (const Object *) this, idx, custom_bg_color);
}

void ItemList::set_item_custom_fg_color(const int64_t idx, const Color custom_fg_color) {
	___godot_icall_void_int_Color(___mb.mb_set_item_custom_fg_color, (const Object *) this, idx, custom_fg_color);
}

void ItemList::set_item_disabled(const int64_t idx, const bool disabled) {
	___godot_icall_void_int_bool(___mb.mb_set_item_disabled, (const Object *) this, idx, disabled);
}

void ItemList::set_item_icon(const int64_t idx, const Ref<Texture> icon) {
	___godot_icall_void_int_Object(___mb.mb_set_item_icon, (const Object *) this, idx, icon.ptr());
}

void ItemList::set_item_icon_modulate(const int64_t idx, const Color modulate) {
	___godot_icall_void_int_Color(___mb.mb_set_item_icon_modulate, (const Object *) this, idx, modulate);
}

void ItemList::set_item_icon_region(const int64_t idx, const Rect2 rect) {
	___godot_icall_void_int_Rect2(___mb.mb_set_item_icon_region, (const Object *) this, idx, rect);
}

void ItemList::set_item_icon_transposed(const int64_t idx, const bool transposed) {
	___godot_icall_void_int_bool(___mb.mb_set_item_icon_transposed, (const Object *) this, idx, transposed);
}

void ItemList::set_item_metadata(const int64_t idx, const Variant metadata) {
	___godot_icall_void_int_Variant(___mb.mb_set_item_metadata, (const Object *) this, idx, metadata);
}

void ItemList::set_item_selectable(const int64_t idx, const bool selectable) {
	___godot_icall_void_int_bool(___mb.mb_set_item_selectable, (const Object *) this, idx, selectable);
}

void ItemList::set_item_text(const int64_t idx, const String text) {
	___godot_icall_void_int_String(___mb.mb_set_item_text, (const Object *) this, idx, text);
}

void ItemList::set_item_tooltip(const int64_t idx, const String tooltip) {
	___godot_icall_void_int_String(___mb.mb_set_item_tooltip, (const Object *) this, idx, tooltip);
}

void ItemList::set_item_tooltip_enabled(const int64_t idx, const bool enable) {
	___godot_icall_void_int_bool(___mb.mb_set_item_tooltip_enabled, (const Object *) this, idx, enable);
}

void ItemList::set_max_columns(const int64_t amount) {
	___godot_icall_void_int(___mb.mb_set_max_columns, (const Object *) this, amount);
}

void ItemList::set_max_text_lines(const int64_t lines) {
	___godot_icall_void_int(___mb.mb_set_max_text_lines, (const Object *) this, lines);
}

void ItemList::set_same_column_width(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_same_column_width, (const Object *) this, enable);
}

void ItemList::set_select_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_select_mode, (const Object *) this, mode);
}

void ItemList::sort_items_by_text() {
	___godot_icall_void(___mb.mb_sort_items_by_text, (const Object *) this);
}

void ItemList::unselect(const int64_t idx) {
	___godot_icall_void_int(___mb.mb_unselect, (const Object *) this, idx);
}

void ItemList::unselect_all() {
	___godot_icall_void(___mb.mb_unselect_all, (const Object *) this);
}

}