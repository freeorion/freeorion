#include "Position2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Position2D::___method_bindings Position2D::___mb = {};

void Position2D::___init_method_bindings() {
	___mb.mb__get_gizmo_extents = godot::api->godot_method_bind_get_method("Position2D", "_get_gizmo_extents");
	___mb.mb__set_gizmo_extents = godot::api->godot_method_bind_get_method("Position2D", "_set_gizmo_extents");
}

Position2D *Position2D::_new()
{
	return (Position2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Position2D")());
}
real_t Position2D::_get_gizmo_extents() const {
	return ___godot_icall_float(___mb.mb__get_gizmo_extents, (const Object *) this);
}

void Position2D::_set_gizmo_extents(const real_t extents) {
	___godot_icall_void_float(___mb.mb__set_gizmo_extents, (const Object *) this, extents);
}

}