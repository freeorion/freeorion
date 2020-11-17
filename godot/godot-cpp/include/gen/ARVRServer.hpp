#ifndef GODOT_CPP_ARVRSERVER_HPP
#define GODOT_CPP_ARVRSERVER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {

class ARVRInterface;
class ARVRPositionalTracker;

class ARVRServer : public Object {
	static ARVRServer *_singleton;

	ARVRServer();

	struct ___method_bindings {
		godot_method_bind *mb_center_on_hmd;
		godot_method_bind *mb_find_interface;
		godot_method_bind *mb_get_hmd_transform;
		godot_method_bind *mb_get_interface;
		godot_method_bind *mb_get_interface_count;
		godot_method_bind *mb_get_interfaces;
		godot_method_bind *mb_get_last_commit_usec;
		godot_method_bind *mb_get_last_frame_usec;
		godot_method_bind *mb_get_last_process_usec;
		godot_method_bind *mb_get_primary_interface;
		godot_method_bind *mb_get_reference_frame;
		godot_method_bind *mb_get_tracker;
		godot_method_bind *mb_get_tracker_count;
		godot_method_bind *mb_get_world_scale;
		godot_method_bind *mb_set_primary_interface;
		godot_method_bind *mb_set_world_scale;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline ARVRServer *get_singleton()
	{
		if (!ARVRServer::_singleton) {
			ARVRServer::_singleton = new ARVRServer;
		}
		return ARVRServer::_singleton;
	}

	static inline const char *___get_class_name() { return (const char *) "ARVRServer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum RotationMode {
		RESET_FULL_ROTATION = 0,
		RESET_BUT_KEEP_TILT = 1,
		DONT_RESET_ROTATION = 2,
	};
	enum TrackerType {
		TRACKER_CONTROLLER = 1,
		TRACKER_BASESTATION = 2,
		TRACKER_ANCHOR = 4,
		TRACKER_ANY_KNOWN = 127,
		TRACKER_UNKNOWN = 128,
		TRACKER_ANY = 255,
	};

	// constants

	// methods
	void center_on_hmd(const int64_t rotation_mode, const bool keep_height);
	Ref<ARVRInterface> find_interface(const String name) const;
	Transform get_hmd_transform();
	Ref<ARVRInterface> get_interface(const int64_t idx) const;
	int64_t get_interface_count() const;
	Array get_interfaces() const;
	int64_t get_last_commit_usec();
	int64_t get_last_frame_usec();
	int64_t get_last_process_usec();
	Ref<ARVRInterface> get_primary_interface() const;
	Transform get_reference_frame() const;
	ARVRPositionalTracker *get_tracker(const int64_t idx) const;
	int64_t get_tracker_count() const;
	real_t get_world_scale() const;
	void set_primary_interface(const Ref<ARVRInterface> interface);
	void set_world_scale(const real_t arg0);

};

}

#endif