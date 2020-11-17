#ifndef GODOT_CPP_JSONRPC_HPP
#define GODOT_CPP_JSONRPC_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {

class Object;

class JSONRPC : public Object {
	struct ___method_bindings {
		godot_method_bind *mb_make_notification;
		godot_method_bind *mb_make_request;
		godot_method_bind *mb_make_response;
		godot_method_bind *mb_make_response_error;
		godot_method_bind *mb_process_action;
		godot_method_bind *mb_process_string;
		godot_method_bind *mb_set_scope;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "JSONRPC"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum ErrorCode {
		PARSE_ERROR = -32700,
		INTERNAL_ERROR = -32603,
		INVALID_PARAMS = -32602,
		METHOD_NOT_FOUND = -32601,
		INVALID_REQUEST = -32600,
	};

	// constants


	static JSONRPC *_new();

	// methods
	Dictionary make_notification(const String method, const Variant params);
	Dictionary make_request(const String method, const Variant params, const Variant id);
	Dictionary make_response(const Variant result, const Variant id);
	Dictionary make_response_error(const int64_t code, const String message, const Variant id = Variant()) const;
	Variant process_action(const Variant action, const bool recurse = false);
	String process_string(const String action);
	void set_scope(const String scope, const Object *target);

};

}

#endif