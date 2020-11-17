#include "VisualScriptFunctionCall.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptFunctionCall::___method_bindings VisualScriptFunctionCall::___mb = {};

void VisualScriptFunctionCall::___init_method_bindings() {
	___mb.mb__get_argument_cache = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "_get_argument_cache");
	___mb.mb__set_argument_cache = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "_set_argument_cache");
	___mb.mb_get_base_path = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "get_base_path");
	___mb.mb_get_base_script = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "get_base_script");
	___mb.mb_get_base_type = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "get_base_type");
	___mb.mb_get_basic_type = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "get_basic_type");
	___mb.mb_get_call_mode = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "get_call_mode");
	___mb.mb_get_function = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "get_function");
	___mb.mb_get_rpc_call_mode = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "get_rpc_call_mode");
	___mb.mb_get_singleton = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "get_singleton");
	___mb.mb_get_use_default_args = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "get_use_default_args");
	___mb.mb_get_validate = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "get_validate");
	___mb.mb_set_base_path = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "set_base_path");
	___mb.mb_set_base_script = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "set_base_script");
	___mb.mb_set_base_type = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "set_base_type");
	___mb.mb_set_basic_type = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "set_basic_type");
	___mb.mb_set_call_mode = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "set_call_mode");
	___mb.mb_set_function = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "set_function");
	___mb.mb_set_rpc_call_mode = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "set_rpc_call_mode");
	___mb.mb_set_singleton = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "set_singleton");
	___mb.mb_set_use_default_args = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "set_use_default_args");
	___mb.mb_set_validate = godot::api->godot_method_bind_get_method("VisualScriptFunctionCall", "set_validate");
}

VisualScriptFunctionCall *VisualScriptFunctionCall::_new()
{
	return (VisualScriptFunctionCall *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptFunctionCall")());
}
Dictionary VisualScriptFunctionCall::_get_argument_cache() const {
	return ___godot_icall_Dictionary(___mb.mb__get_argument_cache, (const Object *) this);
}

void VisualScriptFunctionCall::_set_argument_cache(const Dictionary argument_cache) {
	___godot_icall_void_Dictionary(___mb.mb__set_argument_cache, (const Object *) this, argument_cache);
}

NodePath VisualScriptFunctionCall::get_base_path() const {
	return ___godot_icall_NodePath(___mb.mb_get_base_path, (const Object *) this);
}

String VisualScriptFunctionCall::get_base_script() const {
	return ___godot_icall_String(___mb.mb_get_base_script, (const Object *) this);
}

String VisualScriptFunctionCall::get_base_type() const {
	return ___godot_icall_String(___mb.mb_get_base_type, (const Object *) this);
}

Variant::Type VisualScriptFunctionCall::get_basic_type() const {
	return (Variant::Type) ___godot_icall_int(___mb.mb_get_basic_type, (const Object *) this);
}

VisualScriptFunctionCall::CallMode VisualScriptFunctionCall::get_call_mode() const {
	return (VisualScriptFunctionCall::CallMode) ___godot_icall_int(___mb.mb_get_call_mode, (const Object *) this);
}

String VisualScriptFunctionCall::get_function() const {
	return ___godot_icall_String(___mb.mb_get_function, (const Object *) this);
}

VisualScriptFunctionCall::RPCCallMode VisualScriptFunctionCall::get_rpc_call_mode() const {
	return (VisualScriptFunctionCall::RPCCallMode) ___godot_icall_int(___mb.mb_get_rpc_call_mode, (const Object *) this);
}

String VisualScriptFunctionCall::get_singleton() const {
	return ___godot_icall_String(___mb.mb_get_singleton, (const Object *) this);
}

int64_t VisualScriptFunctionCall::get_use_default_args() const {
	return ___godot_icall_int(___mb.mb_get_use_default_args, (const Object *) this);
}

bool VisualScriptFunctionCall::get_validate() const {
	return ___godot_icall_bool(___mb.mb_get_validate, (const Object *) this);
}

void VisualScriptFunctionCall::set_base_path(const NodePath base_path) {
	___godot_icall_void_NodePath(___mb.mb_set_base_path, (const Object *) this, base_path);
}

void VisualScriptFunctionCall::set_base_script(const String base_script) {
	___godot_icall_void_String(___mb.mb_set_base_script, (const Object *) this, base_script);
}

void VisualScriptFunctionCall::set_base_type(const String base_type) {
	___godot_icall_void_String(___mb.mb_set_base_type, (const Object *) this, base_type);
}

void VisualScriptFunctionCall::set_basic_type(const int64_t basic_type) {
	___godot_icall_void_int(___mb.mb_set_basic_type, (const Object *) this, basic_type);
}

void VisualScriptFunctionCall::set_call_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_call_mode, (const Object *) this, mode);
}

void VisualScriptFunctionCall::set_function(const String function) {
	___godot_icall_void_String(___mb.mb_set_function, (const Object *) this, function);
}

void VisualScriptFunctionCall::set_rpc_call_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_rpc_call_mode, (const Object *) this, mode);
}

void VisualScriptFunctionCall::set_singleton(const String singleton) {
	___godot_icall_void_String(___mb.mb_set_singleton, (const Object *) this, singleton);
}

void VisualScriptFunctionCall::set_use_default_args(const int64_t amount) {
	___godot_icall_void_int(___mb.mb_set_use_default_args, (const Object *) this, amount);
}

void VisualScriptFunctionCall::set_validate(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_validate, (const Object *) this, enable);
}

}