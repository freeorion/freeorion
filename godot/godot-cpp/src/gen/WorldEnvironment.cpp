#include "WorldEnvironment.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Environment.hpp"


namespace godot {


WorldEnvironment::___method_bindings WorldEnvironment::___mb = {};

void WorldEnvironment::___init_method_bindings() {
	___mb.mb_get_environment = godot::api->godot_method_bind_get_method("WorldEnvironment", "get_environment");
	___mb.mb_set_environment = godot::api->godot_method_bind_get_method("WorldEnvironment", "set_environment");
}

WorldEnvironment *WorldEnvironment::_new()
{
	return (WorldEnvironment *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"WorldEnvironment")());
}
Ref<Environment> WorldEnvironment::get_environment() const {
	return Ref<Environment>::__internal_constructor(___godot_icall_Object(___mb.mb_get_environment, (const Object *) this));
}

void WorldEnvironment::set_environment(const Ref<Environment> env) {
	___godot_icall_void_Object(___mb.mb_set_environment, (const Object *) this, env.ptr());
}

}