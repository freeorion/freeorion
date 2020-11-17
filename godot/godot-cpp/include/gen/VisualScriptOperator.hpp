#ifndef GODOT_CPP_VISUALSCRIPTOPERATOR_HPP
#define GODOT_CPP_VISUALSCRIPTOPERATOR_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Variant.hpp"

#include "VisualScriptNode.hpp"
namespace godot {


class VisualScriptOperator : public VisualScriptNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_operator;
		godot_method_bind *mb_get_typed;
		godot_method_bind *mb_set_operator;
		godot_method_bind *mb_set_typed;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualScriptOperator"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static VisualScriptOperator *_new();

	// methods
	Variant::Operator get_operator() const;
	Variant::Type get_typed() const;
	void set_operator(const int64_t op);
	void set_typed(const int64_t type);

};

}

#endif