#include "PhysicsMaterial.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PhysicsMaterial::___method_bindings PhysicsMaterial::___mb = {};

void PhysicsMaterial::___init_method_bindings() {
	___mb.mb_get_bounce = godot::api->godot_method_bind_get_method("PhysicsMaterial", "get_bounce");
	___mb.mb_get_friction = godot::api->godot_method_bind_get_method("PhysicsMaterial", "get_friction");
	___mb.mb_is_absorbent = godot::api->godot_method_bind_get_method("PhysicsMaterial", "is_absorbent");
	___mb.mb_is_rough = godot::api->godot_method_bind_get_method("PhysicsMaterial", "is_rough");
	___mb.mb_set_absorbent = godot::api->godot_method_bind_get_method("PhysicsMaterial", "set_absorbent");
	___mb.mb_set_bounce = godot::api->godot_method_bind_get_method("PhysicsMaterial", "set_bounce");
	___mb.mb_set_friction = godot::api->godot_method_bind_get_method("PhysicsMaterial", "set_friction");
	___mb.mb_set_rough = godot::api->godot_method_bind_get_method("PhysicsMaterial", "set_rough");
}

PhysicsMaterial *PhysicsMaterial::_new()
{
	return (PhysicsMaterial *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PhysicsMaterial")());
}
real_t PhysicsMaterial::get_bounce() const {
	return ___godot_icall_float(___mb.mb_get_bounce, (const Object *) this);
}

real_t PhysicsMaterial::get_friction() const {
	return ___godot_icall_float(___mb.mb_get_friction, (const Object *) this);
}

bool PhysicsMaterial::is_absorbent() const {
	return ___godot_icall_bool(___mb.mb_is_absorbent, (const Object *) this);
}

bool PhysicsMaterial::is_rough() const {
	return ___godot_icall_bool(___mb.mb_is_rough, (const Object *) this);
}

void PhysicsMaterial::set_absorbent(const bool absorbent) {
	___godot_icall_void_bool(___mb.mb_set_absorbent, (const Object *) this, absorbent);
}

void PhysicsMaterial::set_bounce(const real_t bounce) {
	___godot_icall_void_float(___mb.mb_set_bounce, (const Object *) this, bounce);
}

void PhysicsMaterial::set_friction(const real_t friction) {
	___godot_icall_void_float(___mb.mb_set_friction, (const Object *) this, friction);
}

void PhysicsMaterial::set_rough(const bool rough) {
	___godot_icall_void_bool(___mb.mb_set_rough, (const Object *) this, rough);
}

}