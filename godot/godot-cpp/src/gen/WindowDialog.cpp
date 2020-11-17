#include "WindowDialog.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "TextureButton.hpp"


namespace godot {


WindowDialog::___method_bindings WindowDialog::___mb = {};

void WindowDialog::___init_method_bindings() {
	___mb.mb__closed = godot::api->godot_method_bind_get_method("WindowDialog", "_closed");
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("WindowDialog", "_gui_input");
	___mb.mb_get_close_button = godot::api->godot_method_bind_get_method("WindowDialog", "get_close_button");
	___mb.mb_get_resizable = godot::api->godot_method_bind_get_method("WindowDialog", "get_resizable");
	___mb.mb_get_title = godot::api->godot_method_bind_get_method("WindowDialog", "get_title");
	___mb.mb_set_resizable = godot::api->godot_method_bind_get_method("WindowDialog", "set_resizable");
	___mb.mb_set_title = godot::api->godot_method_bind_get_method("WindowDialog", "set_title");
}

WindowDialog *WindowDialog::_new()
{
	return (WindowDialog *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"WindowDialog")());
}
void WindowDialog::_closed() {
	___godot_icall_void(___mb.mb__closed, (const Object *) this);
}

void WindowDialog::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

TextureButton *WindowDialog::get_close_button() {
	return (TextureButton *) ___godot_icall_Object(___mb.mb_get_close_button, (const Object *) this);
}

bool WindowDialog::get_resizable() const {
	return ___godot_icall_bool(___mb.mb_get_resizable, (const Object *) this);
}

String WindowDialog::get_title() const {
	return ___godot_icall_String(___mb.mb_get_title, (const Object *) this);
}

void WindowDialog::set_resizable(const bool resizable) {
	___godot_icall_void_bool(___mb.mb_set_resizable, (const Object *) this, resizable);
}

void WindowDialog::set_title(const String title) {
	___godot_icall_void_String(___mb.mb_set_title, (const Object *) this, title);
}

}