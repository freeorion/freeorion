#include "VisualShader.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "VisualShaderNode.hpp"


namespace godot {


VisualShader::___method_bindings VisualShader::___mb = {};

void VisualShader::___init_method_bindings() {
	___mb.mb__input_type_changed = godot::api->godot_method_bind_get_method("VisualShader", "_input_type_changed");
	___mb.mb__queue_update = godot::api->godot_method_bind_get_method("VisualShader", "_queue_update");
	___mb.mb__update_shader = godot::api->godot_method_bind_get_method("VisualShader", "_update_shader");
	___mb.mb_add_node = godot::api->godot_method_bind_get_method("VisualShader", "add_node");
	___mb.mb_can_connect_nodes = godot::api->godot_method_bind_get_method("VisualShader", "can_connect_nodes");
	___mb.mb_connect_nodes = godot::api->godot_method_bind_get_method("VisualShader", "connect_nodes");
	___mb.mb_connect_nodes_forced = godot::api->godot_method_bind_get_method("VisualShader", "connect_nodes_forced");
	___mb.mb_disconnect_nodes = godot::api->godot_method_bind_get_method("VisualShader", "disconnect_nodes");
	___mb.mb_get_graph_offset = godot::api->godot_method_bind_get_method("VisualShader", "get_graph_offset");
	___mb.mb_get_node = godot::api->godot_method_bind_get_method("VisualShader", "get_node");
	___mb.mb_get_node_connections = godot::api->godot_method_bind_get_method("VisualShader", "get_node_connections");
	___mb.mb_get_node_list = godot::api->godot_method_bind_get_method("VisualShader", "get_node_list");
	___mb.mb_get_node_position = godot::api->godot_method_bind_get_method("VisualShader", "get_node_position");
	___mb.mb_get_valid_node_id = godot::api->godot_method_bind_get_method("VisualShader", "get_valid_node_id");
	___mb.mb_is_node_connection = godot::api->godot_method_bind_get_method("VisualShader", "is_node_connection");
	___mb.mb_remove_node = godot::api->godot_method_bind_get_method("VisualShader", "remove_node");
	___mb.mb_set_graph_offset = godot::api->godot_method_bind_get_method("VisualShader", "set_graph_offset");
	___mb.mb_set_mode = godot::api->godot_method_bind_get_method("VisualShader", "set_mode");
	___mb.mb_set_node_position = godot::api->godot_method_bind_get_method("VisualShader", "set_node_position");
}

VisualShader *VisualShader::_new()
{
	return (VisualShader *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShader")());
}
void VisualShader::_input_type_changed(const int64_t arg0, const int64_t arg1) {
	___godot_icall_void_int_int(___mb.mb__input_type_changed, (const Object *) this, arg0, arg1);
}

void VisualShader::_queue_update() {
	___godot_icall_void(___mb.mb__queue_update, (const Object *) this);
}

void VisualShader::_update_shader() const {
	___godot_icall_void(___mb.mb__update_shader, (const Object *) this);
}

void VisualShader::add_node(const int64_t type, const Ref<VisualShaderNode> node, const Vector2 position, const int64_t id) {
	___godot_icall_void_int_Object_Vector2_int(___mb.mb_add_node, (const Object *) this, type, node.ptr(), position, id);
}

bool VisualShader::can_connect_nodes(const int64_t type, const int64_t from_node, const int64_t from_port, const int64_t to_node, const int64_t to_port) const {
	return ___godot_icall_bool_int_int_int_int_int(___mb.mb_can_connect_nodes, (const Object *) this, type, from_node, from_port, to_node, to_port);
}

Error VisualShader::connect_nodes(const int64_t type, const int64_t from_node, const int64_t from_port, const int64_t to_node, const int64_t to_port) {
	return (Error) ___godot_icall_int_int_int_int_int_int(___mb.mb_connect_nodes, (const Object *) this, type, from_node, from_port, to_node, to_port);
}

void VisualShader::connect_nodes_forced(const int64_t type, const int64_t from_node, const int64_t from_port, const int64_t to_node, const int64_t to_port) {
	___godot_icall_void_int_int_int_int_int(___mb.mb_connect_nodes_forced, (const Object *) this, type, from_node, from_port, to_node, to_port);
}

void VisualShader::disconnect_nodes(const int64_t type, const int64_t from_node, const int64_t from_port, const int64_t to_node, const int64_t to_port) {
	___godot_icall_void_int_int_int_int_int(___mb.mb_disconnect_nodes, (const Object *) this, type, from_node, from_port, to_node, to_port);
}

Vector2 VisualShader::get_graph_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_graph_offset, (const Object *) this);
}

Ref<VisualShaderNode> VisualShader::get_node(const int64_t type, const int64_t id) const {
	return Ref<VisualShaderNode>::__internal_constructor(___godot_icall_Object_int_int(___mb.mb_get_node, (const Object *) this, type, id));
}

Array VisualShader::get_node_connections(const int64_t type) const {
	return ___godot_icall_Array_int(___mb.mb_get_node_connections, (const Object *) this, type);
}

PoolIntArray VisualShader::get_node_list(const int64_t type) const {
	return ___godot_icall_PoolIntArray_int(___mb.mb_get_node_list, (const Object *) this, type);
}

Vector2 VisualShader::get_node_position(const int64_t type, const int64_t id) const {
	return ___godot_icall_Vector2_int_int(___mb.mb_get_node_position, (const Object *) this, type, id);
}

int64_t VisualShader::get_valid_node_id(const int64_t type) const {
	return ___godot_icall_int_int(___mb.mb_get_valid_node_id, (const Object *) this, type);
}

bool VisualShader::is_node_connection(const int64_t type, const int64_t from_node, const int64_t from_port, const int64_t to_node, const int64_t to_port) const {
	return ___godot_icall_bool_int_int_int_int_int(___mb.mb_is_node_connection, (const Object *) this, type, from_node, from_port, to_node, to_port);
}

void VisualShader::remove_node(const int64_t type, const int64_t id) {
	___godot_icall_void_int_int(___mb.mb_remove_node, (const Object *) this, type, id);
}

void VisualShader::set_graph_offset(const Vector2 offset) {
	___godot_icall_void_Vector2(___mb.mb_set_graph_offset, (const Object *) this, offset);
}

void VisualShader::set_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_mode, (const Object *) this, mode);
}

void VisualShader::set_node_position(const int64_t type, const int64_t id, const Vector2 position) {
	___godot_icall_void_int_int_Vector2(___mb.mb_set_node_position, (const Object *) this, type, id, position);
}

}