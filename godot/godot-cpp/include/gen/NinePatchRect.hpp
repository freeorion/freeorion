#ifndef GODOT_CPP_NINEPATCHRECT_HPP
#define GODOT_CPP_NINEPATCHRECT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "NinePatchRect.hpp"

#include "Control.hpp"
namespace godot {

class Texture;

class NinePatchRect : public Control {
	struct ___method_bindings {
		godot_method_bind *mb_get_h_axis_stretch_mode;
		godot_method_bind *mb_get_patch_margin;
		godot_method_bind *mb_get_region_rect;
		godot_method_bind *mb_get_texture;
		godot_method_bind *mb_get_v_axis_stretch_mode;
		godot_method_bind *mb_is_draw_center_enabled;
		godot_method_bind *mb_set_draw_center;
		godot_method_bind *mb_set_h_axis_stretch_mode;
		godot_method_bind *mb_set_patch_margin;
		godot_method_bind *mb_set_region_rect;
		godot_method_bind *mb_set_texture;
		godot_method_bind *mb_set_v_axis_stretch_mode;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "NinePatchRect"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum AxisStretchMode {
		AXIS_STRETCH_MODE_STRETCH = 0,
		AXIS_STRETCH_MODE_TILE = 1,
		AXIS_STRETCH_MODE_TILE_FIT = 2,
	};

	// constants


	static NinePatchRect *_new();

	// methods
	NinePatchRect::AxisStretchMode get_h_axis_stretch_mode() const;
	int64_t get_patch_margin(const int64_t margin) const;
	Rect2 get_region_rect() const;
	Ref<Texture> get_texture() const;
	NinePatchRect::AxisStretchMode get_v_axis_stretch_mode() const;
	bool is_draw_center_enabled() const;
	void set_draw_center(const bool draw_center);
	void set_h_axis_stretch_mode(const int64_t mode);
	void set_patch_margin(const int64_t margin, const int64_t value);
	void set_region_rect(const Rect2 rect);
	void set_texture(const Ref<Texture> texture);
	void set_v_axis_stretch_mode(const int64_t mode);

};

}

#endif