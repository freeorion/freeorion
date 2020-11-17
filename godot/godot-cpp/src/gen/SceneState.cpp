#include "SceneState.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "PackedScene.hpp"


namespace godot {


SceneState::___method_bindings SceneState::___mb = {};

void SceneState::___init_method_bindings() {
	___mb.mb_get_connection_binds = godot::api->godot_method_bind_get_method("SceneState", "get_connection_binds");
	___mb.mb_get_connection_count = godot::api->godot_method_bind_get_method("SceneState", "get_connection_count");
	___mb.mb_get_connection_flags = godot::api->godot_method_bind_get_method("SceneState", "get_connection_flags");
	___mb.mb_get_connection_method = godot::api->godot_method_bind_get_method("SceneState", "get_connection_method");
	___mb.mb_get_connection_signal = godot::api->godot_method_bind_get_method("SceneState", "get_connection_signal");
	___mb.mb_get_connection_source = godot::api->godot_method_bind_get_method("SceneState", "get_connection_source");
	___mb.mb_get_connection_target = godot::api->godot_method_bind_get_method("SceneState", "get_connection_target");
	___mb.mb_get_node_count = godot::api->godot_method_bind_get_method("SceneState", "get_node_count");
	___mb.mb_get_node_groups = godot::api->godot_method_bind_get_method("SceneState", "get_node_groups");
	___mb.mb_get_node_index = godot::api->godot_method_bind_get_method("SceneState", "get_node_index");
	___mb.mb_get_node_instance = godot::api->godot_method_bind_get_method("SceneState", "get_node_instance");
	___mb.mb_get_node_instance_placeholder = godot::api->godot_method_bind_get_method("SceneState", "get_node_instance_placeholder");
	___mb.mb_get_node_name = godot::api->godot_method_bind_get_method("SceneState", "get_node_name");
	___mb.mb_get_node_owner_path = godot::api->godot_method_bind_get_method("SceneState", "get_node_owner_path");
	___mb.mb_get_node_path = godot::api->godot_method_bind_get_method("SceneState", "get_node_path");
	___mb.mb_get_node_property_count = godot::api->godot_method_bind_get_method("SceneState", "get_node_property_count");
	___mb.mb_get_node_property_name = godot::api->godot_method_bind_get_method("SceneState", "get_node_property_name");
	___mb.mb_get_node_property_value = godot::api->godot_method_bind_get_method("SceneState", "get_node_property_value");
	___mb.mb_get_node_type = godot::api->godot_method_bind_get_method("SceneState", "get_node_type");
	___mb.mb_is_node_instance_placeholder = godot::api->godot_method_bind_get_method("SceneState", "is_node_instance_placeholder");
}

Array SceneState::get_connection_binds(const int64_t idx) const {
	return ___godot_icall_Array_int(___mb.mb_get_connection_binds, (const Object *) this, idx);
}

int64_t SceneState::get_connection_count() const {
	return ___godot_icall_int(___mb.mb_get_connection_count, (const Object *) this);
}

int64_t SceneState::get_connection_flags(const int64_t idx) const {
	return ___godot_icall_int_int(___mb.mb_get_connection_flags, (const Object *) this, idx);
}

String SceneState::get_connection_method(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_connection_method, (const Object *) this, idx);
}

String SceneState::get_connection_signal(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_connection_signal, (const Object *) this, idx);
}

NodePath SceneState::get_connection_source(const int64_t idx) const {
	return ___godot_icall_NodePath_int(___mb.mb_get_connection_source, (const Object *) this, idx);
}

NodePath SceneState::get_connection_target(const int64_t idx) const {
	return ___godot_icall_NodePath_int(___mb.mb_get_connection_target, (const Object *) this, idx);
}

int64_t SceneState::get_node_count() const {
	return ___godot_icall_int(___mb.mb_get_node_count, (const Object *) this);
}

PoolStringArray SceneState::get_node_groups(const int64_t idx) const {
	return ___godot_icall_PoolStringArray_int(___mb.mb_get_node_groups, (const Object *) this, idx);
}

int64_t SceneState::get_node_index(const int64_t idx) const {
	return ___godot_icall_int_int(___mb.mb_get_node_index, (const Object *) this, idx);
}

Ref<PackedScene> SceneState::get_node_instance(const int64_t idx) const {
	return Ref<PackedScene>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_node_instance, (const Object *) this, idx));
}

String SceneState::get_node_instance_placeholder(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_node_instance_placeholder, (const Object *) this, idx);
}

String SceneState::get_node_name(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_node_name, (const Object *) this, idx);
}

NodePath SceneState::get_node_owner_path(const int64_t idx) const {
	return ___godot_icall_NodePath_int(___mb.mb_get_node_owner_path, (const Object *) this, idx);
}

NodePath SceneState::get_node_path(const int64_t idx, const bool for_parent) const {
	return ___godot_icall_NodePath_int_bool(___mb.mb_get_node_path, (const Object *) this, idx, for_parent);
}

int64_t SceneState::get_node_property_count(const int64_t idx) const {
	return ___godot_icall_int_int(___mb.mb_get_node_property_count, (const Object *) this, idx);
}

String SceneState::get_node_property_name(const int64_t idx, const int64_t prop_idx) const {
	return ___godot_icall_String_int_int(___mb.mb_get_node_property_name, (const Object *) this, idx, prop_idx);
}

Variant SceneState::get_node_property_value(const int64_t idx, const int64_t prop_idx) const {
	return ___godot_icall_Variant_int_int(___mb.mb_get_node_property_value, (const Object *) this, idx, prop_idx);
}

String SceneState::get_node_type(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_node_type, (const Object *) this, idx);
}

bool SceneState::is_node_instance_placeholder(const int64_t idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_node_instance_placeholder, (const Object *) this, idx);
}

}