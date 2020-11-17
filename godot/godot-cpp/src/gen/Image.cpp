#include "Image.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Image.hpp"


namespace godot {


Image::___method_bindings Image::___mb = {};

void Image::___init_method_bindings() {
	___mb.mb__get_data = godot::api->godot_method_bind_get_method("Image", "_get_data");
	___mb.mb__set_data = godot::api->godot_method_bind_get_method("Image", "_set_data");
	___mb.mb_blend_rect = godot::api->godot_method_bind_get_method("Image", "blend_rect");
	___mb.mb_blend_rect_mask = godot::api->godot_method_bind_get_method("Image", "blend_rect_mask");
	___mb.mb_blit_rect = godot::api->godot_method_bind_get_method("Image", "blit_rect");
	___mb.mb_blit_rect_mask = godot::api->godot_method_bind_get_method("Image", "blit_rect_mask");
	___mb.mb_bumpmap_to_normalmap = godot::api->godot_method_bind_get_method("Image", "bumpmap_to_normalmap");
	___mb.mb_clear_mipmaps = godot::api->godot_method_bind_get_method("Image", "clear_mipmaps");
	___mb.mb_compress = godot::api->godot_method_bind_get_method("Image", "compress");
	___mb.mb_convert = godot::api->godot_method_bind_get_method("Image", "convert");
	___mb.mb_copy_from = godot::api->godot_method_bind_get_method("Image", "copy_from");
	___mb.mb_create = godot::api->godot_method_bind_get_method("Image", "create");
	___mb.mb_create_from_data = godot::api->godot_method_bind_get_method("Image", "create_from_data");
	___mb.mb_crop = godot::api->godot_method_bind_get_method("Image", "crop");
	___mb.mb_decompress = godot::api->godot_method_bind_get_method("Image", "decompress");
	___mb.mb_detect_alpha = godot::api->godot_method_bind_get_method("Image", "detect_alpha");
	___mb.mb_expand_x2_hq2x = godot::api->godot_method_bind_get_method("Image", "expand_x2_hq2x");
	___mb.mb_fill = godot::api->godot_method_bind_get_method("Image", "fill");
	___mb.mb_fix_alpha_edges = godot::api->godot_method_bind_get_method("Image", "fix_alpha_edges");
	___mb.mb_flip_x = godot::api->godot_method_bind_get_method("Image", "flip_x");
	___mb.mb_flip_y = godot::api->godot_method_bind_get_method("Image", "flip_y");
	___mb.mb_generate_mipmaps = godot::api->godot_method_bind_get_method("Image", "generate_mipmaps");
	___mb.mb_get_data = godot::api->godot_method_bind_get_method("Image", "get_data");
	___mb.mb_get_format = godot::api->godot_method_bind_get_method("Image", "get_format");
	___mb.mb_get_height = godot::api->godot_method_bind_get_method("Image", "get_height");
	___mb.mb_get_mipmap_offset = godot::api->godot_method_bind_get_method("Image", "get_mipmap_offset");
	___mb.mb_get_pixel = godot::api->godot_method_bind_get_method("Image", "get_pixel");
	___mb.mb_get_pixelv = godot::api->godot_method_bind_get_method("Image", "get_pixelv");
	___mb.mb_get_rect = godot::api->godot_method_bind_get_method("Image", "get_rect");
	___mb.mb_get_size = godot::api->godot_method_bind_get_method("Image", "get_size");
	___mb.mb_get_used_rect = godot::api->godot_method_bind_get_method("Image", "get_used_rect");
	___mb.mb_get_width = godot::api->godot_method_bind_get_method("Image", "get_width");
	___mb.mb_has_mipmaps = godot::api->godot_method_bind_get_method("Image", "has_mipmaps");
	___mb.mb_is_compressed = godot::api->godot_method_bind_get_method("Image", "is_compressed");
	___mb.mb_is_empty = godot::api->godot_method_bind_get_method("Image", "is_empty");
	___mb.mb_is_invisible = godot::api->godot_method_bind_get_method("Image", "is_invisible");
	___mb.mb_load = godot::api->godot_method_bind_get_method("Image", "load");
	___mb.mb_load_jpg_from_buffer = godot::api->godot_method_bind_get_method("Image", "load_jpg_from_buffer");
	___mb.mb_load_png_from_buffer = godot::api->godot_method_bind_get_method("Image", "load_png_from_buffer");
	___mb.mb_load_webp_from_buffer = godot::api->godot_method_bind_get_method("Image", "load_webp_from_buffer");
	___mb.mb_lock = godot::api->godot_method_bind_get_method("Image", "lock");
	___mb.mb_normalmap_to_xy = godot::api->godot_method_bind_get_method("Image", "normalmap_to_xy");
	___mb.mb_premultiply_alpha = godot::api->godot_method_bind_get_method("Image", "premultiply_alpha");
	___mb.mb_resize = godot::api->godot_method_bind_get_method("Image", "resize");
	___mb.mb_resize_to_po2 = godot::api->godot_method_bind_get_method("Image", "resize_to_po2");
	___mb.mb_rgbe_to_srgb = godot::api->godot_method_bind_get_method("Image", "rgbe_to_srgb");
	___mb.mb_save_exr = godot::api->godot_method_bind_get_method("Image", "save_exr");
	___mb.mb_save_png = godot::api->godot_method_bind_get_method("Image", "save_png");
	___mb.mb_set_pixel = godot::api->godot_method_bind_get_method("Image", "set_pixel");
	___mb.mb_set_pixelv = godot::api->godot_method_bind_get_method("Image", "set_pixelv");
	___mb.mb_shrink_x2 = godot::api->godot_method_bind_get_method("Image", "shrink_x2");
	___mb.mb_srgb_to_linear = godot::api->godot_method_bind_get_method("Image", "srgb_to_linear");
	___mb.mb_unlock = godot::api->godot_method_bind_get_method("Image", "unlock");
}

Image *Image::_new()
{
	return (Image *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Image")());
}
Dictionary Image::_get_data() const {
	return ___godot_icall_Dictionary(___mb.mb__get_data, (const Object *) this);
}

void Image::_set_data(const Dictionary data) {
	___godot_icall_void_Dictionary(___mb.mb__set_data, (const Object *) this, data);
}

void Image::blend_rect(const Ref<Image> src, const Rect2 src_rect, const Vector2 dst) {
	___godot_icall_void_Object_Rect2_Vector2(___mb.mb_blend_rect, (const Object *) this, src.ptr(), src_rect, dst);
}

void Image::blend_rect_mask(const Ref<Image> src, const Ref<Image> mask, const Rect2 src_rect, const Vector2 dst) {
	___godot_icall_void_Object_Object_Rect2_Vector2(___mb.mb_blend_rect_mask, (const Object *) this, src.ptr(), mask.ptr(), src_rect, dst);
}

void Image::blit_rect(const Ref<Image> src, const Rect2 src_rect, const Vector2 dst) {
	___godot_icall_void_Object_Rect2_Vector2(___mb.mb_blit_rect, (const Object *) this, src.ptr(), src_rect, dst);
}

void Image::blit_rect_mask(const Ref<Image> src, const Ref<Image> mask, const Rect2 src_rect, const Vector2 dst) {
	___godot_icall_void_Object_Object_Rect2_Vector2(___mb.mb_blit_rect_mask, (const Object *) this, src.ptr(), mask.ptr(), src_rect, dst);
}

void Image::bumpmap_to_normalmap(const real_t bump_scale) {
	___godot_icall_void_float(___mb.mb_bumpmap_to_normalmap, (const Object *) this, bump_scale);
}

void Image::clear_mipmaps() {
	___godot_icall_void(___mb.mb_clear_mipmaps, (const Object *) this);
}

Error Image::compress(const int64_t mode, const int64_t source, const real_t lossy_quality) {
	return (Error) ___godot_icall_int_int_int_float(___mb.mb_compress, (const Object *) this, mode, source, lossy_quality);
}

void Image::convert(const int64_t format) {
	___godot_icall_void_int(___mb.mb_convert, (const Object *) this, format);
}

void Image::copy_from(const Ref<Image> src) {
	___godot_icall_void_Object(___mb.mb_copy_from, (const Object *) this, src.ptr());
}

void Image::create(const int64_t width, const int64_t height, const bool use_mipmaps, const int64_t format) {
	___godot_icall_void_int_int_bool_int(___mb.mb_create, (const Object *) this, width, height, use_mipmaps, format);
}

void Image::create_from_data(const int64_t width, const int64_t height, const bool use_mipmaps, const int64_t format, const PoolByteArray data) {
	___godot_icall_void_int_int_bool_int_PoolByteArray(___mb.mb_create_from_data, (const Object *) this, width, height, use_mipmaps, format, data);
}

void Image::crop(const int64_t width, const int64_t height) {
	___godot_icall_void_int_int(___mb.mb_crop, (const Object *) this, width, height);
}

Error Image::decompress() {
	return (Error) ___godot_icall_int(___mb.mb_decompress, (const Object *) this);
}

Image::AlphaMode Image::detect_alpha() const {
	return (Image::AlphaMode) ___godot_icall_int(___mb.mb_detect_alpha, (const Object *) this);
}

void Image::expand_x2_hq2x() {
	___godot_icall_void(___mb.mb_expand_x2_hq2x, (const Object *) this);
}

void Image::fill(const Color color) {
	___godot_icall_void_Color(___mb.mb_fill, (const Object *) this, color);
}

void Image::fix_alpha_edges() {
	___godot_icall_void(___mb.mb_fix_alpha_edges, (const Object *) this);
}

void Image::flip_x() {
	___godot_icall_void(___mb.mb_flip_x, (const Object *) this);
}

void Image::flip_y() {
	___godot_icall_void(___mb.mb_flip_y, (const Object *) this);
}

Error Image::generate_mipmaps(const bool renormalize) {
	return (Error) ___godot_icall_int_bool(___mb.mb_generate_mipmaps, (const Object *) this, renormalize);
}

PoolByteArray Image::get_data() const {
	return ___godot_icall_PoolByteArray(___mb.mb_get_data, (const Object *) this);
}

Image::Format Image::get_format() const {
	return (Image::Format) ___godot_icall_int(___mb.mb_get_format, (const Object *) this);
}

int64_t Image::get_height() const {
	return ___godot_icall_int(___mb.mb_get_height, (const Object *) this);
}

int64_t Image::get_mipmap_offset(const int64_t mipmap) const {
	return ___godot_icall_int_int(___mb.mb_get_mipmap_offset, (const Object *) this, mipmap);
}

Color Image::get_pixel(const int64_t x, const int64_t y) const {
	return ___godot_icall_Color_int_int(___mb.mb_get_pixel, (const Object *) this, x, y);
}

Color Image::get_pixelv(const Vector2 src) const {
	return ___godot_icall_Color_Vector2(___mb.mb_get_pixelv, (const Object *) this, src);
}

Ref<Image> Image::get_rect(const Rect2 rect) const {
	return Ref<Image>::__internal_constructor(___godot_icall_Object_Rect2(___mb.mb_get_rect, (const Object *) this, rect));
}

Vector2 Image::get_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_size, (const Object *) this);
}

Rect2 Image::get_used_rect() const {
	return ___godot_icall_Rect2(___mb.mb_get_used_rect, (const Object *) this);
}

int64_t Image::get_width() const {
	return ___godot_icall_int(___mb.mb_get_width, (const Object *) this);
}

bool Image::has_mipmaps() const {
	return ___godot_icall_bool(___mb.mb_has_mipmaps, (const Object *) this);
}

bool Image::is_compressed() const {
	return ___godot_icall_bool(___mb.mb_is_compressed, (const Object *) this);
}

bool Image::is_empty() const {
	return ___godot_icall_bool(___mb.mb_is_empty, (const Object *) this);
}

bool Image::is_invisible() const {
	return ___godot_icall_bool(___mb.mb_is_invisible, (const Object *) this);
}

Error Image::load(const String path) {
	return (Error) ___godot_icall_int_String(___mb.mb_load, (const Object *) this, path);
}

Error Image::load_jpg_from_buffer(const PoolByteArray buffer) {
	return (Error) ___godot_icall_int_PoolByteArray(___mb.mb_load_jpg_from_buffer, (const Object *) this, buffer);
}

Error Image::load_png_from_buffer(const PoolByteArray buffer) {
	return (Error) ___godot_icall_int_PoolByteArray(___mb.mb_load_png_from_buffer, (const Object *) this, buffer);
}

Error Image::load_webp_from_buffer(const PoolByteArray buffer) {
	return (Error) ___godot_icall_int_PoolByteArray(___mb.mb_load_webp_from_buffer, (const Object *) this, buffer);
}

void Image::lock() {
	___godot_icall_void(___mb.mb_lock, (const Object *) this);
}

void Image::normalmap_to_xy() {
	___godot_icall_void(___mb.mb_normalmap_to_xy, (const Object *) this);
}

void Image::premultiply_alpha() {
	___godot_icall_void(___mb.mb_premultiply_alpha, (const Object *) this);
}

void Image::resize(const int64_t width, const int64_t height, const int64_t interpolation) {
	___godot_icall_void_int_int_int(___mb.mb_resize, (const Object *) this, width, height, interpolation);
}

void Image::resize_to_po2(const bool square) {
	___godot_icall_void_bool(___mb.mb_resize_to_po2, (const Object *) this, square);
}

Ref<Image> Image::rgbe_to_srgb() {
	return Ref<Image>::__internal_constructor(___godot_icall_Object(___mb.mb_rgbe_to_srgb, (const Object *) this));
}

Error Image::save_exr(const String path, const bool grayscale) const {
	return (Error) ___godot_icall_int_String_bool(___mb.mb_save_exr, (const Object *) this, path, grayscale);
}

Error Image::save_png(const String path) const {
	return (Error) ___godot_icall_int_String(___mb.mb_save_png, (const Object *) this, path);
}

void Image::set_pixel(const int64_t x, const int64_t y, const Color color) {
	___godot_icall_void_int_int_Color(___mb.mb_set_pixel, (const Object *) this, x, y, color);
}

void Image::set_pixelv(const Vector2 dst, const Color color) {
	___godot_icall_void_Vector2_Color(___mb.mb_set_pixelv, (const Object *) this, dst, color);
}

void Image::shrink_x2() {
	___godot_icall_void(___mb.mb_shrink_x2, (const Object *) this);
}

void Image::srgb_to_linear() {
	___godot_icall_void(___mb.mb_srgb_to_linear, (const Object *) this);
}

void Image::unlock() {
	___godot_icall_void(___mb.mb_unlock, (const Object *) this);
}

}