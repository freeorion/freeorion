#include "Object.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "Reference.hpp"


namespace godot {


Object::___method_bindings Object::___mb = {};

void Object::___init_method_bindings() {
	___mb.mb__get = godot::api->godot_method_bind_get_method("Object", "_get");
	___mb.mb__get_property_list = godot::api->godot_method_bind_get_method("Object", "_get_property_list");
	___mb.mb__init = godot::api->godot_method_bind_get_method("Object", "_init");
	___mb.mb__notification = godot::api->godot_method_bind_get_method("Object", "_notification");
	___mb.mb__set = godot::api->godot_method_bind_get_method("Object", "_set");
	___mb.mb__to_string = godot::api->godot_method_bind_get_method("Object", "_to_string");
	___mb.mb_add_user_signal = godot::api->godot_method_bind_get_method("Object", "add_user_signal");
	___mb.mb_call = godot::api->godot_method_bind_get_method("Object", "call");
	___mb.mb_call_deferred = godot::api->godot_method_bind_get_method("Object", "call_deferred");
	___mb.mb_callv = godot::api->godot_method_bind_get_method("Object", "callv");
	___mb.mb_can_translate_messages = godot::api->godot_method_bind_get_method("Object", "can_translate_messages");
	___mb.mb_connect = godot::api->godot_method_bind_get_method("Object", "connect");
	___mb.mb_disconnect = godot::api->godot_method_bind_get_method("Object", "disconnect");
	___mb.mb_emit_signal = godot::api->godot_method_bind_get_method("Object", "emit_signal");
	___mb.mb_free = godot::api->godot_method_bind_get_method("Object", "free");
	___mb.mb_get = godot::api->godot_method_bind_get_method("Object", "get");
	___mb.mb_get_class = godot::api->godot_method_bind_get_method("Object", "get_class");
	___mb.mb_get_incoming_connections = godot::api->godot_method_bind_get_method("Object", "get_incoming_connections");
	___mb.mb_get_indexed = godot::api->godot_method_bind_get_method("Object", "get_indexed");
	___mb.mb_get_instance_id = godot::api->godot_method_bind_get_method("Object", "get_instance_id");
	___mb.mb_get_meta = godot::api->godot_method_bind_get_method("Object", "get_meta");
	___mb.mb_get_meta_list = godot::api->godot_method_bind_get_method("Object", "get_meta_list");
	___mb.mb_get_method_list = godot::api->godot_method_bind_get_method("Object", "get_method_list");
	___mb.mb_get_property_list = godot::api->godot_method_bind_get_method("Object", "get_property_list");
	___mb.mb_get_script = godot::api->godot_method_bind_get_method("Object", "get_script");
	___mb.mb_get_signal_connection_list = godot::api->godot_method_bind_get_method("Object", "get_signal_connection_list");
	___mb.mb_get_signal_list = godot::api->godot_method_bind_get_method("Object", "get_signal_list");
	___mb.mb_has_meta = godot::api->godot_method_bind_get_method("Object", "has_meta");
	___mb.mb_has_method = godot::api->godot_method_bind_get_method("Object", "has_method");
	___mb.mb_has_user_signal = godot::api->godot_method_bind_get_method("Object", "has_user_signal");
	___mb.mb_is_blocking_signals = godot::api->godot_method_bind_get_method("Object", "is_blocking_signals");
	___mb.mb_is_class = godot::api->godot_method_bind_get_method("Object", "is_class");
	___mb.mb_is_connected = godot::api->godot_method_bind_get_method("Object", "is_connected");
	___mb.mb_is_queued_for_deletion = godot::api->godot_method_bind_get_method("Object", "is_queued_for_deletion");
	___mb.mb_notification = godot::api->godot_method_bind_get_method("Object", "notification");
	___mb.mb_property_list_changed_notify = godot::api->godot_method_bind_get_method("Object", "property_list_changed_notify");
	___mb.mb_remove_meta = godot::api->godot_method_bind_get_method("Object", "remove_meta");
	___mb.mb_set = godot::api->godot_method_bind_get_method("Object", "set");
	___mb.mb_set_block_signals = godot::api->godot_method_bind_get_method("Object", "set_block_signals");
	___mb.mb_set_deferred = godot::api->godot_method_bind_get_method("Object", "set_deferred");
	___mb.mb_set_indexed = godot::api->godot_method_bind_get_method("Object", "set_indexed");
	___mb.mb_set_message_translation = godot::api->godot_method_bind_get_method("Object", "set_message_translation");
	___mb.mb_set_meta = godot::api->godot_method_bind_get_method("Object", "set_meta");
	___mb.mb_set_script = godot::api->godot_method_bind_get_method("Object", "set_script");
	___mb.mb_to_string = godot::api->godot_method_bind_get_method("Object", "to_string");
	___mb.mb_tr = godot::api->godot_method_bind_get_method("Object", "tr");
}

Object *Object::_new()
{
	return (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Object")());
}
Variant Object::_get(const String property) {
	return ___godot_icall_Variant_String(___mb.mb__get, (const Object *) this, property);
}

Array Object::_get_property_list() {
	return ___godot_icall_Array(___mb.mb__get_property_list, (const Object *) this);
}

void Object::_init() {
	___godot_icall_void(___mb.mb__init, (const Object *) this);
}

void Object::_notification(const int64_t what) {
	___godot_icall_void_int(___mb.mb__notification, (const Object *) this, what);
}

bool Object::_set(const String property, const Variant value) {
	return ___godot_icall_bool_String_Variant(___mb.mb__set, (const Object *) this, property, value);
}

String Object::_to_string() {
	return ___godot_icall_String(___mb.mb__to_string, (const Object *) this);
}

void Object::add_user_signal(const String signal, const Array arguments) {
	___godot_icall_void_String_Array(___mb.mb_add_user_signal, (const Object *) this, signal, arguments);
}

Variant Object::call(const String method, const Array& __var_args) {
	Variant __given_args[1];
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[0]);

	__given_args[0] = method;

	godot_variant **__args = (godot_variant **) alloca(sizeof(godot_variant *) * (__var_args.size() + 1));

	__args[0] = (godot_variant *) &__given_args[0];

	for (int i = 0; i < __var_args.size(); i++) {
		__args[i + 1] = (godot_variant *) &((Array &) __var_args)[i];
	}

	Variant __result;
	*(godot_variant *) &__result = godot::api->godot_method_bind_call(___mb.mb_call, ((const Object *) this)->_owner, (const godot_variant **) __args, (__var_args.size() + 1), nullptr);

	godot::api->godot_variant_destroy((godot_variant *) &__given_args[0]);

	return __result;
}

void Object::call_deferred(const String method, const Array& __var_args) {
	Variant __given_args[1];
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[0]);

	__given_args[0] = method;

	godot_variant **__args = (godot_variant **) alloca(sizeof(godot_variant *) * (__var_args.size() + 1));

	__args[0] = (godot_variant *) &__given_args[0];

	for (int i = 0; i < __var_args.size(); i++) {
		__args[i + 1] = (godot_variant *) &((Array &) __var_args)[i];
	}

	Variant __result;
	*(godot_variant *) &__result = godot::api->godot_method_bind_call(___mb.mb_call_deferred, ((const Object *) this)->_owner, (const godot_variant **) __args, (__var_args.size() + 1), nullptr);

	godot::api->godot_variant_destroy((godot_variant *) &__given_args[0]);

}

Variant Object::callv(const String method, const Array arg_array) {
	return ___godot_icall_Variant_String_Array(___mb.mb_callv, (const Object *) this, method, arg_array);
}

bool Object::can_translate_messages() const {
	return ___godot_icall_bool(___mb.mb_can_translate_messages, (const Object *) this);
}

Error Object::connect(const String signal, const Object *target, const String method, const Array binds, const int64_t flags) {
	return (Error) ___godot_icall_int_String_Object_String_Array_int(___mb.mb_connect, (const Object *) this, signal, target, method, binds, flags);
}

void Object::disconnect(const String signal, const Object *target, const String method) {
	___godot_icall_void_String_Object_String(___mb.mb_disconnect, (const Object *) this, signal, target, method);
}

void Object::emit_signal(const String signal, const Array& __var_args) {
	Variant __given_args[1];
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[0]);

	__given_args[0] = signal;

	godot_variant **__args = (godot_variant **) alloca(sizeof(godot_variant *) * (__var_args.size() + 1));

	__args[0] = (godot_variant *) &__given_args[0];

	for (int i = 0; i < __var_args.size(); i++) {
		__args[i + 1] = (godot_variant *) &((Array &) __var_args)[i];
	}

	Variant __result;
	*(godot_variant *) &__result = godot::api->godot_method_bind_call(___mb.mb_emit_signal, ((const Object *) this)->_owner, (const godot_variant **) __args, (__var_args.size() + 1), nullptr);

	godot::api->godot_variant_destroy((godot_variant *) &__given_args[0]);

}

void Object::free() {
	godot::api->godot_object_destroy(_owner);
}

Variant Object::get(const String property) const {
	return ___godot_icall_Variant_String(___mb.mb_get, (const Object *) this, property);
}

String Object::get_class() const {
	return ___godot_icall_String(___mb.mb_get_class, (const Object *) this);
}

Array Object::get_incoming_connections() const {
	return ___godot_icall_Array(___mb.mb_get_incoming_connections, (const Object *) this);
}

Variant Object::get_indexed(const NodePath property) const {
	return ___godot_icall_Variant_NodePath(___mb.mb_get_indexed, (const Object *) this, property);
}

int64_t Object::get_instance_id() const {
	return ___godot_icall_int(___mb.mb_get_instance_id, (const Object *) this);
}

Variant Object::get_meta(const String name) const {
	return ___godot_icall_Variant_String(___mb.mb_get_meta, (const Object *) this, name);
}

PoolStringArray Object::get_meta_list() const {
	return ___godot_icall_PoolStringArray(___mb.mb_get_meta_list, (const Object *) this);
}

Array Object::get_method_list() const {
	return ___godot_icall_Array(___mb.mb_get_method_list, (const Object *) this);
}

Array Object::get_property_list() const {
	return ___godot_icall_Array(___mb.mb_get_property_list, (const Object *) this);
}

Reference *Object::get_script() const {
	return (Reference *) ___godot_icall_Object(___mb.mb_get_script, (const Object *) this);
}

Array Object::get_signal_connection_list(const String signal) const {
	return ___godot_icall_Array_String(___mb.mb_get_signal_connection_list, (const Object *) this, signal);
}

Array Object::get_signal_list() const {
	return ___godot_icall_Array(___mb.mb_get_signal_list, (const Object *) this);
}

bool Object::has_meta(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_meta, (const Object *) this, name);
}

bool Object::has_method(const String method) const {
	return ___godot_icall_bool_String(___mb.mb_has_method, (const Object *) this, method);
}

bool Object::has_user_signal(const String signal) const {
	return ___godot_icall_bool_String(___mb.mb_has_user_signal, (const Object *) this, signal);
}

bool Object::is_blocking_signals() const {
	return ___godot_icall_bool(___mb.mb_is_blocking_signals, (const Object *) this);
}

bool Object::is_class(const String _class) const {
	return ___godot_icall_bool_String(___mb.mb_is_class, (const Object *) this, _class);
}

bool Object::is_connected(const String signal, const Object *target, const String method) const {
	return ___godot_icall_bool_String_Object_String(___mb.mb_is_connected, (const Object *) this, signal, target, method);
}

bool Object::is_queued_for_deletion() const {
	return ___godot_icall_bool(___mb.mb_is_queued_for_deletion, (const Object *) this);
}

void Object::notification(const int64_t what, const bool reversed) {
	___godot_icall_void_int_bool(___mb.mb_notification, (const Object *) this, what, reversed);
}

void Object::property_list_changed_notify() {
	___godot_icall_void(___mb.mb_property_list_changed_notify, (const Object *) this);
}

void Object::remove_meta(const String name) {
	___godot_icall_void_String(___mb.mb_remove_meta, (const Object *) this, name);
}

void Object::set(const String property, const Variant value) {
	___godot_icall_void_String_Variant(___mb.mb_set, (const Object *) this, property, value);
}

void Object::set_block_signals(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_block_signals, (const Object *) this, enable);
}

void Object::set_deferred(const String property, const Variant value) {
	___godot_icall_void_String_Variant(___mb.mb_set_deferred, (const Object *) this, property, value);
}

void Object::set_indexed(const NodePath property, const Variant value) {
	___godot_icall_void_NodePath_Variant(___mb.mb_set_indexed, (const Object *) this, property, value);
}

void Object::set_message_translation(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_message_translation, (const Object *) this, enable);
}

void Object::set_meta(const String name, const Variant value) {
	___godot_icall_void_String_Variant(___mb.mb_set_meta, (const Object *) this, name, value);
}

void Object::set_script(const Reference *script) {
	___godot_icall_void_Object(___mb.mb_set_script, (const Object *) this, script);
}

String Object::to_string() {
	return ___godot_icall_String(___mb.mb_to_string, (const Object *) this);
}

String Object::tr(const String message) const {
	return ___godot_icall_String_String(___mb.mb_tr, (const Object *) this, message);
}

}