#ifndef GODOT_CPP_EXPRESSION_HPP
#define GODOT_CPP_EXPRESSION_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class Object;

class Expression : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_execute;
		godot_method_bind *mb_get_error_text;
		godot_method_bind *mb_has_execute_failed;
		godot_method_bind *mb_parse;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Expression"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Expression *_new();

	// methods
	Variant execute(const Array inputs = Array(), const Object *base_instance = nullptr, const bool show_error = true);
	String get_error_text() const;
	bool has_execute_failed() const;
	Error parse(const String expression, const PoolStringArray input_names = PoolStringArray());

};

}

#endif