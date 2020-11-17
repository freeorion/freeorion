#include "VisualShaderNodeIs.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNodeIs::___method_bindings VisualShaderNodeIs::___mb = {};

void VisualShaderNodeIs::___init_method_bindings() {
	___mb.mb_get_function = godot::api->godot_method_bind_get_method("VisualShaderNodeIs", "get_function");
	___mb.mb_set_function = godot::api->godot_method_bind_get_method("VisualShaderNodeIs", "set_function");
}

VisualShaderNodeIs *VisualShaderNodeIs::_new()
{
	return (VisualShaderNodeIs *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeIs")());
}
VisualShaderNodeIs::Function VisualShaderNodeIs::get_function() const {
	return (VisualShaderNodeIs::Function) ___godot_icall_int(___mb.mb_get_function, (const Object *) this);
}

void VisualShaderNodeIs::set_function(const int64_t func) {
	___godot_icall_void_int(___mb.mb_set_function, (const Object *) this, func);
}

}