#include "GDNativeLibrary.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "ConfigFile.hpp"


namespace godot {


GDNativeLibrary::___method_bindings GDNativeLibrary::___mb = {};

void GDNativeLibrary::___init_method_bindings() {
	___mb.mb_get_config_file = godot::api->godot_method_bind_get_method("GDNativeLibrary", "get_config_file");
	___mb.mb_get_current_dependencies = godot::api->godot_method_bind_get_method("GDNativeLibrary", "get_current_dependencies");
	___mb.mb_get_current_library_path = godot::api->godot_method_bind_get_method("GDNativeLibrary", "get_current_library_path");
	___mb.mb_get_symbol_prefix = godot::api->godot_method_bind_get_method("GDNativeLibrary", "get_symbol_prefix");
	___mb.mb_is_reloadable = godot::api->godot_method_bind_get_method("GDNativeLibrary", "is_reloadable");
	___mb.mb_is_singleton = godot::api->godot_method_bind_get_method("GDNativeLibrary", "is_singleton");
	___mb.mb_set_config_file = godot::api->godot_method_bind_get_method("GDNativeLibrary", "set_config_file");
	___mb.mb_set_load_once = godot::api->godot_method_bind_get_method("GDNativeLibrary", "set_load_once");
	___mb.mb_set_reloadable = godot::api->godot_method_bind_get_method("GDNativeLibrary", "set_reloadable");
	___mb.mb_set_singleton = godot::api->godot_method_bind_get_method("GDNativeLibrary", "set_singleton");
	___mb.mb_set_symbol_prefix = godot::api->godot_method_bind_get_method("GDNativeLibrary", "set_symbol_prefix");
	___mb.mb_should_load_once = godot::api->godot_method_bind_get_method("GDNativeLibrary", "should_load_once");
}

GDNativeLibrary *GDNativeLibrary::_new()
{
	return (GDNativeLibrary *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"GDNativeLibrary")());
}
Ref<ConfigFile> GDNativeLibrary::get_config_file() {
	return Ref<ConfigFile>::__internal_constructor(___godot_icall_Object(___mb.mb_get_config_file, (const Object *) this));
}

PoolStringArray GDNativeLibrary::get_current_dependencies() const {
	return ___godot_icall_PoolStringArray(___mb.mb_get_current_dependencies, (const Object *) this);
}

String GDNativeLibrary::get_current_library_path() const {
	return ___godot_icall_String(___mb.mb_get_current_library_path, (const Object *) this);
}

String GDNativeLibrary::get_symbol_prefix() const {
	return ___godot_icall_String(___mb.mb_get_symbol_prefix, (const Object *) this);
}

bool GDNativeLibrary::is_reloadable() const {
	return ___godot_icall_bool(___mb.mb_is_reloadable, (const Object *) this);
}

bool GDNativeLibrary::is_singleton() const {
	return ___godot_icall_bool(___mb.mb_is_singleton, (const Object *) this);
}

void GDNativeLibrary::set_config_file(const Ref<ConfigFile> config_file) {
	___godot_icall_void_Object(___mb.mb_set_config_file, (const Object *) this, config_file.ptr());
}

void GDNativeLibrary::set_load_once(const bool load_once) {
	___godot_icall_void_bool(___mb.mb_set_load_once, (const Object *) this, load_once);
}

void GDNativeLibrary::set_reloadable(const bool reloadable) {
	___godot_icall_void_bool(___mb.mb_set_reloadable, (const Object *) this, reloadable);
}

void GDNativeLibrary::set_singleton(const bool singleton) {
	___godot_icall_void_bool(___mb.mb_set_singleton, (const Object *) this, singleton);
}

void GDNativeLibrary::set_symbol_prefix(const String symbol_prefix) {
	___godot_icall_void_String(___mb.mb_set_symbol_prefix, (const Object *) this, symbol_prefix);
}

bool GDNativeLibrary::should_load_once() const {
	return ___godot_icall_bool(___mb.mb_should_load_once, (const Object *) this);
}

}