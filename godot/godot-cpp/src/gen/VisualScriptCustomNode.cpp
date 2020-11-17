#include "VisualScriptCustomNode.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptCustomNode::___method_bindings VisualScriptCustomNode::___mb = {};

void VisualScriptCustomNode::___init_method_bindings() {
	___mb.mb__get_caption = godot::api->godot_method_bind_get_method("VisualScriptCustomNode", "_get_caption");
	___mb.mb__get_category = godot::api->godot_method_bind_get_method("VisualScriptCustomNode", "_get_category");
	___mb.mb__get_input_value_port_count = godot::api->godot_method_bind_get_method("VisualScriptCustomNode", "_get_input_value_port_count");
	___mb.mb__get_input_value_port_name = godot::api->godot_method_bind_get_method("VisualScriptCustomNode", "_get_input_value_port_name");
	___mb.mb__get_input_value_port_type = godot::api->godot_method_bind_get_method("VisualScriptCustomNode", "_get_input_value_port_type");
	___mb.mb__get_output_sequence_port_count = godot::api->godot_method_bind_get_method("VisualScriptCustomNode", "_get_output_sequence_port_count");
	___mb.mb__get_output_sequence_port_text = godot::api->godot_method_bind_get_method("VisualScriptCustomNode", "_get_output_sequence_port_text");
	___mb.mb__get_output_value_port_count = godot::api->godot_method_bind_get_method("VisualScriptCustomNode", "_get_output_value_port_count");
	___mb.mb__get_output_value_port_name = godot::api->godot_method_bind_get_method("VisualScriptCustomNode", "_get_output_value_port_name");
	___mb.mb__get_output_value_port_type = godot::api->godot_method_bind_get_method("VisualScriptCustomNode", "_get_output_value_port_type");
	___mb.mb__get_text = godot::api->godot_method_bind_get_method("VisualScriptCustomNode", "_get_text");
	___mb.mb__get_working_memory_size = godot::api->godot_method_bind_get_method("VisualScriptCustomNode", "_get_working_memory_size");
	___mb.mb__has_input_sequence_port = godot::api->godot_method_bind_get_method("VisualScriptCustomNode", "_has_input_sequence_port");
	___mb.mb__script_changed = godot::api->godot_method_bind_get_method("VisualScriptCustomNode", "_script_changed");
	___mb.mb__step = godot::api->godot_method_bind_get_method("VisualScriptCustomNode", "_step");
}

VisualScriptCustomNode *VisualScriptCustomNode::_new()
{
	return (VisualScriptCustomNode *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptCustomNode")());
}
String VisualScriptCustomNode::_get_caption() {
	return ___godot_icall_String(___mb.mb__get_caption, (const Object *) this);
}

String VisualScriptCustomNode::_get_category() {
	return ___godot_icall_String(___mb.mb__get_category, (const Object *) this);
}

int64_t VisualScriptCustomNode::_get_input_value_port_count() {
	return ___godot_icall_int(___mb.mb__get_input_value_port_count, (const Object *) this);
}

String VisualScriptCustomNode::_get_input_value_port_name(const int64_t idx) {
	return ___godot_icall_String_int(___mb.mb__get_input_value_port_name, (const Object *) this, idx);
}

int64_t VisualScriptCustomNode::_get_input_value_port_type(const int64_t idx) {
	return ___godot_icall_int_int(___mb.mb__get_input_value_port_type, (const Object *) this, idx);
}

int64_t VisualScriptCustomNode::_get_output_sequence_port_count() {
	return ___godot_icall_int(___mb.mb__get_output_sequence_port_count, (const Object *) this);
}

String VisualScriptCustomNode::_get_output_sequence_port_text(const int64_t idx) {
	return ___godot_icall_String_int(___mb.mb__get_output_sequence_port_text, (const Object *) this, idx);
}

int64_t VisualScriptCustomNode::_get_output_value_port_count() {
	return ___godot_icall_int(___mb.mb__get_output_value_port_count, (const Object *) this);
}

String VisualScriptCustomNode::_get_output_value_port_name(const int64_t idx) {
	return ___godot_icall_String_int(___mb.mb__get_output_value_port_name, (const Object *) this, idx);
}

int64_t VisualScriptCustomNode::_get_output_value_port_type(const int64_t idx) {
	return ___godot_icall_int_int(___mb.mb__get_output_value_port_type, (const Object *) this, idx);
}

String VisualScriptCustomNode::_get_text() {
	return ___godot_icall_String(___mb.mb__get_text, (const Object *) this);
}

int64_t VisualScriptCustomNode::_get_working_memory_size() {
	return ___godot_icall_int(___mb.mb__get_working_memory_size, (const Object *) this);
}

bool VisualScriptCustomNode::_has_input_sequence_port() {
	return ___godot_icall_bool(___mb.mb__has_input_sequence_port, (const Object *) this);
}

void VisualScriptCustomNode::_script_changed() {
	___godot_icall_void(___mb.mb__script_changed, (const Object *) this);
}

Variant VisualScriptCustomNode::_step(const Array inputs, const Array outputs, const int64_t start_mode, const Array working_mem) {
	return ___godot_icall_Variant_Array_Array_int_Array(___mb.mb__step, (const Object *) this, inputs, outputs, start_mode, working_mem);
}

}