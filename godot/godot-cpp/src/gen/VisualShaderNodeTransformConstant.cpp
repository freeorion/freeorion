#include "VisualShaderNodeTransformConstant.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNodeTransformConstant::___method_bindings VisualShaderNodeTransformConstant::___mb = {};

void VisualShaderNodeTransformConstant::___init_method_bindings() {
	___mb.mb_get_constant = godot::api->godot_method_bind_get_method("VisualShaderNodeTransformConstant", "get_constant");
	___mb.mb_set_constant = godot::api->godot_method_bind_get_method("VisualShaderNodeTransformConstant", "set_constant");
}

VisualShaderNodeTransformConstant *VisualShaderNodeTransformConstant::_new()
{
	return (VisualShaderNodeTransformConstant *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeTransformConstant")());
}
Transform VisualShaderNodeTransformConstant::get_constant() const {
	return ___godot_icall_Transform(___mb.mb_get_constant, (const Object *) this);
}

void VisualShaderNodeTransformConstant::set_constant(const Transform value) {
	___godot_icall_void_Transform(___mb.mb_set_constant, (const Object *) this, value);
}

}