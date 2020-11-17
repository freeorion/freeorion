#include "RemoteTransform.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


RemoteTransform::___method_bindings RemoteTransform::___mb = {};

void RemoteTransform::___init_method_bindings() {
	___mb.mb_force_update_cache = godot::api->godot_method_bind_get_method("RemoteTransform", "force_update_cache");
	___mb.mb_get_remote_node = godot::api->godot_method_bind_get_method("RemoteTransform", "get_remote_node");
	___mb.mb_get_update_position = godot::api->godot_method_bind_get_method("RemoteTransform", "get_update_position");
	___mb.mb_get_update_rotation = godot::api->godot_method_bind_get_method("RemoteTransform", "get_update_rotation");
	___mb.mb_get_update_scale = godot::api->godot_method_bind_get_method("RemoteTransform", "get_update_scale");
	___mb.mb_get_use_global_coordinates = godot::api->godot_method_bind_get_method("RemoteTransform", "get_use_global_coordinates");
	___mb.mb_set_remote_node = godot::api->godot_method_bind_get_method("RemoteTransform", "set_remote_node");
	___mb.mb_set_update_position = godot::api->godot_method_bind_get_method("RemoteTransform", "set_update_position");
	___mb.mb_set_update_rotation = godot::api->godot_method_bind_get_method("RemoteTransform", "set_update_rotation");
	___mb.mb_set_update_scale = godot::api->godot_method_bind_get_method("RemoteTransform", "set_update_scale");
	___mb.mb_set_use_global_coordinates = godot::api->godot_method_bind_get_method("RemoteTransform", "set_use_global_coordinates");
}

RemoteTransform *RemoteTransform::_new()
{
	return (RemoteTransform *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"RemoteTransform")());
}
void RemoteTransform::force_update_cache() {
	___godot_icall_void(___mb.mb_force_update_cache, (const Object *) this);
}

NodePath RemoteTransform::get_remote_node() const {
	return ___godot_icall_NodePath(___mb.mb_get_remote_node, (const Object *) this);
}

bool RemoteTransform::get_update_position() const {
	return ___godot_icall_bool(___mb.mb_get_update_position, (const Object *) this);
}

bool RemoteTransform::get_update_rotation() const {
	return ___godot_icall_bool(___mb.mb_get_update_rotation, (const Object *) this);
}

bool RemoteTransform::get_update_scale() const {
	return ___godot_icall_bool(___mb.mb_get_update_scale, (const Object *) this);
}

bool RemoteTransform::get_use_global_coordinates() const {
	return ___godot_icall_bool(___mb.mb_get_use_global_coordinates, (const Object *) this);
}

void RemoteTransform::set_remote_node(const NodePath path) {
	___godot_icall_void_NodePath(___mb.mb_set_remote_node, (const Object *) this, path);
}

void RemoteTransform::set_update_position(const bool update_remote_position) {
	___godot_icall_void_bool(___mb.mb_set_update_position, (const Object *) this, update_remote_position);
}

void RemoteTransform::set_update_rotation(const bool update_remote_rotation) {
	___godot_icall_void_bool(___mb.mb_set_update_rotation, (const Object *) this, update_remote_rotation);
}

void RemoteTransform::set_update_scale(const bool update_remote_scale) {
	___godot_icall_void_bool(___mb.mb_set_update_scale, (const Object *) this, update_remote_scale);
}

void RemoteTransform::set_use_global_coordinates(const bool use_global_coordinates) {
	___godot_icall_void_bool(___mb.mb_set_use_global_coordinates, (const Object *) this, use_global_coordinates);
}

}