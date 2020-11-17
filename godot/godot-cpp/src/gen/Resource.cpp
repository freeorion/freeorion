#include "Resource.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Resource.hpp"
#include "Node.hpp"


namespace godot {


Resource::___method_bindings Resource::___mb = {};

void Resource::___init_method_bindings() {
	___mb.mb__setup_local_to_scene = godot::api->godot_method_bind_get_method("Resource", "_setup_local_to_scene");
	___mb.mb_duplicate = godot::api->godot_method_bind_get_method("Resource", "duplicate");
	___mb.mb_get_local_scene = godot::api->godot_method_bind_get_method("Resource", "get_local_scene");
	___mb.mb_get_name = godot::api->godot_method_bind_get_method("Resource", "get_name");
	___mb.mb_get_path = godot::api->godot_method_bind_get_method("Resource", "get_path");
	___mb.mb_get_rid = godot::api->godot_method_bind_get_method("Resource", "get_rid");
	___mb.mb_is_local_to_scene = godot::api->godot_method_bind_get_method("Resource", "is_local_to_scene");
	___mb.mb_set_local_to_scene = godot::api->godot_method_bind_get_method("Resource", "set_local_to_scene");
	___mb.mb_set_name = godot::api->godot_method_bind_get_method("Resource", "set_name");
	___mb.mb_set_path = godot::api->godot_method_bind_get_method("Resource", "set_path");
	___mb.mb_setup_local_to_scene = godot::api->godot_method_bind_get_method("Resource", "setup_local_to_scene");
	___mb.mb_take_over_path = godot::api->godot_method_bind_get_method("Resource", "take_over_path");
}

Resource *Resource::_new()
{
	return (Resource *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Resource")());
}
void Resource::_setup_local_to_scene() {
	___godot_icall_void(___mb.mb__setup_local_to_scene, (const Object *) this);
}

Ref<Resource> Resource::duplicate(const bool subresources) const {
	return Ref<Resource>::__internal_constructor(___godot_icall_Object_bool(___mb.mb_duplicate, (const Object *) this, subresources));
}

Node *Resource::get_local_scene() const {
	return (Node *) ___godot_icall_Object(___mb.mb_get_local_scene, (const Object *) this);
}

String Resource::get_name() const {
	return ___godot_icall_String(___mb.mb_get_name, (const Object *) this);
}

String Resource::get_path() const {
	return ___godot_icall_String(___mb.mb_get_path, (const Object *) this);
}

RID Resource::get_rid() const {
	return ___godot_icall_RID(___mb.mb_get_rid, (const Object *) this);
}

bool Resource::is_local_to_scene() const {
	return ___godot_icall_bool(___mb.mb_is_local_to_scene, (const Object *) this);
}

void Resource::set_local_to_scene(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_local_to_scene, (const Object *) this, enable);
}

void Resource::set_name(const String name) {
	___godot_icall_void_String(___mb.mb_set_name, (const Object *) this, name);
}

void Resource::set_path(const String path) {
	___godot_icall_void_String(___mb.mb_set_path, (const Object *) this, path);
}

void Resource::setup_local_to_scene() {
	___godot_icall_void(___mb.mb_setup_local_to_scene, (const Object *) this);
}

void Resource::take_over_path(const String path) {
	___godot_icall_void_String(___mb.mb_take_over_path, (const Object *) this, path);
}

}