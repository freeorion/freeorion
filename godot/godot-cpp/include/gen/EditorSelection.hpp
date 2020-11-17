#ifndef GODOT_CPP_EDITORSELECTION_HPP
#define GODOT_CPP_EDITORSELECTION_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {

class Node;

class EditorSelection : public Object {
	struct ___method_bindings {
		godot_method_bind *mb__emit_change;
		godot_method_bind *mb__node_removed;
		godot_method_bind *mb_add_node;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_get_selected_nodes;
		godot_method_bind *mb_get_transformable_selected_nodes;
		godot_method_bind *mb_remove_node;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorSelection"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void _emit_change();
	void _node_removed(const Node *arg0);
	void add_node(const Node *node);
	void clear();
	Array get_selected_nodes();
	Array get_transformable_selected_nodes();
	void remove_node(const Node *node);

};

}

#endif