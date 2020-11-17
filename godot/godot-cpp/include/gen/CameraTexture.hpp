#ifndef GODOT_CPP_CAMERATEXTURE_HPP
#define GODOT_CPP_CAMERATEXTURE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "CameraServer.hpp"

#include "Texture.hpp"
namespace godot {


class CameraTexture : public Texture {
	struct ___method_bindings {
		godot_method_bind *mb_get_camera_active;
		godot_method_bind *mb_get_camera_feed_id;
		godot_method_bind *mb_get_which_feed;
		godot_method_bind *mb_set_camera_active;
		godot_method_bind *mb_set_camera_feed_id;
		godot_method_bind *mb_set_which_feed;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CameraTexture"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static CameraTexture *_new();

	// methods
	bool get_camera_active() const;
	int64_t get_camera_feed_id() const;
	CameraServer::FeedImage get_which_feed() const;
	void set_camera_active(const bool active);
	void set_camera_feed_id(const int64_t feed_id);
	void set_which_feed(const int64_t which_feed);

};

}

#endif