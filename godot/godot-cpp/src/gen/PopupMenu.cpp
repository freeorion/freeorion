#include "PopupMenu.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "ShortCut.hpp"
#include "Texture.hpp"


namespace godot {


PopupMenu::___method_bindings PopupMenu::___mb = {};

void PopupMenu::___init_method_bindings() {
	___mb.mb__get_items = godot::api->godot_method_bind_get_method("PopupMenu", "_get_items");
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("PopupMenu", "_gui_input");
	___mb.mb__set_items = godot::api->godot_method_bind_get_method("PopupMenu", "_set_items");
	___mb.mb__submenu_timeout = godot::api->godot_method_bind_get_method("PopupMenu", "_submenu_timeout");
	___mb.mb_add_check_item = godot::api->godot_method_bind_get_method("PopupMenu", "add_check_item");
	___mb.mb_add_check_shortcut = godot::api->godot_method_bind_get_method("PopupMenu", "add_check_shortcut");
	___mb.mb_add_icon_check_item = godot::api->godot_method_bind_get_method("PopupMenu", "add_icon_check_item");
	___mb.mb_add_icon_check_shortcut = godot::api->godot_method_bind_get_method("PopupMenu", "add_icon_check_shortcut");
	___mb.mb_add_icon_item = godot::api->godot_method_bind_get_method("PopupMenu", "add_icon_item");
	___mb.mb_add_icon_radio_check_item = godot::api->godot_method_bind_get_method("PopupMenu", "add_icon_radio_check_item");
	___mb.mb_add_icon_radio_check_shortcut = godot::api->godot_method_bind_get_method("PopupMenu", "add_icon_radio_check_shortcut");
	___mb.mb_add_icon_shortcut = godot::api->godot_method_bind_get_method("PopupMenu", "add_icon_shortcut");
	___mb.mb_add_item = godot::api->godot_method_bind_get_method("PopupMenu", "add_item");
	___mb.mb_add_multistate_item = godot::api->godot_method_bind_get_method("PopupMenu", "add_multistate_item");
	___mb.mb_add_radio_check_item = godot::api->godot_method_bind_get_method("PopupMenu", "add_radio_check_item");
	___mb.mb_add_radio_check_shortcut = godot::api->godot_method_bind_get_method("PopupMenu", "add_radio_check_shortcut");
	___mb.mb_add_separator = godot::api->godot_method_bind_get_method("PopupMenu", "add_separator");
	___mb.mb_add_shortcut = godot::api->godot_method_bind_get_method("PopupMenu", "add_shortcut");
	___mb.mb_add_submenu_item = godot::api->godot_method_bind_get_method("PopupMenu", "add_submenu_item");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("PopupMenu", "clear");
	___mb.mb_get_allow_search = godot::api->godot_method_bind_get_method("PopupMenu", "get_allow_search");
	___mb.mb_get_item_accelerator = godot::api->godot_method_bind_get_method("PopupMenu", "get_item_accelerator");
	___mb.mb_get_item_count = godot::api->godot_method_bind_get_method("PopupMenu", "get_item_count");
	___mb.mb_get_item_icon = godot::api->godot_method_bind_get_method("PopupMenu", "get_item_icon");
	___mb.mb_get_item_id = godot::api->godot_method_bind_get_method("PopupMenu", "get_item_id");
	___mb.mb_get_item_index = godot::api->godot_method_bind_get_method("PopupMenu", "get_item_index");
	___mb.mb_get_item_metadata = godot::api->godot_method_bind_get_method("PopupMenu", "get_item_metadata");
	___mb.mb_get_item_shortcut = godot::api->godot_method_bind_get_method("PopupMenu", "get_item_shortcut");
	___mb.mb_get_item_submenu = godot::api->godot_method_bind_get_method("PopupMenu", "get_item_submenu");
	___mb.mb_get_item_text = godot::api->godot_method_bind_get_method("PopupMenu", "get_item_text");
	___mb.mb_get_item_tooltip = godot::api->godot_method_bind_get_method("PopupMenu", "get_item_tooltip");
	___mb.mb_get_submenu_popup_delay = godot::api->godot_method_bind_get_method("PopupMenu", "get_submenu_popup_delay");
	___mb.mb_is_hide_on_checkable_item_selection = godot::api->godot_method_bind_get_method("PopupMenu", "is_hide_on_checkable_item_selection");
	___mb.mb_is_hide_on_item_selection = godot::api->godot_method_bind_get_method("PopupMenu", "is_hide_on_item_selection");
	___mb.mb_is_hide_on_state_item_selection = godot::api->godot_method_bind_get_method("PopupMenu", "is_hide_on_state_item_selection");
	___mb.mb_is_hide_on_window_lose_focus = godot::api->godot_method_bind_get_method("PopupMenu", "is_hide_on_window_lose_focus");
	___mb.mb_is_item_checkable = godot::api->godot_method_bind_get_method("PopupMenu", "is_item_checkable");
	___mb.mb_is_item_checked = godot::api->godot_method_bind_get_method("PopupMenu", "is_item_checked");
	___mb.mb_is_item_disabled = godot::api->godot_method_bind_get_method("PopupMenu", "is_item_disabled");
	___mb.mb_is_item_radio_checkable = godot::api->godot_method_bind_get_method("PopupMenu", "is_item_radio_checkable");
	___mb.mb_is_item_separator = godot::api->godot_method_bind_get_method("PopupMenu", "is_item_separator");
	___mb.mb_is_item_shortcut_disabled = godot::api->godot_method_bind_get_method("PopupMenu", "is_item_shortcut_disabled");
	___mb.mb_remove_item = godot::api->godot_method_bind_get_method("PopupMenu", "remove_item");
	___mb.mb_set_allow_search = godot::api->godot_method_bind_get_method("PopupMenu", "set_allow_search");
	___mb.mb_set_hide_on_checkable_item_selection = godot::api->godot_method_bind_get_method("PopupMenu", "set_hide_on_checkable_item_selection");
	___mb.mb_set_hide_on_item_selection = godot::api->godot_method_bind_get_method("PopupMenu", "set_hide_on_item_selection");
	___mb.mb_set_hide_on_state_item_selection = godot::api->godot_method_bind_get_method("PopupMenu", "set_hide_on_state_item_selection");
	___mb.mb_set_hide_on_window_lose_focus = godot::api->godot_method_bind_get_method("PopupMenu", "set_hide_on_window_lose_focus");
	___mb.mb_set_item_accelerator = godot::api->godot_method_bind_get_method("PopupMenu", "set_item_accelerator");
	___mb.mb_set_item_as_checkable = godot::api->godot_method_bind_get_method("PopupMenu", "set_item_as_checkable");
	___mb.mb_set_item_as_radio_checkable = godot::api->godot_method_bind_get_method("PopupMenu", "set_item_as_radio_checkable");
	___mb.mb_set_item_as_separator = godot::api->godot_method_bind_get_method("PopupMenu", "set_item_as_separator");
	___mb.mb_set_item_checked = godot::api->godot_method_bind_get_method("PopupMenu", "set_item_checked");
	___mb.mb_set_item_disabled = godot::api->godot_method_bind_get_method("PopupMenu", "set_item_disabled");
	___mb.mb_set_item_icon = godot::api->godot_method_bind_get_method("PopupMenu", "set_item_icon");
	___mb.mb_set_item_id = godot::api->godot_method_bind_get_method("PopupMenu", "set_item_id");
	___mb.mb_set_item_metadata = godot::api->godot_method_bind_get_method("PopupMenu", "set_item_metadata");
	___mb.mb_set_item_multistate = godot::api->godot_method_bind_get_method("PopupMenu", "set_item_multistate");
	___mb.mb_set_item_shortcut = godot::api->godot_method_bind_get_method("PopupMenu", "set_item_shortcut");
	___mb.mb_set_item_shortcut_disabled = godot::api->godot_method_bind_get_method("PopupMenu", "set_item_shortcut_disabled");
	___mb.mb_set_item_submenu = godot::api->godot_method_bind_get_method("PopupMenu", "set_item_submenu");
	___mb.mb_set_item_text = godot::api->godot_method_bind_get_method("PopupMenu", "set_item_text");
	___mb.mb_set_item_tooltip = godot::api->godot_method_bind_get_method("PopupMenu", "set_item_tooltip");
	___mb.mb_set_submenu_popup_delay = godot::api->godot_method_bind_get_method("PopupMenu", "set_submenu_popup_delay");
	___mb.mb_toggle_item_checked = godot::api->godot_method_bind_get_method("PopupMenu", "toggle_item_checked");
	___mb.mb_toggle_item_multistate = godot::api->godot_method_bind_get_method("PopupMenu", "toggle_item_multistate");
}

PopupMenu *PopupMenu::_new()
{
	return (PopupMenu *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PopupMenu")());
}
Array PopupMenu::_get_items() const {
	return ___godot_icall_Array(___mb.mb__get_items, (const Object *) this);
}

void PopupMenu::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

void PopupMenu::_set_items(const Array arg0) {
	___godot_icall_void_Array(___mb.mb__set_items, (const Object *) this, arg0);
}

void PopupMenu::_submenu_timeout() {
	___godot_icall_void(___mb.mb__submenu_timeout, (const Object *) this);
}

void PopupMenu::add_check_item(const String label, const int64_t id, const int64_t accel) {
	___godot_icall_void_String_int_int(___mb.mb_add_check_item, (const Object *) this, label, id, accel);
}

void PopupMenu::add_check_shortcut(const Ref<ShortCut> shortcut, const int64_t id, const bool global) {
	___godot_icall_void_Object_int_bool(___mb.mb_add_check_shortcut, (const Object *) this, shortcut.ptr(), id, global);
}

void PopupMenu::add_icon_check_item(const Ref<Texture> texture, const String label, const int64_t id, const int64_t accel) {
	___godot_icall_void_Object_String_int_int(___mb.mb_add_icon_check_item, (const Object *) this, texture.ptr(), label, id, accel);
}

void PopupMenu::add_icon_check_shortcut(const Ref<Texture> texture, const Ref<ShortCut> shortcut, const int64_t id, const bool global) {
	___godot_icall_void_Object_Object_int_bool(___mb.mb_add_icon_check_shortcut, (const Object *) this, texture.ptr(), shortcut.ptr(), id, global);
}

void PopupMenu::add_icon_item(const Ref<Texture> texture, const String label, const int64_t id, const int64_t accel) {
	___godot_icall_void_Object_String_int_int(___mb.mb_add_icon_item, (const Object *) this, texture.ptr(), label, id, accel);
}

void PopupMenu::add_icon_radio_check_item(const Ref<Texture> texture, const String label, const int64_t id, const int64_t accel) {
	___godot_icall_void_Object_String_int_int(___mb.mb_add_icon_radio_check_item, (const Object *) this, texture.ptr(), label, id, accel);
}

void PopupMenu::add_icon_radio_check_shortcut(const Ref<Texture> texture, const Ref<ShortCut> shortcut, const int64_t id, const bool global) {
	___godot_icall_void_Object_Object_int_bool(___mb.mb_add_icon_radio_check_shortcut, (const Object *) this, texture.ptr(), shortcut.ptr(), id, global);
}

void PopupMenu::add_icon_shortcut(const Ref<Texture> texture, const Ref<ShortCut> shortcut, const int64_t id, const bool global) {
	___godot_icall_void_Object_Object_int_bool(___mb.mb_add_icon_shortcut, (const Object *) this, texture.ptr(), shortcut.ptr(), id, global);
}

void PopupMenu::add_item(const String label, const int64_t id, const int64_t accel) {
	___godot_icall_void_String_int_int(___mb.mb_add_item, (const Object *) this, label, id, accel);
}

void PopupMenu::add_multistate_item(const String label, const int64_t max_states, const int64_t default_state, const int64_t id, const int64_t accel) {
	___godot_icall_void_String_int_int_int_int(___mb.mb_add_multistate_item, (const Object *) this, label, max_states, default_state, id, accel);
}

void PopupMenu::add_radio_check_item(const String label, const int64_t id, const int64_t accel) {
	___godot_icall_void_String_int_int(___mb.mb_add_radio_check_item, (const Object *) this, label, id, accel);
}

void PopupMenu::add_radio_check_shortcut(const Ref<ShortCut> shortcut, const int64_t id, const bool global) {
	___godot_icall_void_Object_int_bool(___mb.mb_add_radio_check_shortcut, (const Object *) this, shortcut.ptr(), id, global);
}

void PopupMenu::add_separator(const String label) {
	___godot_icall_void_String(___mb.mb_add_separator, (const Object *) this, label);
}

void PopupMenu::add_shortcut(const Ref<ShortCut> shortcut, const int64_t id, const bool global) {
	___godot_icall_void_Object_int_bool(___mb.mb_add_shortcut, (const Object *) this, shortcut.ptr(), id, global);
}

void PopupMenu::add_submenu_item(const String label, const String submenu, const int64_t id) {
	___godot_icall_void_String_String_int(___mb.mb_add_submenu_item, (const Object *) this, label, submenu, id);
}

void PopupMenu::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

bool PopupMenu::get_allow_search() const {
	return ___godot_icall_bool(___mb.mb_get_allow_search, (const Object *) this);
}

int64_t PopupMenu::get_item_accelerator(const int64_t idx) const {
	return ___godot_icall_int_int(___mb.mb_get_item_accelerator, (const Object *) this, idx);
}

int64_t PopupMenu::get_item_count() const {
	return ___godot_icall_int(___mb.mb_get_item_count, (const Object *) this);
}

Ref<Texture> PopupMenu::get_item_icon(const int64_t idx) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_item_icon, (const Object *) this, idx));
}

int64_t PopupMenu::get_item_id(const int64_t idx) const {
	return ___godot_icall_int_int(___mb.mb_get_item_id, (const Object *) this, idx);
}

int64_t PopupMenu::get_item_index(const int64_t id) const {
	return ___godot_icall_int_int(___mb.mb_get_item_index, (const Object *) this, id);
}

Variant PopupMenu::get_item_metadata(const int64_t idx) const {
	return ___godot_icall_Variant_int(___mb.mb_get_item_metadata, (const Object *) this, idx);
}

Ref<ShortCut> PopupMenu::get_item_shortcut(const int64_t idx) const {
	return Ref<ShortCut>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_item_shortcut, (const Object *) this, idx));
}

String PopupMenu::get_item_submenu(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_item_submenu, (const Object *) this, idx);
}

String PopupMenu::get_item_text(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_item_text, (const Object *) this, idx);
}

String PopupMenu::get_item_tooltip(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_item_tooltip, (const Object *) this, idx);
}

real_t PopupMenu::get_submenu_popup_delay() const {
	return ___godot_icall_float(___mb.mb_get_submenu_popup_delay, (const Object *) this);
}

bool PopupMenu::is_hide_on_checkable_item_selection() const {
	return ___godot_icall_bool(___mb.mb_is_hide_on_checkable_item_selection, (const Object *) this);
}

bool PopupMenu::is_hide_on_item_selection() const {
	return ___godot_icall_bool(___mb.mb_is_hide_on_item_selection, (const Object *) this);
}

bool PopupMenu::is_hide_on_state_item_selection() const {
	return ___godot_icall_bool(___mb.mb_is_hide_on_state_item_selection, (const Object *) this);
}

bool PopupMenu::is_hide_on_window_lose_focus() const {
	return ___godot_icall_bool(___mb.mb_is_hide_on_window_lose_focus, (const Object *) this);
}

bool PopupMenu::is_item_checkable(const int64_t idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_item_checkable, (const Object *) this, idx);
}

bool PopupMenu::is_item_checked(const int64_t idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_item_checked, (const Object *) this, idx);
}

bool PopupMenu::is_item_disabled(const int64_t idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_item_disabled, (const Object *) this, idx);
}

bool PopupMenu::is_item_radio_checkable(const int64_t idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_item_radio_checkable, (const Object *) this, idx);
}

bool PopupMenu::is_item_separator(const int64_t idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_item_separator, (const Object *) this, idx);
}

bool PopupMenu::is_item_shortcut_disabled(const int64_t idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_item_shortcut_disabled, (const Object *) this, idx);
}

void PopupMenu::remove_item(const int64_t idx) {
	___godot_icall_void_int(___mb.mb_remove_item, (const Object *) this, idx);
}

void PopupMenu::set_allow_search(const bool allow) {
	___godot_icall_void_bool(___mb.mb_set_allow_search, (const Object *) this, allow);
}

void PopupMenu::set_hide_on_checkable_item_selection(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_hide_on_checkable_item_selection, (const Object *) this, enable);
}

void PopupMenu::set_hide_on_item_selection(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_hide_on_item_selection, (const Object *) this, enable);
}

void PopupMenu::set_hide_on_state_item_selection(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_hide_on_state_item_selection, (const Object *) this, enable);
}

void PopupMenu::set_hide_on_window_lose_focus(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_hide_on_window_lose_focus, (const Object *) this, enable);
}

void PopupMenu::set_item_accelerator(const int64_t idx, const int64_t accel) {
	___godot_icall_void_int_int(___mb.mb_set_item_accelerator, (const Object *) this, idx, accel);
}

void PopupMenu::set_item_as_checkable(const int64_t idx, const bool enable) {
	___godot_icall_void_int_bool(___mb.mb_set_item_as_checkable, (const Object *) this, idx, enable);
}

void PopupMenu::set_item_as_radio_checkable(const int64_t idx, const bool enable) {
	___godot_icall_void_int_bool(___mb.mb_set_item_as_radio_checkable, (const Object *) this, idx, enable);
}

void PopupMenu::set_item_as_separator(const int64_t idx, const bool enable) {
	___godot_icall_void_int_bool(___mb.mb_set_item_as_separator, (const Object *) this, idx, enable);
}

void PopupMenu::set_item_checked(const int64_t idx, const bool checked) {
	___godot_icall_void_int_bool(___mb.mb_set_item_checked, (const Object *) this, idx, checked);
}

void PopupMenu::set_item_disabled(const int64_t idx, const bool disabled) {
	___godot_icall_void_int_bool(___mb.mb_set_item_disabled, (const Object *) this, idx, disabled);
}

void PopupMenu::set_item_icon(const int64_t idx, const Ref<Texture> icon) {
	___godot_icall_void_int_Object(___mb.mb_set_item_icon, (const Object *) this, idx, icon.ptr());
}

void PopupMenu::set_item_id(const int64_t idx, const int64_t id) {
	___godot_icall_void_int_int(___mb.mb_set_item_id, (const Object *) this, idx, id);
}

void PopupMenu::set_item_metadata(const int64_t idx, const Variant metadata) {
	___godot_icall_void_int_Variant(___mb.mb_set_item_metadata, (const Object *) this, idx, metadata);
}

void PopupMenu::set_item_multistate(const int64_t idx, const int64_t state) {
	___godot_icall_void_int_int(___mb.mb_set_item_multistate, (const Object *) this, idx, state);
}

void PopupMenu::set_item_shortcut(const int64_t idx, const Ref<ShortCut> shortcut, const bool global) {
	___godot_icall_void_int_Object_bool(___mb.mb_set_item_shortcut, (const Object *) this, idx, shortcut.ptr(), global);
}

void PopupMenu::set_item_shortcut_disabled(const int64_t idx, const bool disabled) {
	___godot_icall_void_int_bool(___mb.mb_set_item_shortcut_disabled, (const Object *) this, idx, disabled);
}

void PopupMenu::set_item_submenu(const int64_t idx, const String submenu) {
	___godot_icall_void_int_String(___mb.mb_set_item_submenu, (const Object *) this, idx, submenu);
}

void PopupMenu::set_item_text(const int64_t idx, const String text) {
	___godot_icall_void_int_String(___mb.mb_set_item_text, (const Object *) this, idx, text);
}

void PopupMenu::set_item_tooltip(const int64_t idx, const String tooltip) {
	___godot_icall_void_int_String(___mb.mb_set_item_tooltip, (const Object *) this, idx, tooltip);
}

void PopupMenu::set_submenu_popup_delay(const real_t seconds) {
	___godot_icall_void_float(___mb.mb_set_submenu_popup_delay, (const Object *) this, seconds);
}

void PopupMenu::toggle_item_checked(const int64_t idx) {
	___godot_icall_void_int(___mb.mb_toggle_item_checked, (const Object *) this, idx);
}

void PopupMenu::toggle_item_multistate(const int64_t idx) {
	___godot_icall_void_int(___mb.mb_toggle_item_multistate, (const Object *) this, idx);
}

}