#ifndef GODOT_CPP_TEXTURE_HPP
#define GODOT_CPP_TEXTURE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class Texture;
class Image;

class Texture : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_draw;
		godot_method_bind *mb_draw_rect;
		godot_method_bind *mb_draw_rect_region;
		godot_method_bind *mb_get_data;
		godot_method_bind *mb_get_flags;
		godot_method_bind *mb_get_height;
		godot_method_bind *mb_get_size;
		godot_method_bind *mb_get_width;
		godot_method_bind *mb_has_alpha;
		godot_method_bind *mb_set_flags;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Texture"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Flags {
		FLAG_MIPMAPS = 1,
		FLAG_REPEAT = 2,
		FLAG_FILTER = 4,
		FLAGS_DEFAULT = 7,
		FLAG_ANISOTROPIC_FILTER = 8,
		FLAG_CONVERT_TO_LINEAR = 16,
		FLAG_MIRRORED_REPEAT = 32,
		FLAG_VIDEO_SURFACE = 2048,
	};

	// constants

	// methods
	void draw(const RID canvas_item, const Vector2 position, const Color modulate = Color(1,1,1,1), const bool transpose = false, const Ref<Texture> normal_map = nullptr) const;
	void draw_rect(const RID canvas_item, const Rect2 rect, const bool tile, const Color modulate = Color(1,1,1,1), const bool transpose = false, const Ref<Texture> normal_map = nullptr) const;
	void draw_rect_region(const RID canvas_item, const Rect2 rect, const Rect2 src_rect, const Color modulate = Color(1,1,1,1), const bool transpose = false, const Ref<Texture> normal_map = nullptr, const bool clip_uv = true) const;
	Ref<Image> get_data() const;
	int64_t get_flags() const;
	int64_t get_height() const;
	Vector2 get_size() const;
	int64_t get_width() const;
	bool has_alpha() const;
	void set_flags(const int64_t flags);

};

}

#endif