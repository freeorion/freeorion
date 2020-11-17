#ifndef GODOT_CPP_STYLEBOXTEXTURE_HPP
#define GODOT_CPP_STYLEBOXTEXTURE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "StyleBoxTexture.hpp"

#include "StyleBox.hpp"
namespace godot {

class Texture;

class StyleBoxTexture : public StyleBox {
	struct ___method_bindings {
		godot_method_bind *mb_get_expand_margin_size;
		godot_method_bind *mb_get_h_axis_stretch_mode;
		godot_method_bind *mb_get_margin_size;
		godot_method_bind *mb_get_modulate;
		godot_method_bind *mb_get_normal_map;
		godot_method_bind *mb_get_region_rect;
		godot_method_bind *mb_get_texture;
		godot_method_bind *mb_get_v_axis_stretch_mode;
		godot_method_bind *mb_is_draw_center_enabled;
		godot_method_bind *mb_set_draw_center;
		godot_method_bind *mb_set_expand_margin_all;
		godot_method_bind *mb_set_expand_margin_individual;
		godot_method_bind *mb_set_expand_margin_size;
		godot_method_bind *mb_set_h_axis_stretch_mode;
		godot_method_bind *mb_set_margin_size;
		godot_method_bind *mb_set_modulate;
		godot_method_bind *mb_set_normal_map;
		godot_method_bind *mb_set_region_rect;
		godot_method_bind *mb_set_texture;
		godot_method_bind *mb_set_v_axis_stretch_mode;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "StyleBoxTexture"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum AxisStretchMode {
		AXIS_STRETCH_MODE_STRETCH = 0,
		AXIS_STRETCH_MODE_TILE = 1,
		AXIS_STRETCH_MODE_TILE_FIT = 2,
	};

	// constants


	static StyleBoxTexture *_new();

	// methods
	real_t get_expand_margin_size(const int64_t margin) const;
	StyleBoxTexture::AxisStretchMode get_h_axis_stretch_mode() const;
	real_t get_margin_size(const int64_t margin) const;
	Color get_modulate() const;
	Ref<Texture> get_normal_map() const;
	Rect2 get_region_rect() const;
	Ref<Texture> get_texture() const;
	StyleBoxTexture::AxisStretchMode get_v_axis_stretch_mode() const;
	bool is_draw_center_enabled() const;
	void set_draw_center(const bool enable);
	void set_expand_margin_all(const real_t size);
	void set_expand_margin_individual(const real_t size_left, const real_t size_top, const real_t size_right, const real_t size_bottom);
	void set_expand_margin_size(const int64_t margin, const real_t size);
	void set_h_axis_stretch_mode(const int64_t mode);
	void set_margin_size(const int64_t margin, const real_t size);
	void set_modulate(const Color color);
	void set_normal_map(const Ref<Texture> normal_map);
	void set_region_rect(const Rect2 region);
	void set_texture(const Ref<Texture> texture);
	void set_v_axis_stretch_mode(const int64_t mode);

};

}

#endif