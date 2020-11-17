#include "VisualScriptMathConstant.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptMathConstant::___method_bindings VisualScriptMathConstant::___mb = {};

void VisualScriptMathConstant::___init_method_bindings() {
	___mb.mb_get_math_constant = godot::api->godot_method_bind_get_method("VisualScriptMathConstant", "get_math_constant");
	___mb.mb_set_math_constant = godot::api->godot_method_bind_get_method("VisualScriptMathConstant", "set_math_constant");
}

VisualScriptMathConstant *VisualScriptMathConstant::_new()
{
	return (VisualScriptMathConstant *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptMathConstant")());
}
VisualScriptMathConstant::MathConstant VisualScriptMathConstant::get_math_constant() {
	return (VisualScriptMathConstant::MathConstant) ___godot_icall_int(___mb.mb_get_math_constant, (const Object *) this);
}

void VisualScriptMathConstant::set_math_constant(const int64_t which) {
	___godot_icall_void_int(___mb.mb_set_math_constant, (const Object *) this, which);
}

}