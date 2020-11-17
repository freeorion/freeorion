#ifndef GODOT_CPP_IMAGE_HPP
#define GODOT_CPP_IMAGE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Image.hpp"

#include "Resource.hpp"
namespace godot {

class Image;

class Image : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb__get_data;
		godot_method_bind *mb__set_data;
		godot_method_bind *mb_blend_rect;
		godot_method_bind *mb_blend_rect_mask;
		godot_method_bind *mb_blit_rect;
		godot_method_bind *mb_blit_rect_mask;
		godot_method_bind *mb_bumpmap_to_normalmap;
		godot_method_bind *mb_clear_mipmaps;
		godot_method_bind *mb_compress;
		godot_method_bind *mb_convert;
		godot_method_bind *mb_copy_from;
		godot_method_bind *mb_create;
		godot_method_bind *mb_create_from_data;
		godot_method_bind *mb_crop;
		godot_method_bind *mb_decompress;
		godot_method_bind *mb_detect_alpha;
		godot_method_bind *mb_expand_x2_hq2x;
		godot_method_bind *mb_fill;
		godot_method_bind *mb_fix_alpha_edges;
		godot_method_bind *mb_flip_x;
		godot_method_bind *mb_flip_y;
		godot_method_bind *mb_generate_mipmaps;
		godot_method_bind *mb_get_data;
		godot_method_bind *mb_get_format;
		godot_method_bind *mb_get_height;
		godot_method_bind *mb_get_mipmap_offset;
		godot_method_bind *mb_get_pixel;
		godot_method_bind *mb_get_pixelv;
		godot_method_bind *mb_get_rect;
		godot_method_bind *mb_get_size;
		godot_method_bind *mb_get_used_rect;
		godot_method_bind *mb_get_width;
		godot_method_bind *mb_has_mipmaps;
		godot_method_bind *mb_is_compressed;
		godot_method_bind *mb_is_empty;
		godot_method_bind *mb_is_invisible;
		godot_method_bind *mb_load;
		godot_method_bind *mb_load_jpg_from_buffer;
		godot_method_bind *mb_load_png_from_buffer;
		godot_method_bind *mb_load_webp_from_buffer;
		godot_method_bind *mb_lock;
		godot_method_bind *mb_normalmap_to_xy;
		godot_method_bind *mb_premultiply_alpha;
		godot_method_bind *mb_resize;
		godot_method_bind *mb_resize_to_po2;
		godot_method_bind *mb_rgbe_to_srgb;
		godot_method_bind *mb_save_exr;
		godot_method_bind *mb_save_png;
		godot_method_bind *mb_set_pixel;
		godot_method_bind *mb_set_pixelv;
		godot_method_bind *mb_shrink_x2;
		godot_method_bind *mb_srgb_to_linear;
		godot_method_bind *mb_unlock;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Image"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum AlphaMode {
		ALPHA_NONE = 0,
		ALPHA_BIT = 1,
		ALPHA_BLEND = 2,
	};
	enum CompressSource {
		COMPRESS_SOURCE_GENERIC = 0,
		COMPRESS_SOURCE_SRGB = 1,
		COMPRESS_SOURCE_NORMAL = 2,
	};
	enum Interpolation {
		INTERPOLATE_NEAREST = 0,
		INTERPOLATE_BILINEAR = 1,
		INTERPOLATE_CUBIC = 2,
		INTERPOLATE_TRILINEAR = 3,
		INTERPOLATE_LANCZOS = 4,
	};
	enum CompressMode {
		COMPRESS_S3TC = 0,
		COMPRESS_PVRTC2 = 1,
		COMPRESS_PVRTC4 = 2,
		COMPRESS_ETC = 3,
		COMPRESS_ETC2 = 4,
	};
	enum Format {
		FORMAT_L8 = 0,
		FORMAT_LA8 = 1,
		FORMAT_R8 = 2,
		FORMAT_RG8 = 3,
		FORMAT_RGB8 = 4,
		FORMAT_RGBA8 = 5,
		FORMAT_RGBA4444 = 6,
		FORMAT_RGBA5551 = 7,
		FORMAT_RF = 8,
		FORMAT_RGF = 9,
		FORMAT_RGBF = 10,
		FORMAT_RGBAF = 11,
		FORMAT_RH = 12,
		FORMAT_RGH = 13,
		FORMAT_RGBH = 14,
		FORMAT_RGBAH = 15,
		FORMAT_RGBE9995 = 16,
		FORMAT_DXT1 = 17,
		FORMAT_DXT3 = 18,
		FORMAT_DXT5 = 19,
		FORMAT_RGTC_R = 20,
		FORMAT_RGTC_RG = 21,
		FORMAT_BPTC_RGBA = 22,
		FORMAT_BPTC_RGBF = 23,
		FORMAT_BPTC_RGBFU = 24,
		FORMAT_PVRTC2 = 25,
		FORMAT_PVRTC2A = 26,
		FORMAT_PVRTC4 = 27,
		FORMAT_PVRTC4A = 28,
		FORMAT_ETC = 29,
		FORMAT_ETC2_R11 = 30,
		FORMAT_ETC2_R11S = 31,
		FORMAT_ETC2_RG11 = 32,
		FORMAT_ETC2_RG11S = 33,
		FORMAT_ETC2_RGB8 = 34,
		FORMAT_ETC2_RGBA8 = 35,
		FORMAT_ETC2_RGB8A1 = 36,
		FORMAT_MAX = 37,
	};

	// constants
	const static int MAX_HEIGHT = 16384;
	const static int MAX_WIDTH = 16384;


	static Image *_new();

	// methods
	Dictionary _get_data() const;
	void _set_data(const Dictionary data);
	void blend_rect(const Ref<Image> src, const Rect2 src_rect, const Vector2 dst);
	void blend_rect_mask(const Ref<Image> src, const Ref<Image> mask, const Rect2 src_rect, const Vector2 dst);
	void blit_rect(const Ref<Image> src, const Rect2 src_rect, const Vector2 dst);
	void blit_rect_mask(const Ref<Image> src, const Ref<Image> mask, const Rect2 src_rect, const Vector2 dst);
	void bumpmap_to_normalmap(const real_t bump_scale = 1);
	void clear_mipmaps();
	Error compress(const int64_t mode, const int64_t source, const real_t lossy_quality);
	void convert(const int64_t format);
	void copy_from(const Ref<Image> src);
	void create(const int64_t width, const int64_t height, const bool use_mipmaps, const int64_t format);
	void create_from_data(const int64_t width, const int64_t height, const bool use_mipmaps, const int64_t format, const PoolByteArray data);
	void crop(const int64_t width, const int64_t height);
	Error decompress();
	Image::AlphaMode detect_alpha() const;
	void expand_x2_hq2x();
	void fill(const Color color);
	void fix_alpha_edges();
	void flip_x();
	void flip_y();
	Error generate_mipmaps(const bool renormalize = false);
	PoolByteArray get_data() const;
	Image::Format get_format() const;
	int64_t get_height() const;
	int64_t get_mipmap_offset(const int64_t mipmap) const;
	Color get_pixel(const int64_t x, const int64_t y) const;
	Color get_pixelv(const Vector2 src) const;
	Ref<Image> get_rect(const Rect2 rect) const;
	Vector2 get_size() const;
	Rect2 get_used_rect() const;
	int64_t get_width() const;
	bool has_mipmaps() const;
	bool is_compressed() const;
	bool is_empty() const;
	bool is_invisible() const;
	Error load(const String path);
	Error load_jpg_from_buffer(const PoolByteArray buffer);
	Error load_png_from_buffer(const PoolByteArray buffer);
	Error load_webp_from_buffer(const PoolByteArray buffer);
	void lock();
	void normalmap_to_xy();
	void premultiply_alpha();
	void resize(const int64_t width, const int64_t height, const int64_t interpolation = 1);
	void resize_to_po2(const bool square = false);
	Ref<Image> rgbe_to_srgb();
	Error save_exr(const String path, const bool grayscale = false) const;
	Error save_png(const String path) const;
	void set_pixel(const int64_t x, const int64_t y, const Color color);
	void set_pixelv(const Vector2 dst, const Color color);
	void shrink_x2();
	void srgb_to_linear();
	void unlock();

};

}

#endif