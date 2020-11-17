#ifndef GODOT_CPP_SLIDERJOINT_HPP
#define GODOT_CPP_SLIDERJOINT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Joint.hpp"
namespace godot {


class SliderJoint : public Joint {
	struct ___method_bindings {
		godot_method_bind *mb__get_lower_limit_angular;
		godot_method_bind *mb__get_upper_limit_angular;
		godot_method_bind *mb__set_lower_limit_angular;
		godot_method_bind *mb__set_upper_limit_angular;
		godot_method_bind *mb_get_param;
		godot_method_bind *mb_set_param;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "SliderJoint"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Param {
		PARAM_LINEAR_LIMIT_UPPER = 0,
		PARAM_LINEAR_LIMIT_LOWER = 1,
		PARAM_LINEAR_LIMIT_SOFTNESS = 2,
		PARAM_LINEAR_LIMIT_RESTITUTION = 3,
		PARAM_LINEAR_LIMIT_DAMPING = 4,
		PARAM_LINEAR_MOTION_SOFTNESS = 5,
		PARAM_LINEAR_MOTION_RESTITUTION = 6,
		PARAM_LINEAR_MOTION_DAMPING = 7,
		PARAM_LINEAR_ORTHOGONAL_SOFTNESS = 8,
		PARAM_LINEAR_ORTHOGONAL_RESTITUTION = 9,
		PARAM_LINEAR_ORTHOGONAL_DAMPING = 10,
		PARAM_ANGULAR_LIMIT_UPPER = 11,
		PARAM_ANGULAR_LIMIT_LOWER = 12,
		PARAM_ANGULAR_LIMIT_SOFTNESS = 13,
		PARAM_ANGULAR_LIMIT_RESTITUTION = 14,
		PARAM_ANGULAR_LIMIT_DAMPING = 15,
		PARAM_ANGULAR_MOTION_SOFTNESS = 16,
		PARAM_ANGULAR_MOTION_RESTITUTION = 17,
		PARAM_ANGULAR_MOTION_DAMPING = 18,
		PARAM_ANGULAR_ORTHOGONAL_SOFTNESS = 19,
		PARAM_ANGULAR_ORTHOGONAL_RESTITUTION = 20,
		PARAM_ANGULAR_ORTHOGONAL_DAMPING = 21,
		PARAM_MAX = 22,
	};

	// constants


	static SliderJoint *_new();

	// methods
	real_t _get_lower_limit_angular() const;
	real_t _get_upper_limit_angular() const;
	void _set_lower_limit_angular(const real_t lower_limit_angular);
	void _set_upper_limit_angular(const real_t upper_limit_angular);
	real_t get_param(const int64_t param) const;
	void set_param(const int64_t param, const real_t value);

};

}

#endif