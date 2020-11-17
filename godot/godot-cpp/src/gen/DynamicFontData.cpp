#include "DynamicFontData.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


DynamicFontData::___method_bindings DynamicFontData::___mb = {};

void DynamicFontData::___init_method_bindings() {
	___mb.mb_get_font_path = godot::api->godot_method_bind_get_method("DynamicFontData", "get_font_path");
	___mb.mb_get_hinting = godot::api->godot_method_bind_get_method("DynamicFontData", "get_hinting");
	___mb.mb_is_antialiased = godot::api->godot_method_bind_get_method("DynamicFontData", "is_antialiased");
	___mb.mb_set_antialiased = godot::api->godot_method_bind_get_method("DynamicFontData", "set_antialiased");
	___mb.mb_set_font_path = godot::api->godot_method_bind_get_method("DynamicFontData", "set_font_path");
	___mb.mb_set_hinting = godot::api->godot_method_bind_get_method("DynamicFontData", "set_hinting");
}

DynamicFontData *DynamicFontData::_new()
{
	return (DynamicFontData *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"DynamicFontData")());
}
String DynamicFontData::get_font_path() const {
	return ___godot_icall_String(___mb.mb_get_font_path, (const Object *) this);
}

DynamicFontData::Hinting DynamicFontData::get_hinting() const {
	return (DynamicFontData::Hinting) ___godot_icall_int(___mb.mb_get_hinting, (const Object *) this);
}

bool DynamicFontData::is_antialiased() const {
	return ___godot_icall_bool(___mb.mb_is_antialiased, (const Object *) this);
}

void DynamicFontData::set_antialiased(const bool antialiased) {
	___godot_icall_void_bool(___mb.mb_set_antialiased, (const Object *) this, antialiased);
}

void DynamicFontData::set_font_path(const String path) {
	___godot_icall_void_String(___mb.mb_set_font_path, (const Object *) this, path);
}

void DynamicFontData::set_hinting(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_hinting, (const Object *) this, mode);
}

}