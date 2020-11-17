#include "EditorSettings.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


EditorSettings::___method_bindings EditorSettings::___mb = {};

void EditorSettings::___init_method_bindings() {
	___mb.mb_add_property_info = godot::api->godot_method_bind_get_method("EditorSettings", "add_property_info");
	___mb.mb_erase = godot::api->godot_method_bind_get_method("EditorSettings", "erase");
	___mb.mb_get_favorites = godot::api->godot_method_bind_get_method("EditorSettings", "get_favorites");
	___mb.mb_get_project_metadata = godot::api->godot_method_bind_get_method("EditorSettings", "get_project_metadata");
	___mb.mb_get_project_settings_dir = godot::api->godot_method_bind_get_method("EditorSettings", "get_project_settings_dir");
	___mb.mb_get_recent_dirs = godot::api->godot_method_bind_get_method("EditorSettings", "get_recent_dirs");
	___mb.mb_get_setting = godot::api->godot_method_bind_get_method("EditorSettings", "get_setting");
	___mb.mb_get_settings_dir = godot::api->godot_method_bind_get_method("EditorSettings", "get_settings_dir");
	___mb.mb_has_setting = godot::api->godot_method_bind_get_method("EditorSettings", "has_setting");
	___mb.mb_property_can_revert = godot::api->godot_method_bind_get_method("EditorSettings", "property_can_revert");
	___mb.mb_property_get_revert = godot::api->godot_method_bind_get_method("EditorSettings", "property_get_revert");
	___mb.mb_set_favorites = godot::api->godot_method_bind_get_method("EditorSettings", "set_favorites");
	___mb.mb_set_initial_value = godot::api->godot_method_bind_get_method("EditorSettings", "set_initial_value");
	___mb.mb_set_project_metadata = godot::api->godot_method_bind_get_method("EditorSettings", "set_project_metadata");
	___mb.mb_set_recent_dirs = godot::api->godot_method_bind_get_method("EditorSettings", "set_recent_dirs");
	___mb.mb_set_setting = godot::api->godot_method_bind_get_method("EditorSettings", "set_setting");
}

void EditorSettings::add_property_info(const Dictionary info) {
	___godot_icall_void_Dictionary(___mb.mb_add_property_info, (const Object *) this, info);
}

void EditorSettings::erase(const String property) {
	___godot_icall_void_String(___mb.mb_erase, (const Object *) this, property);
}

PoolStringArray EditorSettings::get_favorites() const {
	return ___godot_icall_PoolStringArray(___mb.mb_get_favorites, (const Object *) this);
}

Variant EditorSettings::get_project_metadata(const String section, const String key, const Variant _default) const {
	return ___godot_icall_Variant_String_String_Variant(___mb.mb_get_project_metadata, (const Object *) this, section, key, _default);
}

String EditorSettings::get_project_settings_dir() const {
	return ___godot_icall_String(___mb.mb_get_project_settings_dir, (const Object *) this);
}

PoolStringArray EditorSettings::get_recent_dirs() const {
	return ___godot_icall_PoolStringArray(___mb.mb_get_recent_dirs, (const Object *) this);
}

Variant EditorSettings::get_setting(const String name) const {
	return ___godot_icall_Variant_String(___mb.mb_get_setting, (const Object *) this, name);
}

String EditorSettings::get_settings_dir() const {
	return ___godot_icall_String(___mb.mb_get_settings_dir, (const Object *) this);
}

bool EditorSettings::has_setting(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_setting, (const Object *) this, name);
}

bool EditorSettings::property_can_revert(const String name) {
	return ___godot_icall_bool_String(___mb.mb_property_can_revert, (const Object *) this, name);
}

Variant EditorSettings::property_get_revert(const String name) {
	return ___godot_icall_Variant_String(___mb.mb_property_get_revert, (const Object *) this, name);
}

void EditorSettings::set_favorites(const PoolStringArray dirs) {
	___godot_icall_void_PoolStringArray(___mb.mb_set_favorites, (const Object *) this, dirs);
}

void EditorSettings::set_initial_value(const String name, const Variant value, const bool update_current) {
	___godot_icall_void_String_Variant_bool(___mb.mb_set_initial_value, (const Object *) this, name, value, update_current);
}

void EditorSettings::set_project_metadata(const String section, const String key, const Variant data) {
	___godot_icall_void_String_String_Variant(___mb.mb_set_project_metadata, (const Object *) this, section, key, data);
}

void EditorSettings::set_recent_dirs(const PoolStringArray dirs) {
	___godot_icall_void_PoolStringArray(___mb.mb_set_recent_dirs, (const Object *) this, dirs);
}

void EditorSettings::set_setting(const String name, const Variant value) {
	___godot_icall_void_String_Variant(___mb.mb_set_setting, (const Object *) this, name, value);
}

}