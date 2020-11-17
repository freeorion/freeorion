#include "VisualShaderNodeCompare.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNodeCompare::___method_bindings VisualShaderNodeCompare::___mb = {};

void VisualShaderNodeCompare::___init_method_bindings() {
	___mb.mb_get_comparison_type = godot::api->godot_method_bind_get_method("VisualShaderNodeCompare", "get_comparison_type");
	___mb.mb_get_condition = godot::api->godot_method_bind_get_method("VisualShaderNodeCompare", "get_condition");
	___mb.mb_get_function = godot::api->godot_method_bind_get_method("VisualShaderNodeCompare", "get_function");
	___mb.mb_set_comparison_type = godot::api->godot_method_bind_get_method("VisualShaderNodeCompare", "set_comparison_type");
	___mb.mb_set_condition = godot::api->godot_method_bind_get_method("VisualShaderNodeCompare", "set_condition");
	___mb.mb_set_function = godot::api->godot_method_bind_get_method("VisualShaderNodeCompare", "set_function");
}

VisualShaderNodeCompare *VisualShaderNodeCompare::_new()
{
	return (VisualShaderNodeCompare *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeCompare")());
}
VisualShaderNodeCompare::ComparisonType VisualShaderNodeCompare::get_comparison_type() const {
	return (VisualShaderNodeCompare::ComparisonType) ___godot_icall_int(___mb.mb_get_comparison_type, (const Object *) this);
}

VisualShaderNodeCompare::Condition VisualShaderNodeCompare::get_condition() const {
	return (VisualShaderNodeCompare::Condition) ___godot_icall_int(___mb.mb_get_condition, (const Object *) this);
}

VisualShaderNodeCompare::Function VisualShaderNodeCompare::get_function() const {
	return (VisualShaderNodeCompare::Function) ___godot_icall_int(___mb.mb_get_function, (const Object *) this);
}

void VisualShaderNodeCompare::set_comparison_type(const int64_t type) {
	___godot_icall_void_int(___mb.mb_set_comparison_type, (const Object *) this, type);
}

void VisualShaderNodeCompare::set_condition(const int64_t condition) {
	___godot_icall_void_int(___mb.mb_set_condition, (const Object *) this, condition);
}

void VisualShaderNodeCompare::set_function(const int64_t func) {
	___godot_icall_void_int(___mb.mb_set_function, (const Object *) this, func);
}

}