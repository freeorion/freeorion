#include "DynamicFont.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "DynamicFontData.hpp"


namespace godot {


DynamicFont::___method_bindings DynamicFont::___mb = {};

void DynamicFont::___init_method_bindings() {
	___mb.mb_add_fallback = godot::api->godot_method_bind_get_method("DynamicFont", "add_fallback");
	___mb.mb_get_fallback = godot::api->godot_method_bind_get_method("DynamicFont", "get_fallback");
	___mb.mb_get_fallback_count = godot::api->godot_method_bind_get_method("DynamicFont", "get_fallback_count");
	___mb.mb_get_font_data = godot::api->godot_method_bind_get_method("DynamicFont", "get_font_data");
	___mb.mb_get_outline_color = godot::api->godot_method_bind_get_method("DynamicFont", "get_outline_color");
	___mb.mb_get_outline_size = godot::api->godot_method_bind_get_method("DynamicFont", "get_outline_size");
	___mb.mb_get_size = godot::api->godot_method_bind_get_method("DynamicFont", "get_size");
	___mb.mb_get_spacing = godot::api->godot_method_bind_get_method("DynamicFont", "get_spacing");
	___mb.mb_get_use_filter = godot::api->godot_method_bind_get_method("DynamicFont", "get_use_filter");
	___mb.mb_get_use_mipmaps = godot::api->godot_method_bind_get_method("DynamicFont", "get_use_mipmaps");
	___mb.mb_remove_fallback = godot::api->godot_method_bind_get_method("DynamicFont", "remove_fallback");
	___mb.mb_set_fallback = godot::api->godot_method_bind_get_method("DynamicFont", "set_fallback");
	___mb.mb_set_font_data = godot::api->godot_method_bind_get_method("DynamicFont", "set_font_data");
	___mb.mb_set_outline_color = godot::api->godot_method_bind_get_method("DynamicFont", "set_outline_color");
	___mb.mb_set_outline_size = godot::api->godot_method_bind_get_method("DynamicFont", "set_outline_size");
	___mb.mb_set_size = godot::api->godot_method_bind_get_method("DynamicFont", "set_size");
	___mb.mb_set_spacing = godot::api->godot_method_bind_get_method("DynamicFont", "set_spacing");
	___mb.mb_set_use_filter = godot::api->godot_method_bind_get_method("DynamicFont", "set_use_filter");
	___mb.mb_set_use_mipmaps = godot::api->godot_method_bind_get_method("DynamicFont", "set_use_mipmaps");
}

DynamicFont *DynamicFont::_new()
{
	return (DynamicFont *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"DynamicFont")());
}
void DynamicFont::add_fallback(const Ref<DynamicFontData> data) {
	___godot_icall_void_Object(___mb.mb_add_fallback, (const Object *) this, data.ptr());
}

Ref<DynamicFontData> DynamicFont::get_fallback(const int64_t idx) const {
	return Ref<DynamicFontData>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_fallback, (const Object *) this, idx));
}

int64_t DynamicFont::get_fallback_count() const {
	return ___godot_icall_int(___mb.mb_get_fallback_count, (const Object *) this);
}

Ref<DynamicFontData> DynamicFont::get_font_data() const {
	return Ref<DynamicFontData>::__internal_constructor(___godot_icall_Object(___mb.mb_get_font_data, (const Object *) this));
}

Color DynamicFont::get_outline_color() const {
	return ___godot_icall_Color(___mb.mb_get_outline_color, (const Object *) this);
}

int64_t DynamicFont::get_outline_size() const {
	return ___godot_icall_int(___mb.mb_get_outline_size, (const Object *) this);
}

int64_t DynamicFont::get_size() const {
	return ___godot_icall_int(___mb.mb_get_size, (const Object *) this);
}

int64_t DynamicFont::get_spacing(const int64_t type) const {
	return ___godot_icall_int_int(___mb.mb_get_spacing, (const Object *) this, type);
}

bool DynamicFont::get_use_filter() const {
	return ___godot_icall_bool(___mb.mb_get_use_filter, (const Object *) this);
}

bool DynamicFont::get_use_mipmaps() const {
	return ___godot_icall_bool(___mb.mb_get_use_mipmaps, (const Object *) this);
}

void DynamicFont::remove_fallback(const int64_t idx) {
	___godot_icall_void_int(___mb.mb_remove_fallback, (const Object *) this, idx);
}

void DynamicFont::set_fallback(const int64_t idx, const Ref<DynamicFontData> data) {
	___godot_icall_void_int_Object(___mb.mb_set_fallback, (const Object *) this, idx, data.ptr());
}

void DynamicFont::set_font_data(const Ref<DynamicFontData> data) {
	___godot_icall_void_Object(___mb.mb_set_font_data, (const Object *) this, data.ptr());
}

void DynamicFont::set_outline_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_outline_color, (const Object *) this, color);
}

void DynamicFont::set_outline_size(const int64_t size) {
	___godot_icall_void_int(___mb.mb_set_outline_size, (const Object *) this, size);
}

void DynamicFont::set_size(const int64_t data) {
	___godot_icall_void_int(___mb.mb_set_size, (const Object *) this, data);
}

void DynamicFont::set_spacing(const int64_t type, const int64_t value) {
	___godot_icall_void_int_int(___mb.mb_set_spacing, (const Object *) this, type, value);
}

void DynamicFont::set_use_filter(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_filter, (const Object *) this, enable);
}

void DynamicFont::set_use_mipmaps(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_mipmaps, (const Object *) this, enable);
}

}