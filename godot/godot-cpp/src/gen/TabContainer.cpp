#include "TabContainer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "Control.hpp"
#include "Popup.hpp"
#include "Texture.hpp"
#include "Node.hpp"


namespace godot {


TabContainer::___method_bindings TabContainer::___mb = {};

void TabContainer::___init_method_bindings() {
	___mb.mb__child_renamed_callback = godot::api->godot_method_bind_get_method("TabContainer", "_child_renamed_callback");
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("TabContainer", "_gui_input");
	___mb.mb__on_mouse_exited = godot::api->godot_method_bind_get_method("TabContainer", "_on_mouse_exited");
	___mb.mb__on_theme_changed = godot::api->godot_method_bind_get_method("TabContainer", "_on_theme_changed");
	___mb.mb__update_current_tab = godot::api->godot_method_bind_get_method("TabContainer", "_update_current_tab");
	___mb.mb_are_tabs_visible = godot::api->godot_method_bind_get_method("TabContainer", "are_tabs_visible");
	___mb.mb_get_current_tab = godot::api->godot_method_bind_get_method("TabContainer", "get_current_tab");
	___mb.mb_get_current_tab_control = godot::api->godot_method_bind_get_method("TabContainer", "get_current_tab_control");
	___mb.mb_get_drag_to_rearrange_enabled = godot::api->godot_method_bind_get_method("TabContainer", "get_drag_to_rearrange_enabled");
	___mb.mb_get_popup = godot::api->godot_method_bind_get_method("TabContainer", "get_popup");
	___mb.mb_get_previous_tab = godot::api->godot_method_bind_get_method("TabContainer", "get_previous_tab");
	___mb.mb_get_tab_align = godot::api->godot_method_bind_get_method("TabContainer", "get_tab_align");
	___mb.mb_get_tab_control = godot::api->godot_method_bind_get_method("TabContainer", "get_tab_control");
	___mb.mb_get_tab_count = godot::api->godot_method_bind_get_method("TabContainer", "get_tab_count");
	___mb.mb_get_tab_disabled = godot::api->godot_method_bind_get_method("TabContainer", "get_tab_disabled");
	___mb.mb_get_tab_icon = godot::api->godot_method_bind_get_method("TabContainer", "get_tab_icon");
	___mb.mb_get_tab_title = godot::api->godot_method_bind_get_method("TabContainer", "get_tab_title");
	___mb.mb_get_tabs_rearrange_group = godot::api->godot_method_bind_get_method("TabContainer", "get_tabs_rearrange_group");
	___mb.mb_get_use_hidden_tabs_for_min_size = godot::api->godot_method_bind_get_method("TabContainer", "get_use_hidden_tabs_for_min_size");
	___mb.mb_set_current_tab = godot::api->godot_method_bind_get_method("TabContainer", "set_current_tab");
	___mb.mb_set_drag_to_rearrange_enabled = godot::api->godot_method_bind_get_method("TabContainer", "set_drag_to_rearrange_enabled");
	___mb.mb_set_popup = godot::api->godot_method_bind_get_method("TabContainer", "set_popup");
	___mb.mb_set_tab_align = godot::api->godot_method_bind_get_method("TabContainer", "set_tab_align");
	___mb.mb_set_tab_disabled = godot::api->godot_method_bind_get_method("TabContainer", "set_tab_disabled");
	___mb.mb_set_tab_icon = godot::api->godot_method_bind_get_method("TabContainer", "set_tab_icon");
	___mb.mb_set_tab_title = godot::api->godot_method_bind_get_method("TabContainer", "set_tab_title");
	___mb.mb_set_tabs_rearrange_group = godot::api->godot_method_bind_get_method("TabContainer", "set_tabs_rearrange_group");
	___mb.mb_set_tabs_visible = godot::api->godot_method_bind_get_method("TabContainer", "set_tabs_visible");
	___mb.mb_set_use_hidden_tabs_for_min_size = godot::api->godot_method_bind_get_method("TabContainer", "set_use_hidden_tabs_for_min_size");
}

TabContainer *TabContainer::_new()
{
	return (TabContainer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"TabContainer")());
}
void TabContainer::_child_renamed_callback() {
	___godot_icall_void(___mb.mb__child_renamed_callback, (const Object *) this);
}

void TabContainer::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

void TabContainer::_on_mouse_exited() {
	___godot_icall_void(___mb.mb__on_mouse_exited, (const Object *) this);
}

void TabContainer::_on_theme_changed() {
	___godot_icall_void(___mb.mb__on_theme_changed, (const Object *) this);
}

void TabContainer::_update_current_tab() {
	___godot_icall_void(___mb.mb__update_current_tab, (const Object *) this);
}

bool TabContainer::are_tabs_visible() const {
	return ___godot_icall_bool(___mb.mb_are_tabs_visible, (const Object *) this);
}

int64_t TabContainer::get_current_tab() const {
	return ___godot_icall_int(___mb.mb_get_current_tab, (const Object *) this);
}

Control *TabContainer::get_current_tab_control() const {
	return (Control *) ___godot_icall_Object(___mb.mb_get_current_tab_control, (const Object *) this);
}

bool TabContainer::get_drag_to_rearrange_enabled() const {
	return ___godot_icall_bool(___mb.mb_get_drag_to_rearrange_enabled, (const Object *) this);
}

Popup *TabContainer::get_popup() const {
	return (Popup *) ___godot_icall_Object(___mb.mb_get_popup, (const Object *) this);
}

int64_t TabContainer::get_previous_tab() const {
	return ___godot_icall_int(___mb.mb_get_previous_tab, (const Object *) this);
}

TabContainer::TabAlign TabContainer::get_tab_align() const {
	return (TabContainer::TabAlign) ___godot_icall_int(___mb.mb_get_tab_align, (const Object *) this);
}

Control *TabContainer::get_tab_control(const int64_t idx) const {
	return (Control *) ___godot_icall_Object_int(___mb.mb_get_tab_control, (const Object *) this, idx);
}

int64_t TabContainer::get_tab_count() const {
	return ___godot_icall_int(___mb.mb_get_tab_count, (const Object *) this);
}

bool TabContainer::get_tab_disabled(const int64_t tab_idx) const {
	return ___godot_icall_bool_int(___mb.mb_get_tab_disabled, (const Object *) this, tab_idx);
}

Ref<Texture> TabContainer::get_tab_icon(const int64_t tab_idx) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_tab_icon, (const Object *) this, tab_idx));
}

String TabContainer::get_tab_title(const int64_t tab_idx) const {
	return ___godot_icall_String_int(___mb.mb_get_tab_title, (const Object *) this, tab_idx);
}

int64_t TabContainer::get_tabs_rearrange_group() const {
	return ___godot_icall_int(___mb.mb_get_tabs_rearrange_group, (const Object *) this);
}

bool TabContainer::get_use_hidden_tabs_for_min_size() const {
	return ___godot_icall_bool(___mb.mb_get_use_hidden_tabs_for_min_size, (const Object *) this);
}

void TabContainer::set_current_tab(const int64_t tab_idx) {
	___godot_icall_void_int(___mb.mb_set_current_tab, (const Object *) this, tab_idx);
}

void TabContainer::set_drag_to_rearrange_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_drag_to_rearrange_enabled, (const Object *) this, enabled);
}

void TabContainer::set_popup(const Node *popup) {
	___godot_icall_void_Object(___mb.mb_set_popup, (const Object *) this, popup);
}

void TabContainer::set_tab_align(const int64_t align) {
	___godot_icall_void_int(___mb.mb_set_tab_align, (const Object *) this, align);
}

void TabContainer::set_tab_disabled(const int64_t tab_idx, const bool disabled) {
	___godot_icall_void_int_bool(___mb.mb_set_tab_disabled, (const Object *) this, tab_idx, disabled);
}

void TabContainer::set_tab_icon(const int64_t tab_idx, const Ref<Texture> icon) {
	___godot_icall_void_int_Object(___mb.mb_set_tab_icon, (const Object *) this, tab_idx, icon.ptr());
}

void TabContainer::set_tab_title(const int64_t tab_idx, const String title) {
	___godot_icall_void_int_String(___mb.mb_set_tab_title, (const Object *) this, tab_idx, title);
}

void TabContainer::set_tabs_rearrange_group(const int64_t group_id) {
	___godot_icall_void_int(___mb.mb_set_tabs_rearrange_group, (const Object *) this, group_id);
}

void TabContainer::set_tabs_visible(const bool visible) {
	___godot_icall_void_bool(___mb.mb_set_tabs_visible, (const Object *) this, visible);
}

void TabContainer::set_use_hidden_tabs_for_min_size(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_use_hidden_tabs_for_min_size, (const Object *) this, enabled);
}

}