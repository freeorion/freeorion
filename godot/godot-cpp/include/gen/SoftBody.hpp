#ifndef GODOT_CPP_SOFTBODY_HPP
#define GODOT_CPP_SOFTBODY_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "MeshInstance.hpp"
namespace godot {

class Node;

class SoftBody : public MeshInstance {
	struct ___method_bindings {
		godot_method_bind *mb__draw_soft_mesh;
		godot_method_bind *mb_add_collision_exception_with;
		godot_method_bind *mb_get_areaAngular_stiffness;
		godot_method_bind *mb_get_collision_exceptions;
		godot_method_bind *mb_get_collision_layer;
		godot_method_bind *mb_get_collision_layer_bit;
		godot_method_bind *mb_get_collision_mask;
		godot_method_bind *mb_get_collision_mask_bit;
		godot_method_bind *mb_get_damping_coefficient;
		godot_method_bind *mb_get_drag_coefficient;
		godot_method_bind *mb_get_linear_stiffness;
		godot_method_bind *mb_get_parent_collision_ignore;
		godot_method_bind *mb_get_pose_matching_coefficient;
		godot_method_bind *mb_get_pressure_coefficient;
		godot_method_bind *mb_get_simulation_precision;
		godot_method_bind *mb_get_total_mass;
		godot_method_bind *mb_get_volume_stiffness;
		godot_method_bind *mb_is_ray_pickable;
		godot_method_bind *mb_remove_collision_exception_with;
		godot_method_bind *mb_set_areaAngular_stiffness;
		godot_method_bind *mb_set_collision_layer;
		godot_method_bind *mb_set_collision_layer_bit;
		godot_method_bind *mb_set_collision_mask;
		godot_method_bind *mb_set_collision_mask_bit;
		godot_method_bind *mb_set_damping_coefficient;
		godot_method_bind *mb_set_drag_coefficient;
		godot_method_bind *mb_set_linear_stiffness;
		godot_method_bind *mb_set_parent_collision_ignore;
		godot_method_bind *mb_set_pose_matching_coefficient;
		godot_method_bind *mb_set_pressure_coefficient;
		godot_method_bind *mb_set_ray_pickable;
		godot_method_bind *mb_set_simulation_precision;
		godot_method_bind *mb_set_total_mass;
		godot_method_bind *mb_set_volume_stiffness;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "SoftBody"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static SoftBody *_new();

	// methods
	void _draw_soft_mesh();
	void add_collision_exception_with(const Node *body);
	real_t get_areaAngular_stiffness();
	Array get_collision_exceptions();
	int64_t get_collision_layer() const;
	bool get_collision_layer_bit(const int64_t bit) const;
	int64_t get_collision_mask() const;
	bool get_collision_mask_bit(const int64_t bit) const;
	real_t get_damping_coefficient();
	real_t get_drag_coefficient();
	real_t get_linear_stiffness();
	NodePath get_parent_collision_ignore() const;
	real_t get_pose_matching_coefficient();
	real_t get_pressure_coefficient();
	int64_t get_simulation_precision();
	real_t get_total_mass();
	real_t get_volume_stiffness();
	bool is_ray_pickable() const;
	void remove_collision_exception_with(const Node *body);
	void set_areaAngular_stiffness(const real_t areaAngular_stiffness);
	void set_collision_layer(const int64_t collision_layer);
	void set_collision_layer_bit(const int64_t bit, const bool value);
	void set_collision_mask(const int64_t collision_mask);
	void set_collision_mask_bit(const int64_t bit, const bool value);
	void set_damping_coefficient(const real_t damping_coefficient);
	void set_drag_coefficient(const real_t drag_coefficient);
	void set_linear_stiffness(const real_t linear_stiffness);
	void set_parent_collision_ignore(const NodePath parent_collision_ignore);
	void set_pose_matching_coefficient(const real_t pose_matching_coefficient);
	void set_pressure_coefficient(const real_t pressure_coefficient);
	void set_ray_pickable(const bool ray_pickable);
	void set_simulation_precision(const int64_t simulation_precision);
	void set_total_mass(const real_t mass);
	void set_volume_stiffness(const real_t volume_stiffness);

};

}

#endif