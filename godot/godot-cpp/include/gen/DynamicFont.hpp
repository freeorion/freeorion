#ifndef GODOT_CPP_DYNAMICFONT_HPP
#define GODOT_CPP_DYNAMICFONT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Font.hpp"
namespace godot {

class DynamicFontData;

class DynamicFont : public Font {
	struct ___method_bindings {
		godot_method_bind *mb_add_fallback;
		godot_method_bind *mb_get_fallback;
		godot_method_bind *mb_get_fallback_count;
		godot_method_bind *mb_get_font_data;
		godot_method_bind *mb_get_outline_color;
		godot_method_bind *mb_get_outline_size;
		godot_method_bind *mb_get_size;
		godot_method_bind *mb_get_spacing;
		godot_method_bind *mb_get_use_filter;
		godot_method_bind *mb_get_use_mipmaps;
		godot_method_bind *mb_remove_fallback;
		godot_method_bind *mb_set_fallback;
		godot_method_bind *mb_set_font_data;
		godot_method_bind *mb_set_outline_color;
		godot_method_bind *mb_set_outline_size;
		godot_method_bind *mb_set_size;
		godot_method_bind *mb_set_spacing;
		godot_method_bind *mb_set_use_filter;
		godot_method_bind *mb_set_use_mipmaps;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "DynamicFont"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum SpacingType {
		SPACING_TOP = 0,
		SPACING_BOTTOM = 1,
		SPACING_CHAR = 2,
		SPACING_SPACE = 3,
	};

	// constants


	static DynamicFont *_new();

	// methods
	void add_fallback(const Ref<DynamicFontData> data);
	Ref<DynamicFontData> get_fallback(const int64_t idx) const;
	int64_t get_fallback_count() const;
	Ref<DynamicFontData> get_font_data() const;
	Color get_outline_color() const;
	int64_t get_outline_size() const;
	int64_t get_size() const;
	int64_t get_spacing(const int64_t type) const;
	bool get_use_filter() const;
	bool get_use_mipmaps() const;
	void remove_fallback(const int64_t idx);
	void set_fallback(const int64_t idx, const Ref<DynamicFontData> data);
	void set_font_data(const Ref<DynamicFontData> data);
	void set_outline_color(const Color color);
	void set_outline_size(const int64_t size);
	void set_size(const int64_t data);
	void set_spacing(const int64_t type, const int64_t value);
	void set_use_filter(const bool enable);
	void set_use_mipmaps(const bool enable);

};

}

#endif