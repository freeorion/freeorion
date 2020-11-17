#ifndef GODOT_CPP_VISUALSHADERNODECOLOROP_HPP
#define GODOT_CPP_VISUALSHADERNODECOLOROP_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "VisualShaderNodeColorOp.hpp"

#include "VisualShaderNode.hpp"
namespace godot {


class VisualShaderNodeColorOp : public VisualShaderNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_operator;
		godot_method_bind *mb_set_operator;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualShaderNodeColorOp"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Operator {
		OP_SCREEN = 0,
		OP_DIFFERENCE = 1,
		OP_DARKEN = 2,
		OP_LIGHTEN = 3,
		OP_OVERLAY = 4,
		OP_DODGE = 5,
		OP_BURN = 6,
		OP_SOFT_LIGHT = 7,
		OP_HARD_LIGHT = 8,
	};

	// constants


	static VisualShaderNodeColorOp *_new();

	// methods
	VisualShaderNodeColorOp::Operator get_operator() const;
	void set_operator(const int64_t op);

};

}

#endif