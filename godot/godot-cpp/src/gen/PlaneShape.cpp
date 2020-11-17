#include "PlaneShape.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PlaneShape::___method_bindings PlaneShape::___mb = {};

void PlaneShape::___init_method_bindings() {
	___mb.mb_get_plane = godot::api->godot_method_bind_get_method("PlaneShape", "get_plane");
	___mb.mb_set_plane = godot::api->godot_method_bind_get_method("PlaneShape", "set_plane");
}

PlaneShape *PlaneShape::_new()
{
	return (PlaneShape *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PlaneShape")());
}
Plane PlaneShape::get_plane() const {
	return ___godot_icall_Plane(___mb.mb_get_plane, (const Object *) this);
}

void PlaneShape::set_plane(const Plane plane) {
	___godot_icall_void_Plane(___mb.mb_set_plane, (const Object *) this, plane);
}

}