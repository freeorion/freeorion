#include "VisualScriptPropertyGet.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptPropertyGet::___method_bindings VisualScriptPropertyGet::___mb = {};

void VisualScriptPropertyGet::___init_method_bindings() {
	___mb.mb__get_type_cache = godot::api->godot_method_bind_get_method("VisualScriptPropertyGet", "_get_type_cache");
	___mb.mb__set_type_cache = godot::api->godot_method_bind_get_method("VisualScriptPropertyGet", "_set_type_cache");
	___mb.mb_get_base_path = godot::api->godot_method_bind_get_method("VisualScriptPropertyGet", "get_base_path");
	___mb.mb_get_base_script = godot::api->godot_method_bind_get_method("VisualScriptPropertyGet", "get_base_script");
	___mb.mb_get_base_type = godot::api->godot_method_bind_get_method("VisualScriptPropertyGet", "get_base_type");
	___mb.mb_get_basic_type = godot::api->godot_method_bind_get_method("VisualScriptPropertyGet", "get_basic_type");
	___mb.mb_get_call_mode = godot::api->godot_method_bind_get_method("VisualScriptPropertyGet", "get_call_mode");
	___mb.mb_get_index = godot::api->godot_method_bind_get_method("VisualScriptPropertyGet", "get_index");
	___mb.mb_get_property = godot::api->godot_method_bind_get_method("VisualScriptPropertyGet", "get_property");
	___mb.mb_set_base_path = godot::api->godot_method_bind_get_method("VisualScriptPropertyGet", "set_base_path");
	___mb.mb_set_base_script = godot::api->godot_method_bind_get_method("VisualScriptPropertyGet", "set_base_script");
	___mb.mb_set_base_type = godot::api->godot_method_bind_get_method("VisualScriptPropertyGet", "set_base_type");
	___mb.mb_set_basic_type = godot::api->godot_method_bind_get_method("VisualScriptPropertyGet", "set_basic_type");
	___mb.mb_set_call_mode = godot::api->godot_method_bind_get_method("VisualScriptPropertyGet", "set_call_mode");
	___mb.mb_set_index = godot::api->godot_method_bind_get_method("VisualScriptPropertyGet", "set_index");
	___mb.mb_set_property = godot::api->godot_method_bind_get_method("VisualScriptPropertyGet", "set_property");
}

VisualScriptPropertyGet *VisualScriptPropertyGet::_new()
{
	return (VisualScriptPropertyGet *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptPropertyGet")());
}
Variant::Type VisualScriptPropertyGet::_get_type_cache() const {
	return (Variant::Type) ___godot_icall_int(___mb.mb__get_type_cache, (const Object *) this);
}

void VisualScriptPropertyGet::_set_type_cache(const int64_t type_cache) {
	___godot_icall_void_int(___mb.mb__set_type_cache, (const Object *) this, type_cache);
}

NodePath VisualScriptPropertyGet::get_base_path() const {
	return ___godot_icall_NodePath(___mb.mb_get_base_path, (const Object *) this);
}

String VisualScriptPropertyGet::get_base_script() const {
	return ___godot_icall_String(___mb.mb_get_base_script, (const Object *) this);
}

String VisualScriptPropertyGet::get_base_type() const {
	return ___godot_icall_String(___mb.mb_get_base_type, (const Object *) this);
}

Variant::Type VisualScriptPropertyGet::get_basic_type() const {
	return (Variant::Type) ___godot_icall_int(___mb.mb_get_basic_type, (const Object *) this);
}

VisualScriptPropertyGet::CallMode VisualScriptPropertyGet::get_call_mode() const {
	return (VisualScriptPropertyGet::CallMode) ___godot_icall_int(___mb.mb_get_call_mode, (const Object *) this);
}

String VisualScriptPropertyGet::get_index() const {
	return ___godot_icall_String(___mb.mb_get_index, (const Object *) this);
}

String VisualScriptPropertyGet::get_property() const {
	return ___godot_icall_String(___mb.mb_get_property, (const Object *) this);
}

void VisualScriptPropertyGet::set_base_path(const NodePath base_path) {
	___godot_icall_void_NodePath(___mb.mb_set_base_path, (const Object *) this, base_path);
}

void VisualScriptPropertyGet::set_base_script(const String base_script) {
	___godot_icall_void_String(___mb.mb_set_base_script, (const Object *) this, base_script);
}

void VisualScriptPropertyGet::set_base_type(const String base_type) {
	___godot_icall_void_String(___mb.mb_set_base_type, (const Object *) this, base_type);
}

void VisualScriptPropertyGet::set_basic_type(const int64_t basic_type) {
	___godot_icall_void_int(___mb.mb_set_basic_type, (const Object *) this, basic_type);
}

void VisualScriptPropertyGet::set_call_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_call_mode, (const Object *) this, mode);
}

void VisualScriptPropertyGet::set_index(const String index) {
	___godot_icall_void_String(___mb.mb_set_index, (const Object *) this, index);
}

void VisualScriptPropertyGet::set_property(const String property) {
	___godot_icall_void_String(___mb.mb_set_property, (const Object *) this, property);
}

}