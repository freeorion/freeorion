#ifndef GODOT_CPP_CHARFXTRANSFORM_HPP
#define GODOT_CPP_CHARFXTRANSFORM_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {


class CharFXTransform : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_get_absolute_index;
		godot_method_bind *mb_get_character;
		godot_method_bind *mb_get_color;
		godot_method_bind *mb_get_elapsed_time;
		godot_method_bind *mb_get_environment;
		godot_method_bind *mb_get_offset;
		godot_method_bind *mb_get_relative_index;
		godot_method_bind *mb_is_visible;
		godot_method_bind *mb_set_absolute_index;
		godot_method_bind *mb_set_character;
		godot_method_bind *mb_set_color;
		godot_method_bind *mb_set_elapsed_time;
		godot_method_bind *mb_set_environment;
		godot_method_bind *mb_set_offset;
		godot_method_bind *mb_set_relative_index;
		godot_method_bind *mb_set_visibility;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CharFXTransform"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static CharFXTransform *_new();

	// methods
	int64_t get_absolute_index();
	int64_t get_character();
	Color get_color();
	real_t get_elapsed_time();
	Dictionary get_environment();
	Vector2 get_offset();
	int64_t get_relative_index();
	bool is_visible();
	void set_absolute_index(const int64_t index);
	void set_character(const int64_t character);
	void set_color(const Color color);
	void set_elapsed_time(const real_t time);
	void set_environment(const Dictionary environment);
	void set_offset(const Vector2 offset);
	void set_relative_index(const int64_t index);
	void set_visibility(const bool visibility);

};

}

#endif