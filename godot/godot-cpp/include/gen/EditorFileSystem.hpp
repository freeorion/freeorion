#ifndef GODOT_CPP_EDITORFILESYSTEM_HPP
#define GODOT_CPP_EDITORFILESYSTEM_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node.hpp"
namespace godot {

class EditorFileSystemDirectory;

class EditorFileSystem : public Node {
	struct ___method_bindings {
		godot_method_bind *mb_get_file_type;
		godot_method_bind *mb_get_filesystem;
		godot_method_bind *mb_get_filesystem_path;
		godot_method_bind *mb_get_scanning_progress;
		godot_method_bind *mb_is_scanning;
		godot_method_bind *mb_scan;
		godot_method_bind *mb_scan_sources;
		godot_method_bind *mb_update_file;
		godot_method_bind *mb_update_script_classes;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorFileSystem"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	String get_file_type(const String path) const;
	EditorFileSystemDirectory *get_filesystem();
	EditorFileSystemDirectory *get_filesystem_path(const String path);
	real_t get_scanning_progress() const;
	bool is_scanning() const;
	void scan();
	void scan_sources();
	void update_file(const String path);
	void update_script_classes();

};

}

#endif