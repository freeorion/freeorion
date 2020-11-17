#ifndef GODOT_CPP_CONFIGFILE_HPP
#define GODOT_CPP_CONFIGFILE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {


class ConfigFile : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_erase_section;
		godot_method_bind *mb_erase_section_key;
		godot_method_bind *mb_get_section_keys;
		godot_method_bind *mb_get_sections;
		godot_method_bind *mb_get_value;
		godot_method_bind *mb_has_section;
		godot_method_bind *mb_has_section_key;
		godot_method_bind *mb_load;
		godot_method_bind *mb_load_encrypted;
		godot_method_bind *mb_load_encrypted_pass;
		godot_method_bind *mb_save;
		godot_method_bind *mb_save_encrypted;
		godot_method_bind *mb_save_encrypted_pass;
		godot_method_bind *mb_set_value;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ConfigFile"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ConfigFile *_new();

	// methods
	void erase_section(const String section);
	void erase_section_key(const String section, const String key);
	PoolStringArray get_section_keys(const String section) const;
	PoolStringArray get_sections() const;
	Variant get_value(const String section, const String key, const Variant _default = Variant()) const;
	bool has_section(const String section) const;
	bool has_section_key(const String section, const String key) const;
	Error load(const String path);
	Error load_encrypted(const String path, const PoolByteArray key);
	Error load_encrypted_pass(const String path, const String pass);
	Error save(const String path);
	Error save_encrypted(const String path, const PoolByteArray key);
	Error save_encrypted_pass(const String path, const String pass);
	void set_value(const String section, const String key, const Variant value);

};

}

#endif