#include "Button.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


Button::___method_bindings Button::___mb = {};

void Button::___init_method_bindings() {
	___mb.mb_get_button_icon = godot::api->godot_method_bind_get_method("Button", "get_button_icon");
	___mb.mb_get_clip_text = godot::api->godot_method_bind_get_method("Button", "get_clip_text");
	___mb.mb_get_text = godot::api->godot_method_bind_get_method("Button", "get_text");
	___mb.mb_get_text_align = godot::api->godot_method_bind_get_method("Button", "get_text_align");
	___mb.mb_is_expand_icon = godot::api->godot_method_bind_get_method("Button", "is_expand_icon");
	___mb.mb_is_flat = godot::api->godot_method_bind_get_method("Button", "is_flat");
	___mb.mb_set_button_icon = godot::api->godot_method_bind_get_method("Button", "set_button_icon");
	___mb.mb_set_clip_text = godot::api->godot_method_bind_get_method("Button", "set_clip_text");
	___mb.mb_set_expand_icon = godot::api->godot_method_bind_get_method("Button", "set_expand_icon");
	___mb.mb_set_flat = godot::api->godot_method_bind_get_method("Button", "set_flat");
	___mb.mb_set_text = godot::api->godot_method_bind_get_method("Button", "set_text");
	___mb.mb_set_text_align = godot::api->godot_method_bind_get_method("Button", "set_text_align");
}

Button *Button::_new()
{
	return (Button *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Button")());
}
Ref<Texture> Button::get_button_icon() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_button_icon, (const Object *) this));
}

bool Button::get_clip_text() const {
	return ___godot_icall_bool(___mb.mb_get_clip_text, (const Object *) this);
}

String Button::get_text() const {
	return ___godot_icall_String(___mb.mb_get_text, (const Object *) this);
}

Button::TextAlign Button::get_text_align() const {
	return (Button::TextAlign) ___godot_icall_int(___mb.mb_get_text_align, (const Object *) this);
}

bool Button::is_expand_icon() const {
	return ___godot_icall_bool(___mb.mb_is_expand_icon, (const Object *) this);
}

bool Button::is_flat() const {
	return ___godot_icall_bool(___mb.mb_is_flat, (const Object *) this);
}

void Button::set_button_icon(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_button_icon, (const Object *) this, texture.ptr());
}

void Button::set_clip_text(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_clip_text, (const Object *) this, enabled);
}

void Button::set_expand_icon(const bool arg0) {
	___godot_icall_void_bool(___mb.mb_set_expand_icon, (const Object *) this, arg0);
}

void Button::set_flat(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_flat, (const Object *) this, enabled);
}

void Button::set_text(const String text) {
	___godot_icall_void_String(___mb.mb_set_text, (const Object *) this, text);
}

void Button::set_text_align(const int64_t align) {
	___godot_icall_void_int(___mb.mb_set_text_align, (const Object *) this, align);
}

}