#ifndef GODOT_CPP_EDITORFILESYSTEMDIRECTORY_HPP
#define GODOT_CPP_EDITORFILESYSTEMDIRECTORY_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {

class EditorFileSystemDirectory;

class EditorFileSystemDirectory : public Object {
	struct ___method_bindings {
		godot_method_bind *mb_find_dir_index;
		godot_method_bind *mb_find_file_index;
		godot_method_bind *mb_get_file;
		godot_method_bind *mb_get_file_count;
		godot_method_bind *mb_get_file_import_is_valid;
		godot_method_bind *mb_get_file_path;
		godot_method_bind *mb_get_file_script_class_extends;
		godot_method_bind *mb_get_file_script_class_name;
		godot_method_bind *mb_get_file_type;
		godot_method_bind *mb_get_name;
		godot_method_bind *mb_get_parent;
		godot_method_bind *mb_get_path;
		godot_method_bind *mb_get_subdir;
		godot_method_bind *mb_get_subdir_count;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorFileSystemDirectory"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	int64_t find_dir_index(const String name) const;
	int64_t find_file_index(const String name) const;
	String get_file(const int64_t idx) const;
	int64_t get_file_count() const;
	bool get_file_import_is_valid(const int64_t idx) const;
	String get_file_path(const int64_t idx) const;
	String get_file_script_class_extends(const int64_t idx) const;
	String get_file_script_class_name(const int64_t idx) const;
	String get_file_type(const int64_t idx) const;
	String get_name();
	EditorFileSystemDirectory *get_parent();
	String get_path() const;
	EditorFileSystemDirectory *get_subdir(const int64_t idx);
	int64_t get_subdir_count() const;

};

}

#endif