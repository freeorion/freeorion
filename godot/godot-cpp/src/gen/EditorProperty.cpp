#include "EditorProperty.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "Control.hpp"
#include "Object.hpp"


namespace godot {


EditorProperty::___method_bindings EditorProperty::___mb = {};

void EditorProperty::___init_method_bindings() {
	___mb.mb__focusable_focused = godot::api->godot_method_bind_get_method("EditorProperty", "_focusable_focused");
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("EditorProperty", "_gui_input");
	___mb.mb_add_focusable = godot::api->godot_method_bind_get_method("EditorProperty", "add_focusable");
	___mb.mb_emit_changed = godot::api->godot_method_bind_get_method("EditorProperty", "emit_changed");
	___mb.mb_get_edited_object = godot::api->godot_method_bind_get_method("EditorProperty", "get_edited_object");
	___mb.mb_get_edited_property = godot::api->godot_method_bind_get_method("EditorProperty", "get_edited_property");
	___mb.mb_get_label = godot::api->godot_method_bind_get_method("EditorProperty", "get_label");
	___mb.mb_get_tooltip_text = godot::api->godot_method_bind_get_method("EditorProperty", "get_tooltip_text");
	___mb.mb_is_checkable = godot::api->godot_method_bind_get_method("EditorProperty", "is_checkable");
	___mb.mb_is_checked = godot::api->godot_method_bind_get_method("EditorProperty", "is_checked");
	___mb.mb_is_draw_red = godot::api->godot_method_bind_get_method("EditorProperty", "is_draw_red");
	___mb.mb_is_keying = godot::api->godot_method_bind_get_method("EditorProperty", "is_keying");
	___mb.mb_is_read_only = godot::api->godot_method_bind_get_method("EditorProperty", "is_read_only");
	___mb.mb_set_bottom_editor = godot::api->godot_method_bind_get_method("EditorProperty", "set_bottom_editor");
	___mb.mb_set_checkable = godot::api->godot_method_bind_get_method("EditorProperty", "set_checkable");
	___mb.mb_set_checked = godot::api->godot_method_bind_get_method("EditorProperty", "set_checked");
	___mb.mb_set_draw_red = godot::api->godot_method_bind_get_method("EditorProperty", "set_draw_red");
	___mb.mb_set_keying = godot::api->godot_method_bind_get_method("EditorProperty", "set_keying");
	___mb.mb_set_label = godot::api->godot_method_bind_get_method("EditorProperty", "set_label");
	___mb.mb_set_read_only = godot::api->godot_method_bind_get_method("EditorProperty", "set_read_only");
	___mb.mb_update_property = godot::api->godot_method_bind_get_method("EditorProperty", "update_property");
}

void EditorProperty::_focusable_focused(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__focusable_focused, (const Object *) this, arg0);
}

void EditorProperty::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

void EditorProperty::add_focusable(const Control *control) {
	___godot_icall_void_Object(___mb.mb_add_focusable, (const Object *) this, control);
}

void EditorProperty::emit_changed(const String property, const Variant value, const String field, const bool changing) {
	___godot_icall_void_String_Variant_String_bool(___mb.mb_emit_changed, (const Object *) this, property, value, field, changing);
}

Object *EditorProperty::get_edited_object() {
	return (Object *) ___godot_icall_Object(___mb.mb_get_edited_object, (const Object *) this);
}

String EditorProperty::get_edited_property() {
	return ___godot_icall_String(___mb.mb_get_edited_property, (const Object *) this);
}

String EditorProperty::get_label() const {
	return ___godot_icall_String(___mb.mb_get_label, (const Object *) this);
}

String EditorProperty::get_tooltip_text() const {
	return ___godot_icall_String(___mb.mb_get_tooltip_text, (const Object *) this);
}

bool EditorProperty::is_checkable() const {
	return ___godot_icall_bool(___mb.mb_is_checkable, (const Object *) this);
}

bool EditorProperty::is_checked() const {
	return ___godot_icall_bool(___mb.mb_is_checked, (const Object *) this);
}

bool EditorProperty::is_draw_red() const {
	return ___godot_icall_bool(___mb.mb_is_draw_red, (const Object *) this);
}

bool EditorProperty::is_keying() const {
	return ___godot_icall_bool(___mb.mb_is_keying, (const Object *) this);
}

bool EditorProperty::is_read_only() const {
	return ___godot_icall_bool(___mb.mb_is_read_only, (const Object *) this);
}

void EditorProperty::set_bottom_editor(const Control *editor) {
	___godot_icall_void_Object(___mb.mb_set_bottom_editor, (const Object *) this, editor);
}

void EditorProperty::set_checkable(const bool checkable) {
	___godot_icall_void_bool(___mb.mb_set_checkable, (const Object *) this, checkable);
}

void EditorProperty::set_checked(const bool checked) {
	___godot_icall_void_bool(___mb.mb_set_checked, (const Object *) this, checked);
}

void EditorProperty::set_draw_red(const bool draw_red) {
	___godot_icall_void_bool(___mb.mb_set_draw_red, (const Object *) this, draw_red);
}

void EditorProperty::set_keying(const bool keying) {
	___godot_icall_void_bool(___mb.mb_set_keying, (const Object *) this, keying);
}

void EditorProperty::set_label(const String text) {
	___godot_icall_void_String(___mb.mb_set_label, (const Object *) this, text);
}

void EditorProperty::set_read_only(const bool read_only) {
	___godot_icall_void_bool(___mb.mb_set_read_only, (const Object *) this, read_only);
}

void EditorProperty::update_property() {
	___godot_icall_void(___mb.mb_update_property, (const Object *) this);
}

}