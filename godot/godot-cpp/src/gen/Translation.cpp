#include "Translation.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Translation::___method_bindings Translation::___mb = {};

void Translation::___init_method_bindings() {
	___mb.mb__get_messages = godot::api->godot_method_bind_get_method("Translation", "_get_messages");
	___mb.mb__set_messages = godot::api->godot_method_bind_get_method("Translation", "_set_messages");
	___mb.mb_add_message = godot::api->godot_method_bind_get_method("Translation", "add_message");
	___mb.mb_erase_message = godot::api->godot_method_bind_get_method("Translation", "erase_message");
	___mb.mb_get_locale = godot::api->godot_method_bind_get_method("Translation", "get_locale");
	___mb.mb_get_message = godot::api->godot_method_bind_get_method("Translation", "get_message");
	___mb.mb_get_message_count = godot::api->godot_method_bind_get_method("Translation", "get_message_count");
	___mb.mb_get_message_list = godot::api->godot_method_bind_get_method("Translation", "get_message_list");
	___mb.mb_set_locale = godot::api->godot_method_bind_get_method("Translation", "set_locale");
}

Translation *Translation::_new()
{
	return (Translation *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Translation")());
}
PoolStringArray Translation::_get_messages() const {
	return ___godot_icall_PoolStringArray(___mb.mb__get_messages, (const Object *) this);
}

void Translation::_set_messages(const PoolStringArray arg0) {
	___godot_icall_void_PoolStringArray(___mb.mb__set_messages, (const Object *) this, arg0);
}

void Translation::add_message(const String src_message, const String xlated_message) {
	___godot_icall_void_String_String(___mb.mb_add_message, (const Object *) this, src_message, xlated_message);
}

void Translation::erase_message(const String src_message) {
	___godot_icall_void_String(___mb.mb_erase_message, (const Object *) this, src_message);
}

String Translation::get_locale() const {
	return ___godot_icall_String(___mb.mb_get_locale, (const Object *) this);
}

String Translation::get_message(const String src_message) const {
	return ___godot_icall_String_String(___mb.mb_get_message, (const Object *) this, src_message);
}

int64_t Translation::get_message_count() const {
	return ___godot_icall_int(___mb.mb_get_message_count, (const Object *) this);
}

PoolStringArray Translation::get_message_list() const {
	return ___godot_icall_PoolStringArray(___mb.mb_get_message_list, (const Object *) this);
}

void Translation::set_locale(const String locale) {
	___godot_icall_void_String(___mb.mb_set_locale, (const Object *) this, locale);
}

}