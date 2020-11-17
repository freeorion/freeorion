#include "RegEx.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "RegExMatch.hpp"


namespace godot {


RegEx::___method_bindings RegEx::___mb = {};

void RegEx::___init_method_bindings() {
	___mb.mb_clear = godot::api->godot_method_bind_get_method("RegEx", "clear");
	___mb.mb_compile = godot::api->godot_method_bind_get_method("RegEx", "compile");
	___mb.mb_get_group_count = godot::api->godot_method_bind_get_method("RegEx", "get_group_count");
	___mb.mb_get_names = godot::api->godot_method_bind_get_method("RegEx", "get_names");
	___mb.mb_get_pattern = godot::api->godot_method_bind_get_method("RegEx", "get_pattern");
	___mb.mb_is_valid = godot::api->godot_method_bind_get_method("RegEx", "is_valid");
	___mb.mb_search = godot::api->godot_method_bind_get_method("RegEx", "search");
	___mb.mb_search_all = godot::api->godot_method_bind_get_method("RegEx", "search_all");
	___mb.mb_sub = godot::api->godot_method_bind_get_method("RegEx", "sub");
}

RegEx *RegEx::_new()
{
	return (RegEx *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"RegEx")());
}
void RegEx::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

Error RegEx::compile(const String pattern) {
	return (Error) ___godot_icall_int_String(___mb.mb_compile, (const Object *) this, pattern);
}

int64_t RegEx::get_group_count() const {
	return ___godot_icall_int(___mb.mb_get_group_count, (const Object *) this);
}

Array RegEx::get_names() const {
	return ___godot_icall_Array(___mb.mb_get_names, (const Object *) this);
}

String RegEx::get_pattern() const {
	return ___godot_icall_String(___mb.mb_get_pattern, (const Object *) this);
}

bool RegEx::is_valid() const {
	return ___godot_icall_bool(___mb.mb_is_valid, (const Object *) this);
}

Ref<RegExMatch> RegEx::search(const String subject, const int64_t offset, const int64_t end) const {
	return Ref<RegExMatch>::__internal_constructor(___godot_icall_Object_String_int_int(___mb.mb_search, (const Object *) this, subject, offset, end));
}

Array RegEx::search_all(const String subject, const int64_t offset, const int64_t end) const {
	return ___godot_icall_Array_String_int_int(___mb.mb_search_all, (const Object *) this, subject, offset, end);
}

String RegEx::sub(const String subject, const String replacement, const bool all, const int64_t offset, const int64_t end) const {
	return ___godot_icall_String_String_String_bool_int_int(___mb.mb_sub, (const Object *) this, subject, replacement, all, offset, end);
}

}