#ifndef GODOT_CPP_KINEMATICBODY2D_HPP
#define GODOT_CPP_KINEMATICBODY2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "PhysicsBody2D.hpp"
namespace godot {

class Object;
class KinematicCollision2D;

class KinematicBody2D : public PhysicsBody2D {
	struct ___method_bindings {
		godot_method_bind *mb__direct_state_changed;
		godot_method_bind *mb_get_floor_normal;
		godot_method_bind *mb_get_floor_velocity;
		godot_method_bind *mb_get_safe_margin;
		godot_method_bind *mb_get_slide_collision;
		godot_method_bind *mb_get_slide_count;
		godot_method_bind *mb_is_on_ceiling;
		godot_method_bind *mb_is_on_floor;
		godot_method_bind *mb_is_on_wall;
		godot_method_bind *mb_is_sync_to_physics_enabled;
		godot_method_bind *mb_move_and_collide;
		godot_method_bind *mb_move_and_slide;
		godot_method_bind *mb_move_and_slide_with_snap;
		godot_method_bind *mb_set_safe_margin;
		godot_method_bind *mb_set_sync_to_physics;
		godot_method_bind *mb_test_move;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "KinematicBody2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static KinematicBody2D *_new();

	// methods
	void _direct_state_changed(const Object *arg0);
	Vector2 get_floor_normal() const;
	Vector2 get_floor_velocity() const;
	real_t get_safe_margin() const;
	Ref<KinematicCollision2D> get_slide_collision(const int64_t slide_idx);
	int64_t get_slide_count() const;
	bool is_on_ceiling() const;
	bool is_on_floor() const;
	bool is_on_wall() const;
	bool is_sync_to_physics_enabled() const;
	Ref<KinematicCollision2D> move_and_collide(const Vector2 rel_vec, const bool infinite_inertia = true, const bool exclude_raycast_shapes = true, const bool test_only = false);
	Vector2 move_and_slide(const Vector2 linear_velocity, const Vector2 up_direction = Vector2(0, 0), const bool stop_on_slope = false, const int64_t max_slides = 4, const real_t floor_max_angle = 0.785398, const bool infinite_inertia = true);
	Vector2 move_and_slide_with_snap(const Vector2 linear_velocity, const Vector2 snap, const Vector2 up_direction = Vector2(0, 0), const bool stop_on_slope = false, const int64_t max_slides = 4, const real_t floor_max_angle = 0.785398, const bool infinite_inertia = true);
	void set_safe_margin(const real_t pixels);
	void set_sync_to_physics(const bool enable);
	bool test_move(const Transform2D from, const Vector2 rel_vec, const bool infinite_inertia = true);

};

}

#endif