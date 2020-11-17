#ifndef GODOT_CPP_FONT_HPP
#define GODOT_CPP_FONT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {


class Font : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_draw;
		godot_method_bind *mb_draw_char;
		godot_method_bind *mb_get_ascent;
		godot_method_bind *mb_get_descent;
		godot_method_bind *mb_get_height;
		godot_method_bind *mb_get_string_size;
		godot_method_bind *mb_get_wordwrap_string_size;
		godot_method_bind *mb_has_outline;
		godot_method_bind *mb_is_distance_field_hint;
		godot_method_bind *mb_update_changes;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Font"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void draw(const RID canvas_item, const Vector2 position, const String string, const Color modulate = Color(1,1,1,1), const int64_t clip_w = -1, const Color outline_modulate = Color(1,1,1,1)) const;
	real_t draw_char(const RID canvas_item, const Vector2 position, const int64_t _char, const int64_t next = -1, const Color modulate = Color(1,1,1,1), const bool outline = false) const;
	real_t get_ascent() const;
	real_t get_descent() const;
	real_t get_height() const;
	Vector2 get_string_size(const String string) const;
	Vector2 get_wordwrap_string_size(const String string, const real_t width) const;
	bool has_outline() const;
	bool is_distance_field_hint() const;
	void update_changes();

};

}

#endif