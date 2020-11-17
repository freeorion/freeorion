#ifndef GODOT_CPP_CANVASLAYER_HPP
#define GODOT_CPP_CANVASLAYER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node.hpp"
namespace godot {

class Node;

class CanvasLayer : public Node {
	struct ___method_bindings {
		godot_method_bind *mb_get_canvas;
		godot_method_bind *mb_get_custom_viewport;
		godot_method_bind *mb_get_follow_viewport_scale;
		godot_method_bind *mb_get_layer;
		godot_method_bind *mb_get_offset;
		godot_method_bind *mb_get_rotation;
		godot_method_bind *mb_get_rotation_degrees;
		godot_method_bind *mb_get_scale;
		godot_method_bind *mb_get_transform;
		godot_method_bind *mb_is_following_viewport;
		godot_method_bind *mb_set_custom_viewport;
		godot_method_bind *mb_set_follow_viewport;
		godot_method_bind *mb_set_follow_viewport_scale;
		godot_method_bind *mb_set_layer;
		godot_method_bind *mb_set_offset;
		godot_method_bind *mb_set_rotation;
		godot_method_bind *mb_set_rotation_degrees;
		godot_method_bind *mb_set_scale;
		godot_method_bind *mb_set_transform;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CanvasLayer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static CanvasLayer *_new();

	// methods
	RID get_canvas() const;
	Node *get_custom_viewport() const;
	real_t get_follow_viewport_scale() const;
	int64_t get_layer() const;
	Vector2 get_offset() const;
	real_t get_rotation() const;
	real_t get_rotation_degrees() const;
	Vector2 get_scale() const;
	Transform2D get_transform() const;
	bool is_following_viewport() const;
	void set_custom_viewport(const Node *viewport);
	void set_follow_viewport(const bool enable);
	void set_follow_viewport_scale(const real_t scale);
	void set_layer(const int64_t layer);
	void set_offset(const Vector2 offset);
	void set_rotation(const real_t radians);
	void set_rotation_degrees(const real_t degrees);
	void set_scale(const Vector2 scale);
	void set_transform(const Transform2D transform);

};

}

#endif