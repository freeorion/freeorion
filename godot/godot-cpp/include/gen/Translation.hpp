#ifndef GODOT_CPP_TRANSLATION_HPP
#define GODOT_CPP_TRANSLATION_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {


class Translation : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb__get_messages;
		godot_method_bind *mb__set_messages;
		godot_method_bind *mb_add_message;
		godot_method_bind *mb_erase_message;
		godot_method_bind *mb_get_locale;
		godot_method_bind *mb_get_message;
		godot_method_bind *mb_get_message_count;
		godot_method_bind *mb_get_message_list;
		godot_method_bind *mb_set_locale;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Translation"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Translation *_new();

	// methods
	PoolStringArray _get_messages() const;
	void _set_messages(const PoolStringArray arg0);
	void add_message(const String src_message, const String xlated_message);
	void erase_message(const String src_message);
	String get_locale() const;
	String get_message(const String src_message) const;
	int64_t get_message_count() const;
	PoolStringArray get_message_list() const;
	void set_locale(const String locale);

};

}

#endif