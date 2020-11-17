#include "EditorInspectorPlugin.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Control.hpp"
#include "Object.hpp"


namespace godot {


EditorInspectorPlugin::___method_bindings EditorInspectorPlugin::___mb = {};

void EditorInspectorPlugin::___init_method_bindings() {
	___mb.mb_add_custom_control = godot::api->godot_method_bind_get_method("EditorInspectorPlugin", "add_custom_control");
	___mb.mb_add_property_editor = godot::api->godot_method_bind_get_method("EditorInspectorPlugin", "add_property_editor");
	___mb.mb_add_property_editor_for_multiple_properties = godot::api->godot_method_bind_get_method("EditorInspectorPlugin", "add_property_editor_for_multiple_properties");
	___mb.mb_can_handle = godot::api->godot_method_bind_get_method("EditorInspectorPlugin", "can_handle");
	___mb.mb_parse_begin = godot::api->godot_method_bind_get_method("EditorInspectorPlugin", "parse_begin");
	___mb.mb_parse_category = godot::api->godot_method_bind_get_method("EditorInspectorPlugin", "parse_category");
	___mb.mb_parse_end = godot::api->godot_method_bind_get_method("EditorInspectorPlugin", "parse_end");
	___mb.mb_parse_property = godot::api->godot_method_bind_get_method("EditorInspectorPlugin", "parse_property");
}

void EditorInspectorPlugin::add_custom_control(const Control *control) {
	___godot_icall_void_Object(___mb.mb_add_custom_control, (const Object *) this, control);
}

void EditorInspectorPlugin::add_property_editor(const String property, const Control *editor) {
	___godot_icall_void_String_Object(___mb.mb_add_property_editor, (const Object *) this, property, editor);
}

void EditorInspectorPlugin::add_property_editor_for_multiple_properties(const String label, const PoolStringArray properties, const Control *editor) {
	___godot_icall_void_String_PoolStringArray_Object(___mb.mb_add_property_editor_for_multiple_properties, (const Object *) this, label, properties, editor);
}

bool EditorInspectorPlugin::can_handle(const Object *object) {
	return ___godot_icall_bool_Object(___mb.mb_can_handle, (const Object *) this, object);
}

void EditorInspectorPlugin::parse_begin(const Object *object) {
	___godot_icall_void_Object(___mb.mb_parse_begin, (const Object *) this, object);
}

void EditorInspectorPlugin::parse_category(const Object *object, const String category) {
	___godot_icall_void_Object_String(___mb.mb_parse_category, (const Object *) this, object, category);
}

void EditorInspectorPlugin::parse_end() {
	___godot_icall_void(___mb.mb_parse_end, (const Object *) this);
}

bool EditorInspectorPlugin::parse_property(const Object *object, const int64_t type, const String path, const int64_t hint, const String hint_text, const int64_t usage) {
	return ___godot_icall_bool_Object_int_String_int_String_int(___mb.mb_parse_property, (const Object *) this, object, type, path, hint, hint_text, usage);
}

}