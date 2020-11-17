#include "ResourceInteractiveLoader.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Resource.hpp"


namespace godot {


ResourceInteractiveLoader::___method_bindings ResourceInteractiveLoader::___mb = {};

void ResourceInteractiveLoader::___init_method_bindings() {
	___mb.mb_get_resource = godot::api->godot_method_bind_get_method("ResourceInteractiveLoader", "get_resource");
	___mb.mb_get_stage = godot::api->godot_method_bind_get_method("ResourceInteractiveLoader", "get_stage");
	___mb.mb_get_stage_count = godot::api->godot_method_bind_get_method("ResourceInteractiveLoader", "get_stage_count");
	___mb.mb_poll = godot::api->godot_method_bind_get_method("ResourceInteractiveLoader", "poll");
	___mb.mb_wait = godot::api->godot_method_bind_get_method("ResourceInteractiveLoader", "wait");
}

Ref<Resource> ResourceInteractiveLoader::get_resource() {
	return Ref<Resource>::__internal_constructor(___godot_icall_Object(___mb.mb_get_resource, (const Object *) this));
}

int64_t ResourceInteractiveLoader::get_stage() const {
	return ___godot_icall_int(___mb.mb_get_stage, (const Object *) this);
}

int64_t ResourceInteractiveLoader::get_stage_count() const {
	return ___godot_icall_int(___mb.mb_get_stage_count, (const Object *) this);
}

Error ResourceInteractiveLoader::poll() {
	return (Error) ___godot_icall_int(___mb.mb_poll, (const Object *) this);
}

Error ResourceInteractiveLoader::wait() {
	return (Error) ___godot_icall_int(___mb.mb_wait, (const Object *) this);
}

}