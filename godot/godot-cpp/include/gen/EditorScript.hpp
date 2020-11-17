#ifndef GODOT_CPP_EDITORSCRIPT_HPP
#define GODOT_CPP_EDITORSCRIPT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class Node;
class EditorInterface;

class EditorScript : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb__run;
		godot_method_bind *mb_add_root_node;
		godot_method_bind *mb_get_editor_interface;
		godot_method_bind *mb_get_scene;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorScript"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void _run();
	void add_root_node(const Node *node);
	EditorInterface *get_editor_interface();
	Node *get_scene();

};

}

#endif