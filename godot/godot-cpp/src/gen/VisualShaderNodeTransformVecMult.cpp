#include "VisualShaderNodeTransformVecMult.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNodeTransformVecMult::___method_bindings VisualShaderNodeTransformVecMult::___mb = {};

void VisualShaderNodeTransformVecMult::___init_method_bindings() {
	___mb.mb_get_operator = godot::api->godot_method_bind_get_method("VisualShaderNodeTransformVecMult", "get_operator");
	___mb.mb_set_operator = godot::api->godot_method_bind_get_method("VisualShaderNodeTransformVecMult", "set_operator");
}

VisualShaderNodeTransformVecMult *VisualShaderNodeTransformVecMult::_new()
{
	return (VisualShaderNodeTransformVecMult *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeTransformVecMult")());
}
VisualShaderNodeTransformVecMult::Operator VisualShaderNodeTransformVecMult::get_operator() const {
	return (VisualShaderNodeTransformVecMult::Operator) ___godot_icall_int(___mb.mb_get_operator, (const Object *) this);
}

void VisualShaderNodeTransformVecMult::set_operator(const int64_t op) {
	___godot_icall_void_int(___mb.mb_set_operator, (const Object *) this, op);
}

}