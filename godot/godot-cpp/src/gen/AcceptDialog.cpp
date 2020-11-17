#include "AcceptDialog.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Button.hpp"
#include "Label.hpp"
#include "Node.hpp"


namespace godot {


AcceptDialog::___method_bindings AcceptDialog::___mb = {};

void AcceptDialog::___init_method_bindings() {
	___mb.mb__builtin_text_entered = godot::api->godot_method_bind_get_method("AcceptDialog", "_builtin_text_entered");
	___mb.mb__custom_action = godot::api->godot_method_bind_get_method("AcceptDialog", "_custom_action");
	___mb.mb__ok = godot::api->godot_method_bind_get_method("AcceptDialog", "_ok");
	___mb.mb_add_button = godot::api->godot_method_bind_get_method("AcceptDialog", "add_button");
	___mb.mb_add_cancel = godot::api->godot_method_bind_get_method("AcceptDialog", "add_cancel");
	___mb.mb_get_hide_on_ok = godot::api->godot_method_bind_get_method("AcceptDialog", "get_hide_on_ok");
	___mb.mb_get_label = godot::api->godot_method_bind_get_method("AcceptDialog", "get_label");
	___mb.mb_get_ok = godot::api->godot_method_bind_get_method("AcceptDialog", "get_ok");
	___mb.mb_get_text = godot::api->godot_method_bind_get_method("AcceptDialog", "get_text");
	___mb.mb_has_autowrap = godot::api->godot_method_bind_get_method("AcceptDialog", "has_autowrap");
	___mb.mb_register_text_enter = godot::api->godot_method_bind_get_method("AcceptDialog", "register_text_enter");
	___mb.mb_set_autowrap = godot::api->godot_method_bind_get_method("AcceptDialog", "set_autowrap");
	___mb.mb_set_hide_on_ok = godot::api->godot_method_bind_get_method("AcceptDialog", "set_hide_on_ok");
	___mb.mb_set_text = godot::api->godot_method_bind_get_method("AcceptDialog", "set_text");
}

AcceptDialog *AcceptDialog::_new()
{
	return (AcceptDialog *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AcceptDialog")());
}
void AcceptDialog::_builtin_text_entered(const String arg0) {
	___godot_icall_void_String(___mb.mb__builtin_text_entered, (const Object *) this, arg0);
}

void AcceptDialog::_custom_action(const String arg0) {
	___godot_icall_void_String(___mb.mb__custom_action, (const Object *) this, arg0);
}

void AcceptDialog::_ok() {
	___godot_icall_void(___mb.mb__ok, (const Object *) this);
}

Button *AcceptDialog::add_button(const String text, const bool right, const String action) {
	return (Button *) ___godot_icall_Object_String_bool_String(___mb.mb_add_button, (const Object *) this, text, right, action);
}

Button *AcceptDialog::add_cancel(const String name) {
	return (Button *) ___godot_icall_Object_String(___mb.mb_add_cancel, (const Object *) this, name);
}

bool AcceptDialog::get_hide_on_ok() const {
	return ___godot_icall_bool(___mb.mb_get_hide_on_ok, (const Object *) this);
}

Label *AcceptDialog::get_label() {
	return (Label *) ___godot_icall_Object(___mb.mb_get_label, (const Object *) this);
}

Button *AcceptDialog::get_ok() {
	return (Button *) ___godot_icall_Object(___mb.mb_get_ok, (const Object *) this);
}

String AcceptDialog::get_text() const {
	return ___godot_icall_String(___mb.mb_get_text, (const Object *) this);
}

bool AcceptDialog::has_autowrap() {
	return ___godot_icall_bool(___mb.mb_has_autowrap, (const Object *) this);
}

void AcceptDialog::register_text_enter(const Node *line_edit) {
	___godot_icall_void_Object(___mb.mb_register_text_enter, (const Object *) this, line_edit);
}

void AcceptDialog::set_autowrap(const bool autowrap) {
	___godot_icall_void_bool(___mb.mb_set_autowrap, (const Object *) this, autowrap);
}

void AcceptDialog::set_hide_on_ok(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_hide_on_ok, (const Object *) this, enabled);
}

void AcceptDialog::set_text(const String text) {
	___godot_icall_void_String(___mb.mb_set_text, (const Object *) this, text);
}

}