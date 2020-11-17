#ifndef GODOT_CPP_EDITOREXPORTPLUGIN_HPP
#define GODOT_CPP_EDITOREXPORTPLUGIN_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {


class EditorExportPlugin : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb__export_begin;
		godot_method_bind *mb__export_end;
		godot_method_bind *mb__export_file;
		godot_method_bind *mb_add_file;
		godot_method_bind *mb_add_ios_bundle_file;
		godot_method_bind *mb_add_ios_cpp_code;
		godot_method_bind *mb_add_ios_framework;
		godot_method_bind *mb_add_ios_linker_flags;
		godot_method_bind *mb_add_ios_plist_content;
		godot_method_bind *mb_add_shared_object;
		godot_method_bind *mb_skip;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorExportPlugin"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void _export_begin(const PoolStringArray features, const bool is_debug, const String path, const int64_t flags);
	void _export_end();
	void _export_file(const String path, const String type, const PoolStringArray features);
	void add_file(const String path, const PoolByteArray file, const bool remap);
	void add_ios_bundle_file(const String path);
	void add_ios_cpp_code(const String code);
	void add_ios_framework(const String path);
	void add_ios_linker_flags(const String flags);
	void add_ios_plist_content(const String plist_content);
	void add_shared_object(const String path, const PoolStringArray tags);
	void skip();

};

}

#endif