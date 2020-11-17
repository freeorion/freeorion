#include "ClassDB.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"


namespace godot {


ClassDB *ClassDB::_singleton = NULL;


ClassDB::ClassDB() {
	_owner = godot::api->godot_global_get_singleton((char *) "ClassDB");
}


ClassDB::___method_bindings ClassDB::___mb = {};

void ClassDB::___init_method_bindings() {
	___mb.mb_can_instance = godot::api->godot_method_bind_get_method("_ClassDB", "can_instance");
	___mb.mb_class_exists = godot::api->godot_method_bind_get_method("_ClassDB", "class_exists");
	___mb.mb_class_get_category = godot::api->godot_method_bind_get_method("_ClassDB", "class_get_category");
	___mb.mb_class_get_integer_constant = godot::api->godot_method_bind_get_method("_ClassDB", "class_get_integer_constant");
	___mb.mb_class_get_integer_constant_list = godot::api->godot_method_bind_get_method("_ClassDB", "class_get_integer_constant_list");
	___mb.mb_class_get_method_list = godot::api->godot_method_bind_get_method("_ClassDB", "class_get_method_list");
	___mb.mb_class_get_property = godot::api->godot_method_bind_get_method("_ClassDB", "class_get_property");
	___mb.mb_class_get_property_list = godot::api->godot_method_bind_get_method("_ClassDB", "class_get_property_list");
	___mb.mb_class_get_signal = godot::api->godot_method_bind_get_method("_ClassDB", "class_get_signal");
	___mb.mb_class_get_signal_list = godot::api->godot_method_bind_get_method("_ClassDB", "class_get_signal_list");
	___mb.mb_class_has_integer_constant = godot::api->godot_method_bind_get_method("_ClassDB", "class_has_integer_constant");
	___mb.mb_class_has_method = godot::api->godot_method_bind_get_method("_ClassDB", "class_has_method");
	___mb.mb_class_has_signal = godot::api->godot_method_bind_get_method("_ClassDB", "class_has_signal");
	___mb.mb_class_set_property = godot::api->godot_method_bind_get_method("_ClassDB", "class_set_property");
	___mb.mb_get_class_list = godot::api->godot_method_bind_get_method("_ClassDB", "get_class_list");
	___mb.mb_get_inheriters_from_class = godot::api->godot_method_bind_get_method("_ClassDB", "get_inheriters_from_class");
	___mb.mb_get_parent_class = godot::api->godot_method_bind_get_method("_ClassDB", "get_parent_class");
	___mb.mb_instance = godot::api->godot_method_bind_get_method("_ClassDB", "instance");
	___mb.mb_is_class_enabled = godot::api->godot_method_bind_get_method("_ClassDB", "is_class_enabled");
	___mb.mb_is_parent_class = godot::api->godot_method_bind_get_method("_ClassDB", "is_parent_class");
}

bool ClassDB::can_instance(const String _class) const {
	return ___godot_icall_bool_String(___mb.mb_can_instance, (const Object *) this, _class);
}

bool ClassDB::class_exists(const String _class) const {
	return ___godot_icall_bool_String(___mb.mb_class_exists, (const Object *) this, _class);
}

String ClassDB::class_get_category(const String _class) const {
	return ___godot_icall_String_String(___mb.mb_class_get_category, (const Object *) this, _class);
}

int64_t ClassDB::class_get_integer_constant(const String _class, const String name) const {
	return ___godot_icall_int_String_String(___mb.mb_class_get_integer_constant, (const Object *) this, _class, name);
}

PoolStringArray ClassDB::class_get_integer_constant_list(const String _class, const bool no_inheritance) const {
	return ___godot_icall_PoolStringArray_String_bool(___mb.mb_class_get_integer_constant_list, (const Object *) this, _class, no_inheritance);
}

Array ClassDB::class_get_method_list(const String _class, const bool no_inheritance) const {
	return ___godot_icall_Array_String_bool(___mb.mb_class_get_method_list, (const Object *) this, _class, no_inheritance);
}

Variant ClassDB::class_get_property(const Object *object, const String property) const {
	return ___godot_icall_Variant_Object_String(___mb.mb_class_get_property, (const Object *) this, object, property);
}

Array ClassDB::class_get_property_list(const String _class, const bool no_inheritance) const {
	return ___godot_icall_Array_String_bool(___mb.mb_class_get_property_list, (const Object *) this, _class, no_inheritance);
}

Dictionary ClassDB::class_get_signal(const String _class, const String signal) const {
	return ___godot_icall_Dictionary_String_String(___mb.mb_class_get_signal, (const Object *) this, _class, signal);
}

Array ClassDB::class_get_signal_list(const String _class, const bool no_inheritance) const {
	return ___godot_icall_Array_String_bool(___mb.mb_class_get_signal_list, (const Object *) this, _class, no_inheritance);
}

bool ClassDB::class_has_integer_constant(const String _class, const String name) const {
	return ___godot_icall_bool_String_String(___mb.mb_class_has_integer_constant, (const Object *) this, _class, name);
}

bool ClassDB::class_has_method(const String _class, const String method, const bool no_inheritance) const {
	return ___godot_icall_bool_String_String_bool(___mb.mb_class_has_method, (const Object *) this, _class, method, no_inheritance);
}

bool ClassDB::class_has_signal(const String _class, const String signal) const {
	return ___godot_icall_bool_String_String(___mb.mb_class_has_signal, (const Object *) this, _class, signal);
}

Error ClassDB::class_set_property(const Object *object, const String property, const Variant value) const {
	return (Error) ___godot_icall_int_Object_String_Variant(___mb.mb_class_set_property, (const Object *) this, object, property, value);
}

PoolStringArray ClassDB::get_class_list() const {
	return ___godot_icall_PoolStringArray(___mb.mb_get_class_list, (const Object *) this);
}

PoolStringArray ClassDB::get_inheriters_from_class(const String _class) const {
	return ___godot_icall_PoolStringArray_String(___mb.mb_get_inheriters_from_class, (const Object *) this, _class);
}

String ClassDB::get_parent_class(const String _class) const {
	return ___godot_icall_String_String(___mb.mb_get_parent_class, (const Object *) this, _class);
}

Variant ClassDB::instance(const String _class) const {
	return ___godot_icall_Variant_String(___mb.mb_instance, (const Object *) this, _class);
}

bool ClassDB::is_class_enabled(const String _class) const {
	return ___godot_icall_bool_String(___mb.mb_is_class_enabled, (const Object *) this, _class);
}

bool ClassDB::is_parent_class(const String _class, const String inherits) const {
	return ___godot_icall_bool_String_String(___mb.mb_is_parent_class, (const Object *) this, _class, inherits);
}

}