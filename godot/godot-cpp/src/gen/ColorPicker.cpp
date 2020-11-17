#include "ColorPicker.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Control.hpp"
#include "InputEvent.hpp"


namespace godot {


ColorPicker::___method_bindings ColorPicker::___mb = {};

void ColorPicker::___init_method_bindings() {
	___mb.mb__add_preset_pressed = godot::api->godot_method_bind_get_method("ColorPicker", "_add_preset_pressed");
	___mb.mb__focus_enter = godot::api->godot_method_bind_get_method("ColorPicker", "_focus_enter");
	___mb.mb__focus_exit = godot::api->godot_method_bind_get_method("ColorPicker", "_focus_exit");
	___mb.mb__hsv_draw = godot::api->godot_method_bind_get_method("ColorPicker", "_hsv_draw");
	___mb.mb__html_entered = godot::api->godot_method_bind_get_method("ColorPicker", "_html_entered");
	___mb.mb__html_focus_exit = godot::api->godot_method_bind_get_method("ColorPicker", "_html_focus_exit");
	___mb.mb__preset_input = godot::api->godot_method_bind_get_method("ColorPicker", "_preset_input");
	___mb.mb__sample_draw = godot::api->godot_method_bind_get_method("ColorPicker", "_sample_draw");
	___mb.mb__screen_input = godot::api->godot_method_bind_get_method("ColorPicker", "_screen_input");
	___mb.mb__screen_pick_pressed = godot::api->godot_method_bind_get_method("ColorPicker", "_screen_pick_pressed");
	___mb.mb__text_type_toggled = godot::api->godot_method_bind_get_method("ColorPicker", "_text_type_toggled");
	___mb.mb__update_presets = godot::api->godot_method_bind_get_method("ColorPicker", "_update_presets");
	___mb.mb__uv_input = godot::api->godot_method_bind_get_method("ColorPicker", "_uv_input");
	___mb.mb__value_changed = godot::api->godot_method_bind_get_method("ColorPicker", "_value_changed");
	___mb.mb__w_input = godot::api->godot_method_bind_get_method("ColorPicker", "_w_input");
	___mb.mb_add_preset = godot::api->godot_method_bind_get_method("ColorPicker", "add_preset");
	___mb.mb_are_presets_enabled = godot::api->godot_method_bind_get_method("ColorPicker", "are_presets_enabled");
	___mb.mb_are_presets_visible = godot::api->godot_method_bind_get_method("ColorPicker", "are_presets_visible");
	___mb.mb_erase_preset = godot::api->godot_method_bind_get_method("ColorPicker", "erase_preset");
	___mb.mb_get_pick_color = godot::api->godot_method_bind_get_method("ColorPicker", "get_pick_color");
	___mb.mb_get_presets = godot::api->godot_method_bind_get_method("ColorPicker", "get_presets");
	___mb.mb_is_deferred_mode = godot::api->godot_method_bind_get_method("ColorPicker", "is_deferred_mode");
	___mb.mb_is_editing_alpha = godot::api->godot_method_bind_get_method("ColorPicker", "is_editing_alpha");
	___mb.mb_is_hsv_mode = godot::api->godot_method_bind_get_method("ColorPicker", "is_hsv_mode");
	___mb.mb_is_raw_mode = godot::api->godot_method_bind_get_method("ColorPicker", "is_raw_mode");
	___mb.mb_set_deferred_mode = godot::api->godot_method_bind_get_method("ColorPicker", "set_deferred_mode");
	___mb.mb_set_edit_alpha = godot::api->godot_method_bind_get_method("ColorPicker", "set_edit_alpha");
	___mb.mb_set_hsv_mode = godot::api->godot_method_bind_get_method("ColorPicker", "set_hsv_mode");
	___mb.mb_set_pick_color = godot::api->godot_method_bind_get_method("ColorPicker", "set_pick_color");
	___mb.mb_set_presets_enabled = godot::api->godot_method_bind_get_method("ColorPicker", "set_presets_enabled");
	___mb.mb_set_presets_visible = godot::api->godot_method_bind_get_method("ColorPicker", "set_presets_visible");
	___mb.mb_set_raw_mode = godot::api->godot_method_bind_get_method("ColorPicker", "set_raw_mode");
}

ColorPicker *ColorPicker::_new()
{
	return (ColorPicker *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ColorPicker")());
}
void ColorPicker::_add_preset_pressed() {
	___godot_icall_void(___mb.mb__add_preset_pressed, (const Object *) this);
}

void ColorPicker::_focus_enter() {
	___godot_icall_void(___mb.mb__focus_enter, (const Object *) this);
}

void ColorPicker::_focus_exit() {
	___godot_icall_void(___mb.mb__focus_exit, (const Object *) this);
}

void ColorPicker::_hsv_draw(const int64_t arg0, const Control *arg1) {
	___godot_icall_void_int_Object(___mb.mb__hsv_draw, (const Object *) this, arg0, arg1);
}

void ColorPicker::_html_entered(const String arg0) {
	___godot_icall_void_String(___mb.mb__html_entered, (const Object *) this, arg0);
}

void ColorPicker::_html_focus_exit() {
	___godot_icall_void(___mb.mb__html_focus_exit, (const Object *) this);
}

void ColorPicker::_preset_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__preset_input, (const Object *) this, arg0.ptr());
}

void ColorPicker::_sample_draw() {
	___godot_icall_void(___mb.mb__sample_draw, (const Object *) this);
}

void ColorPicker::_screen_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__screen_input, (const Object *) this, arg0.ptr());
}

void ColorPicker::_screen_pick_pressed() {
	___godot_icall_void(___mb.mb__screen_pick_pressed, (const Object *) this);
}

void ColorPicker::_text_type_toggled() {
	___godot_icall_void(___mb.mb__text_type_toggled, (const Object *) this);
}

void ColorPicker::_update_presets() {
	___godot_icall_void(___mb.mb__update_presets, (const Object *) this);
}

void ColorPicker::_uv_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__uv_input, (const Object *) this, arg0.ptr());
}

void ColorPicker::_value_changed(const real_t arg0) {
	___godot_icall_void_float(___mb.mb__value_changed, (const Object *) this, arg0);
}

void ColorPicker::_w_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__w_input, (const Object *) this, arg0.ptr());
}

void ColorPicker::add_preset(const Color color) {
	___godot_icall_void_Color(___mb.mb_add_preset, (const Object *) this, color);
}

bool ColorPicker::are_presets_enabled() const {
	return ___godot_icall_bool(___mb.mb_are_presets_enabled, (const Object *) this);
}

bool ColorPicker::are_presets_visible() const {
	return ___godot_icall_bool(___mb.mb_are_presets_visible, (const Object *) this);
}

void ColorPicker::erase_preset(const Color color) {
	___godot_icall_void_Color(___mb.mb_erase_preset, (const Object *) this, color);
}

Color ColorPicker::get_pick_color() const {
	return ___godot_icall_Color(___mb.mb_get_pick_color, (const Object *) this);
}

PoolColorArray ColorPicker::get_presets() const {
	return ___godot_icall_PoolColorArray(___mb.mb_get_presets, (const Object *) this);
}

bool ColorPicker::is_deferred_mode() const {
	return ___godot_icall_bool(___mb.mb_is_deferred_mode, (const Object *) this);
}

bool ColorPicker::is_editing_alpha() const {
	return ___godot_icall_bool(___mb.mb_is_editing_alpha, (const Object *) this);
}

bool ColorPicker::is_hsv_mode() const {
	return ___godot_icall_bool(___mb.mb_is_hsv_mode, (const Object *) this);
}

bool ColorPicker::is_raw_mode() const {
	return ___godot_icall_bool(___mb.mb_is_raw_mode, (const Object *) this);
}

void ColorPicker::set_deferred_mode(const bool mode) {
	___godot_icall_void_bool(___mb.mb_set_deferred_mode, (const Object *) this, mode);
}

void ColorPicker::set_edit_alpha(const bool show) {
	___godot_icall_void_bool(___mb.mb_set_edit_alpha, (const Object *) this, show);
}

void ColorPicker::set_hsv_mode(const bool mode) {
	___godot_icall_void_bool(___mb.mb_set_hsv_mode, (const Object *) this, mode);
}

void ColorPicker::set_pick_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_pick_color, (const Object *) this, color);
}

void ColorPicker::set_presets_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_presets_enabled, (const Object *) this, enabled);
}

void ColorPicker::set_presets_visible(const bool visible) {
	___godot_icall_void_bool(___mb.mb_set_presets_visible, (const Object *) this, visible);
}

void ColorPicker::set_raw_mode(const bool mode) {
	___godot_icall_void_bool(___mb.mb_set_raw_mode, (const Object *) this, mode);
}

}