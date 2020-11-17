#ifndef GODOT_CPP_SPATIALVELOCITYTRACKER_HPP
#define GODOT_CPP_SPATIALVELOCITYTRACKER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {


class SpatialVelocityTracker : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_get_tracked_linear_velocity;
		godot_method_bind *mb_is_tracking_physics_step;
		godot_method_bind *mb_reset;
		godot_method_bind *mb_set_track_physics_step;
		godot_method_bind *mb_update_position;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "SpatialVelocityTracker"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static SpatialVelocityTracker *_new();

	// methods
	Vector3 get_tracked_linear_velocity() const;
	bool is_tracking_physics_step() const;
	void reset(const Vector3 position);
	void set_track_physics_step(const bool enable);
	void update_position(const Vector3 position);

};

}

#endif