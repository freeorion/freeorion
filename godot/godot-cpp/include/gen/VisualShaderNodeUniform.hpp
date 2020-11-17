#ifndef GODOT_CPP_VISUALSHADERNODEUNIFORM_HPP
#define GODOT_CPP_VISUALSHADERNODEUNIFORM_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "VisualShaderNode.hpp"
namespace godot {


class VisualShaderNodeUniform : public VisualShaderNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_uniform_name;
		godot_method_bind *mb_set_uniform_name;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualShaderNodeUniform"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	String get_uniform_name() const;
	void set_uniform_name(const String name);

};

}

#endif