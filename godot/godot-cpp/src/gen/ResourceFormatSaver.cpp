#include "ResourceFormatSaver.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Resource.hpp"


namespace godot {


ResourceFormatSaver::___method_bindings ResourceFormatSaver::___mb = {};

void ResourceFormatSaver::___init_method_bindings() {
	___mb.mb_get_recognized_extensions = godot::api->godot_method_bind_get_method("ResourceFormatSaver", "get_recognized_extensions");
	___mb.mb_recognize = godot::api->godot_method_bind_get_method("ResourceFormatSaver", "recognize");
	___mb.mb_save = godot::api->godot_method_bind_get_method("ResourceFormatSaver", "save");
}

ResourceFormatSaver *ResourceFormatSaver::_new()
{
	return (ResourceFormatSaver *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ResourceFormatSaver")());
}
PoolStringArray ResourceFormatSaver::get_recognized_extensions(const Ref<Resource> resource) {
	return ___godot_icall_PoolStringArray_Object(___mb.mb_get_recognized_extensions, (const Object *) this, resource.ptr());
}

bool ResourceFormatSaver::recognize(const Ref<Resource> resource) {
	return ___godot_icall_bool_Object(___mb.mb_recognize, (const Object *) this, resource.ptr());
}

int64_t ResourceFormatSaver::save(const String path, const Ref<Resource> resource, const int64_t flags) {
	return ___godot_icall_int_String_Object_int(___mb.mb_save, (const Object *) this, path, resource.ptr(), flags);
}

}