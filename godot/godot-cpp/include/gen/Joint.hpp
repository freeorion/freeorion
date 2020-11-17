#ifndef GODOT_CPP_JOINT_HPP
#define GODOT_CPP_JOINT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Spatial.hpp"
namespace godot {


class Joint : public Spatial {
	struct ___method_bindings {
		godot_method_bind *mb_get_exclude_nodes_from_collision;
		godot_method_bind *mb_get_node_a;
		godot_method_bind *mb_get_node_b;
		godot_method_bind *mb_get_solver_priority;
		godot_method_bind *mb_set_exclude_nodes_from_collision;
		godot_method_bind *mb_set_node_a;
		godot_method_bind *mb_set_node_b;
		godot_method_bind *mb_set_solver_priority;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Joint"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	bool get_exclude_nodes_from_collision() const;
	NodePath get_node_a() const;
	NodePath get_node_b() const;
	int64_t get_solver_priority() const;
	void set_exclude_nodes_from_collision(const bool enable);
	void set_node_a(const NodePath node);
	void set_node_b(const NodePath node);
	void set_solver_priority(const int64_t priority);

};

}

#endif