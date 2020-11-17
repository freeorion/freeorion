#ifndef GODOT_CPP_SPRITE_HPP
#define GODOT_CPP_SPRITE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node2D.hpp"
namespace godot {

class Texture;

class Sprite : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb__texture_changed;
		godot_method_bind *mb_get_frame;
		godot_method_bind *mb_get_frame_coords;
		godot_method_bind *mb_get_hframes;
		godot_method_bind *mb_get_normal_map;
		godot_method_bind *mb_get_offset;
		godot_method_bind *mb_get_rect;
		godot_method_bind *mb_get_region_rect;
		godot_method_bind *mb_get_texture;
		godot_method_bind *mb_get_vframes;
		godot_method_bind *mb_is_centered;
		godot_method_bind *mb_is_flipped_h;
		godot_method_bind *mb_is_flipped_v;
		godot_method_bind *mb_is_pixel_opaque;
		godot_method_bind *mb_is_region;
		godot_method_bind *mb_is_region_filter_clip_enabled;
		godot_method_bind *mb_set_centered;
		godot_method_bind *mb_set_flip_h;
		godot_method_bind *mb_set_flip_v;
		godot_method_bind *mb_set_frame;
		godot_method_bind *mb_set_frame_coords;
		godot_method_bind *mb_set_hframes;
		godot_method_bind *mb_set_normal_map;
		godot_method_bind *mb_set_offset;
		godot_method_bind *mb_set_region;
		godot_method_bind *mb_set_region_filter_clip;
		godot_method_bind *mb_set_region_rect;
		godot_method_bind *mb_set_texture;
		godot_method_bind *mb_set_vframes;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Sprite"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Sprite *_new();

	// methods
	void _texture_changed();
	int64_t get_frame() const;
	Vector2 get_frame_coords() const;
	int64_t get_hframes() const;
	Ref<Texture> get_normal_map() const;
	Vector2 get_offset() const;
	Rect2 get_rect() const;
	Rect2 get_region_rect() const;
	Ref<Texture> get_texture() const;
	int64_t get_vframes() const;
	bool is_centered() const;
	bool is_flipped_h() const;
	bool is_flipped_v() const;
	bool is_pixel_opaque(const Vector2 pos) const;
	bool is_region() const;
	bool is_region_filter_clip_enabled() const;
	void set_centered(const bool centered);
	void set_flip_h(const bool flip_h);
	void set_flip_v(const bool flip_v);
	void set_frame(const int64_t frame);
	void set_frame_coords(const Vector2 coords);
	void set_hframes(const int64_t hframes);
	void set_normal_map(const Ref<Texture> normal_map);
	void set_offset(const Vector2 offset);
	void set_region(const bool enabled);
	void set_region_filter_clip(const bool enabled);
	void set_region_rect(const Rect2 rect);
	void set_texture(const Ref<Texture> texture);
	void set_vframes(const int64_t vframes);

};

}

#endif