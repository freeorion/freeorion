#ifndef GODOT_CPP_JOINT2D_HPP
#define GODOT_CPP_JOINT2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node2D.hpp"
namespace godot {


class Joint2D : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb_get_bias;
		godot_method_bind *mb_get_exclude_nodes_from_collision;
		godot_method_bind *mb_get_node_a;
		godot_method_bind *mb_get_node_b;
		godot_method_bind *mb_set_bias;
		godot_method_bind *mb_set_exclude_nodes_from_collision;
		godot_method_bind *mb_set_node_a;
		godot_method_bind *mb_set_node_b;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Joint2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	real_t get_bias() const;
	bool get_exclude_nodes_from_collision() const;
	NodePath get_node_a() const;
	NodePath get_node_b() const;
	void set_bias(const real_t bias);
	void set_exclude_nodes_from_collision(const bool enable);
	void set_node_a(const NodePath node);
	void set_node_b(const NodePath node);

};

}

#endif