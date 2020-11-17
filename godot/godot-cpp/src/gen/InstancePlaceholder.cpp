#include "InstancePlaceholder.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Node.hpp"
#include "PackedScene.hpp"


namespace godot {


InstancePlaceholder::___method_bindings InstancePlaceholder::___mb = {};

void InstancePlaceholder::___init_method_bindings() {
	___mb.mb_create_instance = godot::api->godot_method_bind_get_method("InstancePlaceholder", "create_instance");
	___mb.mb_get_instance_path = godot::api->godot_method_bind_get_method("InstancePlaceholder", "get_instance_path");
	___mb.mb_get_stored_values = godot::api->godot_method_bind_get_method("InstancePlaceholder", "get_stored_values");
	___mb.mb_replace_by_instance = godot::api->godot_method_bind_get_method("InstancePlaceholder", "replace_by_instance");
}

Node *InstancePlaceholder::create_instance(const bool replace, const Ref<PackedScene> custom_scene) {
	return (Node *) ___godot_icall_Object_bool_Object(___mb.mb_create_instance, (const Object *) this, replace, custom_scene.ptr());
}

String InstancePlaceholder::get_instance_path() const {
	return ___godot_icall_String(___mb.mb_get_instance_path, (const Object *) this);
}

Dictionary InstancePlaceholder::get_stored_values(const bool with_order) {
	return ___godot_icall_Dictionary_bool(___mb.mb_get_stored_values, (const Object *) this, with_order);
}

void InstancePlaceholder::replace_by_instance(const Ref<PackedScene> custom_scene) {
	___godot_icall_void_Object(___mb.mb_replace_by_instance, (const Object *) this, custom_scene.ptr());
}

}