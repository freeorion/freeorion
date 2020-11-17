#include "VisualShaderNodeGroupBase.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNodeGroupBase::___method_bindings VisualShaderNodeGroupBase::___mb = {};

void VisualShaderNodeGroupBase::___init_method_bindings() {
	___mb.mb_add_input_port = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "add_input_port");
	___mb.mb_add_output_port = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "add_output_port");
	___mb.mb_clear_input_ports = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "clear_input_ports");
	___mb.mb_clear_output_ports = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "clear_output_ports");
	___mb.mb_get_free_input_port_id = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "get_free_input_port_id");
	___mb.mb_get_free_output_port_id = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "get_free_output_port_id");
	___mb.mb_get_input_port_count = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "get_input_port_count");
	___mb.mb_get_inputs = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "get_inputs");
	___mb.mb_get_output_port_count = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "get_output_port_count");
	___mb.mb_get_outputs = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "get_outputs");
	___mb.mb_get_size = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "get_size");
	___mb.mb_has_input_port = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "has_input_port");
	___mb.mb_has_output_port = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "has_output_port");
	___mb.mb_is_valid_port_name = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "is_valid_port_name");
	___mb.mb_remove_input_port = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "remove_input_port");
	___mb.mb_remove_output_port = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "remove_output_port");
	___mb.mb_set_input_port_name = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "set_input_port_name");
	___mb.mb_set_input_port_type = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "set_input_port_type");
	___mb.mb_set_inputs = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "set_inputs");
	___mb.mb_set_output_port_name = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "set_output_port_name");
	___mb.mb_set_output_port_type = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "set_output_port_type");
	___mb.mb_set_outputs = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "set_outputs");
	___mb.mb_set_size = godot::api->godot_method_bind_get_method("VisualShaderNodeGroupBase", "set_size");
}

VisualShaderNodeGroupBase *VisualShaderNodeGroupBase::_new()
{
	return (VisualShaderNodeGroupBase *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeGroupBase")());
}
void VisualShaderNodeGroupBase::add_input_port(const int64_t id, const int64_t type, const String name) {
	___godot_icall_void_int_int_String(___mb.mb_add_input_port, (const Object *) this, id, type, name);
}

void VisualShaderNodeGroupBase::add_output_port(const int64_t id, const int64_t type, const String name) {
	___godot_icall_void_int_int_String(___mb.mb_add_output_port, (const Object *) this, id, type, name);
}

void VisualShaderNodeGroupBase::clear_input_ports() {
	___godot_icall_void(___mb.mb_clear_input_ports, (const Object *) this);
}

void VisualShaderNodeGroupBase::clear_output_ports() {
	___godot_icall_void(___mb.mb_clear_output_ports, (const Object *) this);
}

int64_t VisualShaderNodeGroupBase::get_free_input_port_id() const {
	return ___godot_icall_int(___mb.mb_get_free_input_port_id, (const Object *) this);
}

int64_t VisualShaderNodeGroupBase::get_free_output_port_id() const {
	return ___godot_icall_int(___mb.mb_get_free_output_port_id, (const Object *) this);
}

int64_t VisualShaderNodeGroupBase::get_input_port_count() const {
	return ___godot_icall_int(___mb.mb_get_input_port_count, (const Object *) this);
}

String VisualShaderNodeGroupBase::get_inputs() const {
	return ___godot_icall_String(___mb.mb_get_inputs, (const Object *) this);
}

int64_t VisualShaderNodeGroupBase::get_output_port_count() const {
	return ___godot_icall_int(___mb.mb_get_output_port_count, (const Object *) this);
}

String VisualShaderNodeGroupBase::get_outputs() const {
	return ___godot_icall_String(___mb.mb_get_outputs, (const Object *) this);
}

Vector2 VisualShaderNodeGroupBase::get_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_size, (const Object *) this);
}

bool VisualShaderNodeGroupBase::has_input_port(const int64_t id) const {
	return ___godot_icall_bool_int(___mb.mb_has_input_port, (const Object *) this, id);
}

bool VisualShaderNodeGroupBase::has_output_port(const int64_t id) const {
	return ___godot_icall_bool_int(___mb.mb_has_output_port, (const Object *) this, id);
}

bool VisualShaderNodeGroupBase::is_valid_port_name(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_is_valid_port_name, (const Object *) this, name);
}

void VisualShaderNodeGroupBase::remove_input_port(const int64_t id) {
	___godot_icall_void_int(___mb.mb_remove_input_port, (const Object *) this, id);
}

void VisualShaderNodeGroupBase::remove_output_port(const int64_t id) {
	___godot_icall_void_int(___mb.mb_remove_output_port, (const Object *) this, id);
}

void VisualShaderNodeGroupBase::set_input_port_name(const int64_t id, const String name) {
	___godot_icall_void_int_String(___mb.mb_set_input_port_name, (const Object *) this, id, name);
}

void VisualShaderNodeGroupBase::set_input_port_type(const int64_t id, const int64_t type) {
	___godot_icall_void_int_int(___mb.mb_set_input_port_type, (const Object *) this, id, type);
}

void VisualShaderNodeGroupBase::set_inputs(const String inputs) {
	___godot_icall_void_String(___mb.mb_set_inputs, (const Object *) this, inputs);
}

void VisualShaderNodeGroupBase::set_output_port_name(const int64_t id, const String name) {
	___godot_icall_void_int_String(___mb.mb_set_output_port_name, (const Object *) this, id, name);
}

void VisualShaderNodeGroupBase::set_output_port_type(const int64_t id, const int64_t type) {
	___godot_icall_void_int_int(___mb.mb_set_output_port_type, (const Object *) this, id, type);
}

void VisualShaderNodeGroupBase::set_outputs(const String outputs) {
	___godot_icall_void_String(___mb.mb_set_outputs, (const Object *) this, outputs);
}

void VisualShaderNodeGroupBase::set_size(const Vector2 size) {
	___godot_icall_void_Vector2(___mb.mb_set_size, (const Object *) this, size);
}

}