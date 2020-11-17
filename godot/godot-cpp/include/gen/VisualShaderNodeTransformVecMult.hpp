#ifndef GODOT_CPP_VISUALSHADERNODETRANSFORMVECMULT_HPP
#define GODOT_CPP_VISUALSHADERNODETRANSFORMVECMULT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "VisualShaderNodeTransformVecMult.hpp"

#include "VisualShaderNode.hpp"
namespace godot {


class VisualShaderNodeTransformVecMult : public VisualShaderNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_operator;
		godot_method_bind *mb_set_operator;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualShaderNodeTransformVecMult"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Operator {
		OP_AxB = 0,
		OP_BxA = 1,
		OP_3x3_AxB = 2,
		OP_3x3_BxA = 3,
	};

	// constants


	static VisualShaderNodeTransformVecMult *_new();

	// methods
	VisualShaderNodeTransformVecMult::Operator get_operator() const;
	void set_operator(const int64_t op);

};

}

#endif