#ifndef GODOT_CPP_EDITORVCSINTERFACE_HPP
#define GODOT_CPP_EDITORVCSINTERFACE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {


class EditorVCSInterface : public Object {
	struct ___method_bindings {
		godot_method_bind *mb__commit;
		godot_method_bind *mb__get_file_diff;
		godot_method_bind *mb__get_modified_files_data;
		godot_method_bind *mb__get_project_name;
		godot_method_bind *mb__get_vcs_name;
		godot_method_bind *mb__initialize;
		godot_method_bind *mb__is_vcs_initialized;
		godot_method_bind *mb__shut_down;
		godot_method_bind *mb__stage_file;
		godot_method_bind *mb__unstage_file;
		godot_method_bind *mb_commit;
		godot_method_bind *mb_get_file_diff;
		godot_method_bind *mb_get_modified_files_data;
		godot_method_bind *mb_get_project_name;
		godot_method_bind *mb_get_vcs_name;
		godot_method_bind *mb_initialize;
		godot_method_bind *mb_is_addon_ready;
		godot_method_bind *mb_is_vcs_initialized;
		godot_method_bind *mb_shut_down;
		godot_method_bind *mb_stage_file;
		godot_method_bind *mb_unstage_file;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorVCSInterface"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void _commit(const String msg);
	Array _get_file_diff(const String file_path);
	Dictionary _get_modified_files_data();
	String _get_project_name();
	String _get_vcs_name();
	bool _initialize(const String project_root_path);
	bool _is_vcs_initialized();
	bool _shut_down();
	void _stage_file(const String file_path);
	void _unstage_file(const String file_path);
	void commit(const String msg);
	Array get_file_diff(const String file_path);
	Dictionary get_modified_files_data();
	String get_project_name();
	String get_vcs_name();
	bool initialize(const String project_root_path);
	bool is_addon_ready();
	bool is_vcs_initialized();
	bool shut_down();
	void stage_file(const String file_path);
	void unstage_file(const String file_path);

};

}

#endif