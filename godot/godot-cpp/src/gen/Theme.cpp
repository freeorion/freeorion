#include "Theme.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Theme.hpp"
#include "Font.hpp"
#include "Texture.hpp"
#include "StyleBox.hpp"


namespace godot {


Theme::___method_bindings Theme::___mb = {};

void Theme::___init_method_bindings() {
	___mb.mb__emit_theme_changed = godot::api->godot_method_bind_get_method("Theme", "_emit_theme_changed");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("Theme", "clear");
	___mb.mb_clear_color = godot::api->godot_method_bind_get_method("Theme", "clear_color");
	___mb.mb_clear_constant = godot::api->godot_method_bind_get_method("Theme", "clear_constant");
	___mb.mb_clear_font = godot::api->godot_method_bind_get_method("Theme", "clear_font");
	___mb.mb_clear_icon = godot::api->godot_method_bind_get_method("Theme", "clear_icon");
	___mb.mb_clear_stylebox = godot::api->godot_method_bind_get_method("Theme", "clear_stylebox");
	___mb.mb_copy_default_theme = godot::api->godot_method_bind_get_method("Theme", "copy_default_theme");
	___mb.mb_copy_theme = godot::api->godot_method_bind_get_method("Theme", "copy_theme");
	___mb.mb_get_color = godot::api->godot_method_bind_get_method("Theme", "get_color");
	___mb.mb_get_color_list = godot::api->godot_method_bind_get_method("Theme", "get_color_list");
	___mb.mb_get_constant = godot::api->godot_method_bind_get_method("Theme", "get_constant");
	___mb.mb_get_constant_list = godot::api->godot_method_bind_get_method("Theme", "get_constant_list");
	___mb.mb_get_default_font = godot::api->godot_method_bind_get_method("Theme", "get_default_font");
	___mb.mb_get_font = godot::api->godot_method_bind_get_method("Theme", "get_font");
	___mb.mb_get_font_list = godot::api->godot_method_bind_get_method("Theme", "get_font_list");
	___mb.mb_get_icon = godot::api->godot_method_bind_get_method("Theme", "get_icon");
	___mb.mb_get_icon_list = godot::api->godot_method_bind_get_method("Theme", "get_icon_list");
	___mb.mb_get_stylebox = godot::api->godot_method_bind_get_method("Theme", "get_stylebox");
	___mb.mb_get_stylebox_list = godot::api->godot_method_bind_get_method("Theme", "get_stylebox_list");
	___mb.mb_get_stylebox_types = godot::api->godot_method_bind_get_method("Theme", "get_stylebox_types");
	___mb.mb_get_type_list = godot::api->godot_method_bind_get_method("Theme", "get_type_list");
	___mb.mb_has_color = godot::api->godot_method_bind_get_method("Theme", "has_color");
	___mb.mb_has_constant = godot::api->godot_method_bind_get_method("Theme", "has_constant");
	___mb.mb_has_font = godot::api->godot_method_bind_get_method("Theme", "has_font");
	___mb.mb_has_icon = godot::api->godot_method_bind_get_method("Theme", "has_icon");
	___mb.mb_has_stylebox = godot::api->godot_method_bind_get_method("Theme", "has_stylebox");
	___mb.mb_set_color = godot::api->godot_method_bind_get_method("Theme", "set_color");
	___mb.mb_set_constant = godot::api->godot_method_bind_get_method("Theme", "set_constant");
	___mb.mb_set_default_font = godot::api->godot_method_bind_get_method("Theme", "set_default_font");
	___mb.mb_set_font = godot::api->godot_method_bind_get_method("Theme", "set_font");
	___mb.mb_set_icon = godot::api->godot_method_bind_get_method("Theme", "set_icon");
	___mb.mb_set_stylebox = godot::api->godot_method_bind_get_method("Theme", "set_stylebox");
}

Theme *Theme::_new()
{
	return (Theme *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Theme")());
}
void Theme::_emit_theme_changed() {
	___godot_icall_void(___mb.mb__emit_theme_changed, (const Object *) this);
}

void Theme::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

void Theme::clear_color(const String name, const String type) {
	___godot_icall_void_String_String(___mb.mb_clear_color, (const Object *) this, name, type);
}

void Theme::clear_constant(const String name, const String type) {
	___godot_icall_void_String_String(___mb.mb_clear_constant, (const Object *) this, name, type);
}

void Theme::clear_font(const String name, const String type) {
	___godot_icall_void_String_String(___mb.mb_clear_font, (const Object *) this, name, type);
}

void Theme::clear_icon(const String name, const String type) {
	___godot_icall_void_String_String(___mb.mb_clear_icon, (const Object *) this, name, type);
}

void Theme::clear_stylebox(const String name, const String type) {
	___godot_icall_void_String_String(___mb.mb_clear_stylebox, (const Object *) this, name, type);
}

void Theme::copy_default_theme() {
	___godot_icall_void(___mb.mb_copy_default_theme, (const Object *) this);
}

void Theme::copy_theme(const Ref<Theme> other) {
	___godot_icall_void_Object(___mb.mb_copy_theme, (const Object *) this, other.ptr());
}

Color Theme::get_color(const String name, const String type) const {
	return ___godot_icall_Color_String_String(___mb.mb_get_color, (const Object *) this, name, type);
}

PoolStringArray Theme::get_color_list(const String type) const {
	return ___godot_icall_PoolStringArray_String(___mb.mb_get_color_list, (const Object *) this, type);
}

int64_t Theme::get_constant(const String name, const String type) const {
	return ___godot_icall_int_String_String(___mb.mb_get_constant, (const Object *) this, name, type);
}

PoolStringArray Theme::get_constant_list(const String type) const {
	return ___godot_icall_PoolStringArray_String(___mb.mb_get_constant_list, (const Object *) this, type);
}

Ref<Font> Theme::get_default_font() const {
	return Ref<Font>::__internal_constructor(___godot_icall_Object(___mb.mb_get_default_font, (const Object *) this));
}

Ref<Font> Theme::get_font(const String name, const String type) const {
	return Ref<Font>::__internal_constructor(___godot_icall_Object_String_String(___mb.mb_get_font, (const Object *) this, name, type));
}

PoolStringArray Theme::get_font_list(const String type) const {
	return ___godot_icall_PoolStringArray_String(___mb.mb_get_font_list, (const Object *) this, type);
}

Ref<Texture> Theme::get_icon(const String name, const String type) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_String_String(___mb.mb_get_icon, (const Object *) this, name, type));
}

PoolStringArray Theme::get_icon_list(const String type) const {
	return ___godot_icall_PoolStringArray_String(___mb.mb_get_icon_list, (const Object *) this, type);
}

Ref<StyleBox> Theme::get_stylebox(const String name, const String type) const {
	return Ref<StyleBox>::__internal_constructor(___godot_icall_Object_String_String(___mb.mb_get_stylebox, (const Object *) this, name, type));
}

PoolStringArray Theme::get_stylebox_list(const String type) const {
	return ___godot_icall_PoolStringArray_String(___mb.mb_get_stylebox_list, (const Object *) this, type);
}

PoolStringArray Theme::get_stylebox_types() const {
	return ___godot_icall_PoolStringArray(___mb.mb_get_stylebox_types, (const Object *) this);
}

PoolStringArray Theme::get_type_list(const String type) const {
	return ___godot_icall_PoolStringArray_String(___mb.mb_get_type_list, (const Object *) this, type);
}

bool Theme::has_color(const String name, const String type) const {
	return ___godot_icall_bool_String_String(___mb.mb_has_color, (const Object *) this, name, type);
}

bool Theme::has_constant(const String name, const String type) const {
	return ___godot_icall_bool_String_String(___mb.mb_has_constant, (const Object *) this, name, type);
}

bool Theme::has_font(const String name, const String type) const {
	return ___godot_icall_bool_String_String(___mb.mb_has_font, (const Object *) this, name, type);
}

bool Theme::has_icon(const String name, const String type) const {
	return ___godot_icall_bool_String_String(___mb.mb_has_icon, (const Object *) this, name, type);
}

bool Theme::has_stylebox(const String name, const String type) const {
	return ___godot_icall_bool_String_String(___mb.mb_has_stylebox, (const Object *) this, name, type);
}

void Theme::set_color(const String name, const String type, const Color color) {
	___godot_icall_void_String_String_Color(___mb.mb_set_color, (const Object *) this, name, type, color);
}

void Theme::set_constant(const String name, const String type, const int64_t constant) {
	___godot_icall_void_String_String_int(___mb.mb_set_constant, (const Object *) this, name, type, constant);
}

void Theme::set_default_font(const Ref<Font> font) {
	___godot_icall_void_Object(___mb.mb_set_default_font, (const Object *) this, font.ptr());
}

void Theme::set_font(const String name, const String type, const Ref<Font> font) {
	___godot_icall_void_String_String_Object(___mb.mb_set_font, (const Object *) this, name, type, font.ptr());
}

void Theme::set_icon(const String name, const String type, const Ref<Texture> texture) {
	___godot_icall_void_String_String_Object(___mb.mb_set_icon, (const Object *) this, name, type, texture.ptr());
}

void Theme::set_stylebox(const String name, const String type, const Ref<StyleBox> texture) {
	___godot_icall_void_String_String_Object(___mb.mb_set_stylebox, (const Object *) this, name, type, texture.ptr());
}

}