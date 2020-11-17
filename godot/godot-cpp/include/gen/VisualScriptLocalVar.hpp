#ifndef GODOT_CPP_VISUALSCRIPTLOCALVAR_HPP
#define GODOT_CPP_VISUALSCRIPTLOCALVAR_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Variant.hpp"

#include "VisualScriptNode.hpp"
namespace godot {


class VisualScriptLocalVar : public VisualScriptNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_var_name;
		godot_method_bind *mb_get_var_type;
		godot_method_bind *mb_set_var_name;
		godot_method_bind *mb_set_var_type;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualScriptLocalVar"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static VisualScriptLocalVar *_new();

	// methods
	String get_var_name() const;
	Variant::Type get_var_type() const;
	void set_var_name(const String name);
	void set_var_type(const int64_t type);

};

}

#endif