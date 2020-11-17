#include "GraphEdit.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Node.hpp"
#include "InputEvent.hpp"
#include "HBoxContainer.hpp"


namespace godot {


GraphEdit::___method_bindings GraphEdit::___mb = {};

void GraphEdit::___init_method_bindings() {
	___mb.mb__connections_layer_draw = godot::api->godot_method_bind_get_method("GraphEdit", "_connections_layer_draw");
	___mb.mb__graph_node_moved = godot::api->godot_method_bind_get_method("GraphEdit", "_graph_node_moved");
	___mb.mb__graph_node_raised = godot::api->godot_method_bind_get_method("GraphEdit", "_graph_node_raised");
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("GraphEdit", "_gui_input");
	___mb.mb__scroll_moved = godot::api->godot_method_bind_get_method("GraphEdit", "_scroll_moved");
	___mb.mb__snap_toggled = godot::api->godot_method_bind_get_method("GraphEdit", "_snap_toggled");
	___mb.mb__snap_value_changed = godot::api->godot_method_bind_get_method("GraphEdit", "_snap_value_changed");
	___mb.mb__top_layer_draw = godot::api->godot_method_bind_get_method("GraphEdit", "_top_layer_draw");
	___mb.mb__top_layer_input = godot::api->godot_method_bind_get_method("GraphEdit", "_top_layer_input");
	___mb.mb__update_scroll_offset = godot::api->godot_method_bind_get_method("GraphEdit", "_update_scroll_offset");
	___mb.mb__zoom_minus = godot::api->godot_method_bind_get_method("GraphEdit", "_zoom_minus");
	___mb.mb__zoom_plus = godot::api->godot_method_bind_get_method("GraphEdit", "_zoom_plus");
	___mb.mb__zoom_reset = godot::api->godot_method_bind_get_method("GraphEdit", "_zoom_reset");
	___mb.mb_add_valid_connection_type = godot::api->godot_method_bind_get_method("GraphEdit", "add_valid_connection_type");
	___mb.mb_add_valid_left_disconnect_type = godot::api->godot_method_bind_get_method("GraphEdit", "add_valid_left_disconnect_type");
	___mb.mb_add_valid_right_disconnect_type = godot::api->godot_method_bind_get_method("GraphEdit", "add_valid_right_disconnect_type");
	___mb.mb_clear_connections = godot::api->godot_method_bind_get_method("GraphEdit", "clear_connections");
	___mb.mb_connect_node = godot::api->godot_method_bind_get_method("GraphEdit", "connect_node");
	___mb.mb_disconnect_node = godot::api->godot_method_bind_get_method("GraphEdit", "disconnect_node");
	___mb.mb_get_connection_list = godot::api->godot_method_bind_get_method("GraphEdit", "get_connection_list");
	___mb.mb_get_scroll_ofs = godot::api->godot_method_bind_get_method("GraphEdit", "get_scroll_ofs");
	___mb.mb_get_snap = godot::api->godot_method_bind_get_method("GraphEdit", "get_snap");
	___mb.mb_get_zoom = godot::api->godot_method_bind_get_method("GraphEdit", "get_zoom");
	___mb.mb_get_zoom_hbox = godot::api->godot_method_bind_get_method("GraphEdit", "get_zoom_hbox");
	___mb.mb_is_node_connected = godot::api->godot_method_bind_get_method("GraphEdit", "is_node_connected");
	___mb.mb_is_right_disconnects_enabled = godot::api->godot_method_bind_get_method("GraphEdit", "is_right_disconnects_enabled");
	___mb.mb_is_using_snap = godot::api->godot_method_bind_get_method("GraphEdit", "is_using_snap");
	___mb.mb_is_valid_connection_type = godot::api->godot_method_bind_get_method("GraphEdit", "is_valid_connection_type");
	___mb.mb_remove_valid_connection_type = godot::api->godot_method_bind_get_method("GraphEdit", "remove_valid_connection_type");
	___mb.mb_remove_valid_left_disconnect_type = godot::api->godot_method_bind_get_method("GraphEdit", "remove_valid_left_disconnect_type");
	___mb.mb_remove_valid_right_disconnect_type = godot::api->godot_method_bind_get_method("GraphEdit", "remove_valid_right_disconnect_type");
	___mb.mb_set_connection_activity = godot::api->godot_method_bind_get_method("GraphEdit", "set_connection_activity");
	___mb.mb_set_right_disconnects = godot::api->godot_method_bind_get_method("GraphEdit", "set_right_disconnects");
	___mb.mb_set_scroll_ofs = godot::api->godot_method_bind_get_method("GraphEdit", "set_scroll_ofs");
	___mb.mb_set_selected = godot::api->godot_method_bind_get_method("GraphEdit", "set_selected");
	___mb.mb_set_snap = godot::api->godot_method_bind_get_method("GraphEdit", "set_snap");
	___mb.mb_set_use_snap = godot::api->godot_method_bind_get_method("GraphEdit", "set_use_snap");
	___mb.mb_set_zoom = godot::api->godot_method_bind_get_method("GraphEdit", "set_zoom");
}

GraphEdit *GraphEdit::_new()
{
	return (GraphEdit *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"GraphEdit")());
}
void GraphEdit::_connections_layer_draw() {
	___godot_icall_void(___mb.mb__connections_layer_draw, (const Object *) this);
}

void GraphEdit::_graph_node_moved(const Node *arg0) {
	___godot_icall_void_Object(___mb.mb__graph_node_moved, (const Object *) this, arg0);
}

void GraphEdit::_graph_node_raised(const Node *arg0) {
	___godot_icall_void_Object(___mb.mb__graph_node_raised, (const Object *) this, arg0);
}

void GraphEdit::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

void GraphEdit::_scroll_moved(const real_t arg0) {
	___godot_icall_void_float(___mb.mb__scroll_moved, (const Object *) this, arg0);
}

void GraphEdit::_snap_toggled() {
	___godot_icall_void(___mb.mb__snap_toggled, (const Object *) this);
}

void GraphEdit::_snap_value_changed(const real_t arg0) {
	___godot_icall_void_float(___mb.mb__snap_value_changed, (const Object *) this, arg0);
}

void GraphEdit::_top_layer_draw() {
	___godot_icall_void(___mb.mb__top_layer_draw, (const Object *) this);
}

void GraphEdit::_top_layer_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__top_layer_input, (const Object *) this, arg0.ptr());
}

void GraphEdit::_update_scroll_offset() {
	___godot_icall_void(___mb.mb__update_scroll_offset, (const Object *) this);
}

void GraphEdit::_zoom_minus() {
	___godot_icall_void(___mb.mb__zoom_minus, (const Object *) this);
}

void GraphEdit::_zoom_plus() {
	___godot_icall_void(___mb.mb__zoom_plus, (const Object *) this);
}

void GraphEdit::_zoom_reset() {
	___godot_icall_void(___mb.mb__zoom_reset, (const Object *) this);
}

void GraphEdit::add_valid_connection_type(const int64_t from_type, const int64_t to_type) {
	___godot_icall_void_int_int(___mb.mb_add_valid_connection_type, (const Object *) this, from_type, to_type);
}

void GraphEdit::add_valid_left_disconnect_type(const int64_t type) {
	___godot_icall_void_int(___mb.mb_add_valid_left_disconnect_type, (const Object *) this, type);
}

void GraphEdit::add_valid_right_disconnect_type(const int64_t type) {
	___godot_icall_void_int(___mb.mb_add_valid_right_disconnect_type, (const Object *) this, type);
}

void GraphEdit::clear_connections() {
	___godot_icall_void(___mb.mb_clear_connections, (const Object *) this);
}

Error GraphEdit::connect_node(const String from, const int64_t from_port, const String to, const int64_t to_port) {
	return (Error) ___godot_icall_int_String_int_String_int(___mb.mb_connect_node, (const Object *) this, from, from_port, to, to_port);
}

void GraphEdit::disconnect_node(const String from, const int64_t from_port, const String to, const int64_t to_port) {
	___godot_icall_void_String_int_String_int(___mb.mb_disconnect_node, (const Object *) this, from, from_port, to, to_port);
}

Array GraphEdit::get_connection_list() const {
	return ___godot_icall_Array(___mb.mb_get_connection_list, (const Object *) this);
}

Vector2 GraphEdit::get_scroll_ofs() const {
	return ___godot_icall_Vector2(___mb.mb_get_scroll_ofs, (const Object *) this);
}

int64_t GraphEdit::get_snap() const {
	return ___godot_icall_int(___mb.mb_get_snap, (const Object *) this);
}

real_t GraphEdit::get_zoom() const {
	return ___godot_icall_float(___mb.mb_get_zoom, (const Object *) this);
}

HBoxContainer *GraphEdit::get_zoom_hbox() {
	return (HBoxContainer *) ___godot_icall_Object(___mb.mb_get_zoom_hbox, (const Object *) this);
}

bool GraphEdit::is_node_connected(const String from, const int64_t from_port, const String to, const int64_t to_port) {
	return ___godot_icall_bool_String_int_String_int(___mb.mb_is_node_connected, (const Object *) this, from, from_port, to, to_port);
}

bool GraphEdit::is_right_disconnects_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_right_disconnects_enabled, (const Object *) this);
}

bool GraphEdit::is_using_snap() const {
	return ___godot_icall_bool(___mb.mb_is_using_snap, (const Object *) this);
}

bool GraphEdit::is_valid_connection_type(const int64_t from_type, const int64_t to_type) const {
	return ___godot_icall_bool_int_int(___mb.mb_is_valid_connection_type, (const Object *) this, from_type, to_type);
}

void GraphEdit::remove_valid_connection_type(const int64_t from_type, const int64_t to_type) {
	___godot_icall_void_int_int(___mb.mb_remove_valid_connection_type, (const Object *) this, from_type, to_type);
}

void GraphEdit::remove_valid_left_disconnect_type(const int64_t type) {
	___godot_icall_void_int(___mb.mb_remove_valid_left_disconnect_type, (const Object *) this, type);
}

void GraphEdit::remove_valid_right_disconnect_type(const int64_t type) {
	___godot_icall_void_int(___mb.mb_remove_valid_right_disconnect_type, (const Object *) this, type);
}

void GraphEdit::set_connection_activity(const String from, const int64_t from_port, const String to, const int64_t to_port, const real_t amount) {
	___godot_icall_void_String_int_String_int_float(___mb.mb_set_connection_activity, (const Object *) this, from, from_port, to, to_port, amount);
}

void GraphEdit::set_right_disconnects(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_right_disconnects, (const Object *) this, enable);
}

void GraphEdit::set_scroll_ofs(const Vector2 ofs) {
	___godot_icall_void_Vector2(___mb.mb_set_scroll_ofs, (const Object *) this, ofs);
}

void GraphEdit::set_selected(const Node *node) {
	___godot_icall_void_Object(___mb.mb_set_selected, (const Object *) this, node);
}

void GraphEdit::set_snap(const int64_t pixels) {
	___godot_icall_void_int(___mb.mb_set_snap, (const Object *) this, pixels);
}

void GraphEdit::set_use_snap(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_snap, (const Object *) this, enable);
}

void GraphEdit::set_zoom(const real_t p_zoom) {
	___godot_icall_void_float(___mb.mb_set_zoom, (const Object *) this, p_zoom);
}

}