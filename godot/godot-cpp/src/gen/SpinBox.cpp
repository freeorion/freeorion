#include "SpinBox.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "LineEdit.hpp"


namespace godot {


SpinBox::___method_bindings SpinBox::___mb = {};

void SpinBox::___init_method_bindings() {
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("SpinBox", "_gui_input");
	___mb.mb__line_edit_focus_exit = godot::api->godot_method_bind_get_method("SpinBox", "_line_edit_focus_exit");
	___mb.mb__line_edit_input = godot::api->godot_method_bind_get_method("SpinBox", "_line_edit_input");
	___mb.mb__range_click_timeout = godot::api->godot_method_bind_get_method("SpinBox", "_range_click_timeout");
	___mb.mb__text_entered = godot::api->godot_method_bind_get_method("SpinBox", "_text_entered");
	___mb.mb_apply = godot::api->godot_method_bind_get_method("SpinBox", "apply");
	___mb.mb_get_align = godot::api->godot_method_bind_get_method("SpinBox", "get_align");
	___mb.mb_get_line_edit = godot::api->godot_method_bind_get_method("SpinBox", "get_line_edit");
	___mb.mb_get_prefix = godot::api->godot_method_bind_get_method("SpinBox", "get_prefix");
	___mb.mb_get_suffix = godot::api->godot_method_bind_get_method("SpinBox", "get_suffix");
	___mb.mb_is_editable = godot::api->godot_method_bind_get_method("SpinBox", "is_editable");
	___mb.mb_set_align = godot::api->godot_method_bind_get_method("SpinBox", "set_align");
	___mb.mb_set_editable = godot::api->godot_method_bind_get_method("SpinBox", "set_editable");
	___mb.mb_set_prefix = godot::api->godot_method_bind_get_method("SpinBox", "set_prefix");
	___mb.mb_set_suffix = godot::api->godot_method_bind_get_method("SpinBox", "set_suffix");
}

SpinBox *SpinBox::_new()
{
	return (SpinBox *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"SpinBox")());
}
void SpinBox::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

void SpinBox::_line_edit_focus_exit() {
	___godot_icall_void(___mb.mb__line_edit_focus_exit, (const Object *) this);
}

void SpinBox::_line_edit_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__line_edit_input, (const Object *) this, arg0.ptr());
}

void SpinBox::_range_click_timeout() {
	___godot_icall_void(___mb.mb__range_click_timeout, (const Object *) this);
}

void SpinBox::_text_entered(const String arg0) {
	___godot_icall_void_String(___mb.mb__text_entered, (const Object *) this, arg0);
}

void SpinBox::apply() {
	___godot_icall_void(___mb.mb_apply, (const Object *) this);
}

LineEdit::Align SpinBox::get_align() const {
	return (LineEdit::Align) ___godot_icall_int(___mb.mb_get_align, (const Object *) this);
}

LineEdit *SpinBox::get_line_edit() {
	return (LineEdit *) ___godot_icall_Object(___mb.mb_get_line_edit, (const Object *) this);
}

String SpinBox::get_prefix() const {
	return ___godot_icall_String(___mb.mb_get_prefix, (const Object *) this);
}

String SpinBox::get_suffix() const {
	return ___godot_icall_String(___mb.mb_get_suffix, (const Object *) this);
}

bool SpinBox::is_editable() const {
	return ___godot_icall_bool(___mb.mb_is_editable, (const Object *) this);
}

void SpinBox::set_align(const int64_t align) {
	___godot_icall_void_int(___mb.mb_set_align, (const Object *) this, align);
}

void SpinBox::set_editable(const bool editable) {
	___godot_icall_void_bool(___mb.mb_set_editable, (const Object *) this, editable);
}

void SpinBox::set_prefix(const String prefix) {
	___godot_icall_void_String(___mb.mb_set_prefix, (const Object *) this, prefix);
}

void SpinBox::set_suffix(const String suffix) {
	___godot_icall_void_String(___mb.mb_set_suffix, (const Object *) this, suffix);
}

}