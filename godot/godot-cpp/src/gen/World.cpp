#include "World.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "PhysicsDirectSpaceState.hpp"
#include "Environment.hpp"


namespace godot {


World::___method_bindings World::___mb = {};

void World::___init_method_bindings() {
	___mb.mb_get_direct_space_state = godot::api->godot_method_bind_get_method("World", "get_direct_space_state");
	___mb.mb_get_environment = godot::api->godot_method_bind_get_method("World", "get_environment");
	___mb.mb_get_fallback_environment = godot::api->godot_method_bind_get_method("World", "get_fallback_environment");
	___mb.mb_get_scenario = godot::api->godot_method_bind_get_method("World", "get_scenario");
	___mb.mb_get_space = godot::api->godot_method_bind_get_method("World", "get_space");
	___mb.mb_set_environment = godot::api->godot_method_bind_get_method("World", "set_environment");
	___mb.mb_set_fallback_environment = godot::api->godot_method_bind_get_method("World", "set_fallback_environment");
}

World *World::_new()
{
	return (World *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"World")());
}
PhysicsDirectSpaceState *World::get_direct_space_state() {
	return (PhysicsDirectSpaceState *) ___godot_icall_Object(___mb.mb_get_direct_space_state, (const Object *) this);
}

Ref<Environment> World::get_environment() const {
	return Ref<Environment>::__internal_constructor(___godot_icall_Object(___mb.mb_get_environment, (const Object *) this));
}

Ref<Environment> World::get_fallback_environment() const {
	return Ref<Environment>::__internal_constructor(___godot_icall_Object(___mb.mb_get_fallback_environment, (const Object *) this));
}

RID World::get_scenario() const {
	return ___godot_icall_RID(___mb.mb_get_scenario, (const Object *) this);
}

RID World::get_space() const {
	return ___godot_icall_RID(___mb.mb_get_space, (const Object *) this);
}

void World::set_environment(const Ref<Environment> env) {
	___godot_icall_void_Object(___mb.mb_set_environment, (const Object *) this, env.ptr());
}

void World::set_fallback_environment(const Ref<Environment> env) {
	___godot_icall_void_Object(___mb.mb_set_fallback_environment, (const Object *) this, env.ptr());
}

}