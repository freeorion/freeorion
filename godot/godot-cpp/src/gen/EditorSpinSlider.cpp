#include "EditorSpinSlider.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"


namespace godot {


EditorSpinSlider::___method_bindings EditorSpinSlider::___mb = {};

void EditorSpinSlider::___init_method_bindings() {
	___mb.mb__grabber_gui_input = godot::api->godot_method_bind_get_method("EditorSpinSlider", "_grabber_gui_input");
	___mb.mb__grabber_mouse_entered = godot::api->godot_method_bind_get_method("EditorSpinSlider", "_grabber_mouse_entered");
	___mb.mb__grabber_mouse_exited = godot::api->godot_method_bind_get_method("EditorSpinSlider", "_grabber_mouse_exited");
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("EditorSpinSlider", "_gui_input");
	___mb.mb__value_focus_exited = godot::api->godot_method_bind_get_method("EditorSpinSlider", "_value_focus_exited");
	___mb.mb__value_input_closed = godot::api->godot_method_bind_get_method("EditorSpinSlider", "_value_input_closed");
	___mb.mb__value_input_entered = godot::api->godot_method_bind_get_method("EditorSpinSlider", "_value_input_entered");
	___mb.mb_get_label = godot::api->godot_method_bind_get_method("EditorSpinSlider", "get_label");
	___mb.mb_is_flat = godot::api->godot_method_bind_get_method("EditorSpinSlider", "is_flat");
	___mb.mb_is_read_only = godot::api->godot_method_bind_get_method("EditorSpinSlider", "is_read_only");
	___mb.mb_set_flat = godot::api->godot_method_bind_get_method("EditorSpinSlider", "set_flat");
	___mb.mb_set_label = godot::api->godot_method_bind_get_method("EditorSpinSlider", "set_label");
	___mb.mb_set_read_only = godot::api->godot_method_bind_get_method("EditorSpinSlider", "set_read_only");
}

void EditorSpinSlider::_grabber_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__grabber_gui_input, (const Object *) this, arg0.ptr());
}

void EditorSpinSlider::_grabber_mouse_entered() {
	___godot_icall_void(___mb.mb__grabber_mouse_entered, (const Object *) this);
}

void EditorSpinSlider::_grabber_mouse_exited() {
	___godot_icall_void(___mb.mb__grabber_mouse_exited, (const Object *) this);
}

void EditorSpinSlider::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

void EditorSpinSlider::_value_focus_exited() {
	___godot_icall_void(___mb.mb__value_focus_exited, (const Object *) this);
}

void EditorSpinSlider::_value_input_closed() {
	___godot_icall_void(___mb.mb__value_input_closed, (const Object *) this);
}

void EditorSpinSlider::_value_input_entered(const String arg0) {
	___godot_icall_void_String(___mb.mb__value_input_entered, (const Object *) this, arg0);
}

String EditorSpinSlider::get_label() const {
	return ___godot_icall_String(___mb.mb_get_label, (const Object *) this);
}

bool EditorSpinSlider::is_flat() const {
	return ___godot_icall_bool(___mb.mb_is_flat, (const Object *) this);
}

bool EditorSpinSlider::is_read_only() const {
	return ___godot_icall_bool(___mb.mb_is_read_only, (const Object *) this);
}

void EditorSpinSlider::set_flat(const bool flat) {
	___godot_icall_void_bool(___mb.mb_set_flat, (const Object *) this, flat);
}

void EditorSpinSlider::set_label(const String label) {
	___godot_icall_void_String(___mb.mb_set_label, (const Object *) this, label);
}

void EditorSpinSlider::set_read_only(const bool read_only) {
	___godot_icall_void_bool(___mb.mb_set_read_only, (const Object *) this, read_only);
}

}