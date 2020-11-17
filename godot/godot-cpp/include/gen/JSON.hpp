#ifndef GODOT_CPP_JSON_HPP
#define GODOT_CPP_JSON_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {

class JSONParseResult;

class JSON : public Object {
	static JSON *_singleton;

	JSON();

	struct ___method_bindings {
		godot_method_bind *mb_parse;
		godot_method_bind *mb_print;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline JSON *get_singleton()
	{
		if (!JSON::_singleton) {
			JSON::_singleton = new JSON;
		}
		return JSON::_singleton;
	}

	static inline const char *___get_class_name() { return (const char *) "JSON"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	Ref<JSONParseResult> parse(const String json);
	String print(const Variant value, const String indent = "", const bool sort_keys = false);

};

}

#endif