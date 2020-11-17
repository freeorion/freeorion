#include "VisualScriptSceneNode.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptSceneNode::___method_bindings VisualScriptSceneNode::___mb = {};

void VisualScriptSceneNode::___init_method_bindings() {
	___mb.mb_get_node_path = godot::api->godot_method_bind_get_method("VisualScriptSceneNode", "get_node_path");
	___mb.mb_set_node_path = godot::api->godot_method_bind_get_method("VisualScriptSceneNode", "set_node_path");
}

VisualScriptSceneNode *VisualScriptSceneNode::_new()
{
	return (VisualScriptSceneNode *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptSceneNode")());
}
NodePath VisualScriptSceneNode::get_node_path() {
	return ___godot_icall_NodePath(___mb.mb_get_node_path, (const Object *) this);
}

void VisualScriptSceneNode::set_node_path(const NodePath path) {
	___godot_icall_void_NodePath(___mb.mb_set_node_path, (const Object *) this, path);
}

}