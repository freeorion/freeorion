#ifndef GODOT_CPP_RESOURCEFORMATLOADER_HPP
#define GODOT_CPP_RESOURCEFORMATLOADER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {


class ResourceFormatLoader : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_get_dependencies;
		godot_method_bind *mb_get_recognized_extensions;
		godot_method_bind *mb_get_resource_type;
		godot_method_bind *mb_handles_type;
		godot_method_bind *mb_load;
		godot_method_bind *mb_rename_dependencies;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ResourceFormatLoader"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ResourceFormatLoader *_new();

	// methods
	void get_dependencies(const String path, const String add_types);
	PoolStringArray get_recognized_extensions();
	String get_resource_type(const String path);
	bool handles_type(const String _typename);
	Variant load(const String path, const String original_path);
	int64_t rename_dependencies(const String path, const String renames);

};

}

#endif