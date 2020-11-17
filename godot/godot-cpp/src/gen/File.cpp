#include "File.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


File::___method_bindings File::___mb = {};

void File::___init_method_bindings() {
	___mb.mb_close = godot::api->godot_method_bind_get_method("_File", "close");
	___mb.mb_eof_reached = godot::api->godot_method_bind_get_method("_File", "eof_reached");
	___mb.mb_file_exists = godot::api->godot_method_bind_get_method("_File", "file_exists");
	___mb.mb_get_16 = godot::api->godot_method_bind_get_method("_File", "get_16");
	___mb.mb_get_32 = godot::api->godot_method_bind_get_method("_File", "get_32");
	___mb.mb_get_64 = godot::api->godot_method_bind_get_method("_File", "get_64");
	___mb.mb_get_8 = godot::api->godot_method_bind_get_method("_File", "get_8");
	___mb.mb_get_as_text = godot::api->godot_method_bind_get_method("_File", "get_as_text");
	___mb.mb_get_buffer = godot::api->godot_method_bind_get_method("_File", "get_buffer");
	___mb.mb_get_csv_line = godot::api->godot_method_bind_get_method("_File", "get_csv_line");
	___mb.mb_get_double = godot::api->godot_method_bind_get_method("_File", "get_double");
	___mb.mb_get_endian_swap = godot::api->godot_method_bind_get_method("_File", "get_endian_swap");
	___mb.mb_get_error = godot::api->godot_method_bind_get_method("_File", "get_error");
	___mb.mb_get_float = godot::api->godot_method_bind_get_method("_File", "get_float");
	___mb.mb_get_len = godot::api->godot_method_bind_get_method("_File", "get_len");
	___mb.mb_get_line = godot::api->godot_method_bind_get_method("_File", "get_line");
	___mb.mb_get_md5 = godot::api->godot_method_bind_get_method("_File", "get_md5");
	___mb.mb_get_modified_time = godot::api->godot_method_bind_get_method("_File", "get_modified_time");
	___mb.mb_get_pascal_string = godot::api->godot_method_bind_get_method("_File", "get_pascal_string");
	___mb.mb_get_path = godot::api->godot_method_bind_get_method("_File", "get_path");
	___mb.mb_get_path_absolute = godot::api->godot_method_bind_get_method("_File", "get_path_absolute");
	___mb.mb_get_position = godot::api->godot_method_bind_get_method("_File", "get_position");
	___mb.mb_get_real = godot::api->godot_method_bind_get_method("_File", "get_real");
	___mb.mb_get_sha256 = godot::api->godot_method_bind_get_method("_File", "get_sha256");
	___mb.mb_get_var = godot::api->godot_method_bind_get_method("_File", "get_var");
	___mb.mb_is_open = godot::api->godot_method_bind_get_method("_File", "is_open");
	___mb.mb_open = godot::api->godot_method_bind_get_method("_File", "open");
	___mb.mb_open_compressed = godot::api->godot_method_bind_get_method("_File", "open_compressed");
	___mb.mb_open_encrypted = godot::api->godot_method_bind_get_method("_File", "open_encrypted");
	___mb.mb_open_encrypted_with_pass = godot::api->godot_method_bind_get_method("_File", "open_encrypted_with_pass");
	___mb.mb_seek = godot::api->godot_method_bind_get_method("_File", "seek");
	___mb.mb_seek_end = godot::api->godot_method_bind_get_method("_File", "seek_end");
	___mb.mb_set_endian_swap = godot::api->godot_method_bind_get_method("_File", "set_endian_swap");
	___mb.mb_store_16 = godot::api->godot_method_bind_get_method("_File", "store_16");
	___mb.mb_store_32 = godot::api->godot_method_bind_get_method("_File", "store_32");
	___mb.mb_store_64 = godot::api->godot_method_bind_get_method("_File", "store_64");
	___mb.mb_store_8 = godot::api->godot_method_bind_get_method("_File", "store_8");
	___mb.mb_store_buffer = godot::api->godot_method_bind_get_method("_File", "store_buffer");
	___mb.mb_store_csv_line = godot::api->godot_method_bind_get_method("_File", "store_csv_line");
	___mb.mb_store_double = godot::api->godot_method_bind_get_method("_File", "store_double");
	___mb.mb_store_float = godot::api->godot_method_bind_get_method("_File", "store_float");
	___mb.mb_store_line = godot::api->godot_method_bind_get_method("_File", "store_line");
	___mb.mb_store_pascal_string = godot::api->godot_method_bind_get_method("_File", "store_pascal_string");
	___mb.mb_store_real = godot::api->godot_method_bind_get_method("_File", "store_real");
	___mb.mb_store_string = godot::api->godot_method_bind_get_method("_File", "store_string");
	___mb.mb_store_var = godot::api->godot_method_bind_get_method("_File", "store_var");
}

File *File::_new()
{
	return (File *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"_File")());
}
void File::close() {
	___godot_icall_void(___mb.mb_close, (const Object *) this);
}

bool File::eof_reached() const {
	return ___godot_icall_bool(___mb.mb_eof_reached, (const Object *) this);
}

bool File::file_exists(const String path) const {
	return ___godot_icall_bool_String(___mb.mb_file_exists, (const Object *) this, path);
}

int64_t File::get_16() const {
	return ___godot_icall_int(___mb.mb_get_16, (const Object *) this);
}

int64_t File::get_32() const {
	return ___godot_icall_int(___mb.mb_get_32, (const Object *) this);
}

int64_t File::get_64() const {
	return ___godot_icall_int(___mb.mb_get_64, (const Object *) this);
}

int64_t File::get_8() const {
	return ___godot_icall_int(___mb.mb_get_8, (const Object *) this);
}

String File::get_as_text() const {
	return ___godot_icall_String(___mb.mb_get_as_text, (const Object *) this);
}

PoolByteArray File::get_buffer(const int64_t len) const {
	return ___godot_icall_PoolByteArray_int(___mb.mb_get_buffer, (const Object *) this, len);
}

PoolStringArray File::get_csv_line(const String delim) const {
	return ___godot_icall_PoolStringArray_String(___mb.mb_get_csv_line, (const Object *) this, delim);
}

real_t File::get_double() const {
	return ___godot_icall_float(___mb.mb_get_double, (const Object *) this);
}

bool File::get_endian_swap() {
	return ___godot_icall_bool(___mb.mb_get_endian_swap, (const Object *) this);
}

Error File::get_error() const {
	return (Error) ___godot_icall_int(___mb.mb_get_error, (const Object *) this);
}

real_t File::get_float() const {
	return ___godot_icall_float(___mb.mb_get_float, (const Object *) this);
}

int64_t File::get_len() const {
	return ___godot_icall_int(___mb.mb_get_len, (const Object *) this);
}

String File::get_line() const {
	return ___godot_icall_String(___mb.mb_get_line, (const Object *) this);
}

String File::get_md5(const String path) const {
	return ___godot_icall_String_String(___mb.mb_get_md5, (const Object *) this, path);
}

int64_t File::get_modified_time(const String file) const {
	return ___godot_icall_int_String(___mb.mb_get_modified_time, (const Object *) this, file);
}

String File::get_pascal_string() {
	return ___godot_icall_String(___mb.mb_get_pascal_string, (const Object *) this);
}

String File::get_path() const {
	return ___godot_icall_String(___mb.mb_get_path, (const Object *) this);
}

String File::get_path_absolute() const {
	return ___godot_icall_String(___mb.mb_get_path_absolute, (const Object *) this);
}

int64_t File::get_position() const {
	return ___godot_icall_int(___mb.mb_get_position, (const Object *) this);
}

real_t File::get_real() const {
	return ___godot_icall_float(___mb.mb_get_real, (const Object *) this);
}

String File::get_sha256(const String path) const {
	return ___godot_icall_String_String(___mb.mb_get_sha256, (const Object *) this, path);
}

Variant File::get_var(const bool allow_objects) const {
	return ___godot_icall_Variant_bool(___mb.mb_get_var, (const Object *) this, allow_objects);
}

bool File::is_open() const {
	return ___godot_icall_bool(___mb.mb_is_open, (const Object *) this);
}

Error File::open(const String path, const int64_t flags) {
	return (Error) ___godot_icall_int_String_int(___mb.mb_open, (const Object *) this, path, flags);
}

Error File::open_compressed(const String path, const int64_t mode_flags, const int64_t compression_mode) {
	return (Error) ___godot_icall_int_String_int_int(___mb.mb_open_compressed, (const Object *) this, path, mode_flags, compression_mode);
}

Error File::open_encrypted(const String path, const int64_t mode_flags, const PoolByteArray key) {
	return (Error) ___godot_icall_int_String_int_PoolByteArray(___mb.mb_open_encrypted, (const Object *) this, path, mode_flags, key);
}

Error File::open_encrypted_with_pass(const String path, const int64_t mode_flags, const String pass) {
	return (Error) ___godot_icall_int_String_int_String(___mb.mb_open_encrypted_with_pass, (const Object *) this, path, mode_flags, pass);
}

void File::seek(const int64_t position) {
	___godot_icall_void_int(___mb.mb_seek, (const Object *) this, position);
}

void File::seek_end(const int64_t position) {
	___godot_icall_void_int(___mb.mb_seek_end, (const Object *) this, position);
}

void File::set_endian_swap(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_endian_swap, (const Object *) this, enable);
}

void File::store_16(const int64_t value) {
	___godot_icall_void_int(___mb.mb_store_16, (const Object *) this, value);
}

void File::store_32(const int64_t value) {
	___godot_icall_void_int(___mb.mb_store_32, (const Object *) this, value);
}

void File::store_64(const int64_t value) {
	___godot_icall_void_int(___mb.mb_store_64, (const Object *) this, value);
}

void File::store_8(const int64_t value) {
	___godot_icall_void_int(___mb.mb_store_8, (const Object *) this, value);
}

void File::store_buffer(const PoolByteArray buffer) {
	___godot_icall_void_PoolByteArray(___mb.mb_store_buffer, (const Object *) this, buffer);
}

void File::store_csv_line(const PoolStringArray values, const String delim) {
	___godot_icall_void_PoolStringArray_String(___mb.mb_store_csv_line, (const Object *) this, values, delim);
}

void File::store_double(const real_t value) {
	___godot_icall_void_float(___mb.mb_store_double, (const Object *) this, value);
}

void File::store_float(const real_t value) {
	___godot_icall_void_float(___mb.mb_store_float, (const Object *) this, value);
}

void File::store_line(const String line) {
	___godot_icall_void_String(___mb.mb_store_line, (const Object *) this, line);
}

void File::store_pascal_string(const String string) {
	___godot_icall_void_String(___mb.mb_store_pascal_string, (const Object *) this, string);
}

void File::store_real(const real_t value) {
	___godot_icall_void_float(___mb.mb_store_real, (const Object *) this, value);
}

void File::store_string(const String string) {
	___godot_icall_void_String(___mb.mb_store_string, (const Object *) this, string);
}

void File::store_var(const Variant value, const bool full_objects) {
	___godot_icall_void_Variant_bool(___mb.mb_store_var, (const Object *) this, value, full_objects);
}

}