#include "MenuButton.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "PopupMenu.hpp"


namespace godot {


MenuButton::___method_bindings MenuButton::___mb = {};

void MenuButton::___init_method_bindings() {
	___mb.mb__get_items = godot::api->godot_method_bind_get_method("MenuButton", "_get_items");
	___mb.mb__set_items = godot::api->godot_method_bind_get_method("MenuButton", "_set_items");
	___mb.mb__unhandled_key_input = godot::api->godot_method_bind_get_method("MenuButton", "_unhandled_key_input");
	___mb.mb_get_popup = godot::api->godot_method_bind_get_method("MenuButton", "get_popup");
	___mb.mb_is_switch_on_hover = godot::api->godot_method_bind_get_method("MenuButton", "is_switch_on_hover");
	___mb.mb_set_disable_shortcuts = godot::api->godot_method_bind_get_method("MenuButton", "set_disable_shortcuts");
	___mb.mb_set_switch_on_hover = godot::api->godot_method_bind_get_method("MenuButton", "set_switch_on_hover");
}

MenuButton *MenuButton::_new()
{
	return (MenuButton *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"MenuButton")());
}
Array MenuButton::_get_items() const {
	return ___godot_icall_Array(___mb.mb__get_items, (const Object *) this);
}

void MenuButton::_set_items(const Array arg0) {
	___godot_icall_void_Array(___mb.mb__set_items, (const Object *) this, arg0);
}

void MenuButton::_unhandled_key_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__unhandled_key_input, (const Object *) this, arg0.ptr());
}

PopupMenu *MenuButton::get_popup() const {
	return (PopupMenu *) ___godot_icall_Object(___mb.mb_get_popup, (const Object *) this);
}

bool MenuButton::is_switch_on_hover() {
	return ___godot_icall_bool(___mb.mb_is_switch_on_hover, (const Object *) this);
}

void MenuButton::set_disable_shortcuts(const bool disabled) {
	___godot_icall_void_bool(___mb.mb_set_disable_shortcuts, (const Object *) this, disabled);
}

void MenuButton::set_switch_on_hover(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_switch_on_hover, (const Object *) this, enable);
}

}