#include "VisualShaderNodeVec3Constant.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNodeVec3Constant::___method_bindings VisualShaderNodeVec3Constant::___mb = {};

void VisualShaderNodeVec3Constant::___init_method_bindings() {
	___mb.mb_get_constant = godot::api->godot_method_bind_get_method("VisualShaderNodeVec3Constant", "get_constant");
	___mb.mb_set_constant = godot::api->godot_method_bind_get_method("VisualShaderNodeVec3Constant", "set_constant");
}

VisualShaderNodeVec3Constant *VisualShaderNodeVec3Constant::_new()
{
	return (VisualShaderNodeVec3Constant *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeVec3Constant")());
}
Vector3 VisualShaderNodeVec3Constant::get_constant() const {
	return ___godot_icall_Vector3(___mb.mb_get_constant, (const Object *) this);
}

void VisualShaderNodeVec3Constant::set_constant(const Vector3 value) {
	___godot_icall_void_Vector3(___mb.mb_set_constant, (const Object *) this, value);
}

}