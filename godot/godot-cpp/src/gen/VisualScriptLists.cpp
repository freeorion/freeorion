#include "VisualScriptLists.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptLists::___method_bindings VisualScriptLists::___mb = {};

void VisualScriptLists::___init_method_bindings() {
	___mb.mb_add_input_data_port = godot::api->godot_method_bind_get_method("VisualScriptLists", "add_input_data_port");
	___mb.mb_add_output_data_port = godot::api->godot_method_bind_get_method("VisualScriptLists", "add_output_data_port");
	___mb.mb_remove_input_data_port = godot::api->godot_method_bind_get_method("VisualScriptLists", "remove_input_data_port");
	___mb.mb_remove_output_data_port = godot::api->godot_method_bind_get_method("VisualScriptLists", "remove_output_data_port");
	___mb.mb_set_input_data_port_name = godot::api->godot_method_bind_get_method("VisualScriptLists", "set_input_data_port_name");
	___mb.mb_set_input_data_port_type = godot::api->godot_method_bind_get_method("VisualScriptLists", "set_input_data_port_type");
	___mb.mb_set_output_data_port_name = godot::api->godot_method_bind_get_method("VisualScriptLists", "set_output_data_port_name");
	___mb.mb_set_output_data_port_type = godot::api->godot_method_bind_get_method("VisualScriptLists", "set_output_data_port_type");
}

void VisualScriptLists::add_input_data_port(const int64_t type, const String name, const int64_t index) {
	___godot_icall_void_int_String_int(___mb.mb_add_input_data_port, (const Object *) this, type, name, index);
}

void VisualScriptLists::add_output_data_port(const int64_t type, const String name, const int64_t index) {
	___godot_icall_void_int_String_int(___mb.mb_add_output_data_port, (const Object *) this, type, name, index);
}

void VisualScriptLists::remove_input_data_port(const int64_t index) {
	___godot_icall_void_int(___mb.mb_remove_input_data_port, (const Object *) this, index);
}

void VisualScriptLists::remove_output_data_port(const int64_t index) {
	___godot_icall_void_int(___mb.mb_remove_output_data_port, (const Object *) this, index);
}

void VisualScriptLists::set_input_data_port_name(const int64_t index, const String name) {
	___godot_icall_void_int_String(___mb.mb_set_input_data_port_name, (const Object *) this, index, name);
}

void VisualScriptLists::set_input_data_port_type(const int64_t index, const int64_t type) {
	___godot_icall_void_int_int(___mb.mb_set_input_data_port_type, (const Object *) this, index, type);
}

void VisualScriptLists::set_output_data_port_name(const int64_t index, const String name) {
	___godot_icall_void_int_String(___mb.mb_set_output_data_port_name, (const Object *) this, index, name);
}

void VisualScriptLists::set_output_data_port_type(const int64_t index, const int64_t type) {
	___godot_icall_void_int_int(___mb.mb_set_output_data_port_type, (const Object *) this, index, type);
}

}