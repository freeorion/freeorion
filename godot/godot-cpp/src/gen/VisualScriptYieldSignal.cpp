#include "VisualScriptYieldSignal.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptYieldSignal::___method_bindings VisualScriptYieldSignal::___mb = {};

void VisualScriptYieldSignal::___init_method_bindings() {
	___mb.mb_get_base_path = godot::api->godot_method_bind_get_method("VisualScriptYieldSignal", "get_base_path");
	___mb.mb_get_base_type = godot::api->godot_method_bind_get_method("VisualScriptYieldSignal", "get_base_type");
	___mb.mb_get_call_mode = godot::api->godot_method_bind_get_method("VisualScriptYieldSignal", "get_call_mode");
	___mb.mb_get_signal = godot::api->godot_method_bind_get_method("VisualScriptYieldSignal", "get_signal");
	___mb.mb_set_base_path = godot::api->godot_method_bind_get_method("VisualScriptYieldSignal", "set_base_path");
	___mb.mb_set_base_type = godot::api->godot_method_bind_get_method("VisualScriptYieldSignal", "set_base_type");
	___mb.mb_set_call_mode = godot::api->godot_method_bind_get_method("VisualScriptYieldSignal", "set_call_mode");
	___mb.mb_set_signal = godot::api->godot_method_bind_get_method("VisualScriptYieldSignal", "set_signal");
}

VisualScriptYieldSignal *VisualScriptYieldSignal::_new()
{
	return (VisualScriptYieldSignal *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptYieldSignal")());
}
NodePath VisualScriptYieldSignal::get_base_path() const {
	return ___godot_icall_NodePath(___mb.mb_get_base_path, (const Object *) this);
}

String VisualScriptYieldSignal::get_base_type() const {
	return ___godot_icall_String(___mb.mb_get_base_type, (const Object *) this);
}

VisualScriptYieldSignal::CallMode VisualScriptYieldSignal::get_call_mode() const {
	return (VisualScriptYieldSignal::CallMode) ___godot_icall_int(___mb.mb_get_call_mode, (const Object *) this);
}

String VisualScriptYieldSignal::get_signal() const {
	return ___godot_icall_String(___mb.mb_get_signal, (const Object *) this);
}

void VisualScriptYieldSignal::set_base_path(const NodePath base_path) {
	___godot_icall_void_NodePath(___mb.mb_set_base_path, (const Object *) this, base_path);
}

void VisualScriptYieldSignal::set_base_type(const String base_type) {
	___godot_icall_void_String(___mb.mb_set_base_type, (const Object *) this, base_type);
}

void VisualScriptYieldSignal::set_call_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_call_mode, (const Object *) this, mode);
}

void VisualScriptYieldSignal::set_signal(const String signal) {
	___godot_icall_void_String(___mb.mb_set_signal, (const Object *) this, signal);
}

}