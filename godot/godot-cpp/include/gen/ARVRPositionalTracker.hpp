#ifndef GODOT_CPP_ARVRPOSITIONALTRACKER_HPP
#define GODOT_CPP_ARVRPOSITIONALTRACKER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "ARVRPositionalTracker.hpp"
#include "ARVRServer.hpp"

#include "Object.hpp"
namespace godot {

class Mesh;

class ARVRPositionalTracker : public Object {
	struct ___method_bindings {
		godot_method_bind *mb__set_joy_id;
		godot_method_bind *mb__set_mesh;
		godot_method_bind *mb__set_name;
		godot_method_bind *mb__set_orientation;
		godot_method_bind *mb__set_rw_position;
		godot_method_bind *mb__set_type;
		godot_method_bind *mb_get_hand;
		godot_method_bind *mb_get_joy_id;
		godot_method_bind *mb_get_mesh;
		godot_method_bind *mb_get_name;
		godot_method_bind *mb_get_orientation;
		godot_method_bind *mb_get_position;
		godot_method_bind *mb_get_rumble;
		godot_method_bind *mb_get_tracks_orientation;
		godot_method_bind *mb_get_tracks_position;
		godot_method_bind *mb_get_transform;
		godot_method_bind *mb_get_type;
		godot_method_bind *mb_set_rumble;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ARVRPositionalTracker"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum TrackerHand {
		TRACKER_HAND_UNKNOWN = 0,
		TRACKER_LEFT_HAND = 1,
		TRACKER_RIGHT_HAND = 2,
	};

	// constants


	static ARVRPositionalTracker *_new();

	// methods
	void _set_joy_id(const int64_t joy_id);
	void _set_mesh(const Ref<Mesh> mesh);
	void _set_name(const String name);
	void _set_orientation(const Basis orientation);
	void _set_rw_position(const Vector3 rw_position);
	void _set_type(const int64_t type);
	ARVRPositionalTracker::TrackerHand get_hand() const;
	int64_t get_joy_id() const;
	Ref<Mesh> get_mesh() const;
	String get_name() const;
	Basis get_orientation() const;
	Vector3 get_position() const;
	real_t get_rumble() const;
	bool get_tracks_orientation() const;
	bool get_tracks_position() const;
	Transform get_transform(const bool adjust_by_reference_frame) const;
	ARVRServer::TrackerType get_type() const;
	void set_rumble(const real_t rumble);

};

}

#endif