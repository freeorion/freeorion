#include "VisualScript.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "VisualScriptNode.hpp"


namespace godot {


VisualScript::___method_bindings VisualScript::___mb = {};

void VisualScript::___init_method_bindings() {
	___mb.mb__get_data = godot::api->godot_method_bind_get_method("VisualScript", "_get_data");
	___mb.mb__node_ports_changed = godot::api->godot_method_bind_get_method("VisualScript", "_node_ports_changed");
	___mb.mb__set_data = godot::api->godot_method_bind_get_method("VisualScript", "_set_data");
	___mb.mb_add_custom_signal = godot::api->godot_method_bind_get_method("VisualScript", "add_custom_signal");
	___mb.mb_add_function = godot::api->godot_method_bind_get_method("VisualScript", "add_function");
	___mb.mb_add_node = godot::api->godot_method_bind_get_method("VisualScript", "add_node");
	___mb.mb_add_variable = godot::api->godot_method_bind_get_method("VisualScript", "add_variable");
	___mb.mb_custom_signal_add_argument = godot::api->godot_method_bind_get_method("VisualScript", "custom_signal_add_argument");
	___mb.mb_custom_signal_get_argument_count = godot::api->godot_method_bind_get_method("VisualScript", "custom_signal_get_argument_count");
	___mb.mb_custom_signal_get_argument_name = godot::api->godot_method_bind_get_method("VisualScript", "custom_signal_get_argument_name");
	___mb.mb_custom_signal_get_argument_type = godot::api->godot_method_bind_get_method("VisualScript", "custom_signal_get_argument_type");
	___mb.mb_custom_signal_remove_argument = godot::api->godot_method_bind_get_method("VisualScript", "custom_signal_remove_argument");
	___mb.mb_custom_signal_set_argument_name = godot::api->godot_method_bind_get_method("VisualScript", "custom_signal_set_argument_name");
	___mb.mb_custom_signal_set_argument_type = godot::api->godot_method_bind_get_method("VisualScript", "custom_signal_set_argument_type");
	___mb.mb_custom_signal_swap_argument = godot::api->godot_method_bind_get_method("VisualScript", "custom_signal_swap_argument");
	___mb.mb_data_connect = godot::api->godot_method_bind_get_method("VisualScript", "data_connect");
	___mb.mb_data_disconnect = godot::api->godot_method_bind_get_method("VisualScript", "data_disconnect");
	___mb.mb_get_function_node_id = godot::api->godot_method_bind_get_method("VisualScript", "get_function_node_id");
	___mb.mb_get_function_scroll = godot::api->godot_method_bind_get_method("VisualScript", "get_function_scroll");
	___mb.mb_get_node = godot::api->godot_method_bind_get_method("VisualScript", "get_node");
	___mb.mb_get_node_position = godot::api->godot_method_bind_get_method("VisualScript", "get_node_position");
	___mb.mb_get_variable_default_value = godot::api->godot_method_bind_get_method("VisualScript", "get_variable_default_value");
	___mb.mb_get_variable_export = godot::api->godot_method_bind_get_method("VisualScript", "get_variable_export");
	___mb.mb_get_variable_info = godot::api->godot_method_bind_get_method("VisualScript", "get_variable_info");
	___mb.mb_has_custom_signal = godot::api->godot_method_bind_get_method("VisualScript", "has_custom_signal");
	___mb.mb_has_data_connection = godot::api->godot_method_bind_get_method("VisualScript", "has_data_connection");
	___mb.mb_has_function = godot::api->godot_method_bind_get_method("VisualScript", "has_function");
	___mb.mb_has_node = godot::api->godot_method_bind_get_method("VisualScript", "has_node");
	___mb.mb_has_sequence_connection = godot::api->godot_method_bind_get_method("VisualScript", "has_sequence_connection");
	___mb.mb_has_variable = godot::api->godot_method_bind_get_method("VisualScript", "has_variable");
	___mb.mb_remove_custom_signal = godot::api->godot_method_bind_get_method("VisualScript", "remove_custom_signal");
	___mb.mb_remove_function = godot::api->godot_method_bind_get_method("VisualScript", "remove_function");
	___mb.mb_remove_node = godot::api->godot_method_bind_get_method("VisualScript", "remove_node");
	___mb.mb_remove_variable = godot::api->godot_method_bind_get_method("VisualScript", "remove_variable");
	___mb.mb_rename_custom_signal = godot::api->godot_method_bind_get_method("VisualScript", "rename_custom_signal");
	___mb.mb_rename_function = godot::api->godot_method_bind_get_method("VisualScript", "rename_function");
	___mb.mb_rename_variable = godot::api->godot_method_bind_get_method("VisualScript", "rename_variable");
	___mb.mb_sequence_connect = godot::api->godot_method_bind_get_method("VisualScript", "sequence_connect");
	___mb.mb_sequence_disconnect = godot::api->godot_method_bind_get_method("VisualScript", "sequence_disconnect");
	___mb.mb_set_function_scroll = godot::api->godot_method_bind_get_method("VisualScript", "set_function_scroll");
	___mb.mb_set_instance_base_type = godot::api->godot_method_bind_get_method("VisualScript", "set_instance_base_type");
	___mb.mb_set_node_position = godot::api->godot_method_bind_get_method("VisualScript", "set_node_position");
	___mb.mb_set_variable_default_value = godot::api->godot_method_bind_get_method("VisualScript", "set_variable_default_value");
	___mb.mb_set_variable_export = godot::api->godot_method_bind_get_method("VisualScript", "set_variable_export");
	___mb.mb_set_variable_info = godot::api->godot_method_bind_get_method("VisualScript", "set_variable_info");
}

VisualScript *VisualScript::_new()
{
	return (VisualScript *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScript")());
}
Dictionary VisualScript::_get_data() const {
	return ___godot_icall_Dictionary(___mb.mb__get_data, (const Object *) this);
}

void VisualScript::_node_ports_changed(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__node_ports_changed, (const Object *) this, arg0);
}

void VisualScript::_set_data(const Dictionary data) {
	___godot_icall_void_Dictionary(___mb.mb__set_data, (const Object *) this, data);
}

void VisualScript::add_custom_signal(const String name) {
	___godot_icall_void_String(___mb.mb_add_custom_signal, (const Object *) this, name);
}

void VisualScript::add_function(const String name) {
	___godot_icall_void_String(___mb.mb_add_function, (const Object *) this, name);
}

void VisualScript::add_node(const String func, const int64_t id, const Ref<VisualScriptNode> node, const Vector2 position) {
	___godot_icall_void_String_int_Object_Vector2(___mb.mb_add_node, (const Object *) this, func, id, node.ptr(), position);
}

void VisualScript::add_variable(const String name, const Variant default_value, const bool _export) {
	___godot_icall_void_String_Variant_bool(___mb.mb_add_variable, (const Object *) this, name, default_value, _export);
}

void VisualScript::custom_signal_add_argument(const String name, const int64_t type, const String argname, const int64_t index) {
	___godot_icall_void_String_int_String_int(___mb.mb_custom_signal_add_argument, (const Object *) this, name, type, argname, index);
}

int64_t VisualScript::custom_signal_get_argument_count(const String name) const {
	return ___godot_icall_int_String(___mb.mb_custom_signal_get_argument_count, (const Object *) this, name);
}

String VisualScript::custom_signal_get_argument_name(const String name, const int64_t argidx) const {
	return ___godot_icall_String_String_int(___mb.mb_custom_signal_get_argument_name, (const Object *) this, name, argidx);
}

Variant::Type VisualScript::custom_signal_get_argument_type(const String name, const int64_t argidx) const {
	return (Variant::Type) ___godot_icall_int_String_int(___mb.mb_custom_signal_get_argument_type, (const Object *) this, name, argidx);
}

void VisualScript::custom_signal_remove_argument(const String name, const int64_t argidx) {
	___godot_icall_void_String_int(___mb.mb_custom_signal_remove_argument, (const Object *) this, name, argidx);
}

void VisualScript::custom_signal_set_argument_name(const String name, const int64_t argidx, const String argname) {
	___godot_icall_void_String_int_String(___mb.mb_custom_signal_set_argument_name, (const Object *) this, name, argidx, argname);
}

void VisualScript::custom_signal_set_argument_type(const String name, const int64_t argidx, const int64_t type) {
	___godot_icall_void_String_int_int(___mb.mb_custom_signal_set_argument_type, (const Object *) this, name, argidx, type);
}

void VisualScript::custom_signal_swap_argument(const String name, const int64_t argidx, const int64_t withidx) {
	___godot_icall_void_String_int_int(___mb.mb_custom_signal_swap_argument, (const Object *) this, name, argidx, withidx);
}

void VisualScript::data_connect(const String func, const int64_t from_node, const int64_t from_port, const int64_t to_node, const int64_t to_port) {
	___godot_icall_void_String_int_int_int_int(___mb.mb_data_connect, (const Object *) this, func, from_node, from_port, to_node, to_port);
}

void VisualScript::data_disconnect(const String func, const int64_t from_node, const int64_t from_port, const int64_t to_node, const int64_t to_port) {
	___godot_icall_void_String_int_int_int_int(___mb.mb_data_disconnect, (const Object *) this, func, from_node, from_port, to_node, to_port);
}

int64_t VisualScript::get_function_node_id(const String name) const {
	return ___godot_icall_int_String(___mb.mb_get_function_node_id, (const Object *) this, name);
}

Vector2 VisualScript::get_function_scroll(const String name) const {
	return ___godot_icall_Vector2_String(___mb.mb_get_function_scroll, (const Object *) this, name);
}

Ref<VisualScriptNode> VisualScript::get_node(const String func, const int64_t id) const {
	return Ref<VisualScriptNode>::__internal_constructor(___godot_icall_Object_String_int(___mb.mb_get_node, (const Object *) this, func, id));
}

Vector2 VisualScript::get_node_position(const String func, const int64_t id) const {
	return ___godot_icall_Vector2_String_int(___mb.mb_get_node_position, (const Object *) this, func, id);
}

Variant VisualScript::get_variable_default_value(const String name) const {
	return ___godot_icall_Variant_String(___mb.mb_get_variable_default_value, (const Object *) this, name);
}

bool VisualScript::get_variable_export(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_get_variable_export, (const Object *) this, name);
}

Dictionary VisualScript::get_variable_info(const String name) const {
	return ___godot_icall_Dictionary_String(___mb.mb_get_variable_info, (const Object *) this, name);
}

bool VisualScript::has_custom_signal(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_custom_signal, (const Object *) this, name);
}

bool VisualScript::has_data_connection(const String func, const int64_t from_node, const int64_t from_port, const int64_t to_node, const int64_t to_port) const {
	return ___godot_icall_bool_String_int_int_int_int(___mb.mb_has_data_connection, (const Object *) this, func, from_node, from_port, to_node, to_port);
}

bool VisualScript::has_function(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_function, (const Object *) this, name);
}

bool VisualScript::has_node(const String func, const int64_t id) const {
	return ___godot_icall_bool_String_int(___mb.mb_has_node, (const Object *) this, func, id);
}

bool VisualScript::has_sequence_connection(const String func, const int64_t from_node, const int64_t from_output, const int64_t to_node) const {
	return ___godot_icall_bool_String_int_int_int(___mb.mb_has_sequence_connection, (const Object *) this, func, from_node, from_output, to_node);
}

bool VisualScript::has_variable(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_variable, (const Object *) this, name);
}

void VisualScript::remove_custom_signal(const String name) {
	___godot_icall_void_String(___mb.mb_remove_custom_signal, (const Object *) this, name);
}

void VisualScript::remove_function(const String name) {
	___godot_icall_void_String(___mb.mb_remove_function, (const Object *) this, name);
}

void VisualScript::remove_node(const String func, const int64_t id) {
	___godot_icall_void_String_int(___mb.mb_remove_node, (const Object *) this, func, id);
}

void VisualScript::remove_variable(const String name) {
	___godot_icall_void_String(___mb.mb_remove_variable, (const Object *) this, name);
}

void VisualScript::rename_custom_signal(const String name, const String new_name) {
	___godot_icall_void_String_String(___mb.mb_rename_custom_signal, (const Object *) this, name, new_name);
}

void VisualScript::rename_function(const String name, const String new_name) {
	___godot_icall_void_String_String(___mb.mb_rename_function, (const Object *) this, name, new_name);
}

void VisualScript::rename_variable(const String name, const String new_name) {
	___godot_icall_void_String_String(___mb.mb_rename_variable, (const Object *) this, name, new_name);
}

void VisualScript::sequence_connect(const String func, const int64_t from_node, const int64_t from_output, const int64_t to_node) {
	___godot_icall_void_String_int_int_int(___mb.mb_sequence_connect, (const Object *) this, func, from_node, from_output, to_node);
}

void VisualScript::sequence_disconnect(const String func, const int64_t from_node, const int64_t from_output, const int64_t to_node) {
	___godot_icall_void_String_int_int_int(___mb.mb_sequence_disconnect, (const Object *) this, func, from_node, from_output, to_node);
}

void VisualScript::set_function_scroll(const String name, const Vector2 ofs) {
	___godot_icall_void_String_Vector2(___mb.mb_set_function_scroll, (const Object *) this, name, ofs);
}

void VisualScript::set_instance_base_type(const String type) {
	___godot_icall_void_String(___mb.mb_set_instance_base_type, (const Object *) this, type);
}

void VisualScript::set_node_position(const String func, const int64_t id, const Vector2 position) {
	___godot_icall_void_String_int_Vector2(___mb.mb_set_node_position, (const Object *) this, func, id, position);
}

void VisualScript::set_variable_default_value(const String name, const Variant value) {
	___godot_icall_void_String_Variant(___mb.mb_set_variable_default_value, (const Object *) this, name, value);
}

void VisualScript::set_variable_export(const String name, const bool enable) {
	___godot_icall_void_String_bool(___mb.mb_set_variable_export, (const Object *) this, name, enable);
}

void VisualScript::set_variable_info(const String name, const Dictionary value) {
	___godot_icall_void_String_Dictionary(___mb.mb_set_variable_info, (const Object *) this, name, value);
}

}