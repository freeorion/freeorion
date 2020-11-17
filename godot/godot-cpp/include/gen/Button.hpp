#ifndef GODOT_CPP_BUTTON_HPP
#define GODOT_CPP_BUTTON_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Button.hpp"

#include "BaseButton.hpp"
namespace godot {

class Texture;

class Button : public BaseButton {
	struct ___method_bindings {
		godot_method_bind *mb_get_button_icon;
		godot_method_bind *mb_get_clip_text;
		godot_method_bind *mb_get_text;
		godot_method_bind *mb_get_text_align;
		godot_method_bind *mb_is_expand_icon;
		godot_method_bind *mb_is_flat;
		godot_method_bind *mb_set_button_icon;
		godot_method_bind *mb_set_clip_text;
		godot_method_bind *mb_set_expand_icon;
		godot_method_bind *mb_set_flat;
		godot_method_bind *mb_set_text;
		godot_method_bind *mb_set_text_align;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Button"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum TextAlign {
		ALIGN_LEFT = 0,
		ALIGN_CENTER = 1,
		ALIGN_RIGHT = 2,
	};

	// constants


	static Button *_new();

	// methods
	Ref<Texture> get_button_icon() const;
	bool get_clip_text() const;
	String get_text() const;
	Button::TextAlign get_text_align() const;
	bool is_expand_icon() const;
	bool is_flat() const;
	void set_button_icon(const Ref<Texture> texture);
	void set_clip_text(const bool enabled);
	void set_expand_icon(const bool arg0);
	void set_flat(const bool enabled);
	void set_text(const String text);
	void set_text_align(const int64_t align);

};

}

#endif