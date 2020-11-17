#ifndef GODOT_CPP_JSONPARSERESULT_HPP
#define GODOT_CPP_JSONPARSERESULT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {


class JSONParseResult : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_get_error;
		godot_method_bind *mb_get_error_line;
		godot_method_bind *mb_get_error_string;
		godot_method_bind *mb_get_result;
		godot_method_bind *mb_set_error;
		godot_method_bind *mb_set_error_line;
		godot_method_bind *mb_set_error_string;
		godot_method_bind *mb_set_result;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "JSONParseResult"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static JSONParseResult *_new();

	// methods
	Error get_error() const;
	int64_t get_error_line() const;
	String get_error_string() const;
	Variant get_result() const;
	void set_error(const int64_t error);
	void set_error_line(const int64_t error_line);
	void set_error_string(const String error_string);
	void set_result(const Variant result);

};

}

#endif