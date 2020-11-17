#include "StaticBody2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "PhysicsMaterial.hpp"


namespace godot {


StaticBody2D::___method_bindings StaticBody2D::___mb = {};

void StaticBody2D::___init_method_bindings() {
	___mb.mb__reload_physics_characteristics = godot::api->godot_method_bind_get_method("StaticBody2D", "_reload_physics_characteristics");
	___mb.mb_get_bounce = godot::api->godot_method_bind_get_method("StaticBody2D", "get_bounce");
	___mb.mb_get_constant_angular_velocity = godot::api->godot_method_bind_get_method("StaticBody2D", "get_constant_angular_velocity");
	___mb.mb_get_constant_linear_velocity = godot::api->godot_method_bind_get_method("StaticBody2D", "get_constant_linear_velocity");
	___mb.mb_get_friction = godot::api->godot_method_bind_get_method("StaticBody2D", "get_friction");
	___mb.mb_get_physics_material_override = godot::api->godot_method_bind_get_method("StaticBody2D", "get_physics_material_override");
	___mb.mb_set_bounce = godot::api->godot_method_bind_get_method("StaticBody2D", "set_bounce");
	___mb.mb_set_constant_angular_velocity = godot::api->godot_method_bind_get_method("StaticBody2D", "set_constant_angular_velocity");
	___mb.mb_set_constant_linear_velocity = godot::api->godot_method_bind_get_method("StaticBody2D", "set_constant_linear_velocity");
	___mb.mb_set_friction = godot::api->godot_method_bind_get_method("StaticBody2D", "set_friction");
	___mb.mb_set_physics_material_override = godot::api->godot_method_bind_get_method("StaticBody2D", "set_physics_material_override");
}

StaticBody2D *StaticBody2D::_new()
{
	return (StaticBody2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"StaticBody2D")());
}
void StaticBody2D::_reload_physics_characteristics() {
	___godot_icall_void(___mb.mb__reload_physics_characteristics, (const Object *) this);
}

real_t StaticBody2D::get_bounce() const {
	return ___godot_icall_float(___mb.mb_get_bounce, (const Object *) this);
}

real_t StaticBody2D::get_constant_angular_velocity() const {
	return ___godot_icall_float(___mb.mb_get_constant_angular_velocity, (const Object *) this);
}

Vector2 StaticBody2D::get_constant_linear_velocity() const {
	return ___godot_icall_Vector2(___mb.mb_get_constant_linear_velocity, (const Object *) this);
}

real_t StaticBody2D::get_friction() const {
	return ___godot_icall_float(___mb.mb_get_friction, (const Object *) this);
}

Ref<PhysicsMaterial> StaticBody2D::get_physics_material_override() const {
	return Ref<PhysicsMaterial>::__internal_constructor(___godot_icall_Object(___mb.mb_get_physics_material_override, (const Object *) this));
}

void StaticBody2D::set_bounce(const real_t bounce) {
	___godot_icall_void_float(___mb.mb_set_bounce, (const Object *) this, bounce);
}

void StaticBody2D::set_constant_angular_velocity(const real_t vel) {
	___godot_icall_void_float(___mb.mb_set_constant_angular_velocity, (const Object *) this, vel);
}

void StaticBody2D::set_constant_linear_velocity(const Vector2 vel) {
	___godot_icall_void_Vector2(___mb.mb_set_constant_linear_velocity, (const Object *) this, vel);
}

void StaticBody2D::set_friction(const real_t friction) {
	___godot_icall_void_float(___mb.mb_set_friction, (const Object *) this, friction);
}

void StaticBody2D::set_physics_material_override(const Ref<PhysicsMaterial> physics_material_override) {
	___godot_icall_void_Object(___mb.mb_set_physics_material_override, (const Object *) this, physics_material_override.ptr());
}

}