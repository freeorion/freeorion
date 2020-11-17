#ifndef GODOT_CPP_CAMERASERVER_HPP
#define GODOT_CPP_CAMERASERVER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {

class CameraFeed;

class CameraServer : public Object {
	static CameraServer *_singleton;

	CameraServer();

	struct ___method_bindings {
		godot_method_bind *mb_add_feed;
		godot_method_bind *mb_feeds;
		godot_method_bind *mb_get_feed;
		godot_method_bind *mb_get_feed_count;
		godot_method_bind *mb_remove_feed;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline CameraServer *get_singleton()
	{
		if (!CameraServer::_singleton) {
			CameraServer::_singleton = new CameraServer;
		}
		return CameraServer::_singleton;
	}

	static inline const char *___get_class_name() { return (const char *) "CameraServer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum FeedImage {
		FEED_RGBA_IMAGE = 0,
		FEED_YCBCR_IMAGE = 0,
		FEED_Y_IMAGE = 0,
		FEED_CBCR_IMAGE = 1,
	};

	// constants

	// methods
	void add_feed(const Ref<CameraFeed> feed);
	Array feeds();
	Ref<CameraFeed> get_feed(const int64_t index);
	int64_t get_feed_count();
	void remove_feed(const Ref<CameraFeed> feed);

};

}

#endif