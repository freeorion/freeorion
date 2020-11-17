#include "VisualScriptBasicTypeConstant.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptBasicTypeConstant::___method_bindings VisualScriptBasicTypeConstant::___mb = {};

void VisualScriptBasicTypeConstant::___init_method_bindings() {
	___mb.mb_get_basic_type = godot::api->godot_method_bind_get_method("VisualScriptBasicTypeConstant", "get_basic_type");
	___mb.mb_get_basic_type_constant = godot::api->godot_method_bind_get_method("VisualScriptBasicTypeConstant", "get_basic_type_constant");
	___mb.mb_set_basic_type = godot::api->godot_method_bind_get_method("VisualScriptBasicTypeConstant", "set_basic_type");
	___mb.mb_set_basic_type_constant = godot::api->godot_method_bind_get_method("VisualScriptBasicTypeConstant", "set_basic_type_constant");
}

VisualScriptBasicTypeConstant *VisualScriptBasicTypeConstant::_new()
{
	return (VisualScriptBasicTypeConstant *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptBasicTypeConstant")());
}
Variant::Type VisualScriptBasicTypeConstant::get_basic_type() const {
	return (Variant::Type) ___godot_icall_int(___mb.mb_get_basic_type, (const Object *) this);
}

String VisualScriptBasicTypeConstant::get_basic_type_constant() const {
	return ___godot_icall_String(___mb.mb_get_basic_type_constant, (const Object *) this);
}

void VisualScriptBasicTypeConstant::set_basic_type(const int64_t name) {
	___godot_icall_void_int(___mb.mb_set_basic_type, (const Object *) this, name);
}

void VisualScriptBasicTypeConstant::set_basic_type_constant(const String name) {
	___godot_icall_void_String(___mb.mb_set_basic_type_constant, (const Object *) this, name);
}

}