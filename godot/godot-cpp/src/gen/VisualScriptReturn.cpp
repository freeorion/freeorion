#include "VisualScriptReturn.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptReturn::___method_bindings VisualScriptReturn::___mb = {};

void VisualScriptReturn::___init_method_bindings() {
	___mb.mb_get_return_type = godot::api->godot_method_bind_get_method("VisualScriptReturn", "get_return_type");
	___mb.mb_is_return_value_enabled = godot::api->godot_method_bind_get_method("VisualScriptReturn", "is_return_value_enabled");
	___mb.mb_set_enable_return_value = godot::api->godot_method_bind_get_method("VisualScriptReturn", "set_enable_return_value");
	___mb.mb_set_return_type = godot::api->godot_method_bind_get_method("VisualScriptReturn", "set_return_type");
}

VisualScriptReturn *VisualScriptReturn::_new()
{
	return (VisualScriptReturn *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptReturn")());
}
Variant::Type VisualScriptReturn::get_return_type() const {
	return (Variant::Type) ___godot_icall_int(___mb.mb_get_return_type, (const Object *) this);
}

bool VisualScriptReturn::is_return_value_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_return_value_enabled, (const Object *) this);
}

void VisualScriptReturn::set_enable_return_value(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_enable_return_value, (const Object *) this, enable);
}

void VisualScriptReturn::set_return_type(const int64_t type) {
	___godot_icall_void_int(___mb.mb_set_return_type, (const Object *) this, type);
}

}