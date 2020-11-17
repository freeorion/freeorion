#include "EditorScript.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Node.hpp"
#include "EditorInterface.hpp"


namespace godot {


EditorScript::___method_bindings EditorScript::___mb = {};

void EditorScript::___init_method_bindings() {
	___mb.mb__run = godot::api->godot_method_bind_get_method("EditorScript", "_run");
	___mb.mb_add_root_node = godot::api->godot_method_bind_get_method("EditorScript", "add_root_node");
	___mb.mb_get_editor_interface = godot::api->godot_method_bind_get_method("EditorScript", "get_editor_interface");
	___mb.mb_get_scene = godot::api->godot_method_bind_get_method("EditorScript", "get_scene");
}

void EditorScript::_run() {
	___godot_icall_void(___mb.mb__run, (const Object *) this);
}

void EditorScript::add_root_node(const Node *node) {
	___godot_icall_void_Object(___mb.mb_add_root_node, (const Object *) this, node);
}

EditorInterface *EditorScript::get_editor_interface() {
	return (EditorInterface *) ___godot_icall_Object(___mb.mb_get_editor_interface, (const Object *) this);
}

Node *EditorScript::get_scene() {
	return (Node *) ___godot_icall_Object(___mb.mb_get_scene, (const Object *) this);
}

}