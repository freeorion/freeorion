#include "VisualScriptComment.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptComment::___method_bindings VisualScriptComment::___mb = {};

void VisualScriptComment::___init_method_bindings() {
	___mb.mb_get_description = godot::api->godot_method_bind_get_method("VisualScriptComment", "get_description");
	___mb.mb_get_size = godot::api->godot_method_bind_get_method("VisualScriptComment", "get_size");
	___mb.mb_get_title = godot::api->godot_method_bind_get_method("VisualScriptComment", "get_title");
	___mb.mb_set_description = godot::api->godot_method_bind_get_method("VisualScriptComment", "set_description");
	___mb.mb_set_size = godot::api->godot_method_bind_get_method("VisualScriptComment", "set_size");
	___mb.mb_set_title = godot::api->godot_method_bind_get_method("VisualScriptComment", "set_title");
}

VisualScriptComment *VisualScriptComment::_new()
{
	return (VisualScriptComment *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptComment")());
}
String VisualScriptComment::get_description() const {
	return ___godot_icall_String(___mb.mb_get_description, (const Object *) this);
}

Vector2 VisualScriptComment::get_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_size, (const Object *) this);
}

String VisualScriptComment::get_title() const {
	return ___godot_icall_String(___mb.mb_get_title, (const Object *) this);
}

void VisualScriptComment::set_description(const String description) {
	___godot_icall_void_String(___mb.mb_set_description, (const Object *) this, description);
}

void VisualScriptComment::set_size(const Vector2 size) {
	___godot_icall_void_Vector2(___mb.mb_set_size, (const Object *) this, size);
}

void VisualScriptComment::set_title(const String title) {
	___godot_icall_void_String(___mb.mb_set_title, (const Object *) this, title);
}

}