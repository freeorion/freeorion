#ifndef GODOT_CPP_ANIMATIONNODESTATEMACHINE_HPP
#define GODOT_CPP_ANIMATIONNODESTATEMACHINE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "AnimationRootNode.hpp"
namespace godot {

class AnimationNode;
class AnimationNodeStateMachineTransition;

class AnimationNodeStateMachine : public AnimationRootNode {
	struct ___method_bindings {
		godot_method_bind *mb__tree_changed;
		godot_method_bind *mb_add_node;
		godot_method_bind *mb_add_transition;
		godot_method_bind *mb_get_end_node;
		godot_method_bind *mb_get_graph_offset;
		godot_method_bind *mb_get_node;
		godot_method_bind *mb_get_node_name;
		godot_method_bind *mb_get_node_position;
		godot_method_bind *mb_get_start_node;
		godot_method_bind *mb_get_transition;
		godot_method_bind *mb_get_transition_count;
		godot_method_bind *mb_get_transition_from;
		godot_method_bind *mb_get_transition_to;
		godot_method_bind *mb_has_node;
		godot_method_bind *mb_has_transition;
		godot_method_bind *mb_remove_node;
		godot_method_bind *mb_remove_transition;
		godot_method_bind *mb_remove_transition_by_index;
		godot_method_bind *mb_rename_node;
		godot_method_bind *mb_set_end_node;
		godot_method_bind *mb_set_graph_offset;
		godot_method_bind *mb_set_node_position;
		godot_method_bind *mb_set_start_node;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AnimationNodeStateMachine"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static AnimationNodeStateMachine *_new();

	// methods
	void _tree_changed();
	void add_node(const String name, const Ref<AnimationNode> node, const Vector2 position = Vector2(0, 0));
	void add_transition(const String from, const String to, const Ref<AnimationNodeStateMachineTransition> transition);
	String get_end_node() const;
	Vector2 get_graph_offset() const;
	Ref<AnimationNode> get_node(const String name) const;
	String get_node_name(const Ref<AnimationNode> node) const;
	Vector2 get_node_position(const String name) const;
	String get_start_node() const;
	Ref<AnimationNodeStateMachineTransition> get_transition(const int64_t idx) const;
	int64_t get_transition_count() const;
	String get_transition_from(const int64_t idx) const;
	String get_transition_to(const int64_t idx) const;
	bool has_node(const String name) const;
	bool has_transition(const String from, const String to) const;
	void remove_node(const String name);
	void remove_transition(const String from, const String to);
	void remove_transition_by_index(const int64_t idx);
	void rename_node(const String name, const String new_name);
	void set_end_node(const String name);
	void set_graph_offset(const Vector2 offset);
	void set_node_position(const String name, const Vector2 position);
	void set_start_node(const String name);

};

}

#endif