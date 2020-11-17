#include "StreamPeer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


StreamPeer::___method_bindings StreamPeer::___mb = {};

void StreamPeer::___init_method_bindings() {
	___mb.mb_get_16 = godot::api->godot_method_bind_get_method("StreamPeer", "get_16");
	___mb.mb_get_32 = godot::api->godot_method_bind_get_method("StreamPeer", "get_32");
	___mb.mb_get_64 = godot::api->godot_method_bind_get_method("StreamPeer", "get_64");
	___mb.mb_get_8 = godot::api->godot_method_bind_get_method("StreamPeer", "get_8");
	___mb.mb_get_available_bytes = godot::api->godot_method_bind_get_method("StreamPeer", "get_available_bytes");
	___mb.mb_get_data = godot::api->godot_method_bind_get_method("StreamPeer", "get_data");
	___mb.mb_get_double = godot::api->godot_method_bind_get_method("StreamPeer", "get_double");
	___mb.mb_get_float = godot::api->godot_method_bind_get_method("StreamPeer", "get_float");
	___mb.mb_get_partial_data = godot::api->godot_method_bind_get_method("StreamPeer", "get_partial_data");
	___mb.mb_get_string = godot::api->godot_method_bind_get_method("StreamPeer", "get_string");
	___mb.mb_get_u16 = godot::api->godot_method_bind_get_method("StreamPeer", "get_u16");
	___mb.mb_get_u32 = godot::api->godot_method_bind_get_method("StreamPeer", "get_u32");
	___mb.mb_get_u64 = godot::api->godot_method_bind_get_method("StreamPeer", "get_u64");
	___mb.mb_get_u8 = godot::api->godot_method_bind_get_method("StreamPeer", "get_u8");
	___mb.mb_get_utf8_string = godot::api->godot_method_bind_get_method("StreamPeer", "get_utf8_string");
	___mb.mb_get_var = godot::api->godot_method_bind_get_method("StreamPeer", "get_var");
	___mb.mb_is_big_endian_enabled = godot::api->godot_method_bind_get_method("StreamPeer", "is_big_endian_enabled");
	___mb.mb_put_16 = godot::api->godot_method_bind_get_method("StreamPeer", "put_16");
	___mb.mb_put_32 = godot::api->godot_method_bind_get_method("StreamPeer", "put_32");
	___mb.mb_put_64 = godot::api->godot_method_bind_get_method("StreamPeer", "put_64");
	___mb.mb_put_8 = godot::api->godot_method_bind_get_method("StreamPeer", "put_8");
	___mb.mb_put_data = godot::api->godot_method_bind_get_method("StreamPeer", "put_data");
	___mb.mb_put_double = godot::api->godot_method_bind_get_method("StreamPeer", "put_double");
	___mb.mb_put_float = godot::api->godot_method_bind_get_method("StreamPeer", "put_float");
	___mb.mb_put_partial_data = godot::api->godot_method_bind_get_method("StreamPeer", "put_partial_data");
	___mb.mb_put_string = godot::api->godot_method_bind_get_method("StreamPeer", "put_string");
	___mb.mb_put_u16 = godot::api->godot_method_bind_get_method("StreamPeer", "put_u16");
	___mb.mb_put_u32 = godot::api->godot_method_bind_get_method("StreamPeer", "put_u32");
	___mb.mb_put_u64 = godot::api->godot_method_bind_get_method("StreamPeer", "put_u64");
	___mb.mb_put_u8 = godot::api->godot_method_bind_get_method("StreamPeer", "put_u8");
	___mb.mb_put_utf8_string = godot::api->godot_method_bind_get_method("StreamPeer", "put_utf8_string");
	___mb.mb_put_var = godot::api->godot_method_bind_get_method("StreamPeer", "put_var");
	___mb.mb_set_big_endian = godot::api->godot_method_bind_get_method("StreamPeer", "set_big_endian");
}

int64_t StreamPeer::get_16() {
	return ___godot_icall_int(___mb.mb_get_16, (const Object *) this);
}

int64_t StreamPeer::get_32() {
	return ___godot_icall_int(___mb.mb_get_32, (const Object *) this);
}

int64_t StreamPeer::get_64() {
	return ___godot_icall_int(___mb.mb_get_64, (const Object *) this);
}

int64_t StreamPeer::get_8() {
	return ___godot_icall_int(___mb.mb_get_8, (const Object *) this);
}

int64_t StreamPeer::get_available_bytes() const {
	return ___godot_icall_int(___mb.mb_get_available_bytes, (const Object *) this);
}

Array StreamPeer::get_data(const int64_t bytes) {
	return ___godot_icall_Array_int(___mb.mb_get_data, (const Object *) this, bytes);
}

real_t StreamPeer::get_double() {
	return ___godot_icall_float(___mb.mb_get_double, (const Object *) this);
}

real_t StreamPeer::get_float() {
	return ___godot_icall_float(___mb.mb_get_float, (const Object *) this);
}

Array StreamPeer::get_partial_data(const int64_t bytes) {
	return ___godot_icall_Array_int(___mb.mb_get_partial_data, (const Object *) this, bytes);
}

String StreamPeer::get_string(const int64_t bytes) {
	return ___godot_icall_String_int(___mb.mb_get_string, (const Object *) this, bytes);
}

int64_t StreamPeer::get_u16() {
	return ___godot_icall_int(___mb.mb_get_u16, (const Object *) this);
}

int64_t StreamPeer::get_u32() {
	return ___godot_icall_int(___mb.mb_get_u32, (const Object *) this);
}

int64_t StreamPeer::get_u64() {
	return ___godot_icall_int(___mb.mb_get_u64, (const Object *) this);
}

int64_t StreamPeer::get_u8() {
	return ___godot_icall_int(___mb.mb_get_u8, (const Object *) this);
}

String StreamPeer::get_utf8_string(const int64_t bytes) {
	return ___godot_icall_String_int(___mb.mb_get_utf8_string, (const Object *) this, bytes);
}

Variant StreamPeer::get_var(const bool allow_objects) {
	return ___godot_icall_Variant_bool(___mb.mb_get_var, (const Object *) this, allow_objects);
}

bool StreamPeer::is_big_endian_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_big_endian_enabled, (const Object *) this);
}

void StreamPeer::put_16(const int64_t value) {
	___godot_icall_void_int(___mb.mb_put_16, (const Object *) this, value);
}

void StreamPeer::put_32(const int64_t value) {
	___godot_icall_void_int(___mb.mb_put_32, (const Object *) this, value);
}

void StreamPeer::put_64(const int64_t value) {
	___godot_icall_void_int(___mb.mb_put_64, (const Object *) this, value);
}

void StreamPeer::put_8(const int64_t value) {
	___godot_icall_void_int(___mb.mb_put_8, (const Object *) this, value);
}

Error StreamPeer::put_data(const PoolByteArray data) {
	return (Error) ___godot_icall_int_PoolByteArray(___mb.mb_put_data, (const Object *) this, data);
}

void StreamPeer::put_double(const real_t value) {
	___godot_icall_void_float(___mb.mb_put_double, (const Object *) this, value);
}

void StreamPeer::put_float(const real_t value) {
	___godot_icall_void_float(___mb.mb_put_float, (const Object *) this, value);
}

Array StreamPeer::put_partial_data(const PoolByteArray data) {
	return ___godot_icall_Array_PoolByteArray(___mb.mb_put_partial_data, (const Object *) this, data);
}

void StreamPeer::put_string(const String value) {
	___godot_icall_void_String(___mb.mb_put_string, (const Object *) this, value);
}

void StreamPeer::put_u16(const int64_t value) {
	___godot_icall_void_int(___mb.mb_put_u16, (const Object *) this, value);
}

void StreamPeer::put_u32(const int64_t value) {
	___godot_icall_void_int(___mb.mb_put_u32, (const Object *) this, value);
}

void StreamPeer::put_u64(const int64_t value) {
	___godot_icall_void_int(___mb.mb_put_u64, (const Object *) this, value);
}

void StreamPeer::put_u8(const int64_t value) {
	___godot_icall_void_int(___mb.mb_put_u8, (const Object *) this, value);
}

void StreamPeer::put_utf8_string(const String value) {
	___godot_icall_void_String(___mb.mb_put_utf8_string, (const Object *) this, value);
}

void StreamPeer::put_var(const Variant value, const bool full_objects) {
	___godot_icall_void_Variant_bool(___mb.mb_put_var, (const Object *) this, value, full_objects);
}

void StreamPeer::set_big_endian(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_big_endian, (const Object *) this, enable);
}

}