#ifndef GODOT_CPP_EDITORIMPORTPLUGIN_HPP
#define GODOT_CPP_EDITORIMPORTPLUGIN_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "ResourceImporter.hpp"
namespace godot {


class EditorImportPlugin : public ResourceImporter {
	struct ___method_bindings {
		godot_method_bind *mb_get_import_options;
		godot_method_bind *mb_get_import_order;
		godot_method_bind *mb_get_importer_name;
		godot_method_bind *mb_get_option_visibility;
		godot_method_bind *mb_get_preset_count;
		godot_method_bind *mb_get_preset_name;
		godot_method_bind *mb_get_priority;
		godot_method_bind *mb_get_recognized_extensions;
		godot_method_bind *mb_get_resource_type;
		godot_method_bind *mb_get_save_extension;
		godot_method_bind *mb_get_visible_name;
		godot_method_bind *mb_import;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorImportPlugin"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	Array get_import_options(const int64_t preset);
	int64_t get_import_order();
	String get_importer_name();
	bool get_option_visibility(const String option, const Dictionary options);
	int64_t get_preset_count();
	String get_preset_name(const int64_t preset);
	real_t get_priority();
	Array get_recognized_extensions();
	String get_resource_type();
	String get_save_extension();
	String get_visible_name();
	int64_t import(const String source_file, const String save_path, const Dictionary options, const Array platform_variants, const Array gen_files);

};

}

#endif