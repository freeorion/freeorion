#ifndef GODOT_CPP_SPRITEBASE3D_HPP
#define GODOT_CPP_SPRITEBASE3D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "SpriteBase3D.hpp"
#include "Vector3.hpp"
#include "SpatialMaterial.hpp"

#include "GeometryInstance.hpp"
namespace godot {

class TriangleMesh;

class SpriteBase3D : public GeometryInstance {
	struct ___method_bindings {
		godot_method_bind *mb__im_update;
		godot_method_bind *mb__queue_update;
		godot_method_bind *mb_generate_triangle_mesh;
		godot_method_bind *mb_get_alpha_cut_mode;
		godot_method_bind *mb_get_axis;
		godot_method_bind *mb_get_billboard_mode;
		godot_method_bind *mb_get_draw_flag;
		godot_method_bind *mb_get_item_rect;
		godot_method_bind *mb_get_modulate;
		godot_method_bind *mb_get_offset;
		godot_method_bind *mb_get_opacity;
		godot_method_bind *mb_get_pixel_size;
		godot_method_bind *mb_is_centered;
		godot_method_bind *mb_is_flipped_h;
		godot_method_bind *mb_is_flipped_v;
		godot_method_bind *mb_set_alpha_cut_mode;
		godot_method_bind *mb_set_axis;
		godot_method_bind *mb_set_billboard_mode;
		godot_method_bind *mb_set_centered;
		godot_method_bind *mb_set_draw_flag;
		godot_method_bind *mb_set_flip_h;
		godot_method_bind *mb_set_flip_v;
		godot_method_bind *mb_set_modulate;
		godot_method_bind *mb_set_offset;
		godot_method_bind *mb_set_opacity;
		godot_method_bind *mb_set_pixel_size;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "SpriteBase3D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum DrawFlags {
		FLAG_TRANSPARENT = 0,
		FLAG_SHADED = 1,
		FLAG_DOUBLE_SIDED = 2,
		FLAG_MAX = 3,
	};
	enum AlphaCutMode {
		ALPHA_CUT_DISABLED = 0,
		ALPHA_CUT_DISCARD = 1,
		ALPHA_CUT_OPAQUE_PREPASS = 2,
	};

	// constants

	// methods
	void _im_update();
	void _queue_update();
	Ref<TriangleMesh> generate_triangle_mesh() const;
	SpriteBase3D::AlphaCutMode get_alpha_cut_mode() const;
	Vector3::Axis get_axis() const;
	SpatialMaterial::BillboardMode get_billboard_mode() const;
	bool get_draw_flag(const int64_t flag) const;
	Rect2 get_item_rect() const;
	Color get_modulate() const;
	Vector2 get_offset() const;
	real_t get_opacity() const;
	real_t get_pixel_size() const;
	bool is_centered() const;
	bool is_flipped_h() const;
	bool is_flipped_v() const;
	void set_alpha_cut_mode(const int64_t mode);
	void set_axis(const int64_t axis);
	void set_billboard_mode(const int64_t mode);
	void set_centered(const bool centered);
	void set_draw_flag(const int64_t flag, const bool enabled);
	void set_flip_h(const bool flip_h);
	void set_flip_v(const bool flip_v);
	void set_modulate(const Color modulate);
	void set_offset(const Vector2 offset);
	void set_opacity(const real_t opacity);
	void set_pixel_size(const real_t pixel_size);

};

}

#endif