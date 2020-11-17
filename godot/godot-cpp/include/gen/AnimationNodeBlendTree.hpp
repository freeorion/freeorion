#ifndef GODOT_CPP_ANIMATIONNODEBLENDTREE_HPP
#define GODOT_CPP_ANIMATIONNODEBLENDTREE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "AnimationRootNode.hpp"
namespace godot {

class AnimationNode;

class AnimationNodeBlendTree : public AnimationRootNode {
	struct ___method_bindings {
		godot_method_bind *mb__node_changed;
		godot_method_bind *mb__tree_changed;
		godot_method_bind *mb_add_node;
		godot_method_bind *mb_connect_node;
		godot_method_bind *mb_disconnect_node;
		godot_method_bind *mb_get_graph_offset;
		godot_method_bind *mb_get_node;
		godot_method_bind *mb_get_node_position;
		godot_method_bind *mb_has_node;
		godot_method_bind *mb_remove_node;
		godot_method_bind *mb_rename_node;
		godot_method_bind *mb_set_graph_offset;
		godot_method_bind *mb_set_node_position;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AnimationNodeBlendTree"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants
	const static int CONNECTION_ERROR_CONNECTION_EXISTS = 5;
	const static int CONNECTION_ERROR_NO_INPUT = 1;
	const static int CONNECTION_ERROR_NO_INPUT_INDEX = 2;
	const static int CONNECTION_ERROR_NO_OUTPUT = 3;
	const static int CONNECTION_ERROR_SAME_NODE = 4;
	const static int CONNECTION_OK = 0;


	static AnimationNodeBlendTree *_new();

	// methods
	void _node_changed(const String node);
	void _tree_changed();
	void add_node(const String name, const Ref<AnimationNode> node, const Vector2 position = Vector2(0, 0));
	void connect_node(const String input_node, const int64_t input_index, const String output_node);
	void disconnect_node(const String input_node, const int64_t input_index);
	Vector2 get_graph_offset() const;
	Ref<AnimationNode> get_node(const String name) const;
	Vector2 get_node_position(const String name) const;
	bool has_node(const String name) const;
	void remove_node(const String name);
	void rename_node(const String name, const String new_name);
	void set_graph_offset(const Vector2 offset);
	void set_node_position(const String name, const Vector2 position);

};

}

#endif