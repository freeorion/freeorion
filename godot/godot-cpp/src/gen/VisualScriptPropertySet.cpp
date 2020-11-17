#include "VisualScriptPropertySet.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptPropertySet::___method_bindings VisualScriptPropertySet::___mb = {};

void VisualScriptPropertySet::___init_method_bindings() {
	___mb.mb__get_type_cache = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "_get_type_cache");
	___mb.mb__set_type_cache = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "_set_type_cache");
	___mb.mb_get_assign_op = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "get_assign_op");
	___mb.mb_get_base_path = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "get_base_path");
	___mb.mb_get_base_script = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "get_base_script");
	___mb.mb_get_base_type = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "get_base_type");
	___mb.mb_get_basic_type = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "get_basic_type");
	___mb.mb_get_call_mode = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "get_call_mode");
	___mb.mb_get_index = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "get_index");
	___mb.mb_get_property = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "get_property");
	___mb.mb_set_assign_op = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "set_assign_op");
	___mb.mb_set_base_path = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "set_base_path");
	___mb.mb_set_base_script = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "set_base_script");
	___mb.mb_set_base_type = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "set_base_type");
	___mb.mb_set_basic_type = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "set_basic_type");
	___mb.mb_set_call_mode = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "set_call_mode");
	___mb.mb_set_index = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "set_index");
	___mb.mb_set_property = godot::api->godot_method_bind_get_method("VisualScriptPropertySet", "set_property");
}

VisualScriptPropertySet *VisualScriptPropertySet::_new()
{
	return (VisualScriptPropertySet *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptPropertySet")());
}
Dictionary VisualScriptPropertySet::_get_type_cache() const {
	return ___godot_icall_Dictionary(___mb.mb__get_type_cache, (const Object *) this);
}

void VisualScriptPropertySet::_set_type_cache(const Dictionary type_cache) {
	___godot_icall_void_Dictionary(___mb.mb__set_type_cache, (const Object *) this, type_cache);
}

VisualScriptPropertySet::AssignOp VisualScriptPropertySet::get_assign_op() const {
	return (VisualScriptPropertySet::AssignOp) ___godot_icall_int(___mb.mb_get_assign_op, (const Object *) this);
}

NodePath VisualScriptPropertySet::get_base_path() const {
	return ___godot_icall_NodePath(___mb.mb_get_base_path, (const Object *) this);
}

String VisualScriptPropertySet::get_base_script() const {
	return ___godot_icall_String(___mb.mb_get_base_script, (const Object *) this);
}

String VisualScriptPropertySet::get_base_type() const {
	return ___godot_icall_String(___mb.mb_get_base_type, (const Object *) this);
}

Variant::Type VisualScriptPropertySet::get_basic_type() const {
	return (Variant::Type) ___godot_icall_int(___mb.mb_get_basic_type, (const Object *) this);
}

VisualScriptPropertySet::CallMode VisualScriptPropertySet::get_call_mode() const {
	return (VisualScriptPropertySet::CallMode) ___godot_icall_int(___mb.mb_get_call_mode, (const Object *) this);
}

String VisualScriptPropertySet::get_index() const {
	return ___godot_icall_String(___mb.mb_get_index, (const Object *) this);
}

String VisualScriptPropertySet::get_property() const {
	return ___godot_icall_String(___mb.mb_get_property, (const Object *) this);
}

void VisualScriptPropertySet::set_assign_op(const int64_t assign_op) {
	___godot_icall_void_int(___mb.mb_set_assign_op, (const Object *) this, assign_op);
}

void VisualScriptPropertySet::set_base_path(const NodePath base_path) {
	___godot_icall_void_NodePath(___mb.mb_set_base_path, (const Object *) this, base_path);
}

void VisualScriptPropertySet::set_base_script(const String base_script) {
	___godot_icall_void_String(___mb.mb_set_base_script, (const Object *) this, base_script);
}

void VisualScriptPropertySet::set_base_type(const String base_type) {
	___godot_icall_void_String(___mb.mb_set_base_type, (const Object *) this, base_type);
}

void VisualScriptPropertySet::set_basic_type(const int64_t basic_type) {
	___godot_icall_void_int(___mb.mb_set_basic_type, (const Object *) this, basic_type);
}

void VisualScriptPropertySet::set_call_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_call_mode, (const Object *) this, mode);
}

void VisualScriptPropertySet::set_index(const String index) {
	___godot_icall_void_String(___mb.mb_set_index, (const Object *) this, index);
}

void VisualScriptPropertySet::set_property(const String property) {
	___godot_icall_void_String(___mb.mb_set_property, (const Object *) this, property);
}

}