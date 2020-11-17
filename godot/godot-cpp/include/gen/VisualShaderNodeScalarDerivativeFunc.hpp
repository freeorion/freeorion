#ifndef GODOT_CPP_VISUALSHADERNODESCALARDERIVATIVEFUNC_HPP
#define GODOT_CPP_VISUALSHADERNODESCALARDERIVATIVEFUNC_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "VisualShaderNodeScalarDerivativeFunc.hpp"

#include "VisualShaderNode.hpp"
namespace godot {


class VisualShaderNodeScalarDerivativeFunc : public VisualShaderNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_function;
		godot_method_bind *mb_set_function;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualShaderNodeScalarDerivativeFunc"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Function {
		FUNC_SUM = 0,
		FUNC_X = 1,
		FUNC_Y = 2,
	};

	// constants


	static VisualShaderNodeScalarDerivativeFunc *_new();

	// methods
	VisualShaderNodeScalarDerivativeFunc::Function get_function() const;
	void set_function(const int64_t func);

};

}

#endif