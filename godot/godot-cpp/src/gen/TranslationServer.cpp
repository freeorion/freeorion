#include "TranslationServer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Translation.hpp"


namespace godot {


TranslationServer *TranslationServer::_singleton = NULL;


TranslationServer::TranslationServer() {
	_owner = godot::api->godot_global_get_singleton((char *) "TranslationServer");
}


TranslationServer::___method_bindings TranslationServer::___mb = {};

void TranslationServer::___init_method_bindings() {
	___mb.mb_add_translation = godot::api->godot_method_bind_get_method("TranslationServer", "add_translation");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("TranslationServer", "clear");
	___mb.mb_get_loaded_locales = godot::api->godot_method_bind_get_method("TranslationServer", "get_loaded_locales");
	___mb.mb_get_locale = godot::api->godot_method_bind_get_method("TranslationServer", "get_locale");
	___mb.mb_get_locale_name = godot::api->godot_method_bind_get_method("TranslationServer", "get_locale_name");
	___mb.mb_remove_translation = godot::api->godot_method_bind_get_method("TranslationServer", "remove_translation");
	___mb.mb_set_locale = godot::api->godot_method_bind_get_method("TranslationServer", "set_locale");
	___mb.mb_translate = godot::api->godot_method_bind_get_method("TranslationServer", "translate");
}

void TranslationServer::add_translation(const Ref<Translation> translation) {
	___godot_icall_void_Object(___mb.mb_add_translation, (const Object *) this, translation.ptr());
}

void TranslationServer::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

Array TranslationServer::get_loaded_locales() const {
	return ___godot_icall_Array(___mb.mb_get_loaded_locales, (const Object *) this);
}

String TranslationServer::get_locale() const {
	return ___godot_icall_String(___mb.mb_get_locale, (const Object *) this);
}

String TranslationServer::get_locale_name(const String locale) const {
	return ___godot_icall_String_String(___mb.mb_get_locale_name, (const Object *) this, locale);
}

void TranslationServer::remove_translation(const Ref<Translation> translation) {
	___godot_icall_void_Object(___mb.mb_remove_translation, (const Object *) this, translation.ptr());
}

void TranslationServer::set_locale(const String locale) {
	___godot_icall_void_String(___mb.mb_set_locale, (const Object *) this, locale);
}

String TranslationServer::translate(const String message) const {
	return ___godot_icall_String_String(___mb.mb_translate, (const Object *) this, message);
}

}