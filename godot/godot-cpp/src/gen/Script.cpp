#include "Script.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Script.hpp"
#include "Object.hpp"


namespace godot {


Script::___method_bindings Script::___mb = {};

void Script::___init_method_bindings() {
	___mb.mb_can_instance = godot::api->godot_method_bind_get_method("Script", "can_instance");
	___mb.mb_get_base_script = godot::api->godot_method_bind_get_method("Script", "get_base_script");
	___mb.mb_get_instance_base_type = godot::api->godot_method_bind_get_method("Script", "get_instance_base_type");
	___mb.mb_get_property_default_value = godot::api->godot_method_bind_get_method("Script", "get_property_default_value");
	___mb.mb_get_script_constant_map = godot::api->godot_method_bind_get_method("Script", "get_script_constant_map");
	___mb.mb_get_script_method_list = godot::api->godot_method_bind_get_method("Script", "get_script_method_list");
	___mb.mb_get_script_property_list = godot::api->godot_method_bind_get_method("Script", "get_script_property_list");
	___mb.mb_get_script_signal_list = godot::api->godot_method_bind_get_method("Script", "get_script_signal_list");
	___mb.mb_get_source_code = godot::api->godot_method_bind_get_method("Script", "get_source_code");
	___mb.mb_has_script_signal = godot::api->godot_method_bind_get_method("Script", "has_script_signal");
	___mb.mb_has_source_code = godot::api->godot_method_bind_get_method("Script", "has_source_code");
	___mb.mb_instance_has = godot::api->godot_method_bind_get_method("Script", "instance_has");
	___mb.mb_is_tool = godot::api->godot_method_bind_get_method("Script", "is_tool");
	___mb.mb_reload = godot::api->godot_method_bind_get_method("Script", "reload");
	___mb.mb_set_source_code = godot::api->godot_method_bind_get_method("Script", "set_source_code");
}

bool Script::can_instance() const {
	return ___godot_icall_bool(___mb.mb_can_instance, (const Object *) this);
}

Ref<Script> Script::get_base_script() const {
	return Ref<Script>::__internal_constructor(___godot_icall_Object(___mb.mb_get_base_script, (const Object *) this));
}

String Script::get_instance_base_type() const {
	return ___godot_icall_String(___mb.mb_get_instance_base_type, (const Object *) this);
}

Variant Script::get_property_default_value(const String property) {
	return ___godot_icall_Variant_String(___mb.mb_get_property_default_value, (const Object *) this, property);
}

Dictionary Script::get_script_constant_map() {
	return ___godot_icall_Dictionary(___mb.mb_get_script_constant_map, (const Object *) this);
}

Array Script::get_script_method_list() {
	return ___godot_icall_Array(___mb.mb_get_script_method_list, (const Object *) this);
}

Array Script::get_script_property_list() {
	return ___godot_icall_Array(___mb.mb_get_script_property_list, (const Object *) this);
}

Array Script::get_script_signal_list() {
	return ___godot_icall_Array(___mb.mb_get_script_signal_list, (const Object *) this);
}

String Script::get_source_code() const {
	return ___godot_icall_String(___mb.mb_get_source_code, (const Object *) this);
}

bool Script::has_script_signal(const String signal_name) const {
	return ___godot_icall_bool_String(___mb.mb_has_script_signal, (const Object *) this, signal_name);
}

bool Script::has_source_code() const {
	return ___godot_icall_bool(___mb.mb_has_source_code, (const Object *) this);
}

bool Script::instance_has(const Object *base_object) const {
	return ___godot_icall_bool_Object(___mb.mb_instance_has, (const Object *) this, base_object);
}

bool Script::is_tool() const {
	return ___godot_icall_bool(___mb.mb_is_tool, (const Object *) this);
}

Error Script::reload(const bool keep_state) {
	return (Error) ___godot_icall_int_bool(___mb.mb_reload, (const Object *) this, keep_state);
}

void Script::set_source_code(const String source) {
	___godot_icall_void_String(___mb.mb_set_source_code, (const Object *) this, source);
}

}