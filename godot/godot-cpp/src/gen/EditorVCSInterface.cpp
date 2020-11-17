#include "EditorVCSInterface.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


EditorVCSInterface::___method_bindings EditorVCSInterface::___mb = {};

void EditorVCSInterface::___init_method_bindings() {
	___mb.mb__commit = godot::api->godot_method_bind_get_method("EditorVCSInterface", "_commit");
	___mb.mb__get_file_diff = godot::api->godot_method_bind_get_method("EditorVCSInterface", "_get_file_diff");
	___mb.mb__get_modified_files_data = godot::api->godot_method_bind_get_method("EditorVCSInterface", "_get_modified_files_data");
	___mb.mb__get_project_name = godot::api->godot_method_bind_get_method("EditorVCSInterface", "_get_project_name");
	___mb.mb__get_vcs_name = godot::api->godot_method_bind_get_method("EditorVCSInterface", "_get_vcs_name");
	___mb.mb__initialize = godot::api->godot_method_bind_get_method("EditorVCSInterface", "_initialize");
	___mb.mb__is_vcs_initialized = godot::api->godot_method_bind_get_method("EditorVCSInterface", "_is_vcs_initialized");
	___mb.mb__shut_down = godot::api->godot_method_bind_get_method("EditorVCSInterface", "_shut_down");
	___mb.mb__stage_file = godot::api->godot_method_bind_get_method("EditorVCSInterface", "_stage_file");
	___mb.mb__unstage_file = godot::api->godot_method_bind_get_method("EditorVCSInterface", "_unstage_file");
	___mb.mb_commit = godot::api->godot_method_bind_get_method("EditorVCSInterface", "commit");
	___mb.mb_get_file_diff = godot::api->godot_method_bind_get_method("EditorVCSInterface", "get_file_diff");
	___mb.mb_get_modified_files_data = godot::api->godot_method_bind_get_method("EditorVCSInterface", "get_modified_files_data");
	___mb.mb_get_project_name = godot::api->godot_method_bind_get_method("EditorVCSInterface", "get_project_name");
	___mb.mb_get_vcs_name = godot::api->godot_method_bind_get_method("EditorVCSInterface", "get_vcs_name");
	___mb.mb_initialize = godot::api->godot_method_bind_get_method("EditorVCSInterface", "initialize");
	___mb.mb_is_addon_ready = godot::api->godot_method_bind_get_method("EditorVCSInterface", "is_addon_ready");
	___mb.mb_is_vcs_initialized = godot::api->godot_method_bind_get_method("EditorVCSInterface", "is_vcs_initialized");
	___mb.mb_shut_down = godot::api->godot_method_bind_get_method("EditorVCSInterface", "shut_down");
	___mb.mb_stage_file = godot::api->godot_method_bind_get_method("EditorVCSInterface", "stage_file");
	___mb.mb_unstage_file = godot::api->godot_method_bind_get_method("EditorVCSInterface", "unstage_file");
}

void EditorVCSInterface::_commit(const String msg) {
	___godot_icall_void_String(___mb.mb__commit, (const Object *) this, msg);
}

Array EditorVCSInterface::_get_file_diff(const String file_path) {
	return ___godot_icall_Array_String(___mb.mb__get_file_diff, (const Object *) this, file_path);
}

Dictionary EditorVCSInterface::_get_modified_files_data() {
	return ___godot_icall_Dictionary(___mb.mb__get_modified_files_data, (const Object *) this);
}

String EditorVCSInterface::_get_project_name() {
	return ___godot_icall_String(___mb.mb__get_project_name, (const Object *) this);
}

String EditorVCSInterface::_get_vcs_name() {
	return ___godot_icall_String(___mb.mb__get_vcs_name, (const Object *) this);
}

bool EditorVCSInterface::_initialize(const String project_root_path) {
	return ___godot_icall_bool_String(___mb.mb__initialize, (const Object *) this, project_root_path);
}

bool EditorVCSInterface::_is_vcs_initialized() {
	return ___godot_icall_bool(___mb.mb__is_vcs_initialized, (const Object *) this);
}

bool EditorVCSInterface::_shut_down() {
	return ___godot_icall_bool(___mb.mb__shut_down, (const Object *) this);
}

void EditorVCSInterface::_stage_file(const String file_path) {
	___godot_icall_void_String(___mb.mb__stage_file, (const Object *) this, file_path);
}

void EditorVCSInterface::_unstage_file(const String file_path) {
	___godot_icall_void_String(___mb.mb__unstage_file, (const Object *) this, file_path);
}

void EditorVCSInterface::commit(const String msg) {
	___godot_icall_void_String(___mb.mb_commit, (const Object *) this, msg);
}

Array EditorVCSInterface::get_file_diff(const String file_path) {
	return ___godot_icall_Array_String(___mb.mb_get_file_diff, (const Object *) this, file_path);
}

Dictionary EditorVCSInterface::get_modified_files_data() {
	return ___godot_icall_Dictionary(___mb.mb_get_modified_files_data, (const Object *) this);
}

String EditorVCSInterface::get_project_name() {
	return ___godot_icall_String(___mb.mb_get_project_name, (const Object *) this);
}

String EditorVCSInterface::get_vcs_name() {
	return ___godot_icall_String(___mb.mb_get_vcs_name, (const Object *) this);
}

bool EditorVCSInterface::initialize(const String project_root_path) {
	return ___godot_icall_bool_String(___mb.mb_initialize, (const Object *) this, project_root_path);
}

bool EditorVCSInterface::is_addon_ready() {
	return ___godot_icall_bool(___mb.mb_is_addon_ready, (const Object *) this);
}

bool EditorVCSInterface::is_vcs_initialized() {
	return ___godot_icall_bool(___mb.mb_is_vcs_initialized, (const Object *) this);
}

bool EditorVCSInterface::shut_down() {
	return ___godot_icall_bool(___mb.mb_shut_down, (const Object *) this);
}

void EditorVCSInterface::stage_file(const String file_path) {
	___godot_icall_void_String(___mb.mb_stage_file, (const Object *) this, file_path);
}

void EditorVCSInterface::unstage_file(const String file_path) {
	___godot_icall_void_String(___mb.mb_unstage_file, (const Object *) this, file_path);
}

}