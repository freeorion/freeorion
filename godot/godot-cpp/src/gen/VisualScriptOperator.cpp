#include "VisualScriptOperator.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptOperator::___method_bindings VisualScriptOperator::___mb = {};

void VisualScriptOperator::___init_method_bindings() {
	___mb.mb_get_operator = godot::api->godot_method_bind_get_method("VisualScriptOperator", "get_operator");
	___mb.mb_get_typed = godot::api->godot_method_bind_get_method("VisualScriptOperator", "get_typed");
	___mb.mb_set_operator = godot::api->godot_method_bind_get_method("VisualScriptOperator", "set_operator");
	___mb.mb_set_typed = godot::api->godot_method_bind_get_method("VisualScriptOperator", "set_typed");
}

VisualScriptOperator *VisualScriptOperator::_new()
{
	return (VisualScriptOperator *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptOperator")());
}
Variant::Operator VisualScriptOperator::get_operator() const {
	return (Variant::Operator) ___godot_icall_int(___mb.mb_get_operator, (const Object *) this);
}

Variant::Type VisualScriptOperator::get_typed() const {
	return (Variant::Type) ___godot_icall_int(___mb.mb_get_typed, (const Object *) this);
}

void VisualScriptOperator::set_operator(const int64_t op) {
	___godot_icall_void_int(___mb.mb_set_operator, (const Object *) this, op);
}

void VisualScriptOperator::set_typed(const int64_t type) {
	___godot_icall_void_int(___mb.mb_set_typed, (const Object *) this, type);
}

}