#include "ResourcePreloader.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Resource.hpp"


namespace godot {


ResourcePreloader::___method_bindings ResourcePreloader::___mb = {};

void ResourcePreloader::___init_method_bindings() {
	___mb.mb__get_resources = godot::api->godot_method_bind_get_method("ResourcePreloader", "_get_resources");
	___mb.mb__set_resources = godot::api->godot_method_bind_get_method("ResourcePreloader", "_set_resources");
	___mb.mb_add_resource = godot::api->godot_method_bind_get_method("ResourcePreloader", "add_resource");
	___mb.mb_get_resource = godot::api->godot_method_bind_get_method("ResourcePreloader", "get_resource");
	___mb.mb_get_resource_list = godot::api->godot_method_bind_get_method("ResourcePreloader", "get_resource_list");
	___mb.mb_has_resource = godot::api->godot_method_bind_get_method("ResourcePreloader", "has_resource");
	___mb.mb_remove_resource = godot::api->godot_method_bind_get_method("ResourcePreloader", "remove_resource");
	___mb.mb_rename_resource = godot::api->godot_method_bind_get_method("ResourcePreloader", "rename_resource");
}

ResourcePreloader *ResourcePreloader::_new()
{
	return (ResourcePreloader *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ResourcePreloader")());
}
Array ResourcePreloader::_get_resources() const {
	return ___godot_icall_Array(___mb.mb__get_resources, (const Object *) this);
}

void ResourcePreloader::_set_resources(const Array arg0) {
	___godot_icall_void_Array(___mb.mb__set_resources, (const Object *) this, arg0);
}

void ResourcePreloader::add_resource(const String name, const Ref<Resource> resource) {
	___godot_icall_void_String_Object(___mb.mb_add_resource, (const Object *) this, name, resource.ptr());
}

Ref<Resource> ResourcePreloader::get_resource(const String name) const {
	return Ref<Resource>::__internal_constructor(___godot_icall_Object_String(___mb.mb_get_resource, (const Object *) this, name));
}

PoolStringArray ResourcePreloader::get_resource_list() const {
	return ___godot_icall_PoolStringArray(___mb.mb_get_resource_list, (const Object *) this);
}

bool ResourcePreloader::has_resource(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_resource, (const Object *) this, name);
}

void ResourcePreloader::remove_resource(const String name) {
	___godot_icall_void_String(___mb.mb_remove_resource, (const Object *) this, name);
}

void ResourcePreloader::rename_resource(const String name, const String newname) {
	___godot_icall_void_String_String(___mb.mb_rename_resource, (const Object *) this, name, newname);
}

}