#include "RayShape2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


RayShape2D::___method_bindings RayShape2D::___mb = {};

void RayShape2D::___init_method_bindings() {
	___mb.mb_get_length = godot::api->godot_method_bind_get_method("RayShape2D", "get_length");
	___mb.mb_get_slips_on_slope = godot::api->godot_method_bind_get_method("RayShape2D", "get_slips_on_slope");
	___mb.mb_set_length = godot::api->godot_method_bind_get_method("RayShape2D", "set_length");
	___mb.mb_set_slips_on_slope = godot::api->godot_method_bind_get_method("RayShape2D", "set_slips_on_slope");
}

RayShape2D *RayShape2D::_new()
{
	return (RayShape2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"RayShape2D")());
}
real_t RayShape2D::get_length() const {
	return ___godot_icall_float(___mb.mb_get_length, (const Object *) this);
}

bool RayShape2D::get_slips_on_slope() const {
	return ___godot_icall_bool(___mb.mb_get_slips_on_slope, (const Object *) this);
}

void RayShape2D::set_length(const real_t length) {
	___godot_icall_void_float(___mb.mb_set_length, (const Object *) this, length);
}

void RayShape2D::set_slips_on_slope(const bool active) {
	___godot_icall_void_bool(___mb.mb_set_slips_on_slope, (const Object *) this, active);
}

}