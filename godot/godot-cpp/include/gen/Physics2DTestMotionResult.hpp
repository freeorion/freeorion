#ifndef GODOT_CPP_PHYSICS2DTESTMOTIONRESULT_HPP
#define GODOT_CPP_PHYSICS2DTESTMOTIONRESULT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class Object;

class Physics2DTestMotionResult : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_get_collider;
		godot_method_bind *mb_get_collider_id;
		godot_method_bind *mb_get_collider_rid;
		godot_method_bind *mb_get_collider_shape;
		godot_method_bind *mb_get_collider_velocity;
		godot_method_bind *mb_get_collision_normal;
		godot_method_bind *mb_get_collision_point;
		godot_method_bind *mb_get_motion;
		godot_method_bind *mb_get_motion_remainder;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Physics2DTestMotionResult"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Physics2DTestMotionResult *_new();

	// methods
	Object *get_collider() const;
	int64_t get_collider_id() const;
	RID get_collider_rid() const;
	int64_t get_collider_shape() const;
	Vector2 get_collider_velocity() const;
	Vector2 get_collision_normal() const;
	Vector2 get_collision_point() const;
	Vector2 get_motion() const;
	Vector2 get_motion_remainder() const;

};

}

#endif