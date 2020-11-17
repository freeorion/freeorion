#include "CircleShape2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


CircleShape2D::___method_bindings CircleShape2D::___mb = {};

void CircleShape2D::___init_method_bindings() {
	___mb.mb_get_radius = godot::api->godot_method_bind_get_method("CircleShape2D", "get_radius");
	___mb.mb_set_radius = godot::api->godot_method_bind_get_method("CircleShape2D", "set_radius");
}

CircleShape2D *CircleShape2D::_new()
{
	return (CircleShape2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CircleShape2D")());
}
real_t CircleShape2D::get_radius() const {
	return ___godot_icall_float(___mb.mb_get_radius, (const Object *) this);
}

void CircleShape2D::set_radius(const real_t radius) {
	___godot_icall_void_float(___mb.mb_set_radius, (const Object *) this, radius);
}

}