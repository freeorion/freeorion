#ifndef GODOT_CPP_PHYSICSBODY_HPP
#define GODOT_CPP_PHYSICSBODY_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "CollisionObject.hpp"
namespace godot {

class Node;

class PhysicsBody : public CollisionObject {
	struct ___method_bindings {
		godot_method_bind *mb__get_layers;
		godot_method_bind *mb__set_layers;
		godot_method_bind *mb_add_collision_exception_with;
		godot_method_bind *mb_get_collision_exceptions;
		godot_method_bind *mb_get_collision_layer;
		godot_method_bind *mb_get_collision_layer_bit;
		godot_method_bind *mb_get_collision_mask;
		godot_method_bind *mb_get_collision_mask_bit;
		godot_method_bind *mb_remove_collision_exception_with;
		godot_method_bind *mb_set_collision_layer;
		godot_method_bind *mb_set_collision_layer_bit;
		godot_method_bind *mb_set_collision_mask;
		godot_method_bind *mb_set_collision_mask_bit;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "PhysicsBody"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	int64_t _get_layers() const;
	void _set_layers(const int64_t mask);
	void add_collision_exception_with(const Node *body);
	Array get_collision_exceptions();
	int64_t get_collision_layer() const;
	bool get_collision_layer_bit(const int64_t bit) const;
	int64_t get_collision_mask() const;
	bool get_collision_mask_bit(const int64_t bit) const;
	void remove_collision_exception_with(const Node *body);
	void set_collision_layer(const int64_t layer);
	void set_collision_layer_bit(const int64_t bit, const bool value);
	void set_collision_mask(const int64_t mask);
	void set_collision_mask_bit(const int64_t bit, const bool value);

};

}

#endif