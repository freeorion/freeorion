#include "Directory.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Directory::___method_bindings Directory::___mb = {};

void Directory::___init_method_bindings() {
	___mb.mb_change_dir = godot::api->godot_method_bind_get_method("_Directory", "change_dir");
	___mb.mb_copy = godot::api->godot_method_bind_get_method("_Directory", "copy");
	___mb.mb_current_is_dir = godot::api->godot_method_bind_get_method("_Directory", "current_is_dir");
	___mb.mb_dir_exists = godot::api->godot_method_bind_get_method("_Directory", "dir_exists");
	___mb.mb_file_exists = godot::api->godot_method_bind_get_method("_Directory", "file_exists");
	___mb.mb_get_current_dir = godot::api->godot_method_bind_get_method("_Directory", "get_current_dir");
	___mb.mb_get_current_drive = godot::api->godot_method_bind_get_method("_Directory", "get_current_drive");
	___mb.mb_get_drive = godot::api->godot_method_bind_get_method("_Directory", "get_drive");
	___mb.mb_get_drive_count = godot::api->godot_method_bind_get_method("_Directory", "get_drive_count");
	___mb.mb_get_next = godot::api->godot_method_bind_get_method("_Directory", "get_next");
	___mb.mb_get_space_left = godot::api->godot_method_bind_get_method("_Directory", "get_space_left");
	___mb.mb_list_dir_begin = godot::api->godot_method_bind_get_method("_Directory", "list_dir_begin");
	___mb.mb_list_dir_end = godot::api->godot_method_bind_get_method("_Directory", "list_dir_end");
	___mb.mb_make_dir = godot::api->godot_method_bind_get_method("_Directory", "make_dir");
	___mb.mb_make_dir_recursive = godot::api->godot_method_bind_get_method("_Directory", "make_dir_recursive");
	___mb.mb_open = godot::api->godot_method_bind_get_method("_Directory", "open");
	___mb.mb_remove = godot::api->godot_method_bind_get_method("_Directory", "remove");
	___mb.mb_rename = godot::api->godot_method_bind_get_method("_Directory", "rename");
}

Directory *Directory::_new()
{
	return (Directory *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"_Directory")());
}
Error Directory::change_dir(const String todir) {
	return (Error) ___godot_icall_int_String(___mb.mb_change_dir, (const Object *) this, todir);
}

Error Directory::copy(const String from, const String to) {
	return (Error) ___godot_icall_int_String_String(___mb.mb_copy, (const Object *) this, from, to);
}

bool Directory::current_is_dir() const {
	return ___godot_icall_bool(___mb.mb_current_is_dir, (const Object *) this);
}

bool Directory::dir_exists(const String path) {
	return ___godot_icall_bool_String(___mb.mb_dir_exists, (const Object *) this, path);
}

bool Directory::file_exists(const String path) {
	return ___godot_icall_bool_String(___mb.mb_file_exists, (const Object *) this, path);
}

String Directory::get_current_dir() {
	return ___godot_icall_String(___mb.mb_get_current_dir, (const Object *) this);
}

int64_t Directory::get_current_drive() {
	return ___godot_icall_int(___mb.mb_get_current_drive, (const Object *) this);
}

String Directory::get_drive(const int64_t idx) {
	return ___godot_icall_String_int(___mb.mb_get_drive, (const Object *) this, idx);
}

int64_t Directory::get_drive_count() {
	return ___godot_icall_int(___mb.mb_get_drive_count, (const Object *) this);
}

String Directory::get_next() {
	return ___godot_icall_String(___mb.mb_get_next, (const Object *) this);
}

int64_t Directory::get_space_left() {
	return ___godot_icall_int(___mb.mb_get_space_left, (const Object *) this);
}

Error Directory::list_dir_begin(const bool skip_navigational, const bool skip_hidden) {
	return (Error) ___godot_icall_int_bool_bool(___mb.mb_list_dir_begin, (const Object *) this, skip_navigational, skip_hidden);
}

void Directory::list_dir_end() {
	___godot_icall_void(___mb.mb_list_dir_end, (const Object *) this);
}

Error Directory::make_dir(const String path) {
	return (Error) ___godot_icall_int_String(___mb.mb_make_dir, (const Object *) this, path);
}

Error Directory::make_dir_recursive(const String path) {
	return (Error) ___godot_icall_int_String(___mb.mb_make_dir_recursive, (const Object *) this, path);
}

Error Directory::open(const String path) {
	return (Error) ___godot_icall_int_String(___mb.mb_open, (const Object *) this, path);
}

Error Directory::remove(const String path) {
	return (Error) ___godot_icall_int_String(___mb.mb_remove, (const Object *) this, path);
}

Error Directory::rename(const String from, const String to) {
	return (Error) ___godot_icall_int_String_String(___mb.mb_rename, (const Object *) this, from, to);
}

}