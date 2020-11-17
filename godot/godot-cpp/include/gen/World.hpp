#ifndef GODOT_CPP_WORLD_HPP
#define GODOT_CPP_WORLD_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class PhysicsDirectSpaceState;
class Environment;

class World : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_get_direct_space_state;
		godot_method_bind *mb_get_environment;
		godot_method_bind *mb_get_fallback_environment;
		godot_method_bind *mb_get_scenario;
		godot_method_bind *mb_get_space;
		godot_method_bind *mb_set_environment;
		godot_method_bind *mb_set_fallback_environment;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "World"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static World *_new();

	// methods
	PhysicsDirectSpaceState *get_direct_space_state();
	Ref<Environment> get_environment() const;
	Ref<Environment> get_fallback_environment() const;
	RID get_scenario() const;
	RID get_space() const;
	void set_environment(const Ref<Environment> env);
	void set_fallback_environment(const Ref<Environment> env);

};

}

#endif