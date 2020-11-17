#ifndef GODOT_CPP_NODE2D_HPP
#define GODOT_CPP_NODE2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "CanvasItem.hpp"
namespace godot {

class Node;

class Node2D : public CanvasItem {
	struct ___method_bindings {
		godot_method_bind *mb_apply_scale;
		godot_method_bind *mb_get_angle_to;
		godot_method_bind *mb_get_global_position;
		godot_method_bind *mb_get_global_rotation;
		godot_method_bind *mb_get_global_rotation_degrees;
		godot_method_bind *mb_get_global_scale;
		godot_method_bind *mb_get_position;
		godot_method_bind *mb_get_relative_transform_to_parent;
		godot_method_bind *mb_get_rotation;
		godot_method_bind *mb_get_rotation_degrees;
		godot_method_bind *mb_get_scale;
		godot_method_bind *mb_get_z_index;
		godot_method_bind *mb_global_translate;
		godot_method_bind *mb_is_z_relative;
		godot_method_bind *mb_look_at;
		godot_method_bind *mb_move_local_x;
		godot_method_bind *mb_move_local_y;
		godot_method_bind *mb_rotate;
		godot_method_bind *mb_set_global_position;
		godot_method_bind *mb_set_global_rotation;
		godot_method_bind *mb_set_global_rotation_degrees;
		godot_method_bind *mb_set_global_scale;
		godot_method_bind *mb_set_global_transform;
		godot_method_bind *mb_set_position;
		godot_method_bind *mb_set_rotation;
		godot_method_bind *mb_set_rotation_degrees;
		godot_method_bind *mb_set_scale;
		godot_method_bind *mb_set_transform;
		godot_method_bind *mb_set_z_as_relative;
		godot_method_bind *mb_set_z_index;
		godot_method_bind *mb_to_global;
		godot_method_bind *mb_to_local;
		godot_method_bind *mb_translate;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Node2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Node2D *_new();

	// methods
	void apply_scale(const Vector2 ratio);
	real_t get_angle_to(const Vector2 point) const;
	Vector2 get_global_position() const;
	real_t get_global_rotation() const;
	real_t get_global_rotation_degrees() const;
	Vector2 get_global_scale() const;
	Vector2 get_position() const;
	Transform2D get_relative_transform_to_parent(const Node *parent) const;
	real_t get_rotation() const;
	real_t get_rotation_degrees() const;
	Vector2 get_scale() const;
	int64_t get_z_index() const;
	void global_translate(const Vector2 offset);
	bool is_z_relative() const;
	void look_at(const Vector2 point);
	void move_local_x(const real_t delta, const bool scaled = false);
	void move_local_y(const real_t delta, const bool scaled = false);
	void rotate(const real_t radians);
	void set_global_position(const Vector2 position);
	void set_global_rotation(const real_t radians);
	void set_global_rotation_degrees(const real_t degrees);
	void set_global_scale(const Vector2 scale);
	void set_global_transform(const Transform2D xform);
	void set_position(const Vector2 position);
	void set_rotation(const real_t radians);
	void set_rotation_degrees(const real_t degrees);
	void set_scale(const Vector2 scale);
	void set_transform(const Transform2D xform);
	void set_z_as_relative(const bool enable);
	void set_z_index(const int64_t z_index);
	Vector2 to_global(const Vector2 local_point) const;
	Vector2 to_local(const Vector2 global_point) const;
	void translate(const Vector2 offset);

};

}

#endif