#ifndef GODOT_CPP_VISUALSCRIPTFUNCTIONCALL_HPP
#define GODOT_CPP_VISUALSCRIPTFUNCTIONCALL_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Variant.hpp"
#include "VisualScriptFunctionCall.hpp"

#include "VisualScriptNode.hpp"
namespace godot {


class VisualScriptFunctionCall : public VisualScriptNode {
	struct ___method_bindings {
		godot_method_bind *mb__get_argument_cache;
		godot_method_bind *mb__set_argument_cache;
		godot_method_bind *mb_get_base_path;
		godot_method_bind *mb_get_base_script;
		godot_method_bind *mb_get_base_type;
		godot_method_bind *mb_get_basic_type;
		godot_method_bind *mb_get_call_mode;
		godot_method_bind *mb_get_function;
		godot_method_bind *mb_get_rpc_call_mode;
		godot_method_bind *mb_get_singleton;
		godot_method_bind *mb_get_use_default_args;
		godot_method_bind *mb_get_validate;
		godot_method_bind *mb_set_base_path;
		godot_method_bind *mb_set_base_script;
		godot_method_bind *mb_set_base_type;
		godot_method_bind *mb_set_basic_type;
		godot_method_bind *mb_set_call_mode;
		godot_method_bind *mb_set_function;
		godot_method_bind *mb_set_rpc_call_mode;
		godot_method_bind *mb_set_singleton;
		godot_method_bind *mb_set_use_default_args;
		godot_method_bind *mb_set_validate;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualScriptFunctionCall"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum RPCCallMode {
		RPC_DISABLED = 0,
		RPC_RELIABLE = 1,
		RPC_UNRELIABLE = 2,
		RPC_RELIABLE_TO_ID = 3,
		RPC_UNRELIABLE_TO_ID = 4,
	};
	enum CallMode {
		CALL_MODE_SELF = 0,
		CALL_MODE_NODE_PATH = 1,
		CALL_MODE_INSTANCE = 2,
		CALL_MODE_BASIC_TYPE = 3,
		CALL_MODE_SINGLETON = 4,
	};

	// constants


	static VisualScriptFunctionCall *_new();

	// methods
	Dictionary _get_argument_cache() const;
	void _set_argument_cache(const Dictionary argument_cache);
	NodePath get_base_path() const;
	String get_base_script() const;
	String get_base_type() const;
	Variant::Type get_basic_type() const;
	VisualScriptFunctionCall::CallMode get_call_mode() const;
	String get_function() const;
	VisualScriptFunctionCall::RPCCallMode get_rpc_call_mode() const;
	String get_singleton() const;
	int64_t get_use_default_args() const;
	bool get_validate() const;
	void set_base_path(const NodePath base_path);
	void set_base_script(const String base_script);
	void set_base_type(const String base_type);
	void set_basic_type(const int64_t basic_type);
	void set_call_mode(const int64_t mode);
	void set_function(const String function);
	void set_rpc_call_mode(const int64_t mode);
	void set_singleton(const String singleton);
	void set_use_default_args(const int64_t amount);
	void set_validate(const bool enable);

};

}

#endif