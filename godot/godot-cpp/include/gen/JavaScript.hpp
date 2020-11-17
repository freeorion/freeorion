#ifndef GODOT_CPP_JAVASCRIPT_HPP
#define GODOT_CPP_JAVASCRIPT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {


class JavaScript : public Object {
	static JavaScript *_singleton;

	JavaScript();

	struct ___method_bindings {
		godot_method_bind *mb_eval;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline JavaScript *get_singleton()
	{
		if (!JavaScript::_singleton) {
			JavaScript::_singleton = new JavaScript;
		}
		return JavaScript::_singleton;
	}

	static inline const char *___get_class_name() { return (const char *) "JavaScript"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	Variant eval(const String code, const bool use_global_execution_context = false);

};

}

#endif