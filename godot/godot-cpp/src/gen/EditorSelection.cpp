#include "EditorSelection.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Node.hpp"


namespace godot {


EditorSelection::___method_bindings EditorSelection::___mb = {};

void EditorSelection::___init_method_bindings() {
	___mb.mb__emit_change = godot::api->godot_method_bind_get_method("EditorSelection", "_emit_change");
	___mb.mb__node_removed = godot::api->godot_method_bind_get_method("EditorSelection", "_node_removed");
	___mb.mb_add_node = godot::api->godot_method_bind_get_method("EditorSelection", "add_node");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("EditorSelection", "clear");
	___mb.mb_get_selected_nodes = godot::api->godot_method_bind_get_method("EditorSelection", "get_selected_nodes");
	___mb.mb_get_transformable_selected_nodes = godot::api->godot_method_bind_get_method("EditorSelection", "get_transformable_selected_nodes");
	___mb.mb_remove_node = godot::api->godot_method_bind_get_method("EditorSelection", "remove_node");
}

void EditorSelection::_emit_change() {
	___godot_icall_void(___mb.mb__emit_change, (const Object *) this);
}

void EditorSelection::_node_removed(const Node *arg0) {
	___godot_icall_void_Object(___mb.mb__node_removed, (const Object *) this, arg0);
}

void EditorSelection::add_node(const Node *node) {
	___godot_icall_void_Object(___mb.mb_add_node, (const Object *) this, node);
}

void EditorSelection::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

Array EditorSelection::get_selected_nodes() {
	return ___godot_icall_Array(___mb.mb_get_selected_nodes, (const Object *) this);
}

Array EditorSelection::get_transformable_selected_nodes() {
	return ___godot_icall_Array(___mb.mb_get_transformable_selected_nodes, (const Object *) this);
}

void EditorSelection::remove_node(const Node *node) {
	___godot_icall_void_Object(___mb.mb_remove_node, (const Object *) this, node);
}

}