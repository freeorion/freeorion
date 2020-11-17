#include "VisualScriptInputAction.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptInputAction::___method_bindings VisualScriptInputAction::___mb = {};

void VisualScriptInputAction::___init_method_bindings() {
	___mb.mb_get_action_mode = godot::api->godot_method_bind_get_method("VisualScriptInputAction", "get_action_mode");
	___mb.mb_get_action_name = godot::api->godot_method_bind_get_method("VisualScriptInputAction", "get_action_name");
	___mb.mb_set_action_mode = godot::api->godot_method_bind_get_method("VisualScriptInputAction", "set_action_mode");
	___mb.mb_set_action_name = godot::api->godot_method_bind_get_method("VisualScriptInputAction", "set_action_name");
}

VisualScriptInputAction *VisualScriptInputAction::_new()
{
	return (VisualScriptInputAction *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptInputAction")());
}
VisualScriptInputAction::Mode VisualScriptInputAction::get_action_mode() const {
	return (VisualScriptInputAction::Mode) ___godot_icall_int(___mb.mb_get_action_mode, (const Object *) this);
}

String VisualScriptInputAction::get_action_name() const {
	return ___godot_icall_String(___mb.mb_get_action_name, (const Object *) this);
}

void VisualScriptInputAction::set_action_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_action_mode, (const Object *) this, mode);
}

void VisualScriptInputAction::set_action_name(const String name) {
	___godot_icall_void_String(___mb.mb_set_action_name, (const Object *) this, name);
}

}