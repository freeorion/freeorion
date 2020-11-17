#include "Expression.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"


namespace godot {


Expression::___method_bindings Expression::___mb = {};

void Expression::___init_method_bindings() {
	___mb.mb_execute = godot::api->godot_method_bind_get_method("Expression", "execute");
	___mb.mb_get_error_text = godot::api->godot_method_bind_get_method("Expression", "get_error_text");
	___mb.mb_has_execute_failed = godot::api->godot_method_bind_get_method("Expression", "has_execute_failed");
	___mb.mb_parse = godot::api->godot_method_bind_get_method("Expression", "parse");
}

Expression *Expression::_new()
{
	return (Expression *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Expression")());
}
Variant Expression::execute(const Array inputs, const Object *base_instance, const bool show_error) {
	return ___godot_icall_Variant_Array_Object_bool(___mb.mb_execute, (const Object *) this, inputs, base_instance, show_error);
}

String Expression::get_error_text() const {
	return ___godot_icall_String(___mb.mb_get_error_text, (const Object *) this);
}

bool Expression::has_execute_failed() const {
	return ___godot_icall_bool(___mb.mb_has_execute_failed, (const Object *) this);
}

Error Expression::parse(const String expression, const PoolStringArray input_names) {
	return (Error) ___godot_icall_int_String_PoolStringArray(___mb.mb_parse, (const Object *) this, expression, input_names);
}

}