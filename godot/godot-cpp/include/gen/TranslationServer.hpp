#ifndef GODOT_CPP_TRANSLATIONSERVER_HPP
#define GODOT_CPP_TRANSLATIONSERVER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {

class Translation;

class TranslationServer : public Object {
	static TranslationServer *_singleton;

	TranslationServer();

	struct ___method_bindings {
		godot_method_bind *mb_add_translation;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_get_loaded_locales;
		godot_method_bind *mb_get_locale;
		godot_method_bind *mb_get_locale_name;
		godot_method_bind *mb_remove_translation;
		godot_method_bind *mb_set_locale;
		godot_method_bind *mb_translate;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline TranslationServer *get_singleton()
	{
		if (!TranslationServer::_singleton) {
			TranslationServer::_singleton = new TranslationServer;
		}
		return TranslationServer::_singleton;
	}

	static inline const char *___get_class_name() { return (const char *) "TranslationServer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void add_translation(const Ref<Translation> translation);
	void clear();
	Array get_loaded_locales() const;
	String get_locale() const;
	String get_locale_name(const String locale) const;
	void remove_translation(const Ref<Translation> translation);
	void set_locale(const String locale);
	String translate(const String message) const;

};

}

#endif