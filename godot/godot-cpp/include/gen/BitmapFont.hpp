#ifndef GODOT_CPP_BITMAPFONT_HPP
#define GODOT_CPP_BITMAPFONT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Font.hpp"
namespace godot {

class Texture;
class BitmapFont;

class BitmapFont : public Font {
	struct ___method_bindings {
		godot_method_bind *mb__get_chars;
		godot_method_bind *mb__get_kernings;
		godot_method_bind *mb__get_textures;
		godot_method_bind *mb__set_chars;
		godot_method_bind *mb__set_kernings;
		godot_method_bind *mb__set_textures;
		godot_method_bind *mb_add_char;
		godot_method_bind *mb_add_kerning_pair;
		godot_method_bind *mb_add_texture;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_create_from_fnt;
		godot_method_bind *mb_get_char_size;
		godot_method_bind *mb_get_fallback;
		godot_method_bind *mb_get_kerning_pair;
		godot_method_bind *mb_get_texture;
		godot_method_bind *mb_get_texture_count;
		godot_method_bind *mb_set_ascent;
		godot_method_bind *mb_set_distance_field_hint;
		godot_method_bind *mb_set_fallback;
		godot_method_bind *mb_set_height;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "BitmapFont"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static BitmapFont *_new();

	// methods
	PoolIntArray _get_chars() const;
	PoolIntArray _get_kernings() const;
	Array _get_textures() const;
	void _set_chars(const PoolIntArray arg0);
	void _set_kernings(const PoolIntArray arg0);
	void _set_textures(const Array arg0);
	void add_char(const int64_t character, const int64_t texture, const Rect2 rect, const Vector2 align = Vector2(0, 0), const real_t advance = -1);
	void add_kerning_pair(const int64_t char_a, const int64_t char_b, const int64_t kerning);
	void add_texture(const Ref<Texture> texture);
	void clear();
	Error create_from_fnt(const String path);
	Vector2 get_char_size(const int64_t _char, const int64_t next = 0) const;
	Ref<BitmapFont> get_fallback() const;
	int64_t get_kerning_pair(const int64_t char_a, const int64_t char_b) const;
	Ref<Texture> get_texture(const int64_t idx) const;
	int64_t get_texture_count() const;
	void set_ascent(const real_t px);
	void set_distance_field_hint(const bool enable);
	void set_fallback(const Ref<BitmapFont> fallback);
	void set_height(const real_t px);

};

}

#endif