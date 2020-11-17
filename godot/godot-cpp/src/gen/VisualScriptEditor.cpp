#include "VisualScriptEditor.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Script.hpp"


namespace godot {


VisualScriptEditor *VisualScriptEditor::_singleton = NULL;


VisualScriptEditor::VisualScriptEditor() {
	_owner = godot::api->godot_global_get_singleton((char *) "VisualScriptEditor");
}


VisualScriptEditor::___method_bindings VisualScriptEditor::___mb = {};

void VisualScriptEditor::___init_method_bindings() {
	___mb.mb_add_custom_node = godot::api->godot_method_bind_get_method("_VisualScriptEditor", "add_custom_node");
	___mb.mb_remove_custom_node = godot::api->godot_method_bind_get_method("_VisualScriptEditor", "remove_custom_node");
}

void VisualScriptEditor::add_custom_node(const String name, const String category, const Ref<Script> script) {
	___godot_icall_void_String_String_Object(___mb.mb_add_custom_node, (const Object *) this, name, category, script.ptr());
}

void VisualScriptEditor::remove_custom_node(const String name, const String category) {
	___godot_icall_void_String_String(___mb.mb_remove_custom_node, (const Object *) this, name, category);
}

}