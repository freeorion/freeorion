#include "ScrollContainer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Control.hpp"
#include "InputEvent.hpp"
#include "HScrollBar.hpp"
#include "VScrollBar.hpp"


namespace godot {


ScrollContainer::___method_bindings ScrollContainer::___mb = {};

void ScrollContainer::___init_method_bindings() {
	___mb.mb__ensure_focused_visible = godot::api->godot_method_bind_get_method("ScrollContainer", "_ensure_focused_visible");
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("ScrollContainer", "_gui_input");
	___mb.mb__scroll_moved = godot::api->godot_method_bind_get_method("ScrollContainer", "_scroll_moved");
	___mb.mb__update_scrollbar_position = godot::api->godot_method_bind_get_method("ScrollContainer", "_update_scrollbar_position");
	___mb.mb_get_deadzone = godot::api->godot_method_bind_get_method("ScrollContainer", "get_deadzone");
	___mb.mb_get_h_scroll = godot::api->godot_method_bind_get_method("ScrollContainer", "get_h_scroll");
	___mb.mb_get_h_scrollbar = godot::api->godot_method_bind_get_method("ScrollContainer", "get_h_scrollbar");
	___mb.mb_get_v_scroll = godot::api->godot_method_bind_get_method("ScrollContainer", "get_v_scroll");
	___mb.mb_get_v_scrollbar = godot::api->godot_method_bind_get_method("ScrollContainer", "get_v_scrollbar");
	___mb.mb_is_following_focus = godot::api->godot_method_bind_get_method("ScrollContainer", "is_following_focus");
	___mb.mb_is_h_scroll_enabled = godot::api->godot_method_bind_get_method("ScrollContainer", "is_h_scroll_enabled");
	___mb.mb_is_v_scroll_enabled = godot::api->godot_method_bind_get_method("ScrollContainer", "is_v_scroll_enabled");
	___mb.mb_set_deadzone = godot::api->godot_method_bind_get_method("ScrollContainer", "set_deadzone");
	___mb.mb_set_enable_h_scroll = godot::api->godot_method_bind_get_method("ScrollContainer", "set_enable_h_scroll");
	___mb.mb_set_enable_v_scroll = godot::api->godot_method_bind_get_method("ScrollContainer", "set_enable_v_scroll");
	___mb.mb_set_follow_focus = godot::api->godot_method_bind_get_method("ScrollContainer", "set_follow_focus");
	___mb.mb_set_h_scroll = godot::api->godot_method_bind_get_method("ScrollContainer", "set_h_scroll");
	___mb.mb_set_v_scroll = godot::api->godot_method_bind_get_method("ScrollContainer", "set_v_scroll");
}

ScrollContainer *ScrollContainer::_new()
{
	return (ScrollContainer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ScrollContainer")());
}
void ScrollContainer::_ensure_focused_visible(const Control *arg0) {
	___godot_icall_void_Object(___mb.mb__ensure_focused_visible, (const Object *) this, arg0);
}

void ScrollContainer::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

void ScrollContainer::_scroll_moved(const real_t arg0) {
	___godot_icall_void_float(___mb.mb__scroll_moved, (const Object *) this, arg0);
}

void ScrollContainer::_update_scrollbar_position() {
	___godot_icall_void(___mb.mb__update_scrollbar_position, (const Object *) this);
}

int64_t ScrollContainer::get_deadzone() const {
	return ___godot_icall_int(___mb.mb_get_deadzone, (const Object *) this);
}

int64_t ScrollContainer::get_h_scroll() const {
	return ___godot_icall_int(___mb.mb_get_h_scroll, (const Object *) this);
}

HScrollBar *ScrollContainer::get_h_scrollbar() {
	return (HScrollBar *) ___godot_icall_Object(___mb.mb_get_h_scrollbar, (const Object *) this);
}

int64_t ScrollContainer::get_v_scroll() const {
	return ___godot_icall_int(___mb.mb_get_v_scroll, (const Object *) this);
}

VScrollBar *ScrollContainer::get_v_scrollbar() {
	return (VScrollBar *) ___godot_icall_Object(___mb.mb_get_v_scrollbar, (const Object *) this);
}

bool ScrollContainer::is_following_focus() const {
	return ___godot_icall_bool(___mb.mb_is_following_focus, (const Object *) this);
}

bool ScrollContainer::is_h_scroll_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_h_scroll_enabled, (const Object *) this);
}

bool ScrollContainer::is_v_scroll_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_v_scroll_enabled, (const Object *) this);
}

void ScrollContainer::set_deadzone(const int64_t deadzone) {
	___godot_icall_void_int(___mb.mb_set_deadzone, (const Object *) this, deadzone);
}

void ScrollContainer::set_enable_h_scroll(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_enable_h_scroll, (const Object *) this, enable);
}

void ScrollContainer::set_enable_v_scroll(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_enable_v_scroll, (const Object *) this, enable);
}

void ScrollContainer::set_follow_focus(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_follow_focus, (const Object *) this, enabled);
}

void ScrollContainer::set_h_scroll(const int64_t value) {
	___godot_icall_void_int(___mb.mb_set_h_scroll, (const Object *) this, value);
}

void ScrollContainer::set_v_scroll(const int64_t value) {
	___godot_icall_void_int(___mb.mb_set_v_scroll, (const Object *) this, value);
}

}