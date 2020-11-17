#ifndef GODOT_CPP_VISUALSCRIPTVARIABLEGET_HPP
#define GODOT_CPP_VISUALSCRIPTVARIABLEGET_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "VisualScriptNode.hpp"
namespace godot {


class VisualScriptVariableGet : public VisualScriptNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_variable;
		godot_method_bind *mb_set_variable;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualScriptVariableGet"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static VisualScriptVariableGet *_new();

	// methods
	String get_variable() const;
	void set_variable(const String name);

};

}

#endif