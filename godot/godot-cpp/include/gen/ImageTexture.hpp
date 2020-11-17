#ifndef GODOT_CPP_IMAGETEXTURE_HPP
#define GODOT_CPP_IMAGETEXTURE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Image.hpp"
#include "ImageTexture.hpp"

#include "Texture.hpp"
namespace godot {

class Image;

class ImageTexture : public Texture {
	struct ___method_bindings {
		godot_method_bind *mb__reload_hook;
		godot_method_bind *mb_create;
		godot_method_bind *mb_create_from_image;
		godot_method_bind *mb_get_format;
		godot_method_bind *mb_get_lossy_storage_quality;
		godot_method_bind *mb_get_storage;
		godot_method_bind *mb_load;
		godot_method_bind *mb_set_data;
		godot_method_bind *mb_set_lossy_storage_quality;
		godot_method_bind *mb_set_size_override;
		godot_method_bind *mb_set_storage;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ImageTexture"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Storage {
		STORAGE_RAW = 0,
		STORAGE_COMPRESS_LOSSY = 1,
		STORAGE_COMPRESS_LOSSLESS = 2,
	};

	// constants


	static ImageTexture *_new();

	// methods
	void _reload_hook(const RID rid);
	void create(const int64_t width, const int64_t height, const int64_t format, const int64_t flags = 7);
	void create_from_image(const Ref<Image> image, const int64_t flags = 7);
	Image::Format get_format() const;
	real_t get_lossy_storage_quality() const;
	ImageTexture::Storage get_storage() const;
	Error load(const String path);
	void set_data(const Ref<Image> image);
	void set_lossy_storage_quality(const real_t quality);
	void set_size_override(const Vector2 size);
	void set_storage(const int64_t mode);

};

}

#endif