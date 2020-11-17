#include "BaseButton.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "ButtonGroup.hpp"
#include "ShortCut.hpp"


namespace godot {


BaseButton::___method_bindings BaseButton::___mb = {};

void BaseButton::___init_method_bindings() {
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("BaseButton", "_gui_input");
	___mb.mb__pressed = godot::api->godot_method_bind_get_method("BaseButton", "_pressed");
	___mb.mb__toggled = godot::api->godot_method_bind_get_method("BaseButton", "_toggled");
	___mb.mb__unhandled_input = godot::api->godot_method_bind_get_method("BaseButton", "_unhandled_input");
	___mb.mb_get_action_mode = godot::api->godot_method_bind_get_method("BaseButton", "get_action_mode");
	___mb.mb_get_button_group = godot::api->godot_method_bind_get_method("BaseButton", "get_button_group");
	___mb.mb_get_button_mask = godot::api->godot_method_bind_get_method("BaseButton", "get_button_mask");
	___mb.mb_get_draw_mode = godot::api->godot_method_bind_get_method("BaseButton", "get_draw_mode");
	___mb.mb_get_enabled_focus_mode = godot::api->godot_method_bind_get_method("BaseButton", "get_enabled_focus_mode");
	___mb.mb_get_shortcut = godot::api->godot_method_bind_get_method("BaseButton", "get_shortcut");
	___mb.mb_is_disabled = godot::api->godot_method_bind_get_method("BaseButton", "is_disabled");
	___mb.mb_is_hovered = godot::api->godot_method_bind_get_method("BaseButton", "is_hovered");
	___mb.mb_is_keep_pressed_outside = godot::api->godot_method_bind_get_method("BaseButton", "is_keep_pressed_outside");
	___mb.mb_is_pressed = godot::api->godot_method_bind_get_method("BaseButton", "is_pressed");
	___mb.mb_is_shortcut_in_tooltip_enabled = godot::api->godot_method_bind_get_method("BaseButton", "is_shortcut_in_tooltip_enabled");
	___mb.mb_is_toggle_mode = godot::api->godot_method_bind_get_method("BaseButton", "is_toggle_mode");
	___mb.mb_set_action_mode = godot::api->godot_method_bind_get_method("BaseButton", "set_action_mode");
	___mb.mb_set_button_group = godot::api->godot_method_bind_get_method("BaseButton", "set_button_group");
	___mb.mb_set_button_mask = godot::api->godot_method_bind_get_method("BaseButton", "set_button_mask");
	___mb.mb_set_disabled = godot::api->godot_method_bind_get_method("BaseButton", "set_disabled");
	___mb.mb_set_enabled_focus_mode = godot::api->godot_method_bind_get_method("BaseButton", "set_enabled_focus_mode");
	___mb.mb_set_keep_pressed_outside = godot::api->godot_method_bind_get_method("BaseButton", "set_keep_pressed_outside");
	___mb.mb_set_pressed = godot::api->godot_method_bind_get_method("BaseButton", "set_pressed");
	___mb.mb_set_shortcut = godot::api->godot_method_bind_get_method("BaseButton", "set_shortcut");
	___mb.mb_set_shortcut_in_tooltip = godot::api->godot_method_bind_get_method("BaseButton", "set_shortcut_in_tooltip");
	___mb.mb_set_toggle_mode = godot::api->godot_method_bind_get_method("BaseButton", "set_toggle_mode");
}

void BaseButton::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

void BaseButton::_pressed() {
	___godot_icall_void(___mb.mb__pressed, (const Object *) this);
}

void BaseButton::_toggled(const bool button_pressed) {
	___godot_icall_void_bool(___mb.mb__toggled, (const Object *) this, button_pressed);
}

void BaseButton::_unhandled_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__unhandled_input, (const Object *) this, arg0.ptr());
}

BaseButton::ActionMode BaseButton::get_action_mode() const {
	return (BaseButton::ActionMode) ___godot_icall_int(___mb.mb_get_action_mode, (const Object *) this);
}

Ref<ButtonGroup> BaseButton::get_button_group() const {
	return Ref<ButtonGroup>::__internal_constructor(___godot_icall_Object(___mb.mb_get_button_group, (const Object *) this));
}

int64_t BaseButton::get_button_mask() const {
	return ___godot_icall_int(___mb.mb_get_button_mask, (const Object *) this);
}

BaseButton::DrawMode BaseButton::get_draw_mode() const {
	return (BaseButton::DrawMode) ___godot_icall_int(___mb.mb_get_draw_mode, (const Object *) this);
}

Control::FocusMode BaseButton::get_enabled_focus_mode() const {
	return (Control::FocusMode) ___godot_icall_int(___mb.mb_get_enabled_focus_mode, (const Object *) this);
}

Ref<ShortCut> BaseButton::get_shortcut() const {
	return Ref<ShortCut>::__internal_constructor(___godot_icall_Object(___mb.mb_get_shortcut, (const Object *) this));
}

bool BaseButton::is_disabled() const {
	return ___godot_icall_bool(___mb.mb_is_disabled, (const Object *) this);
}

bool BaseButton::is_hovered() const {
	return ___godot_icall_bool(___mb.mb_is_hovered, (const Object *) this);
}

bool BaseButton::is_keep_pressed_outside() const {
	return ___godot_icall_bool(___mb.mb_is_keep_pressed_outside, (const Object *) this);
}

bool BaseButton::is_pressed() const {
	return ___godot_icall_bool(___mb.mb_is_pressed, (const Object *) this);
}

bool BaseButton::is_shortcut_in_tooltip_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_shortcut_in_tooltip_enabled, (const Object *) this);
}

bool BaseButton::is_toggle_mode() const {
	return ___godot_icall_bool(___mb.mb_is_toggle_mode, (const Object *) this);
}

void BaseButton::set_action_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_action_mode, (const Object *) this, mode);
}

void BaseButton::set_button_group(const Ref<ButtonGroup> button_group) {
	___godot_icall_void_Object(___mb.mb_set_button_group, (const Object *) this, button_group.ptr());
}

void BaseButton::set_button_mask(const int64_t mask) {
	___godot_icall_void_int(___mb.mb_set_button_mask, (const Object *) this, mask);
}

void BaseButton::set_disabled(const bool disabled) {
	___godot_icall_void_bool(___mb.mb_set_disabled, (const Object *) this, disabled);
}

void BaseButton::set_enabled_focus_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_enabled_focus_mode, (const Object *) this, mode);
}

void BaseButton::set_keep_pressed_outside(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_keep_pressed_outside, (const Object *) this, enabled);
}

void BaseButton::set_pressed(const bool pressed) {
	___godot_icall_void_bool(___mb.mb_set_pressed, (const Object *) this, pressed);
}

void BaseButton::set_shortcut(const Ref<ShortCut> shortcut) {
	___godot_icall_void_Object(___mb.mb_set_shortcut, (const Object *) this, shortcut.ptr());
}

void BaseButton::set_shortcut_in_tooltip(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_shortcut_in_tooltip, (const Object *) this, enabled);
}

void BaseButton::set_toggle_mode(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_toggle_mode, (const Object *) this, enabled);
}

}