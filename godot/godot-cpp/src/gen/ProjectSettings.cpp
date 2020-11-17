#include "ProjectSettings.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ProjectSettings *ProjectSettings::_singleton = NULL;


ProjectSettings::ProjectSettings() {
	_owner = godot::api->godot_global_get_singleton((char *) "ProjectSettings");
}


ProjectSettings::___method_bindings ProjectSettings::___mb = {};

void ProjectSettings::___init_method_bindings() {
	___mb.mb_add_property_info = godot::api->godot_method_bind_get_method("ProjectSettings", "add_property_info");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("ProjectSettings", "clear");
	___mb.mb_get_order = godot::api->godot_method_bind_get_method("ProjectSettings", "get_order");
	___mb.mb_get_setting = godot::api->godot_method_bind_get_method("ProjectSettings", "get_setting");
	___mb.mb_globalize_path = godot::api->godot_method_bind_get_method("ProjectSettings", "globalize_path");
	___mb.mb_has_setting = godot::api->godot_method_bind_get_method("ProjectSettings", "has_setting");
	___mb.mb_load_resource_pack = godot::api->godot_method_bind_get_method("ProjectSettings", "load_resource_pack");
	___mb.mb_localize_path = godot::api->godot_method_bind_get_method("ProjectSettings", "localize_path");
	___mb.mb_property_can_revert = godot::api->godot_method_bind_get_method("ProjectSettings", "property_can_revert");
	___mb.mb_property_get_revert = godot::api->godot_method_bind_get_method("ProjectSettings", "property_get_revert");
	___mb.mb_save = godot::api->godot_method_bind_get_method("ProjectSettings", "save");
	___mb.mb_save_custom = godot::api->godot_method_bind_get_method("ProjectSettings", "save_custom");
	___mb.mb_set_initial_value = godot::api->godot_method_bind_get_method("ProjectSettings", "set_initial_value");
	___mb.mb_set_order = godot::api->godot_method_bind_get_method("ProjectSettings", "set_order");
	___mb.mb_set_setting = godot::api->godot_method_bind_get_method("ProjectSettings", "set_setting");
}

void ProjectSettings::add_property_info(const Dictionary hint) {
	___godot_icall_void_Dictionary(___mb.mb_add_property_info, (const Object *) this, hint);
}

void ProjectSettings::clear(const String name) {
	___godot_icall_void_String(___mb.mb_clear, (const Object *) this, name);
}

int64_t ProjectSettings::get_order(const String name) const {
	return ___godot_icall_int_String(___mb.mb_get_order, (const Object *) this, name);
}

Variant ProjectSettings::get_setting(const String name) const {
	return ___godot_icall_Variant_String(___mb.mb_get_setting, (const Object *) this, name);
}

String ProjectSettings::globalize_path(const String path) const {
	return ___godot_icall_String_String(___mb.mb_globalize_path, (const Object *) this, path);
}

bool ProjectSettings::has_setting(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_setting, (const Object *) this, name);
}

bool ProjectSettings::load_resource_pack(const String pack, const bool replace_files) {
	return ___godot_icall_bool_String_bool(___mb.mb_load_resource_pack, (const Object *) this, pack, replace_files);
}

String ProjectSettings::localize_path(const String path) const {
	return ___godot_icall_String_String(___mb.mb_localize_path, (const Object *) this, path);
}

bool ProjectSettings::property_can_revert(const String name) {
	return ___godot_icall_bool_String(___mb.mb_property_can_revert, (const Object *) this, name);
}

Variant ProjectSettings::property_get_revert(const String name) {
	return ___godot_icall_Variant_String(___mb.mb_property_get_revert, (const Object *) this, name);
}

Error ProjectSettings::save() {
	return (Error) ___godot_icall_int(___mb.mb_save, (const Object *) this);
}

Error ProjectSettings::save_custom(const String file) {
	return (Error) ___godot_icall_int_String(___mb.mb_save_custom, (const Object *) this, file);
}

void ProjectSettings::set_initial_value(const String name, const Variant value) {
	___godot_icall_void_String_Variant(___mb.mb_set_initial_value, (const Object *) this, name, value);
}

void ProjectSettings::set_order(const String name, const int64_t position) {
	___godot_icall_void_String_int(___mb.mb_set_order, (const Object *) this, name, position);
}

void ProjectSettings::set_setting(const String name, const Variant value) {
	___godot_icall_void_String_Variant(___mb.mb_set_setting, (const Object *) this, name, value);
}

}