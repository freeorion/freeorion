#include "ResourceFormatLoader.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ResourceFormatLoader::___method_bindings ResourceFormatLoader::___mb = {};

void ResourceFormatLoader::___init_method_bindings() {
	___mb.mb_get_dependencies = godot::api->godot_method_bind_get_method("ResourceFormatLoader", "get_dependencies");
	___mb.mb_get_recognized_extensions = godot::api->godot_method_bind_get_method("ResourceFormatLoader", "get_recognized_extensions");
	___mb.mb_get_resource_type = godot::api->godot_method_bind_get_method("ResourceFormatLoader", "get_resource_type");
	___mb.mb_handles_type = godot::api->godot_method_bind_get_method("ResourceFormatLoader", "handles_type");
	___mb.mb_load = godot::api->godot_method_bind_get_method("ResourceFormatLoader", "load");
	___mb.mb_rename_dependencies = godot::api->godot_method_bind_get_method("ResourceFormatLoader", "rename_dependencies");
}

ResourceFormatLoader *ResourceFormatLoader::_new()
{
	return (ResourceFormatLoader *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ResourceFormatLoader")());
}
void ResourceFormatLoader::get_dependencies(const String path, const String add_types) {
	___godot_icall_void_String_String(___mb.mb_get_dependencies, (const Object *) this, path, add_types);
}

PoolStringArray ResourceFormatLoader::get_recognized_extensions() {
	return ___godot_icall_PoolStringArray(___mb.mb_get_recognized_extensions, (const Object *) this);
}

String ResourceFormatLoader::get_resource_type(const String path) {
	return ___godot_icall_String_String(___mb.mb_get_resource_type, (const Object *) this, path);
}

bool ResourceFormatLoader::handles_type(const String _typename) {
	return ___godot_icall_bool_String(___mb.mb_handles_type, (const Object *) this, _typename);
}

Variant ResourceFormatLoader::load(const String path, const String original_path) {
	return ___godot_icall_Variant_String_String(___mb.mb_load, (const Object *) this, path, original_path);
}

int64_t ResourceFormatLoader::rename_dependencies(const String path, const String renames) {
	return ___godot_icall_int_String_String(___mb.mb_rename_dependencies, (const Object *) this, path, renames);
}

}