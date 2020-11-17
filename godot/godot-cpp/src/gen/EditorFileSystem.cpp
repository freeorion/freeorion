#include "EditorFileSystem.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "EditorFileSystemDirectory.hpp"


namespace godot {


EditorFileSystem::___method_bindings EditorFileSystem::___mb = {};

void EditorFileSystem::___init_method_bindings() {
	___mb.mb_get_file_type = godot::api->godot_method_bind_get_method("EditorFileSystem", "get_file_type");
	___mb.mb_get_filesystem = godot::api->godot_method_bind_get_method("EditorFileSystem", "get_filesystem");
	___mb.mb_get_filesystem_path = godot::api->godot_method_bind_get_method("EditorFileSystem", "get_filesystem_path");
	___mb.mb_get_scanning_progress = godot::api->godot_method_bind_get_method("EditorFileSystem", "get_scanning_progress");
	___mb.mb_is_scanning = godot::api->godot_method_bind_get_method("EditorFileSystem", "is_scanning");
	___mb.mb_scan = godot::api->godot_method_bind_get_method("EditorFileSystem", "scan");
	___mb.mb_scan_sources = godot::api->godot_method_bind_get_method("EditorFileSystem", "scan_sources");
	___mb.mb_update_file = godot::api->godot_method_bind_get_method("EditorFileSystem", "update_file");
	___mb.mb_update_script_classes = godot::api->godot_method_bind_get_method("EditorFileSystem", "update_script_classes");
}

String EditorFileSystem::get_file_type(const String path) const {
	return ___godot_icall_String_String(___mb.mb_get_file_type, (const Object *) this, path);
}

EditorFileSystemDirectory *EditorFileSystem::get_filesystem() {
	return (EditorFileSystemDirectory *) ___godot_icall_Object(___mb.mb_get_filesystem, (const Object *) this);
}

EditorFileSystemDirectory *EditorFileSystem::get_filesystem_path(const String path) {
	return (EditorFileSystemDirectory *) ___godot_icall_Object_String(___mb.mb_get_filesystem_path, (const Object *) this, path);
}

real_t EditorFileSystem::get_scanning_progress() const {
	return ___godot_icall_float(___mb.mb_get_scanning_progress, (const Object *) this);
}

bool EditorFileSystem::is_scanning() const {
	return ___godot_icall_bool(___mb.mb_is_scanning, (const Object *) this);
}

void EditorFileSystem::scan() {
	___godot_icall_void(___mb.mb_scan, (const Object *) this);
}

void EditorFileSystem::scan_sources() {
	___godot_icall_void(___mb.mb_scan_sources, (const Object *) this);
}

void EditorFileSystem::update_file(const String path) {
	___godot_icall_void_String(___mb.mb_update_file, (const Object *) this, path);
}

void EditorFileSystem::update_script_classes() {
	___godot_icall_void(___mb.mb_update_script_classes, (const Object *) this);
}

}