#include "RegExMatch.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


RegExMatch::___method_bindings RegExMatch::___mb = {};

void RegExMatch::___init_method_bindings() {
	___mb.mb_get_end = godot::api->godot_method_bind_get_method("RegExMatch", "get_end");
	___mb.mb_get_group_count = godot::api->godot_method_bind_get_method("RegExMatch", "get_group_count");
	___mb.mb_get_names = godot::api->godot_method_bind_get_method("RegExMatch", "get_names");
	___mb.mb_get_start = godot::api->godot_method_bind_get_method("RegExMatch", "get_start");
	___mb.mb_get_string = godot::api->godot_method_bind_get_method("RegExMatch", "get_string");
	___mb.mb_get_strings = godot::api->godot_method_bind_get_method("RegExMatch", "get_strings");
	___mb.mb_get_subject = godot::api->godot_method_bind_get_method("RegExMatch", "get_subject");
}

RegExMatch *RegExMatch::_new()
{
	return (RegExMatch *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"RegExMatch")());
}
int64_t RegExMatch::get_end(const Variant name) const {
	return ___godot_icall_int_Variant(___mb.mb_get_end, (const Object *) this, name);
}

int64_t RegExMatch::get_group_count() const {
	return ___godot_icall_int(___mb.mb_get_group_count, (const Object *) this);
}

Dictionary RegExMatch::get_names() const {
	return ___godot_icall_Dictionary(___mb.mb_get_names, (const Object *) this);
}

int64_t RegExMatch::get_start(const Variant name) const {
	return ___godot_icall_int_Variant(___mb.mb_get_start, (const Object *) this, name);
}

String RegExMatch::get_string(const Variant name) const {
	return ___godot_icall_String_Variant(___mb.mb_get_string, (const Object *) this, name);
}

Array RegExMatch::get_strings() const {
	return ___godot_icall_Array(___mb.mb_get_strings, (const Object *) this);
}

String RegExMatch::get_subject() const {
	return ___godot_icall_String(___mb.mb_get_subject, (const Object *) this);
}

}