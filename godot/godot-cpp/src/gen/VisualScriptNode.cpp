#include "VisualScriptNode.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "VisualScript.hpp"


namespace godot {


VisualScriptNode::___method_bindings VisualScriptNode::___mb = {};

void VisualScriptNode::___init_method_bindings() {
	___mb.mb__get_default_input_values = godot::api->godot_method_bind_get_method("VisualScriptNode", "_get_default_input_values");
	___mb.mb__set_default_input_values = godot::api->godot_method_bind_get_method("VisualScriptNode", "_set_default_input_values");
	___mb.mb_get_default_input_value = godot::api->godot_method_bind_get_method("VisualScriptNode", "get_default_input_value");
	___mb.mb_get_visual_script = godot::api->godot_method_bind_get_method("VisualScriptNode", "get_visual_script");
	___mb.mb_ports_changed_notify = godot::api->godot_method_bind_get_method("VisualScriptNode", "ports_changed_notify");
	___mb.mb_set_default_input_value = godot::api->godot_method_bind_get_method("VisualScriptNode", "set_default_input_value");
}

Array VisualScriptNode::_get_default_input_values() const {
	return ___godot_icall_Array(___mb.mb__get_default_input_values, (const Object *) this);
}

void VisualScriptNode::_set_default_input_values(const Array values) {
	___godot_icall_void_Array(___mb.mb__set_default_input_values, (const Object *) this, values);
}

Variant VisualScriptNode::get_default_input_value(const int64_t port_idx) const {
	return ___godot_icall_Variant_int(___mb.mb_get_default_input_value, (const Object *) this, port_idx);
}

Ref<VisualScript> VisualScriptNode::get_visual_script() const {
	return Ref<VisualScript>::__internal_constructor(___godot_icall_Object(___mb.mb_get_visual_script, (const Object *) this));
}

void VisualScriptNode::ports_changed_notify() {
	___godot_icall_void(___mb.mb_ports_changed_notify, (const Object *) this);
}

void VisualScriptNode::set_default_input_value(const int64_t port_idx, const Variant value) {
	___godot_icall_void_int_Variant(___mb.mb_set_default_input_value, (const Object *) this, port_idx, value);
}

}