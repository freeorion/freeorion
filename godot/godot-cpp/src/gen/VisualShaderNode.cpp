#include "VisualShaderNode.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNode::___method_bindings VisualShaderNode::___mb = {};

void VisualShaderNode::___init_method_bindings() {
	___mb.mb_get_default_input_values = godot::api->godot_method_bind_get_method("VisualShaderNode", "get_default_input_values");
	___mb.mb_get_input_port_default_value = godot::api->godot_method_bind_get_method("VisualShaderNode", "get_input_port_default_value");
	___mb.mb_get_output_port_for_preview = godot::api->godot_method_bind_get_method("VisualShaderNode", "get_output_port_for_preview");
	___mb.mb_set_default_input_values = godot::api->godot_method_bind_get_method("VisualShaderNode", "set_default_input_values");
	___mb.mb_set_input_port_default_value = godot::api->godot_method_bind_get_method("VisualShaderNode", "set_input_port_default_value");
	___mb.mb_set_output_port_for_preview = godot::api->godot_method_bind_get_method("VisualShaderNode", "set_output_port_for_preview");
}

Array VisualShaderNode::get_default_input_values() const {
	return ___godot_icall_Array(___mb.mb_get_default_input_values, (const Object *) this);
}

Variant VisualShaderNode::get_input_port_default_value(const int64_t port) const {
	return ___godot_icall_Variant_int(___mb.mb_get_input_port_default_value, (const Object *) this, port);
}

int64_t VisualShaderNode::get_output_port_for_preview() const {
	return ___godot_icall_int(___mb.mb_get_output_port_for_preview, (const Object *) this);
}

void VisualShaderNode::set_default_input_values(const Array values) {
	___godot_icall_void_Array(___mb.mb_set_default_input_values, (const Object *) this, values);
}

void VisualShaderNode::set_input_port_default_value(const int64_t port, const Variant value) {
	___godot_icall_void_int_Variant(___mb.mb_set_input_port_default_value, (const Object *) this, port, value);
}

void VisualShaderNode::set_output_port_for_preview(const int64_t port) {
	___godot_icall_void_int(___mb.mb_set_output_port_for_preview, (const Object *) this, port);
}

}