#ifndef GODOT_CPP_CAMERAFEED_HPP
#define GODOT_CPP_CAMERAFEED_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "CameraFeed.hpp"

#include "Reference.hpp"
namespace godot {

class Image;

class CameraFeed : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb__allocate_texture;
		godot_method_bind *mb__set_RGB_img;
		godot_method_bind *mb__set_YCbCr_img;
		godot_method_bind *mb__set_YCbCr_imgs;
		godot_method_bind *mb__set_name;
		godot_method_bind *mb__set_position;
		godot_method_bind *mb_get_id;
		godot_method_bind *mb_get_name;
		godot_method_bind *mb_get_position;
		godot_method_bind *mb_get_transform;
		godot_method_bind *mb_is_active;
		godot_method_bind *mb_set_active;
		godot_method_bind *mb_set_transform;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CameraFeed"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum FeedDataType {
		FEED_NOIMAGE = 0,
		FEED_RGB = 1,
		FEED_YCBCR = 2,
		FEED_YCBCR_SEP = 3,
	};
	enum FeedPosition {
		FEED_UNSPECIFIED = 0,
		FEED_FRONT = 1,
		FEED_BACK = 2,
	};

	// constants


	static CameraFeed *_new();

	// methods
	void _allocate_texture(const int64_t width, const int64_t height, const int64_t format, const int64_t texture_type, const int64_t data_type);
	void _set_RGB_img(const Ref<Image> rgb_img);
	void _set_YCbCr_img(const Ref<Image> ycbcr_img);
	void _set_YCbCr_imgs(const Ref<Image> y_img, const Ref<Image> cbcr_img);
	void _set_name(const String name);
	void _set_position(const int64_t position);
	int64_t get_id() const;
	String get_name() const;
	CameraFeed::FeedPosition get_position() const;
	Transform2D get_transform() const;
	bool is_active() const;
	void set_active(const bool active);
	void set_transform(const Transform2D transform);

};

}

#endif