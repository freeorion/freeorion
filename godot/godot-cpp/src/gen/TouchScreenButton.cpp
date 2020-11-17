#include "TouchScreenButton.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "BitMap.hpp"
#include "Shape2D.hpp"
#include "Texture.hpp"


namespace godot {


TouchScreenButton::___method_bindings TouchScreenButton::___mb = {};

void TouchScreenButton::___init_method_bindings() {
	___mb.mb__input = godot::api->godot_method_bind_get_method("TouchScreenButton", "_input");
	___mb.mb_get_action = godot::api->godot_method_bind_get_method("TouchScreenButton", "get_action");
	___mb.mb_get_bitmask = godot::api->godot_method_bind_get_method("TouchScreenButton", "get_bitmask");
	___mb.mb_get_shape = godot::api->godot_method_bind_get_method("TouchScreenButton", "get_shape");
	___mb.mb_get_texture = godot::api->godot_method_bind_get_method("TouchScreenButton", "get_texture");
	___mb.mb_get_texture_pressed = godot::api->godot_method_bind_get_method("TouchScreenButton", "get_texture_pressed");
	___mb.mb_get_visibility_mode = godot::api->godot_method_bind_get_method("TouchScreenButton", "get_visibility_mode");
	___mb.mb_is_passby_press_enabled = godot::api->godot_method_bind_get_method("TouchScreenButton", "is_passby_press_enabled");
	___mb.mb_is_pressed = godot::api->godot_method_bind_get_method("TouchScreenButton", "is_pressed");
	___mb.mb_is_shape_centered = godot::api->godot_method_bind_get_method("TouchScreenButton", "is_shape_centered");
	___mb.mb_is_shape_visible = godot::api->godot_method_bind_get_method("TouchScreenButton", "is_shape_visible");
	___mb.mb_set_action = godot::api->godot_method_bind_get_method("TouchScreenButton", "set_action");
	___mb.mb_set_bitmask = godot::api->godot_method_bind_get_method("TouchScreenButton", "set_bitmask");
	___mb.mb_set_passby_press = godot::api->godot_method_bind_get_method("TouchScreenButton", "set_passby_press");
	___mb.mb_set_shape = godot::api->godot_method_bind_get_method("TouchScreenButton", "set_shape");
	___mb.mb_set_shape_centered = godot::api->godot_method_bind_get_method("TouchScreenButton", "set_shape_centered");
	___mb.mb_set_shape_visible = godot::api->godot_method_bind_get_method("TouchScreenButton", "set_shape_visible");
	___mb.mb_set_texture = godot::api->godot_method_bind_get_method("TouchScreenButton", "set_texture");
	___mb.mb_set_texture_pressed = godot::api->godot_method_bind_get_method("TouchScreenButton", "set_texture_pressed");
	___mb.mb_set_visibility_mode = godot::api->godot_method_bind_get_method("TouchScreenButton", "set_visibility_mode");
}

TouchScreenButton *TouchScreenButton::_new()
{
	return (TouchScreenButton *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"TouchScreenButton")());
}
void TouchScreenButton::_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__input, (const Object *) this, arg0.ptr());
}

String TouchScreenButton::get_action() const {
	return ___godot_icall_String(___mb.mb_get_action, (const Object *) this);
}

Ref<BitMap> TouchScreenButton::get_bitmask() const {
	return Ref<BitMap>::__internal_constructor(___godot_icall_Object(___mb.mb_get_bitmask, (const Object *) this));
}

Ref<Shape2D> TouchScreenButton::get_shape() const {
	return Ref<Shape2D>::__internal_constructor(___godot_icall_Object(___mb.mb_get_shape, (const Object *) this));
}

Ref<Texture> TouchScreenButton::get_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_texture, (const Object *) this));
}

Ref<Texture> TouchScreenButton::get_texture_pressed() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_texture_pressed, (const Object *) this));
}

TouchScreenButton::VisibilityMode TouchScreenButton::get_visibility_mode() const {
	return (TouchScreenButton::VisibilityMode) ___godot_icall_int(___mb.mb_get_visibility_mode, (const Object *) this);
}

bool TouchScreenButton::is_passby_press_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_passby_press_enabled, (const Object *) this);
}

bool TouchScreenButton::is_pressed() const {
	return ___godot_icall_bool(___mb.mb_is_pressed, (const Object *) this);
}

bool TouchScreenButton::is_shape_centered() const {
	return ___godot_icall_bool(___mb.mb_is_shape_centered, (const Object *) this);
}

bool TouchScreenButton::is_shape_visible() const {
	return ___godot_icall_bool(___mb.mb_is_shape_visible, (const Object *) this);
}

void TouchScreenButton::set_action(const String action) {
	___godot_icall_void_String(___mb.mb_set_action, (const Object *) this, action);
}

void TouchScreenButton::set_bitmask(const Ref<BitMap> bitmask) {
	___godot_icall_void_Object(___mb.mb_set_bitmask, (const Object *) this, bitmask.ptr());
}

void TouchScreenButton::set_passby_press(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_passby_press, (const Object *) this, enabled);
}

void TouchScreenButton::set_shape(const Ref<Shape2D> shape) {
	___godot_icall_void_Object(___mb.mb_set_shape, (const Object *) this, shape.ptr());
}

void TouchScreenButton::set_shape_centered(const bool _bool) {
	___godot_icall_void_bool(___mb.mb_set_shape_centered, (const Object *) this, _bool);
}

void TouchScreenButton::set_shape_visible(const bool _bool) {
	___godot_icall_void_bool(___mb.mb_set_shape_visible, (const Object *) this, _bool);
}

void TouchScreenButton::set_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_texture, (const Object *) this, texture.ptr());
}

void TouchScreenButton::set_texture_pressed(const Ref<Texture> texture_pressed) {
	___godot_icall_void_Object(___mb.mb_set_texture_pressed, (const Object *) this, texture_pressed.ptr());
}

void TouchScreenButton::set_visibility_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_visibility_mode, (const Object *) this, mode);
}

}