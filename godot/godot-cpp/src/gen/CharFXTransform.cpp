#include "CharFXTransform.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


CharFXTransform::___method_bindings CharFXTransform::___mb = {};

void CharFXTransform::___init_method_bindings() {
	___mb.mb_get_absolute_index = godot::api->godot_method_bind_get_method("CharFXTransform", "get_absolute_index");
	___mb.mb_get_character = godot::api->godot_method_bind_get_method("CharFXTransform", "get_character");
	___mb.mb_get_color = godot::api->godot_method_bind_get_method("CharFXTransform", "get_color");
	___mb.mb_get_elapsed_time = godot::api->godot_method_bind_get_method("CharFXTransform", "get_elapsed_time");
	___mb.mb_get_environment = godot::api->godot_method_bind_get_method("CharFXTransform", "get_environment");
	___mb.mb_get_offset = godot::api->godot_method_bind_get_method("CharFXTransform", "get_offset");
	___mb.mb_get_relative_index = godot::api->godot_method_bind_get_method("CharFXTransform", "get_relative_index");
	___mb.mb_is_visible = godot::api->godot_method_bind_get_method("CharFXTransform", "is_visible");
	___mb.mb_set_absolute_index = godot::api->godot_method_bind_get_method("CharFXTransform", "set_absolute_index");
	___mb.mb_set_character = godot::api->godot_method_bind_get_method("CharFXTransform", "set_character");
	___mb.mb_set_color = godot::api->godot_method_bind_get_method("CharFXTransform", "set_color");
	___mb.mb_set_elapsed_time = godot::api->godot_method_bind_get_method("CharFXTransform", "set_elapsed_time");
	___mb.mb_set_environment = godot::api->godot_method_bind_get_method("CharFXTransform", "set_environment");
	___mb.mb_set_offset = godot::api->godot_method_bind_get_method("CharFXTransform", "set_offset");
	___mb.mb_set_relative_index = godot::api->godot_method_bind_get_method("CharFXTransform", "set_relative_index");
	___mb.mb_set_visibility = godot::api->godot_method_bind_get_method("CharFXTransform", "set_visibility");
}

CharFXTransform *CharFXTransform::_new()
{
	return (CharFXTransform *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CharFXTransform")());
}
int64_t CharFXTransform::get_absolute_index() {
	return ___godot_icall_int(___mb.mb_get_absolute_index, (const Object *) this);
}

int64_t CharFXTransform::get_character() {
	return ___godot_icall_int(___mb.mb_get_character, (const Object *) this);
}

Color CharFXTransform::get_color() {
	return ___godot_icall_Color(___mb.mb_get_color, (const Object *) this);
}

real_t CharFXTransform::get_elapsed_time() {
	return ___godot_icall_float(___mb.mb_get_elapsed_time, (const Object *) this);
}

Dictionary CharFXTransform::get_environment() {
	return ___godot_icall_Dictionary(___mb.mb_get_environment, (const Object *) this);
}

Vector2 CharFXTransform::get_offset() {
	return ___godot_icall_Vector2(___mb.mb_get_offset, (const Object *) this);
}

int64_t CharFXTransform::get_relative_index() {
	return ___godot_icall_int(___mb.mb_get_relative_index, (const Object *) this);
}

bool CharFXTransform::is_visible() {
	return ___godot_icall_bool(___mb.mb_is_visible, (const Object *) this);
}

void CharFXTransform::set_absolute_index(const int64_t index) {
	___godot_icall_void_int(___mb.mb_set_absolute_index, (const Object *) this, index);
}

void CharFXTransform::set_character(const int64_t character) {
	___godot_icall_void_int(___mb.mb_set_character, (const Object *) this, character);
}

void CharFXTransform::set_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_color, (const Object *) this, color);
}

void CharFXTransform::set_elapsed_time(const real_t time) {
	___godot_icall_void_float(___mb.mb_set_elapsed_time, (const Object *) this, time);
}

void CharFXTransform::set_environment(const Dictionary environment) {
	___godot_icall_void_Dictionary(___mb.mb_set_environment, (const Object *) this, environment);
}

void CharFXTransform::set_offset(const Vector2 offset) {
	___godot_icall_void_Vector2(___mb.mb_set_offset, (const Object *) this, offset);
}

void CharFXTransform::set_relative_index(const int64_t index) {
	___godot_icall_void_int(___mb.mb_set_relative_index, (const Object *) this, index);
}

void CharFXTransform::set_visibility(const bool visibility) {
	___godot_icall_void_bool(___mb.mb_set_visibility, (const Object *) this, visibility);
}

}