#include "ResourceLoader.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Resource.hpp"
#include "ResourceInteractiveLoader.hpp"


namespace godot {


ResourceLoader *ResourceLoader::_singleton = NULL;


ResourceLoader::ResourceLoader() {
	_owner = godot::api->godot_global_get_singleton((char *) "ResourceLoader");
}


ResourceLoader::___method_bindings ResourceLoader::___mb = {};

void ResourceLoader::___init_method_bindings() {
	___mb.mb_exists = godot::api->godot_method_bind_get_method("_ResourceLoader", "exists");
	___mb.mb_get_dependencies = godot::api->godot_method_bind_get_method("_ResourceLoader", "get_dependencies");
	___mb.mb_get_recognized_extensions_for_type = godot::api->godot_method_bind_get_method("_ResourceLoader", "get_recognized_extensions_for_type");
	___mb.mb_has = godot::api->godot_method_bind_get_method("_ResourceLoader", "has");
	___mb.mb_has_cached = godot::api->godot_method_bind_get_method("_ResourceLoader", "has_cached");
	___mb.mb_load = godot::api->godot_method_bind_get_method("_ResourceLoader", "load");
	___mb.mb_load_interactive = godot::api->godot_method_bind_get_method("_ResourceLoader", "load_interactive");
	___mb.mb_set_abort_on_missing_resources = godot::api->godot_method_bind_get_method("_ResourceLoader", "set_abort_on_missing_resources");
}

bool ResourceLoader::exists(const String path, const String type_hint) {
	return ___godot_icall_bool_String_String(___mb.mb_exists, (const Object *) this, path, type_hint);
}

PoolStringArray ResourceLoader::get_dependencies(const String path) {
	return ___godot_icall_PoolStringArray_String(___mb.mb_get_dependencies, (const Object *) this, path);
}

PoolStringArray ResourceLoader::get_recognized_extensions_for_type(const String type) {
	return ___godot_icall_PoolStringArray_String(___mb.mb_get_recognized_extensions_for_type, (const Object *) this, type);
}

bool ResourceLoader::has(const String path) {
	return ___godot_icall_bool_String(___mb.mb_has, (const Object *) this, path);
}

bool ResourceLoader::has_cached(const String path) {
	return ___godot_icall_bool_String(___mb.mb_has_cached, (const Object *) this, path);
}

Ref<Resource> ResourceLoader::load(const String path, const String type_hint, const bool no_cache) {
	return Ref<Resource>::__internal_constructor(___godot_icall_Object_String_String_bool(___mb.mb_load, (const Object *) this, path, type_hint, no_cache));
}

Ref<ResourceInteractiveLoader> ResourceLoader::load_interactive(const String path, const String type_hint) {
	return Ref<ResourceInteractiveLoader>::__internal_constructor(___godot_icall_Object_String_String(___mb.mb_load_interactive, (const Object *) this, path, type_hint));
}

void ResourceLoader::set_abort_on_missing_resources(const bool abort) {
	___godot_icall_void_bool(___mb.mb_set_abort_on_missing_resources, (const Object *) this, abort);
}

}