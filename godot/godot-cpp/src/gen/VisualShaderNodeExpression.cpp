#include "VisualShaderNodeExpression.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNodeExpression::___method_bindings VisualShaderNodeExpression::___mb = {};

void VisualShaderNodeExpression::___init_method_bindings() {
	___mb.mb_get_expression = godot::api->godot_method_bind_get_method("VisualShaderNodeExpression", "get_expression");
	___mb.mb_set_expression = godot::api->godot_method_bind_get_method("VisualShaderNodeExpression", "set_expression");
}

VisualShaderNodeExpression *VisualShaderNodeExpression::_new()
{
	return (VisualShaderNodeExpression *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeExpression")());
}
String VisualShaderNodeExpression::get_expression() const {
	return ___godot_icall_String(___mb.mb_get_expression, (const Object *) this);
}

void VisualShaderNodeExpression::set_expression(const String expression) {
	___godot_icall_void_String(___mb.mb_set_expression, (const Object *) this, expression);
}

}