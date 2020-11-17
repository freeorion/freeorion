#include "CapsuleShape.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


CapsuleShape::___method_bindings CapsuleShape::___mb = {};

void CapsuleShape::___init_method_bindings() {
	___mb.mb_get_height = godot::api->godot_method_bind_get_method("CapsuleShape", "get_height");
	___mb.mb_get_radius = godot::api->godot_method_bind_get_method("CapsuleShape", "get_radius");
	___mb.mb_set_height = godot::api->godot_method_bind_get_method("CapsuleShape", "set_height");
	___mb.mb_set_radius = godot::api->godot_method_bind_get_method("CapsuleShape", "set_radius");
}

CapsuleShape *CapsuleShape::_new()
{
	return (CapsuleShape *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CapsuleShape")());
}
real_t CapsuleShape::get_height() const {
	return ___godot_icall_float(___mb.mb_get_height, (const Object *) this);
}

real_t CapsuleShape::get_radius() const {
	return ___godot_icall_float(___mb.mb_get_radius, (const Object *) this);
}

void CapsuleShape::set_height(const real_t height) {
	___godot_icall_void_float(___mb.mb_set_height, (const Object *) this, height);
}

void CapsuleShape::set_radius(const real_t radius) {
	___godot_icall_void_float(___mb.mb_set_radius, (const Object *) this, radius);
}

}