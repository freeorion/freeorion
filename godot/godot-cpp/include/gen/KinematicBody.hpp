#ifndef GODOT_CPP_KINEMATICBODY_HPP
#define GODOT_CPP_KINEMATICBODY_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "PhysicsBody.hpp"
namespace godot {

class KinematicCollision;

class KinematicBody : public PhysicsBody {
	struct ___method_bindings {
		godot_method_bind *mb_get_axis_lock;
		godot_method_bind *mb_get_floor_normal;
		godot_method_bind *mb_get_floor_velocity;
		godot_method_bind *mb_get_safe_margin;
		godot_method_bind *mb_get_slide_collision;
		godot_method_bind *mb_get_slide_count;
		godot_method_bind *mb_is_on_ceiling;
		godot_method_bind *mb_is_on_floor;
		godot_method_bind *mb_is_on_wall;
		godot_method_bind *mb_move_and_collide;
		godot_method_bind *mb_move_and_slide;
		godot_method_bind *mb_move_and_slide_with_snap;
		godot_method_bind *mb_set_axis_lock;
		godot_method_bind *mb_set_safe_margin;
		godot_method_bind *mb_test_move;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "KinematicBody"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static KinematicBody *_new();

	// methods
	bool get_axis_lock(const int64_t axis) const;
	Vector3 get_floor_normal() const;
	Vector3 get_floor_velocity() const;
	real_t get_safe_margin() const;
	Ref<KinematicCollision> get_slide_collision(const int64_t slide_idx);
	int64_t get_slide_count() const;
	bool is_on_ceiling() const;
	bool is_on_floor() const;
	bool is_on_wall() const;
	Ref<KinematicCollision> move_and_collide(const Vector3 rel_vec, const bool infinite_inertia = true, const bool exclude_raycast_shapes = true, const bool test_only = false);
	Vector3 move_and_slide(const Vector3 linear_velocity, const Vector3 up_direction = Vector3(0, 0, 0), const bool stop_on_slope = false, const int64_t max_slides = 4, const real_t floor_max_angle = 0.785398, const bool infinite_inertia = true);
	Vector3 move_and_slide_with_snap(const Vector3 linear_velocity, const Vector3 snap, const Vector3 up_direction = Vector3(0, 0, 0), const bool stop_on_slope = false, const int64_t max_slides = 4, const real_t floor_max_angle = 0.785398, const bool infinite_inertia = true);
	void set_axis_lock(const int64_t axis, const bool lock);
	void set_safe_margin(const real_t pixels);
	bool test_move(const Transform from, const Vector3 rel_vec, const bool infinite_inertia = true);

};

}

#endif