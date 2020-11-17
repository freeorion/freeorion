#ifndef GODOT_CPP_THEME_HPP
#define GODOT_CPP_THEME_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class Theme;
class Font;
class Texture;
class StyleBox;

class Theme : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb__emit_theme_changed;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_clear_color;
		godot_method_bind *mb_clear_constant;
		godot_method_bind *mb_clear_font;
		godot_method_bind *mb_clear_icon;
		godot_method_bind *mb_clear_stylebox;
		godot_method_bind *mb_copy_default_theme;
		godot_method_bind *mb_copy_theme;
		godot_method_bind *mb_get_color;
		godot_method_bind *mb_get_color_list;
		godot_method_bind *mb_get_constant;
		godot_method_bind *mb_get_constant_list;
		godot_method_bind *mb_get_default_font;
		godot_method_bind *mb_get_font;
		godot_method_bind *mb_get_font_list;
		godot_method_bind *mb_get_icon;
		godot_method_bind *mb_get_icon_list;
		godot_method_bind *mb_get_stylebox;
		godot_method_bind *mb_get_stylebox_list;
		godot_method_bind *mb_get_stylebox_types;
		godot_method_bind *mb_get_type_list;
		godot_method_bind *mb_has_color;
		godot_method_bind *mb_has_constant;
		godot_method_bind *mb_has_font;
		godot_method_bind *mb_has_icon;
		godot_method_bind *mb_has_stylebox;
		godot_method_bind *mb_set_color;
		godot_method_bind *mb_set_constant;
		godot_method_bind *mb_set_default_font;
		godot_method_bind *mb_set_font;
		godot_method_bind *mb_set_icon;
		godot_method_bind *mb_set_stylebox;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Theme"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Theme *_new();

	// methods
	void _emit_theme_changed();
	void clear();
	void clear_color(const String name, const String type);
	void clear_constant(const String name, const String type);
	void clear_font(const String name, const String type);
	void clear_icon(const String name, const String type);
	void clear_stylebox(const String name, const String type);
	void copy_default_theme();
	void copy_theme(const Ref<Theme> other);
	Color get_color(const String name, const String type) const;
	PoolStringArray get_color_list(const String type) const;
	int64_t get_constant(const String name, const String type) const;
	PoolStringArray get_constant_list(const String type) const;
	Ref<Font> get_default_font() const;
	Ref<Font> get_font(const String name, const String type) const;
	PoolStringArray get_font_list(const String type) const;
	Ref<Texture> get_icon(const String name, const String type) const;
	PoolStringArray get_icon_list(const String type) const;
	Ref<StyleBox> get_stylebox(const String name, const String type) const;
	PoolStringArray get_stylebox_list(const String type) const;
	PoolStringArray get_stylebox_types() const;
	PoolStringArray get_type_list(const String type) const;
	bool has_color(const String name, const String type) const;
	bool has_constant(const String name, const String type) const;
	bool has_font(const String name, const String type) const;
	bool has_icon(const String name, const String type) const;
	bool has_stylebox(const String name, const String type) const;
	void set_color(const String name, const String type, const Color color);
	void set_constant(const String name, const String type, const int64_t constant);
	void set_default_font(const Ref<Font> font);
	void set_font(const String name, const String type, const Ref<Font> font);
	void set_icon(const String name, const String type, const Ref<Texture> texture);
	void set_stylebox(const String name, const String type, const Ref<StyleBox> texture);

};

}

#endif