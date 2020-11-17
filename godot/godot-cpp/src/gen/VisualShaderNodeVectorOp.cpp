#include "VisualShaderNodeVectorOp.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNodeVectorOp::___method_bindings VisualShaderNodeVectorOp::___mb = {};

void VisualShaderNodeVectorOp::___init_method_bindings() {
	___mb.mb_get_operator = godot::api->godot_method_bind_get_method("VisualShaderNodeVectorOp", "get_operator");
	___mb.mb_set_operator = godot::api->godot_method_bind_get_method("VisualShaderNodeVectorOp", "set_operator");
}

VisualShaderNodeVectorOp *VisualShaderNodeVectorOp::_new()
{
	return (VisualShaderNodeVectorOp *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeVectorOp")());
}
VisualShaderNodeVectorOp::Operator VisualShaderNodeVectorOp::get_operator() const {
	return (VisualShaderNodeVectorOp::Operator) ___godot_icall_int(___mb.mb_get_operator, (const Object *) this);
}

void VisualShaderNodeVectorOp::set_operator(const int64_t op) {
	___godot_icall_void_int(___mb.mb_set_operator, (const Object *) this, op);
}

}