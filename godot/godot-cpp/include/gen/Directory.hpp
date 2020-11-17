#ifndef GODOT_CPP_DIRECTORY_HPP
#define GODOT_CPP_DIRECTORY_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {


class Directory : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_change_dir;
		godot_method_bind *mb_copy;
		godot_method_bind *mb_current_is_dir;
		godot_method_bind *mb_dir_exists;
		godot_method_bind *mb_file_exists;
		godot_method_bind *mb_get_current_dir;
		godot_method_bind *mb_get_current_drive;
		godot_method_bind *mb_get_drive;
		godot_method_bind *mb_get_drive_count;
		godot_method_bind *mb_get_next;
		godot_method_bind *mb_get_space_left;
		godot_method_bind *mb_list_dir_begin;
		godot_method_bind *mb_list_dir_end;
		godot_method_bind *mb_make_dir;
		godot_method_bind *mb_make_dir_recursive;
		godot_method_bind *mb_open;
		godot_method_bind *mb_remove;
		godot_method_bind *mb_rename;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Directory"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Directory *_new();

	// methods
	Error change_dir(const String todir);
	Error copy(const String from, const String to);
	bool current_is_dir() const;
	bool dir_exists(const String path);
	bool file_exists(const String path);
	String get_current_dir();
	int64_t get_current_drive();
	String get_drive(const int64_t idx);
	int64_t get_drive_count();
	String get_next();
	int64_t get_space_left();
	Error list_dir_begin(const bool skip_navigational = false, const bool skip_hidden = false);
	void list_dir_end();
	Error make_dir(const String path);
	Error make_dir_recursive(const String path);
	Error open(const String path);
	Error remove(const String path);
	Error rename(const String from, const String to);

};

}

#endif