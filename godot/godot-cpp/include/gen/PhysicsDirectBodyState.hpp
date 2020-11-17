#ifndef GODOT_CPP_PHYSICSDIRECTBODYSTATE_HPP
#define GODOT_CPP_PHYSICSDIRECTBODYSTATE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {

class Object;
class PhysicsDirectSpaceState;

class PhysicsDirectBodyState : public Object {
	struct ___method_bindings {
		godot_method_bind *mb_add_central_force;
		godot_method_bind *mb_add_force;
		godot_method_bind *mb_add_torque;
		godot_method_bind *mb_apply_central_impulse;
		godot_method_bind *mb_apply_impulse;
		godot_method_bind *mb_apply_torque_impulse;
		godot_method_bind *mb_get_angular_velocity;
		godot_method_bind *mb_get_center_of_mass;
		godot_method_bind *mb_get_contact_collider;
		godot_method_bind *mb_get_contact_collider_id;
		godot_method_bind *mb_get_contact_collider_object;
		godot_method_bind *mb_get_contact_collider_position;
		godot_method_bind *mb_get_contact_collider_shape;
		godot_method_bind *mb_get_contact_collider_velocity_at_position;
		godot_method_bind *mb_get_contact_count;
		godot_method_bind *mb_get_contact_impulse;
		godot_method_bind *mb_get_contact_local_normal;
		godot_method_bind *mb_get_contact_local_position;
		godot_method_bind *mb_get_contact_local_shape;
		godot_method_bind *mb_get_inverse_inertia;
		godot_method_bind *mb_get_inverse_mass;
		godot_method_bind *mb_get_linear_velocity;
		godot_method_bind *mb_get_principal_inertia_axes;
		godot_method_bind *mb_get_space_state;
		godot_method_bind *mb_get_step;
		godot_method_bind *mb_get_total_angular_damp;
		godot_method_bind *mb_get_total_gravity;
		godot_method_bind *mb_get_total_linear_damp;
		godot_method_bind *mb_get_transform;
		godot_method_bind *mb_integrate_forces;
		godot_method_bind *mb_is_sleeping;
		godot_method_bind *mb_set_angular_velocity;
		godot_method_bind *mb_set_linear_velocity;
		godot_method_bind *mb_set_sleep_state;
		godot_method_bind *mb_set_transform;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "PhysicsDirectBodyState"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void add_central_force(const Vector3 force);
	void add_force(const Vector3 force, const Vector3 position);
	void add_torque(const Vector3 torque);
	void apply_central_impulse(const Vector3 j);
	void apply_impulse(const Vector3 position, const Vector3 j);
	void apply_torque_impulse(const Vector3 j);
	Vector3 get_angular_velocity() const;
	Vector3 get_center_of_mass() const;
	RID get_contact_collider(const int64_t contact_idx) const;
	int64_t get_contact_collider_id(const int64_t contact_idx) const;
	Object *get_contact_collider_object(const int64_t contact_idx) const;
	Vector3 get_contact_collider_position(const int64_t contact_idx) const;
	int64_t get_contact_collider_shape(const int64_t contact_idx) const;
	Vector3 get_contact_collider_velocity_at_position(const int64_t contact_idx) const;
	int64_t get_contact_count() const;
	real_t get_contact_impulse(const int64_t contact_idx) const;
	Vector3 get_contact_local_normal(const int64_t contact_idx) const;
	Vector3 get_contact_local_position(const int64_t contact_idx) const;
	int64_t get_contact_local_shape(const int64_t contact_idx) const;
	Vector3 get_inverse_inertia() const;
	real_t get_inverse_mass() const;
	Vector3 get_linear_velocity() const;
	Basis get_principal_inertia_axes() const;
	PhysicsDirectSpaceState *get_space_state();
	real_t get_step() const;
	real_t get_total_angular_damp() const;
	Vector3 get_total_gravity() const;
	real_t get_total_linear_damp() const;
	Transform get_transform() const;
	void integrate_forces();
	bool is_sleeping() const;
	void set_angular_velocity(const Vector3 velocity);
	void set_linear_velocity(const Vector3 velocity);
	void set_sleep_state(const bool enabled);
	void set_transform(const Transform transform);

};

}

#endif