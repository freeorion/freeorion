#include "GraphNode.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "Texture.hpp"


namespace godot {


GraphNode::___method_bindings GraphNode::___mb = {};

void GraphNode::___init_method_bindings() {
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("GraphNode", "_gui_input");
	___mb.mb_clear_all_slots = godot::api->godot_method_bind_get_method("GraphNode", "clear_all_slots");
	___mb.mb_clear_slot = godot::api->godot_method_bind_get_method("GraphNode", "clear_slot");
	___mb.mb_get_connection_input_color = godot::api->godot_method_bind_get_method("GraphNode", "get_connection_input_color");
	___mb.mb_get_connection_input_count = godot::api->godot_method_bind_get_method("GraphNode", "get_connection_input_count");
	___mb.mb_get_connection_input_position = godot::api->godot_method_bind_get_method("GraphNode", "get_connection_input_position");
	___mb.mb_get_connection_input_type = godot::api->godot_method_bind_get_method("GraphNode", "get_connection_input_type");
	___mb.mb_get_connection_output_color = godot::api->godot_method_bind_get_method("GraphNode", "get_connection_output_color");
	___mb.mb_get_connection_output_count = godot::api->godot_method_bind_get_method("GraphNode", "get_connection_output_count");
	___mb.mb_get_connection_output_position = godot::api->godot_method_bind_get_method("GraphNode", "get_connection_output_position");
	___mb.mb_get_connection_output_type = godot::api->godot_method_bind_get_method("GraphNode", "get_connection_output_type");
	___mb.mb_get_offset = godot::api->godot_method_bind_get_method("GraphNode", "get_offset");
	___mb.mb_get_overlay = godot::api->godot_method_bind_get_method("GraphNode", "get_overlay");
	___mb.mb_get_slot_color_left = godot::api->godot_method_bind_get_method("GraphNode", "get_slot_color_left");
	___mb.mb_get_slot_color_right = godot::api->godot_method_bind_get_method("GraphNode", "get_slot_color_right");
	___mb.mb_get_slot_type_left = godot::api->godot_method_bind_get_method("GraphNode", "get_slot_type_left");
	___mb.mb_get_slot_type_right = godot::api->godot_method_bind_get_method("GraphNode", "get_slot_type_right");
	___mb.mb_get_title = godot::api->godot_method_bind_get_method("GraphNode", "get_title");
	___mb.mb_is_close_button_visible = godot::api->godot_method_bind_get_method("GraphNode", "is_close_button_visible");
	___mb.mb_is_comment = godot::api->godot_method_bind_get_method("GraphNode", "is_comment");
	___mb.mb_is_resizable = godot::api->godot_method_bind_get_method("GraphNode", "is_resizable");
	___mb.mb_is_selected = godot::api->godot_method_bind_get_method("GraphNode", "is_selected");
	___mb.mb_is_slot_enabled_left = godot::api->godot_method_bind_get_method("GraphNode", "is_slot_enabled_left");
	___mb.mb_is_slot_enabled_right = godot::api->godot_method_bind_get_method("GraphNode", "is_slot_enabled_right");
	___mb.mb_set_comment = godot::api->godot_method_bind_get_method("GraphNode", "set_comment");
	___mb.mb_set_offset = godot::api->godot_method_bind_get_method("GraphNode", "set_offset");
	___mb.mb_set_overlay = godot::api->godot_method_bind_get_method("GraphNode", "set_overlay");
	___mb.mb_set_resizable = godot::api->godot_method_bind_get_method("GraphNode", "set_resizable");
	___mb.mb_set_selected = godot::api->godot_method_bind_get_method("GraphNode", "set_selected");
	___mb.mb_set_show_close_button = godot::api->godot_method_bind_get_method("GraphNode", "set_show_close_button");
	___mb.mb_set_slot = godot::api->godot_method_bind_get_method("GraphNode", "set_slot");
	___mb.mb_set_title = godot::api->godot_method_bind_get_method("GraphNode", "set_title");
}

GraphNode *GraphNode::_new()
{
	return (GraphNode *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"GraphNode")());
}
void GraphNode::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

void GraphNode::clear_all_slots() {
	___godot_icall_void(___mb.mb_clear_all_slots, (const Object *) this);
}

void GraphNode::clear_slot(const int64_t idx) {
	___godot_icall_void_int(___mb.mb_clear_slot, (const Object *) this, idx);
}

Color GraphNode::get_connection_input_color(const int64_t idx) {
	return ___godot_icall_Color_int(___mb.mb_get_connection_input_color, (const Object *) this, idx);
}

int64_t GraphNode::get_connection_input_count() {
	return ___godot_icall_int(___mb.mb_get_connection_input_count, (const Object *) this);
}

Vector2 GraphNode::get_connection_input_position(const int64_t idx) {
	return ___godot_icall_Vector2_int(___mb.mb_get_connection_input_position, (const Object *) this, idx);
}

int64_t GraphNode::get_connection_input_type(const int64_t idx) {
	return ___godot_icall_int_int(___mb.mb_get_connection_input_type, (const Object *) this, idx);
}

Color GraphNode::get_connection_output_color(const int64_t idx) {
	return ___godot_icall_Color_int(___mb.mb_get_connection_output_color, (const Object *) this, idx);
}

int64_t GraphNode::get_connection_output_count() {
	return ___godot_icall_int(___mb.mb_get_connection_output_count, (const Object *) this);
}

Vector2 GraphNode::get_connection_output_position(const int64_t idx) {
	return ___godot_icall_Vector2_int(___mb.mb_get_connection_output_position, (const Object *) this, idx);
}

int64_t GraphNode::get_connection_output_type(const int64_t idx) {
	return ___godot_icall_int_int(___mb.mb_get_connection_output_type, (const Object *) this, idx);
}

Vector2 GraphNode::get_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_offset, (const Object *) this);
}

GraphNode::Overlay GraphNode::get_overlay() const {
	return (GraphNode::Overlay) ___godot_icall_int(___mb.mb_get_overlay, (const Object *) this);
}

Color GraphNode::get_slot_color_left(const int64_t idx) const {
	return ___godot_icall_Color_int(___mb.mb_get_slot_color_left, (const Object *) this, idx);
}

Color GraphNode::get_slot_color_right(const int64_t idx) const {
	return ___godot_icall_Color_int(___mb.mb_get_slot_color_right, (const Object *) this, idx);
}

int64_t GraphNode::get_slot_type_left(const int64_t idx) const {
	return ___godot_icall_int_int(___mb.mb_get_slot_type_left, (const Object *) this, idx);
}

int64_t GraphNode::get_slot_type_right(const int64_t idx) const {
	return ___godot_icall_int_int(___mb.mb_get_slot_type_right, (const Object *) this, idx);
}

String GraphNode::get_title() const {
	return ___godot_icall_String(___mb.mb_get_title, (const Object *) this);
}

bool GraphNode::is_close_button_visible() const {
	return ___godot_icall_bool(___mb.mb_is_close_button_visible, (const Object *) this);
}

bool GraphNode::is_comment() const {
	return ___godot_icall_bool(___mb.mb_is_comment, (const Object *) this);
}

bool GraphNode::is_resizable() const {
	return ___godot_icall_bool(___mb.mb_is_resizable, (const Object *) this);
}

bool GraphNode::is_selected() {
	return ___godot_icall_bool(___mb.mb_is_selected, (const Object *) this);
}

bool GraphNode::is_slot_enabled_left(const int64_t idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_slot_enabled_left, (const Object *) this, idx);
}

bool GraphNode::is_slot_enabled_right(const int64_t idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_slot_enabled_right, (const Object *) this, idx);
}

void GraphNode::set_comment(const bool comment) {
	___godot_icall_void_bool(___mb.mb_set_comment, (const Object *) this, comment);
}

void GraphNode::set_offset(const Vector2 offset) {
	___godot_icall_void_Vector2(___mb.mb_set_offset, (const Object *) this, offset);
}

void GraphNode::set_overlay(const int64_t overlay) {
	___godot_icall_void_int(___mb.mb_set_overlay, (const Object *) this, overlay);
}

void GraphNode::set_resizable(const bool resizable) {
	___godot_icall_void_bool(___mb.mb_set_resizable, (const Object *) this, resizable);
}

void GraphNode::set_selected(const bool selected) {
	___godot_icall_void_bool(___mb.mb_set_selected, (const Object *) this, selected);
}

void GraphNode::set_show_close_button(const bool show) {
	___godot_icall_void_bool(___mb.mb_set_show_close_button, (const Object *) this, show);
}

void GraphNode::set_slot(const int64_t idx, const bool enable_left, const int64_t type_left, const Color color_left, const bool enable_right, const int64_t type_right, const Color color_right, const Ref<Texture> custom_left, const Ref<Texture> custom_right) {
	___godot_icall_void_int_bool_int_Color_bool_int_Color_Object_Object(___mb.mb_set_slot, (const Object *) this, idx, enable_left, type_left, color_left, enable_right, type_right, color_right, custom_left.ptr(), custom_right.ptr());
}

void GraphNode::set_title(const String title) {
	___godot_icall_void_String(___mb.mb_set_title, (const Object *) this, title);
}

}