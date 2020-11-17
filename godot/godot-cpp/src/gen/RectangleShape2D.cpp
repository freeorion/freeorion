#include "RectangleShape2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


RectangleShape2D::___method_bindings RectangleShape2D::___mb = {};

void RectangleShape2D::___init_method_bindings() {
	___mb.mb_get_extents = godot::api->godot_method_bind_get_method("RectangleShape2D", "get_extents");
	___mb.mb_set_extents = godot::api->godot_method_bind_get_method("RectangleShape2D", "set_extents");
}

RectangleShape2D *RectangleShape2D::_new()
{
	return (RectangleShape2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"RectangleShape2D")());
}
Vector2 RectangleShape2D::get_extents() const {
	return ___godot_icall_Vector2(___mb.mb_get_extents, (const Object *) this);
}

void RectangleShape2D::set_extents(const Vector2 extents) {
	___godot_icall_void_Vector2(___mb.mb_set_extents, (const Object *) this, extents);
}

}