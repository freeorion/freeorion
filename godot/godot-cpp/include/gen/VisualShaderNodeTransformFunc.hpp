#ifndef GODOT_CPP_VISUALSHADERNODETRANSFORMFUNC_HPP
#define GODOT_CPP_VISUALSHADERNODETRANSFORMFUNC_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "VisualShaderNodeTransformFunc.hpp"

#include "VisualShaderNode.hpp"
namespace godot {


class VisualShaderNodeTransformFunc : public VisualShaderNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_function;
		godot_method_bind *mb_set_function;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualShaderNodeTransformFunc"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Function {
		FUNC_INVERSE = 0,
		FUNC_TRANSPOSE = 1,
	};

	// constants


	static VisualShaderNodeTransformFunc *_new();

	// methods
	VisualShaderNodeTransformFunc::Function get_function() const;
	void set_function(const int64_t func);

};

}

#endif