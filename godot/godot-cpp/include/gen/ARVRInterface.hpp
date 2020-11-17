#ifndef GODOT_CPP_ARVRINTERFACE_HPP
#define GODOT_CPP_ARVRINTERFACE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "ARVRInterface.hpp"

#include "Reference.hpp"
namespace godot {


class ARVRInterface : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_get_anchor_detection_is_enabled;
		godot_method_bind *mb_get_camera_feed_id;
		godot_method_bind *mb_get_capabilities;
		godot_method_bind *mb_get_name;
		godot_method_bind *mb_get_render_targetsize;
		godot_method_bind *mb_get_tracking_status;
		godot_method_bind *mb_initialize;
		godot_method_bind *mb_is_initialized;
		godot_method_bind *mb_is_primary;
		godot_method_bind *mb_is_stereo;
		godot_method_bind *mb_set_anchor_detection_is_enabled;
		godot_method_bind *mb_set_is_initialized;
		godot_method_bind *mb_set_is_primary;
		godot_method_bind *mb_uninitialize;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ARVRInterface"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Tracking_status {
		ARVR_NORMAL_TRACKING = 0,
		ARVR_EXCESSIVE_MOTION = 1,
		ARVR_INSUFFICIENT_FEATURES = 2,
		ARVR_UNKNOWN_TRACKING = 3,
		ARVR_NOT_TRACKING = 4,
	};
	enum Eyes {
		EYE_MONO = 0,
		EYE_LEFT = 1,
		EYE_RIGHT = 2,
	};
	enum Capabilities {
		ARVR_NONE = 0,
		ARVR_MONO = 1,
		ARVR_STEREO = 2,
		ARVR_AR = 4,
		ARVR_EXTERNAL = 8,
	};

	// constants

	// methods
	bool get_anchor_detection_is_enabled() const;
	int64_t get_camera_feed_id();
	int64_t get_capabilities() const;
	String get_name() const;
	Vector2 get_render_targetsize();
	ARVRInterface::Tracking_status get_tracking_status() const;
	bool initialize();
	bool is_initialized() const;
	bool is_primary();
	bool is_stereo();
	void set_anchor_detection_is_enabled(const bool enable);
	void set_is_initialized(const bool initialized);
	void set_is_primary(const bool enable);
	void uninitialize();

};

}

#endif