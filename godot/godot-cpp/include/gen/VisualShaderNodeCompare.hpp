#ifndef GODOT_CPP_VISUALSHADERNODECOMPARE_HPP
#define GODOT_CPP_VISUALSHADERNODECOMPARE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "VisualShaderNodeCompare.hpp"

#include "VisualShaderNode.hpp"
namespace godot {


class VisualShaderNodeCompare : public VisualShaderNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_comparison_type;
		godot_method_bind *mb_get_condition;
		godot_method_bind *mb_get_function;
		godot_method_bind *mb_set_comparison_type;
		godot_method_bind *mb_set_condition;
		godot_method_bind *mb_set_function;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualShaderNodeCompare"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum ComparisonType {
		CTYPE_SCALAR = 0,
		CTYPE_VECTOR = 1,
		CTYPE_BOOLEAN = 2,
		CTYPE_TRANSFORM = 3,
	};
	enum Function {
		FUNC_EQUAL = 0,
		FUNC_NOT_EQUAL = 1,
		FUNC_GREATER_THAN = 2,
		FUNC_GREATER_THAN_EQUAL = 3,
		FUNC_LESS_THAN = 4,
		FUNC_LESS_THAN_EQUAL = 5,
	};
	enum Condition {
		COND_ALL = 0,
		COND_ANY = 1,
	};

	// constants


	static VisualShaderNodeCompare *_new();

	// methods
	VisualShaderNodeCompare::ComparisonType get_comparison_type() const;
	VisualShaderNodeCompare::Condition get_condition() const;
	VisualShaderNodeCompare::Function get_function() const;
	void set_comparison_type(const int64_t type);
	void set_condition(const int64_t condition);
	void set_function(const int64_t func);

};

}

#endif