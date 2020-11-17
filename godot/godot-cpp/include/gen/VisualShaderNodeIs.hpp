#ifndef GODOT_CPP_VISUALSHADERNODEIS_HPP
#define GODOT_CPP_VISUALSHADERNODEIS_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "VisualShaderNodeIs.hpp"

#include "VisualShaderNode.hpp"
namespace godot {


class VisualShaderNodeIs : public VisualShaderNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_function;
		godot_method_bind *mb_set_function;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualShaderNodeIs"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Function {
		FUNC_IS_INF = 0,
		FUNC_IS_NAN = 1,
	};

	// constants


	static VisualShaderNodeIs *_new();

	// methods
	VisualShaderNodeIs::Function get_function() const;
	void set_function(const int64_t func);

};

}

#endif