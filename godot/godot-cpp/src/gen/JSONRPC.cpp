#include "JSONRPC.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"


namespace godot {


JSONRPC::___method_bindings JSONRPC::___mb = {};

void JSONRPC::___init_method_bindings() {
	___mb.mb_make_notification = godot::api->godot_method_bind_get_method("JSONRPC", "make_notification");
	___mb.mb_make_request = godot::api->godot_method_bind_get_method("JSONRPC", "make_request");
	___mb.mb_make_response = godot::api->godot_method_bind_get_method("JSONRPC", "make_response");
	___mb.mb_make_response_error = godot::api->godot_method_bind_get_method("JSONRPC", "make_response_error");
	___mb.mb_process_action = godot::api->godot_method_bind_get_method("JSONRPC", "process_action");
	___mb.mb_process_string = godot::api->godot_method_bind_get_method("JSONRPC", "process_string");
	___mb.mb_set_scope = godot::api->godot_method_bind_get_method("JSONRPC", "set_scope");
}

JSONRPC *JSONRPC::_new()
{
	return (JSONRPC *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"JSONRPC")());
}
Dictionary JSONRPC::make_notification(const String method, const Variant params) {
	return ___godot_icall_Dictionary_String_Variant(___mb.mb_make_notification, (const Object *) this, method, params);
}

Dictionary JSONRPC::make_request(const String method, const Variant params, const Variant id) {
	return ___godot_icall_Dictionary_String_Variant_Variant(___mb.mb_make_request, (const Object *) this, method, params, id);
}

Dictionary JSONRPC::make_response(const Variant result, const Variant id) {
	return ___godot_icall_Dictionary_Variant_Variant(___mb.mb_make_response, (const Object *) this, result, id);
}

Dictionary JSONRPC::make_response_error(const int64_t code, const String message, const Variant id) const {
	return ___godot_icall_Dictionary_int_String_Variant(___mb.mb_make_response_error, (const Object *) this, code, message, id);
}

Variant JSONRPC::process_action(const Variant action, const bool recurse) {
	return ___godot_icall_Variant_Variant_bool(___mb.mb_process_action, (const Object *) this, action, recurse);
}

String JSONRPC::process_string(const String action) {
	return ___godot_icall_String_String(___mb.mb_process_string, (const Object *) this, action);
}

void JSONRPC::set_scope(const String scope, const Object *target) {
	___godot_icall_void_String_Object(___mb.mb_set_scope, (const Object *) this, scope, target);
}

}