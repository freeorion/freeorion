#ifndef GODOT_CPP_VEHICLEBODY_HPP
#define GODOT_CPP_VEHICLEBODY_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "RigidBody.hpp"
namespace godot {


class VehicleBody : public RigidBody {
	struct ___method_bindings {
		godot_method_bind *mb_get_brake;
		godot_method_bind *mb_get_engine_force;
		godot_method_bind *mb_get_steering;
		godot_method_bind *mb_set_brake;
		godot_method_bind *mb_set_engine_force;
		godot_method_bind *mb_set_steering;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VehicleBody"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static VehicleBody *_new();

	// methods
	real_t get_brake() const;
	real_t get_engine_force() const;
	real_t get_steering() const;
	void set_brake(const real_t brake);
	void set_engine_force(const real_t engine_force);
	void set_steering(const real_t steering);

};

}

#endif