#include "CapsuleShape2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


CapsuleShape2D::___method_bindings CapsuleShape2D::___mb = {};

void CapsuleShape2D::___init_method_bindings() {
	___mb.mb_get_height = godot::api->godot_method_bind_get_method("CapsuleShape2D", "get_height");
	___mb.mb_get_radius = godot::api->godot_method_bind_get_method("CapsuleShape2D", "get_radius");
	___mb.mb_set_height = godot::api->godot_method_bind_get_method("CapsuleShape2D", "set_height");
	___mb.mb_set_radius = godot::api->godot_method_bind_get_method("CapsuleShape2D", "set_radius");
}

CapsuleShape2D *CapsuleShape2D::_new()
{
	return (CapsuleShape2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CapsuleShape2D")());
}
real_t CapsuleShape2D::get_height() const {
	return ___godot_icall_float(___mb.mb_get_height, (const Object *) this);
}

real_t CapsuleShape2D::get_radius() const {
	return ___godot_icall_float(___mb.mb_get_radius, (const Object *) this);
}

void CapsuleShape2D::set_height(const real_t height) {
	___godot_icall_void_float(___mb.mb_set_height, (const Object *) this, height);
}

void CapsuleShape2D::set_radius(const real_t radius) {
	___godot_icall_void_float(___mb.mb_set_radius, (const Object *) this, radius);
}

}