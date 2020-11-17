#include "EditorFileSystemDirectory.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "EditorFileSystemDirectory.hpp"


namespace godot {


EditorFileSystemDirectory::___method_bindings EditorFileSystemDirectory::___mb = {};

void EditorFileSystemDirectory::___init_method_bindings() {
	___mb.mb_find_dir_index = godot::api->godot_method_bind_get_method("EditorFileSystemDirectory", "find_dir_index");
	___mb.mb_find_file_index = godot::api->godot_method_bind_get_method("EditorFileSystemDirectory", "find_file_index");
	___mb.mb_get_file = godot::api->godot_method_bind_get_method("EditorFileSystemDirectory", "get_file");
	___mb.mb_get_file_count = godot::api->godot_method_bind_get_method("EditorFileSystemDirectory", "get_file_count");
	___mb.mb_get_file_import_is_valid = godot::api->godot_method_bind_get_method("EditorFileSystemDirectory", "get_file_import_is_valid");
	___mb.mb_get_file_path = godot::api->godot_method_bind_get_method("EditorFileSystemDirectory", "get_file_path");
	___mb.mb_get_file_script_class_extends = godot::api->godot_method_bind_get_method("EditorFileSystemDirectory", "get_file_script_class_extends");
	___mb.mb_get_file_script_class_name = godot::api->godot_method_bind_get_method("EditorFileSystemDirectory", "get_file_script_class_name");
	___mb.mb_get_file_type = godot::api->godot_method_bind_get_method("EditorFileSystemDirectory", "get_file_type");
	___mb.mb_get_name = godot::api->godot_method_bind_get_method("EditorFileSystemDirectory", "get_name");
	___mb.mb_get_parent = godot::api->godot_method_bind_get_method("EditorFileSystemDirectory", "get_parent");
	___mb.mb_get_path = godot::api->godot_method_bind_get_method("EditorFileSystemDirectory", "get_path");
	___mb.mb_get_subdir = godot::api->godot_method_bind_get_method("EditorFileSystemDirectory", "get_subdir");
	___mb.mb_get_subdir_count = godot::api->godot_method_bind_get_method("EditorFileSystemDirectory", "get_subdir_count");
}

int64_t EditorFileSystemDirectory::find_dir_index(const String name) const {
	return ___godot_icall_int_String(___mb.mb_find_dir_index, (const Object *) this, name);
}

int64_t EditorFileSystemDirectory::find_file_index(const String name) const {
	return ___godot_icall_int_String(___mb.mb_find_file_index, (const Object *) this, name);
}

String EditorFileSystemDirectory::get_file(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_file, (const Object *) this, idx);
}

int64_t EditorFileSystemDirectory::get_file_count() const {
	return ___godot_icall_int(___mb.mb_get_file_count, (const Object *) this);
}

bool EditorFileSystemDirectory::get_file_import_is_valid(const int64_t idx) const {
	return ___godot_icall_bool_int(___mb.mb_get_file_import_is_valid, (const Object *) this, idx);
}

String EditorFileSystemDirectory::get_file_path(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_file_path, (const Object *) this, idx);
}

String EditorFileSystemDirectory::get_file_script_class_extends(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_file_script_class_extends, (const Object *) this, idx);
}

String EditorFileSystemDirectory::get_file_script_class_name(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_file_script_class_name, (const Object *) this, idx);
}

String EditorFileSystemDirectory::get_file_type(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_file_type, (const Object *) this, idx);
}

String EditorFileSystemDirectory::get_name() {
	return ___godot_icall_String(___mb.mb_get_name, (const Object *) this);
}

EditorFileSystemDirectory *EditorFileSystemDirectory::get_parent() {
	return (EditorFileSystemDirectory *) ___godot_icall_Object(___mb.mb_get_parent, (const Object *) this);
}

String EditorFileSystemDirectory::get_path() const {
	return ___godot_icall_String(___mb.mb_get_path, (const Object *) this);
}

EditorFileSystemDirectory *EditorFileSystemDirectory::get_subdir(const int64_t idx) {
	return (EditorFileSystemDirectory *) ___godot_icall_Object_int(___mb.mb_get_subdir, (const Object *) this, idx);
}

int64_t EditorFileSystemDirectory::get_subdir_count() const {
	return ___godot_icall_int(___mb.mb_get_subdir_count, (const Object *) this);
}

}