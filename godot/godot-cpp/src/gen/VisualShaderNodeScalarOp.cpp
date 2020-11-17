#include "VisualShaderNodeScalarOp.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNodeScalarOp::___method_bindings VisualShaderNodeScalarOp::___mb = {};

void VisualShaderNodeScalarOp::___init_method_bindings() {
	___mb.mb_get_operator = godot::api->godot_method_bind_get_method("VisualShaderNodeScalarOp", "get_operator");
	___mb.mb_set_operator = godot::api->godot_method_bind_get_method("VisualShaderNodeScalarOp", "set_operator");
}

VisualShaderNodeScalarOp *VisualShaderNodeScalarOp::_new()
{
	return (VisualShaderNodeScalarOp *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeScalarOp")());
}
VisualShaderNodeScalarOp::Operator VisualShaderNodeScalarOp::get_operator() const {
	return (VisualShaderNodeScalarOp::Operator) ___godot_icall_int(___mb.mb_get_operator, (const Object *) this);
}

void VisualShaderNodeScalarOp::set_operator(const int64_t op) {
	___godot_icall_void_int(___mb.mb_set_operator, (const Object *) this, op);
}

}