#include "JSONParseResult.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


JSONParseResult::___method_bindings JSONParseResult::___mb = {};

void JSONParseResult::___init_method_bindings() {
	___mb.mb_get_error = godot::api->godot_method_bind_get_method("JSONParseResult", "get_error");
	___mb.mb_get_error_line = godot::api->godot_method_bind_get_method("JSONParseResult", "get_error_line");
	___mb.mb_get_error_string = godot::api->godot_method_bind_get_method("JSONParseResult", "get_error_string");
	___mb.mb_get_result = godot::api->godot_method_bind_get_method("JSONParseResult", "get_result");
	___mb.mb_set_error = godot::api->godot_method_bind_get_method("JSONParseResult", "set_error");
	___mb.mb_set_error_line = godot::api->godot_method_bind_get_method("JSONParseResult", "set_error_line");
	___mb.mb_set_error_string = godot::api->godot_method_bind_get_method("JSONParseResult", "set_error_string");
	___mb.mb_set_result = godot::api->godot_method_bind_get_method("JSONParseResult", "set_result");
}

JSONParseResult *JSONParseResult::_new()
{
	return (JSONParseResult *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"JSONParseResult")());
}
Error JSONParseResult::get_error() const {
	return (Error) ___godot_icall_int(___mb.mb_get_error, (const Object *) this);
}

int64_t JSONParseResult::get_error_line() const {
	return ___godot_icall_int(___mb.mb_get_error_line, (const Object *) this);
}

String JSONParseResult::get_error_string() const {
	return ___godot_icall_String(___mb.mb_get_error_string, (const Object *) this);
}

Variant JSONParseResult::get_result() const {
	return ___godot_icall_Variant(___mb.mb_get_result, (const Object *) this);
}

void JSONParseResult::set_error(const int64_t error) {
	___godot_icall_void_int(___mb.mb_set_error, (const Object *) this, error);
}

void JSONParseResult::set_error_line(const int64_t error_line) {
	___godot_icall_void_int(___mb.mb_set_error_line, (const Object *) this, error_line);
}

void JSONParseResult::set_error_string(const String error_string) {
	___godot_icall_void_String(___mb.mb_set_error_string, (const Object *) this, error_string);
}

void JSONParseResult::set_result(const Variant result) {
	___godot_icall_void_Variant(___mb.mb_set_result, (const Object *) this, result);
}

}