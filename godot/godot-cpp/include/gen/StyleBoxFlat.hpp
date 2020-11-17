#ifndef GODOT_CPP_STYLEBOXFLAT_HPP
#define GODOT_CPP_STYLEBOXFLAT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "StyleBox.hpp"
namespace godot {


class StyleBoxFlat : public StyleBox {
	struct ___method_bindings {
		godot_method_bind *mb_get_aa_size;
		godot_method_bind *mb_get_bg_color;
		godot_method_bind *mb_get_border_blend;
		godot_method_bind *mb_get_border_color;
		godot_method_bind *mb_get_border_width;
		godot_method_bind *mb_get_border_width_min;
		godot_method_bind *mb_get_corner_detail;
		godot_method_bind *mb_get_corner_radius;
		godot_method_bind *mb_get_expand_margin;
		godot_method_bind *mb_get_shadow_color;
		godot_method_bind *mb_get_shadow_offset;
		godot_method_bind *mb_get_shadow_size;
		godot_method_bind *mb_is_anti_aliased;
		godot_method_bind *mb_is_draw_center_enabled;
		godot_method_bind *mb_set_aa_size;
		godot_method_bind *mb_set_anti_aliased;
		godot_method_bind *mb_set_bg_color;
		godot_method_bind *mb_set_border_blend;
		godot_method_bind *mb_set_border_color;
		godot_method_bind *mb_set_border_width;
		godot_method_bind *mb_set_border_width_all;
		godot_method_bind *mb_set_corner_detail;
		godot_method_bind *mb_set_corner_radius;
		godot_method_bind *mb_set_corner_radius_all;
		godot_method_bind *mb_set_corner_radius_individual;
		godot_method_bind *mb_set_draw_center;
		godot_method_bind *mb_set_expand_margin;
		godot_method_bind *mb_set_expand_margin_all;
		godot_method_bind *mb_set_expand_margin_individual;
		godot_method_bind *mb_set_shadow_color;
		godot_method_bind *mb_set_shadow_offset;
		godot_method_bind *mb_set_shadow_size;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "StyleBoxFlat"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static StyleBoxFlat *_new();

	// methods
	int64_t get_aa_size() const;
	Color get_bg_color() const;
	bool get_border_blend() const;
	Color get_border_color() const;
	int64_t get_border_width(const int64_t margin) const;
	int64_t get_border_width_min() const;
	int64_t get_corner_detail() const;
	int64_t get_corner_radius(const int64_t corner) const;
	real_t get_expand_margin(const int64_t margin) const;
	Color get_shadow_color() const;
	Vector2 get_shadow_offset() const;
	int64_t get_shadow_size() const;
	bool is_anti_aliased() const;
	bool is_draw_center_enabled() const;
	void set_aa_size(const int64_t size);
	void set_anti_aliased(const bool anti_aliased);
	void set_bg_color(const Color color);
	void set_border_blend(const bool blend);
	void set_border_color(const Color color);
	void set_border_width(const int64_t margin, const int64_t width);
	void set_border_width_all(const int64_t width);
	void set_corner_detail(const int64_t detail);
	void set_corner_radius(const int64_t corner, const int64_t radius);
	void set_corner_radius_all(const int64_t radius);
	void set_corner_radius_individual(const int64_t radius_top_left, const int64_t radius_top_right, const int64_t radius_bottom_right, const int64_t radius_bottom_left);
	void set_draw_center(const bool draw_center);
	void set_expand_margin(const int64_t margin, const real_t size);
	void set_expand_margin_all(const real_t size);
	void set_expand_margin_individual(const real_t size_left, const real_t size_top, const real_t size_right, const real_t size_bottom);
	void set_shadow_color(const Color color);
	void set_shadow_offset(const Vector2 offset);
	void set_shadow_size(const int64_t size);

};

}

#endif