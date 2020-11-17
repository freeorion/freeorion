#include "XMLParser.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


XMLParser::___method_bindings XMLParser::___mb = {};

void XMLParser::___init_method_bindings() {
	___mb.mb_get_attribute_count = godot::api->godot_method_bind_get_method("XMLParser", "get_attribute_count");
	___mb.mb_get_attribute_name = godot::api->godot_method_bind_get_method("XMLParser", "get_attribute_name");
	___mb.mb_get_attribute_value = godot::api->godot_method_bind_get_method("XMLParser", "get_attribute_value");
	___mb.mb_get_current_line = godot::api->godot_method_bind_get_method("XMLParser", "get_current_line");
	___mb.mb_get_named_attribute_value = godot::api->godot_method_bind_get_method("XMLParser", "get_named_attribute_value");
	___mb.mb_get_named_attribute_value_safe = godot::api->godot_method_bind_get_method("XMLParser", "get_named_attribute_value_safe");
	___mb.mb_get_node_data = godot::api->godot_method_bind_get_method("XMLParser", "get_node_data");
	___mb.mb_get_node_name = godot::api->godot_method_bind_get_method("XMLParser", "get_node_name");
	___mb.mb_get_node_offset = godot::api->godot_method_bind_get_method("XMLParser", "get_node_offset");
	___mb.mb_get_node_type = godot::api->godot_method_bind_get_method("XMLParser", "get_node_type");
	___mb.mb_has_attribute = godot::api->godot_method_bind_get_method("XMLParser", "has_attribute");
	___mb.mb_is_empty = godot::api->godot_method_bind_get_method("XMLParser", "is_empty");
	___mb.mb_open = godot::api->godot_method_bind_get_method("XMLParser", "open");
	___mb.mb_open_buffer = godot::api->godot_method_bind_get_method("XMLParser", "open_buffer");
	___mb.mb_read = godot::api->godot_method_bind_get_method("XMLParser", "read");
	___mb.mb_seek = godot::api->godot_method_bind_get_method("XMLParser", "seek");
	___mb.mb_skip_section = godot::api->godot_method_bind_get_method("XMLParser", "skip_section");
}

XMLParser *XMLParser::_new()
{
	return (XMLParser *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"XMLParser")());
}
int64_t XMLParser::get_attribute_count() const {
	return ___godot_icall_int(___mb.mb_get_attribute_count, (const Object *) this);
}

String XMLParser::get_attribute_name(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_attribute_name, (const Object *) this, idx);
}

String XMLParser::get_attribute_value(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_attribute_value, (const Object *) this, idx);
}

int64_t XMLParser::get_current_line() const {
	return ___godot_icall_int(___mb.mb_get_current_line, (const Object *) this);
}

String XMLParser::get_named_attribute_value(const String name) const {
	return ___godot_icall_String_String(___mb.mb_get_named_attribute_value, (const Object *) this, name);
}

String XMLParser::get_named_attribute_value_safe(const String name) const {
	return ___godot_icall_String_String(___mb.mb_get_named_attribute_value_safe, (const Object *) this, name);
}

String XMLParser::get_node_data() const {
	return ___godot_icall_String(___mb.mb_get_node_data, (const Object *) this);
}

String XMLParser::get_node_name() const {
	return ___godot_icall_String(___mb.mb_get_node_name, (const Object *) this);
}

int64_t XMLParser::get_node_offset() const {
	return ___godot_icall_int(___mb.mb_get_node_offset, (const Object *) this);
}

XMLParser::NodeType XMLParser::get_node_type() {
	return (XMLParser::NodeType) ___godot_icall_int(___mb.mb_get_node_type, (const Object *) this);
}

bool XMLParser::has_attribute(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_attribute, (const Object *) this, name);
}

bool XMLParser::is_empty() const {
	return ___godot_icall_bool(___mb.mb_is_empty, (const Object *) this);
}

Error XMLParser::open(const String file) {
	return (Error) ___godot_icall_int_String(___mb.mb_open, (const Object *) this, file);
}

Error XMLParser::open_buffer(const PoolByteArray buffer) {
	return (Error) ___godot_icall_int_PoolByteArray(___mb.mb_open_buffer, (const Object *) this, buffer);
}

Error XMLParser::read() {
	return (Error) ___godot_icall_int(___mb.mb_read, (const Object *) this);
}

Error XMLParser::seek(const int64_t position) {
	return (Error) ___godot_icall_int_int(___mb.mb_seek, (const Object *) this, position);
}

void XMLParser::skip_section() {
	___godot_icall_void(___mb.mb_skip_section, (const Object *) this);
}

}