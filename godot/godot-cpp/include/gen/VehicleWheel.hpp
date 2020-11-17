#ifndef GODOT_CPP_VEHICLEWHEEL_HPP
#define GODOT_CPP_VEHICLEWHEEL_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Spatial.hpp"
namespace godot {


class VehicleWheel : public Spatial {
	struct ___method_bindings {
		godot_method_bind *mb_get_brake;
		godot_method_bind *mb_get_damping_compression;
		godot_method_bind *mb_get_damping_relaxation;
		godot_method_bind *mb_get_engine_force;
		godot_method_bind *mb_get_friction_slip;
		godot_method_bind *mb_get_radius;
		godot_method_bind *mb_get_roll_influence;
		godot_method_bind *mb_get_rpm;
		godot_method_bind *mb_get_skidinfo;
		godot_method_bind *mb_get_steering;
		godot_method_bind *mb_get_suspension_max_force;
		godot_method_bind *mb_get_suspension_rest_length;
		godot_method_bind *mb_get_suspension_stiffness;
		godot_method_bind *mb_get_suspension_travel;
		godot_method_bind *mb_is_in_contact;
		godot_method_bind *mb_is_used_as_steering;
		godot_method_bind *mb_is_used_as_traction;
		godot_method_bind *mb_set_brake;
		godot_method_bind *mb_set_damping_compression;
		godot_method_bind *mb_set_damping_relaxation;
		godot_method_bind *mb_set_engine_force;
		godot_method_bind *mb_set_friction_slip;
		godot_method_bind *mb_set_radius;
		godot_method_bind *mb_set_roll_influence;
		godot_method_bind *mb_set_steering;
		godot_method_bind *mb_set_suspension_max_force;
		godot_method_bind *mb_set_suspension_rest_length;
		godot_method_bind *mb_set_suspension_stiffness;
		godot_method_bind *mb_set_suspension_travel;
		godot_method_bind *mb_set_use_as_steering;
		godot_method_bind *mb_set_use_as_traction;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VehicleWheel"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static VehicleWheel *_new();

	// methods
	real_t get_brake() const;
	real_t get_damping_compression() const;
	real_t get_damping_relaxation() const;
	real_t get_engine_force() const;
	real_t get_friction_slip() const;
	real_t get_radius() const;
	real_t get_roll_influence() const;
	real_t get_rpm() const;
	real_t get_skidinfo() const;
	real_t get_steering() const;
	real_t get_suspension_max_force() const;
	real_t get_suspension_rest_length() const;
	real_t get_suspension_stiffness() const;
	real_t get_suspension_travel() const;
	bool is_in_contact() const;
	bool is_used_as_steering() const;
	bool is_used_as_traction() const;
	void set_brake(const real_t brake);
	void set_damping_compression(const real_t length);
	void set_damping_relaxation(const real_t length);
	void set_engine_force(const real_t engine_force);
	void set_friction_slip(const real_t length);
	void set_radius(const real_t length);
	void set_roll_influence(const real_t roll_influence);
	void set_steering(const real_t steering);
	void set_suspension_max_force(const real_t length);
	void set_suspension_rest_length(const real_t length);
	void set_suspension_stiffness(const real_t length);
	void set_suspension_travel(const real_t length);
	void set_use_as_steering(const bool enable);
	void set_use_as_traction(const bool enable);

};

}

#endif