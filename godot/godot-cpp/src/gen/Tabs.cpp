#include "Tabs.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "Texture.hpp"


namespace godot {


Tabs::___method_bindings Tabs::___mb = {};

void Tabs::___init_method_bindings() {
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("Tabs", "_gui_input");
	___mb.mb__on_mouse_exited = godot::api->godot_method_bind_get_method("Tabs", "_on_mouse_exited");
	___mb.mb__update_hover = godot::api->godot_method_bind_get_method("Tabs", "_update_hover");
	___mb.mb_add_tab = godot::api->godot_method_bind_get_method("Tabs", "add_tab");
	___mb.mb_ensure_tab_visible = godot::api->godot_method_bind_get_method("Tabs", "ensure_tab_visible");
	___mb.mb_get_current_tab = godot::api->godot_method_bind_get_method("Tabs", "get_current_tab");
	___mb.mb_get_drag_to_rearrange_enabled = godot::api->godot_method_bind_get_method("Tabs", "get_drag_to_rearrange_enabled");
	___mb.mb_get_offset_buttons_visible = godot::api->godot_method_bind_get_method("Tabs", "get_offset_buttons_visible");
	___mb.mb_get_scrolling_enabled = godot::api->godot_method_bind_get_method("Tabs", "get_scrolling_enabled");
	___mb.mb_get_select_with_rmb = godot::api->godot_method_bind_get_method("Tabs", "get_select_with_rmb");
	___mb.mb_get_tab_align = godot::api->godot_method_bind_get_method("Tabs", "get_tab_align");
	___mb.mb_get_tab_close_display_policy = godot::api->godot_method_bind_get_method("Tabs", "get_tab_close_display_policy");
	___mb.mb_get_tab_count = godot::api->godot_method_bind_get_method("Tabs", "get_tab_count");
	___mb.mb_get_tab_disabled = godot::api->godot_method_bind_get_method("Tabs", "get_tab_disabled");
	___mb.mb_get_tab_icon = godot::api->godot_method_bind_get_method("Tabs", "get_tab_icon");
	___mb.mb_get_tab_offset = godot::api->godot_method_bind_get_method("Tabs", "get_tab_offset");
	___mb.mb_get_tab_rect = godot::api->godot_method_bind_get_method("Tabs", "get_tab_rect");
	___mb.mb_get_tab_title = godot::api->godot_method_bind_get_method("Tabs", "get_tab_title");
	___mb.mb_get_tabs_rearrange_group = godot::api->godot_method_bind_get_method("Tabs", "get_tabs_rearrange_group");
	___mb.mb_move_tab = godot::api->godot_method_bind_get_method("Tabs", "move_tab");
	___mb.mb_remove_tab = godot::api->godot_method_bind_get_method("Tabs", "remove_tab");
	___mb.mb_set_current_tab = godot::api->godot_method_bind_get_method("Tabs", "set_current_tab");
	___mb.mb_set_drag_to_rearrange_enabled = godot::api->godot_method_bind_get_method("Tabs", "set_drag_to_rearrange_enabled");
	___mb.mb_set_scrolling_enabled = godot::api->godot_method_bind_get_method("Tabs", "set_scrolling_enabled");
	___mb.mb_set_select_with_rmb = godot::api->godot_method_bind_get_method("Tabs", "set_select_with_rmb");
	___mb.mb_set_tab_align = godot::api->godot_method_bind_get_method("Tabs", "set_tab_align");
	___mb.mb_set_tab_close_display_policy = godot::api->godot_method_bind_get_method("Tabs", "set_tab_close_display_policy");
	___mb.mb_set_tab_disabled = godot::api->godot_method_bind_get_method("Tabs", "set_tab_disabled");
	___mb.mb_set_tab_icon = godot::api->godot_method_bind_get_method("Tabs", "set_tab_icon");
	___mb.mb_set_tab_title = godot::api->godot_method_bind_get_method("Tabs", "set_tab_title");
	___mb.mb_set_tabs_rearrange_group = godot::api->godot_method_bind_get_method("Tabs", "set_tabs_rearrange_group");
}

Tabs *Tabs::_new()
{
	return (Tabs *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Tabs")());
}
void Tabs::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

void Tabs::_on_mouse_exited() {
	___godot_icall_void(___mb.mb__on_mouse_exited, (const Object *) this);
}

void Tabs::_update_hover() {
	___godot_icall_void(___mb.mb__update_hover, (const Object *) this);
}

void Tabs::add_tab(const String title, const Ref<Texture> icon) {
	___godot_icall_void_String_Object(___mb.mb_add_tab, (const Object *) this, title, icon.ptr());
}

void Tabs::ensure_tab_visible(const int64_t idx) {
	___godot_icall_void_int(___mb.mb_ensure_tab_visible, (const Object *) this, idx);
}

int64_t Tabs::get_current_tab() const {
	return ___godot_icall_int(___mb.mb_get_current_tab, (const Object *) this);
}

bool Tabs::get_drag_to_rearrange_enabled() const {
	return ___godot_icall_bool(___mb.mb_get_drag_to_rearrange_enabled, (const Object *) this);
}

bool Tabs::get_offset_buttons_visible() const {
	return ___godot_icall_bool(___mb.mb_get_offset_buttons_visible, (const Object *) this);
}

bool Tabs::get_scrolling_enabled() const {
	return ___godot_icall_bool(___mb.mb_get_scrolling_enabled, (const Object *) this);
}

bool Tabs::get_select_with_rmb() const {
	return ___godot_icall_bool(___mb.mb_get_select_with_rmb, (const Object *) this);
}

Tabs::TabAlign Tabs::get_tab_align() const {
	return (Tabs::TabAlign) ___godot_icall_int(___mb.mb_get_tab_align, (const Object *) this);
}

Tabs::CloseButtonDisplayPolicy Tabs::get_tab_close_display_policy() const {
	return (Tabs::CloseButtonDisplayPolicy) ___godot_icall_int(___mb.mb_get_tab_close_display_policy, (const Object *) this);
}

int64_t Tabs::get_tab_count() const {
	return ___godot_icall_int(___mb.mb_get_tab_count, (const Object *) this);
}

bool Tabs::get_tab_disabled(const int64_t tab_idx) const {
	return ___godot_icall_bool_int(___mb.mb_get_tab_disabled, (const Object *) this, tab_idx);
}

Ref<Texture> Tabs::get_tab_icon(const int64_t tab_idx) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_tab_icon, (const Object *) this, tab_idx));
}

int64_t Tabs::get_tab_offset() const {
	return ___godot_icall_int(___mb.mb_get_tab_offset, (const Object *) this);
}

Rect2 Tabs::get_tab_rect(const int64_t tab_idx) const {
	return ___godot_icall_Rect2_int(___mb.mb_get_tab_rect, (const Object *) this, tab_idx);
}

String Tabs::get_tab_title(const int64_t tab_idx) const {
	return ___godot_icall_String_int(___mb.mb_get_tab_title, (const Object *) this, tab_idx);
}

int64_t Tabs::get_tabs_rearrange_group() const {
	return ___godot_icall_int(___mb.mb_get_tabs_rearrange_group, (const Object *) this);
}

void Tabs::move_tab(const int64_t from, const int64_t to) {
	___godot_icall_void_int_int(___mb.mb_move_tab, (const Object *) this, from, to);
}

void Tabs::remove_tab(const int64_t tab_idx) {
	___godot_icall_void_int(___mb.mb_remove_tab, (const Object *) this, tab_idx);
}

void Tabs::set_current_tab(const int64_t tab_idx) {
	___godot_icall_void_int(___mb.mb_set_current_tab, (const Object *) this, tab_idx);
}

void Tabs::set_drag_to_rearrange_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_drag_to_rearrange_enabled, (const Object *) this, enabled);
}

void Tabs::set_scrolling_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_scrolling_enabled, (const Object *) this, enabled);
}

void Tabs::set_select_with_rmb(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_select_with_rmb, (const Object *) this, enabled);
}

void Tabs::set_tab_align(const int64_t align) {
	___godot_icall_void_int(___mb.mb_set_tab_align, (const Object *) this, align);
}

void Tabs::set_tab_close_display_policy(const int64_t policy) {
	___godot_icall_void_int(___mb.mb_set_tab_close_display_policy, (const Object *) this, policy);
}

void Tabs::set_tab_disabled(const int64_t tab_idx, const bool disabled) {
	___godot_icall_void_int_bool(___mb.mb_set_tab_disabled, (const Object *) this, tab_idx, disabled);
}

void Tabs::set_tab_icon(const int64_t tab_idx, const Ref<Texture> icon) {
	___godot_icall_void_int_Object(___mb.mb_set_tab_icon, (const Object *) this, tab_idx, icon.ptr());
}

void Tabs::set_tab_title(const int64_t tab_idx, const String title) {
	___godot_icall_void_int_String(___mb.mb_set_tab_title, (const Object *) this, tab_idx, title);
}

void Tabs::set_tabs_rearrange_group(const int64_t group_id) {
	___godot_icall_void_int(___mb.mb_set_tabs_rearrange_group, (const Object *) this, group_id);
}

}