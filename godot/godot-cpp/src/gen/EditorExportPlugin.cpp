#include "EditorExportPlugin.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


EditorExportPlugin::___method_bindings EditorExportPlugin::___mb = {};

void EditorExportPlugin::___init_method_bindings() {
	___mb.mb__export_begin = godot::api->godot_method_bind_get_method("EditorExportPlugin", "_export_begin");
	___mb.mb__export_end = godot::api->godot_method_bind_get_method("EditorExportPlugin", "_export_end");
	___mb.mb__export_file = godot::api->godot_method_bind_get_method("EditorExportPlugin", "_export_file");
	___mb.mb_add_file = godot::api->godot_method_bind_get_method("EditorExportPlugin", "add_file");
	___mb.mb_add_ios_bundle_file = godot::api->godot_method_bind_get_method("EditorExportPlugin", "add_ios_bundle_file");
	___mb.mb_add_ios_cpp_code = godot::api->godot_method_bind_get_method("EditorExportPlugin", "add_ios_cpp_code");
	___mb.mb_add_ios_framework = godot::api->godot_method_bind_get_method("EditorExportPlugin", "add_ios_framework");
	___mb.mb_add_ios_linker_flags = godot::api->godot_method_bind_get_method("EditorExportPlugin", "add_ios_linker_flags");
	___mb.mb_add_ios_plist_content = godot::api->godot_method_bind_get_method("EditorExportPlugin", "add_ios_plist_content");
	___mb.mb_add_shared_object = godot::api->godot_method_bind_get_method("EditorExportPlugin", "add_shared_object");
	___mb.mb_skip = godot::api->godot_method_bind_get_method("EditorExportPlugin", "skip");
}

void EditorExportPlugin::_export_begin(const PoolStringArray features, const bool is_debug, const String path, const int64_t flags) {
	___godot_icall_void_PoolStringArray_bool_String_int(___mb.mb__export_begin, (const Object *) this, features, is_debug, path, flags);
}

void EditorExportPlugin::_export_end() {
	___godot_icall_void(___mb.mb__export_end, (const Object *) this);
}

void EditorExportPlugin::_export_file(const String path, const String type, const PoolStringArray features) {
	___godot_icall_void_String_String_PoolStringArray(___mb.mb__export_file, (const Object *) this, path, type, features);
}

void EditorExportPlugin::add_file(const String path, const PoolByteArray file, const bool remap) {
	___godot_icall_void_String_PoolByteArray_bool(___mb.mb_add_file, (const Object *) this, path, file, remap);
}

void EditorExportPlugin::add_ios_bundle_file(const String path) {
	___godot_icall_void_String(___mb.mb_add_ios_bundle_file, (const Object *) this, path);
}

void EditorExportPlugin::add_ios_cpp_code(const String code) {
	___godot_icall_void_String(___mb.mb_add_ios_cpp_code, (const Object *) this, code);
}

void EditorExportPlugin::add_ios_framework(const String path) {
	___godot_icall_void_String(___mb.mb_add_ios_framework, (const Object *) this, path);
}

void EditorExportPlugin::add_ios_linker_flags(const String flags) {
	___godot_icall_void_String(___mb.mb_add_ios_linker_flags, (const Object *) this, flags);
}

void EditorExportPlugin::add_ios_plist_content(const String plist_content) {
	___godot_icall_void_String(___mb.mb_add_ios_plist_content, (const Object *) this, plist_content);
}

void EditorExportPlugin::add_shared_object(const String path, const PoolStringArray tags) {
	___godot_icall_void_String_PoolStringArray(___mb.mb_add_shared_object, (const Object *) this, path, tags);
}

void EditorExportPlugin::skip() {
	___godot_icall_void(___mb.mb_skip, (const Object *) this);
}

}