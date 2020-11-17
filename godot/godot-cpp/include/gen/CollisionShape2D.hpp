#ifndef GODOT_CPP_COLLISIONSHAPE2D_HPP
#define GODOT_CPP_COLLISIONSHAPE2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node2D.hpp"
namespace godot {

class Shape2D;

class CollisionShape2D : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb__shape_changed;
		godot_method_bind *mb_get_one_way_collision_margin;
		godot_method_bind *mb_get_shape;
		godot_method_bind *mb_is_disabled;
		godot_method_bind *mb_is_one_way_collision_enabled;
		godot_method_bind *mb_set_disabled;
		godot_method_bind *mb_set_one_way_collision;
		godot_method_bind *mb_set_one_way_collision_margin;
		godot_method_bind *mb_set_shape;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CollisionShape2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static CollisionShape2D *_new();

	// methods
	void _shape_changed();
	real_t get_one_way_collision_margin() const;
	Ref<Shape2D> get_shape() const;
	bool is_disabled() const;
	bool is_one_way_collision_enabled() const;
	void set_disabled(const bool disabled);
	void set_one_way_collision(const bool enabled);
	void set_one_way_collision_margin(const real_t margin);
	void set_shape(const Ref<Shape2D> shape);

};

}

#endif