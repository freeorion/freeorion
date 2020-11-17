#ifndef GODOT_CPP_VISUALSCRIPTCLASSCONSTANT_HPP
#define GODOT_CPP_VISUALSCRIPTCLASSCONSTANT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "VisualScriptNode.hpp"
namespace godot {


class VisualScriptClassConstant : public VisualScriptNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_base_type;
		godot_method_bind *mb_get_class_constant;
		godot_method_bind *mb_set_base_type;
		godot_method_bind *mb_set_class_constant;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualScriptClassConstant"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static VisualScriptClassConstant *_new();

	// methods
	String get_base_type();
	String get_class_constant();
	void set_base_type(const String name);
	void set_class_constant(const String name);

};

}

#endif