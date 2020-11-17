#include "ColorPickerButton.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "ColorPicker.hpp"
#include "PopupPanel.hpp"


namespace godot {


ColorPickerButton::___method_bindings ColorPickerButton::___mb = {};

void ColorPickerButton::___init_method_bindings() {
	___mb.mb__color_changed = godot::api->godot_method_bind_get_method("ColorPickerButton", "_color_changed");
	___mb.mb__modal_closed = godot::api->godot_method_bind_get_method("ColorPickerButton", "_modal_closed");
	___mb.mb_get_pick_color = godot::api->godot_method_bind_get_method("ColorPickerButton", "get_pick_color");
	___mb.mb_get_picker = godot::api->godot_method_bind_get_method("ColorPickerButton", "get_picker");
	___mb.mb_get_popup = godot::api->godot_method_bind_get_method("ColorPickerButton", "get_popup");
	___mb.mb_is_editing_alpha = godot::api->godot_method_bind_get_method("ColorPickerButton", "is_editing_alpha");
	___mb.mb_set_edit_alpha = godot::api->godot_method_bind_get_method("ColorPickerButton", "set_edit_alpha");
	___mb.mb_set_pick_color = godot::api->godot_method_bind_get_method("ColorPickerButton", "set_pick_color");
}

ColorPickerButton *ColorPickerButton::_new()
{
	return (ColorPickerButton *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ColorPickerButton")());
}
void ColorPickerButton::_color_changed(const Color arg0) {
	___godot_icall_void_Color(___mb.mb__color_changed, (const Object *) this, arg0);
}

void ColorPickerButton::_modal_closed() {
	___godot_icall_void(___mb.mb__modal_closed, (const Object *) this);
}

Color ColorPickerButton::get_pick_color() const {
	return ___godot_icall_Color(___mb.mb_get_pick_color, (const Object *) this);
}

ColorPicker *ColorPickerButton::get_picker() {
	return (ColorPicker *) ___godot_icall_Object(___mb.mb_get_picker, (const Object *) this);
}

PopupPanel *ColorPickerButton::get_popup() {
	return (PopupPanel *) ___godot_icall_Object(___mb.mb_get_popup, (const Object *) this);
}

bool ColorPickerButton::is_editing_alpha() const {
	return ___godot_icall_bool(___mb.mb_is_editing_alpha, (const Object *) this);
}

void ColorPickerButton::set_edit_alpha(const bool show) {
	___godot_icall_void_bool(___mb.mb_set_edit_alpha, (const Object *) this, show);
}

void ColorPickerButton::set_pick_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_pick_color, (const Object *) this, color);
}

}