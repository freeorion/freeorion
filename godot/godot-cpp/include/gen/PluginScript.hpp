#ifndef GODOT_CPP_PLUGINSCRIPT_HPP
#define GODOT_CPP_PLUGINSCRIPT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Script.hpp"
namespace godot {


class PluginScript : public Script {
	struct ___method_bindings {
		godot_method_bind *mb_new;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "PluginScript"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static PluginScript *_new();

	// methods
	Variant new_(const Array& __var_args = Array());
	template <class... Args> Variant new_(Args... args){
		return new_(Array::make(args...));
	}

};

}

#endif