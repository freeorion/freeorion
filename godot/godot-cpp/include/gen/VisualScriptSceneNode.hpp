#ifndef GODOT_CPP_VISUALSCRIPTSCENENODE_HPP
#define GODOT_CPP_VISUALSCRIPTSCENENODE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "VisualScriptNode.hpp"
namespace godot {


class VisualScriptSceneNode : public VisualScriptNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_node_path;
		godot_method_bind *mb_set_node_path;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualScriptSceneNode"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static VisualScriptSceneNode *_new();

	// methods
	NodePath get_node_path();
	void set_node_path(const NodePath path);

};

}

#endif