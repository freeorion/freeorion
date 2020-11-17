#include "CylinderShape.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


CylinderShape::___method_bindings CylinderShape::___mb = {};

void CylinderShape::___init_method_bindings() {
	___mb.mb_get_height = godot::api->godot_method_bind_get_method("CylinderShape", "get_height");
	___mb.mb_get_radius = godot::api->godot_method_bind_get_method("CylinderShape", "get_radius");
	___mb.mb_set_height = godot::api->godot_method_bind_get_method("CylinderShape", "set_height");
	___mb.mb_set_radius = godot::api->godot_method_bind_get_method("CylinderShape", "set_radius");
}

CylinderShape *CylinderShape::_new()
{
	return (CylinderShape *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CylinderShape")());
}
real_t CylinderShape::get_height() const {
	return ___godot_icall_float(___mb.mb_get_height, (const Object *) this);
}

real_t CylinderShape::get_radius() const {
	return ___godot_icall_float(___mb.mb_get_radius, (const Object *) this);
}

void CylinderShape::set_height(const real_t height) {
	___godot_icall_void_float(___mb.mb_set_height, (const Object *) this, height);
}

void CylinderShape::set_radius(const real_t radius) {
	___godot_icall_void_float(___mb.mb_set_radius, (const Object *) this, radius);
}

}