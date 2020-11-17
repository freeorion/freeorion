#ifndef GODOT_CPP_LABEL_HPP
#define GODOT_CPP_LABEL_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Label.hpp"

#include "Control.hpp"
namespace godot {


class Label : public Control {
	struct ___method_bindings {
		godot_method_bind *mb_get_align;
		godot_method_bind *mb_get_line_count;
		godot_method_bind *mb_get_line_height;
		godot_method_bind *mb_get_lines_skipped;
		godot_method_bind *mb_get_max_lines_visible;
		godot_method_bind *mb_get_percent_visible;
		godot_method_bind *mb_get_text;
		godot_method_bind *mb_get_total_character_count;
		godot_method_bind *mb_get_valign;
		godot_method_bind *mb_get_visible_characters;
		godot_method_bind *mb_get_visible_line_count;
		godot_method_bind *mb_has_autowrap;
		godot_method_bind *mb_is_clipping_text;
		godot_method_bind *mb_is_uppercase;
		godot_method_bind *mb_set_align;
		godot_method_bind *mb_set_autowrap;
		godot_method_bind *mb_set_clip_text;
		godot_method_bind *mb_set_lines_skipped;
		godot_method_bind *mb_set_max_lines_visible;
		godot_method_bind *mb_set_percent_visible;
		godot_method_bind *mb_set_text;
		godot_method_bind *mb_set_uppercase;
		godot_method_bind *mb_set_valign;
		godot_method_bind *mb_set_visible_characters;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Label"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Align {
		ALIGN_LEFT = 0,
		ALIGN_CENTER = 1,
		ALIGN_RIGHT = 2,
		ALIGN_FILL = 3,
	};
	enum VAlign {
		VALIGN_TOP = 0,
		VALIGN_CENTER = 1,
		VALIGN_BOTTOM = 2,
		VALIGN_FILL = 3,
	};

	// constants


	static Label *_new();

	// methods
	Label::Align get_align() const;
	int64_t get_line_count() const;
	int64_t get_line_height() const;
	int64_t get_lines_skipped() const;
	int64_t get_max_lines_visible() const;
	real_t get_percent_visible() const;
	String get_text() const;
	int64_t get_total_character_count() const;
	Label::VAlign get_valign() const;
	int64_t get_visible_characters() const;
	int64_t get_visible_line_count() const;
	bool has_autowrap() const;
	bool is_clipping_text() const;
	bool is_uppercase() const;
	void set_align(const int64_t align);
	void set_autowrap(const bool enable);
	void set_clip_text(const bool enable);
	void set_lines_skipped(const int64_t lines_skipped);
	void set_max_lines_visible(const int64_t lines_visible);
	void set_percent_visible(const real_t percent_visible);
	void set_text(const String text);
	void set_uppercase(const bool enable);
	void set_valign(const int64_t valign);
	void set_visible_characters(const int64_t amount);

};

}

#endif