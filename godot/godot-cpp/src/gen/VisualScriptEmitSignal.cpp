#include "VisualScriptEmitSignal.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptEmitSignal::___method_bindings VisualScriptEmitSignal::___mb = {};

void VisualScriptEmitSignal::___init_method_bindings() {
	___mb.mb_get_signal = godot::api->godot_method_bind_get_method("VisualScriptEmitSignal", "get_signal");
	___mb.mb_set_signal = godot::api->godot_method_bind_get_method("VisualScriptEmitSignal", "set_signal");
}

VisualScriptEmitSignal *VisualScriptEmitSignal::_new()
{
	return (VisualScriptEmitSignal *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptEmitSignal")());
}
String VisualScriptEmitSignal::get_signal() const {
	return ___godot_icall_String(___mb.mb_get_signal, (const Object *) this);
}

void VisualScriptEmitSignal::set_signal(const String name) {
	___godot_icall_void_String(___mb.mb_set_signal, (const Object *) this, name);
}

}