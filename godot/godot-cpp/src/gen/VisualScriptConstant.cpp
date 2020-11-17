#include "VisualScriptConstant.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptConstant::___method_bindings VisualScriptConstant::___mb = {};

void VisualScriptConstant::___init_method_bindings() {
	___mb.mb_get_constant_type = godot::api->godot_method_bind_get_method("VisualScriptConstant", "get_constant_type");
	___mb.mb_get_constant_value = godot::api->godot_method_bind_get_method("VisualScriptConstant", "get_constant_value");
	___mb.mb_set_constant_type = godot::api->godot_method_bind_get_method("VisualScriptConstant", "set_constant_type");
	___mb.mb_set_constant_value = godot::api->godot_method_bind_get_method("VisualScriptConstant", "set_constant_value");
}

VisualScriptConstant *VisualScriptConstant::_new()
{
	return (VisualScriptConstant *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptConstant")());
}
Variant::Type VisualScriptConstant::get_constant_type() const {
	return (Variant::Type) ___godot_icall_int(___mb.mb_get_constant_type, (const Object *) this);
}

Variant VisualScriptConstant::get_constant_value() const {
	return ___godot_icall_Variant(___mb.mb_get_constant_value, (const Object *) this);
}

void VisualScriptConstant::set_constant_type(const int64_t type) {
	___godot_icall_void_int(___mb.mb_set_constant_type, (const Object *) this, type);
}

void VisualScriptConstant::set_constant_value(const Variant value) {
	___godot_icall_void_Variant(___mb.mb_set_constant_value, (const Object *) this, value);
}

}