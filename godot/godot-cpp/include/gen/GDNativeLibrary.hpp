#ifndef GODOT_CPP_GDNATIVELIBRARY_HPP
#define GODOT_CPP_GDNATIVELIBRARY_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class ConfigFile;

class GDNativeLibrary : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_get_config_file;
		godot_method_bind *mb_get_current_dependencies;
		godot_method_bind *mb_get_current_library_path;
		godot_method_bind *mb_get_symbol_prefix;
		godot_method_bind *mb_is_reloadable;
		godot_method_bind *mb_is_singleton;
		godot_method_bind *mb_set_config_file;
		godot_method_bind *mb_set_load_once;
		godot_method_bind *mb_set_reloadable;
		godot_method_bind *mb_set_singleton;
		godot_method_bind *mb_set_symbol_prefix;
		godot_method_bind *mb_should_load_once;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "GDNativeLibrary"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static GDNativeLibrary *_new();

	// methods
	Ref<ConfigFile> get_config_file();
	PoolStringArray get_current_dependencies() const;
	String get_current_library_path() const;
	String get_symbol_prefix() const;
	bool is_reloadable() const;
	bool is_singleton() const;
	void set_config_file(const Ref<ConfigFile> config_file);
	void set_load_once(const bool load_once);
	void set_reloadable(const bool reloadable);
	void set_singleton(const bool singleton);
	void set_symbol_prefix(const String symbol_prefix);
	bool should_load_once() const;

};

}

#endif