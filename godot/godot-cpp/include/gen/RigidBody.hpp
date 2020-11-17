#ifndef GODOT_CPP_RIGIDBODY_HPP
#define GODOT_CPP_RIGIDBODY_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "RigidBody.hpp"

#include "PhysicsBody.hpp"
namespace godot {

class Object;
class PhysicsDirectBodyState;
class PhysicsMaterial;

class RigidBody : public PhysicsBody {
	struct ___method_bindings {
		godot_method_bind *mb__body_enter_tree;
		godot_method_bind *mb__body_exit_tree;
		godot_method_bind *mb__direct_state_changed;
		godot_method_bind *mb__integrate_forces;
		godot_method_bind *mb__reload_physics_characteristics;
		godot_method_bind *mb_add_central_force;
		godot_method_bind *mb_add_force;
		godot_method_bind *mb_add_torque;
		godot_method_bind *mb_apply_central_impulse;
		godot_method_bind *mb_apply_impulse;
		godot_method_bind *mb_apply_torque_impulse;
		godot_method_bind *mb_get_angular_damp;
		godot_method_bind *mb_get_angular_velocity;
		godot_method_bind *mb_get_axis_lock;
		godot_method_bind *mb_get_bounce;
		godot_method_bind *mb_get_colliding_bodies;
		godot_method_bind *mb_get_friction;
		godot_method_bind *mb_get_gravity_scale;
		godot_method_bind *mb_get_linear_damp;
		godot_method_bind *mb_get_linear_velocity;
		godot_method_bind *mb_get_mass;
		godot_method_bind *mb_get_max_contacts_reported;
		godot_method_bind *mb_get_mode;
		godot_method_bind *mb_get_physics_material_override;
		godot_method_bind *mb_get_weight;
		godot_method_bind *mb_is_able_to_sleep;
		godot_method_bind *mb_is_contact_monitor_enabled;
		godot_method_bind *mb_is_sleeping;
		godot_method_bind *mb_is_using_continuous_collision_detection;
		godot_method_bind *mb_is_using_custom_integrator;
		godot_method_bind *mb_set_angular_damp;
		godot_method_bind *mb_set_angular_velocity;
		godot_method_bind *mb_set_axis_lock;
		godot_method_bind *mb_set_axis_velocity;
		godot_method_bind *mb_set_bounce;
		godot_method_bind *mb_set_can_sleep;
		godot_method_bind *mb_set_contact_monitor;
		godot_method_bind *mb_set_friction;
		godot_method_bind *mb_set_gravity_scale;
		godot_method_bind *mb_set_linear_damp;
		godot_method_bind *mb_set_linear_velocity;
		godot_method_bind *mb_set_mass;
		godot_method_bind *mb_set_max_contacts_reported;
		godot_method_bind *mb_set_mode;
		godot_method_bind *mb_set_physics_material_override;
		godot_method_bind *mb_set_sleeping;
		godot_method_bind *mb_set_use_continuous_collision_detection;
		godot_method_bind *mb_set_use_custom_integrator;
		godot_method_bind *mb_set_weight;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "RigidBody"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Mode {
		MODE_RIGID = 0,
		MODE_STATIC = 1,
		MODE_CHARACTER = 2,
		MODE_KINEMATIC = 3,
	};

	// constants


	static RigidBody *_new();

	// methods
	void _body_enter_tree(const int64_t arg0);
	void _body_exit_tree(const int64_t arg0);
	void _direct_state_changed(const Object *arg0);
	void _integrate_forces(const PhysicsDirectBodyState *state);
	void _reload_physics_characteristics();
	void add_central_force(const Vector3 force);
	void add_force(const Vector3 force, const Vector3 position);
	void add_torque(const Vector3 torque);
	void apply_central_impulse(const Vector3 impulse);
	void apply_impulse(const Vector3 position, const Vector3 impulse);
	void apply_torque_impulse(const Vector3 impulse);
	real_t get_angular_damp() const;
	Vector3 get_angular_velocity() const;
	bool get_axis_lock(const int64_t axis) const;
	real_t get_bounce() const;
	Array get_colliding_bodies() const;
	real_t get_friction() const;
	real_t get_gravity_scale() const;
	real_t get_linear_damp() const;
	Vector3 get_linear_velocity() const;
	real_t get_mass() const;
	int64_t get_max_contacts_reported() const;
	RigidBody::Mode get_mode() const;
	Ref<PhysicsMaterial> get_physics_material_override() const;
	real_t get_weight() const;
	bool is_able_to_sleep() const;
	bool is_contact_monitor_enabled() const;
	bool is_sleeping() const;
	bool is_using_continuous_collision_detection() const;
	bool is_using_custom_integrator();
	void set_angular_damp(const real_t angular_damp);
	void set_angular_velocity(const Vector3 angular_velocity);
	void set_axis_lock(const int64_t axis, const bool lock);
	void set_axis_velocity(const Vector3 axis_velocity);
	void set_bounce(const real_t bounce);
	void set_can_sleep(const bool able_to_sleep);
	void set_contact_monitor(const bool enabled);
	void set_friction(const real_t friction);
	void set_gravity_scale(const real_t gravity_scale);
	void set_linear_damp(const real_t linear_damp);
	void set_linear_velocity(const Vector3 linear_velocity);
	void set_mass(const real_t mass);
	void set_max_contacts_reported(const int64_t amount);
	void set_mode(const int64_t mode);
	void set_physics_material_override(const Ref<PhysicsMaterial> physics_material_override);
	void set_sleeping(const bool sleeping);
	void set_use_continuous_collision_detection(const bool enable);
	void set_use_custom_integrator(const bool enable);
	void set_weight(const real_t weight);

};

}

#endif