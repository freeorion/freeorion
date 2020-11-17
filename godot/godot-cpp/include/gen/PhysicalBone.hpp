#ifndef GODOT_CPP_PHYSICALBONE_HPP
#define GODOT_CPP_PHYSICALBONE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "PhysicalBone.hpp"

#include "PhysicsBody.hpp"
namespace godot {

class Object;

class PhysicalBone : public PhysicsBody {
	struct ___method_bindings {
		godot_method_bind *mb__direct_state_changed;
		godot_method_bind *mb_apply_central_impulse;
		godot_method_bind *mb_apply_impulse;
		godot_method_bind *mb_get_body_offset;
		godot_method_bind *mb_get_bone_id;
		godot_method_bind *mb_get_bounce;
		godot_method_bind *mb_get_friction;
		godot_method_bind *mb_get_gravity_scale;
		godot_method_bind *mb_get_joint_offset;
		godot_method_bind *mb_get_joint_type;
		godot_method_bind *mb_get_mass;
		godot_method_bind *mb_get_simulate_physics;
		godot_method_bind *mb_get_weight;
		godot_method_bind *mb_is_simulating_physics;
		godot_method_bind *mb_is_static_body;
		godot_method_bind *mb_set_body_offset;
		godot_method_bind *mb_set_bounce;
		godot_method_bind *mb_set_friction;
		godot_method_bind *mb_set_gravity_scale;
		godot_method_bind *mb_set_joint_offset;
		godot_method_bind *mb_set_joint_type;
		godot_method_bind *mb_set_mass;
		godot_method_bind *mb_set_weight;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "PhysicalBone"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum JointType {
		JOINT_TYPE_NONE = 0,
		JOINT_TYPE_PIN = 1,
		JOINT_TYPE_CONE = 2,
		JOINT_TYPE_HINGE = 3,
		JOINT_TYPE_SLIDER = 4,
		JOINT_TYPE_6DOF = 5,
	};

	// constants


	static PhysicalBone *_new();

	// methods
	void _direct_state_changed(const Object *arg0);
	void apply_central_impulse(const Vector3 impulse);
	void apply_impulse(const Vector3 position, const Vector3 impulse);
	Transform get_body_offset() const;
	int64_t get_bone_id() const;
	real_t get_bounce() const;
	real_t get_friction() const;
	real_t get_gravity_scale() const;
	Transform get_joint_offset() const;
	PhysicalBone::JointType get_joint_type() const;
	real_t get_mass() const;
	bool get_simulate_physics();
	real_t get_weight() const;
	bool is_simulating_physics();
	bool is_static_body();
	void set_body_offset(const Transform offset);
	void set_bounce(const real_t bounce);
	void set_friction(const real_t friction);
	void set_gravity_scale(const real_t gravity_scale);
	void set_joint_offset(const Transform offset);
	void set_joint_type(const int64_t joint_type);
	void set_mass(const real_t mass);
	void set_weight(const real_t weight);

};

}

#endif