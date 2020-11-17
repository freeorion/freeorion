#ifndef GODOT_CPP_HINGEJOINT_HPP
#define GODOT_CPP_HINGEJOINT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Joint.hpp"
namespace godot {


class HingeJoint : public Joint {
	struct ___method_bindings {
		godot_method_bind *mb__get_lower_limit;
		godot_method_bind *mb__get_upper_limit;
		godot_method_bind *mb__set_lower_limit;
		godot_method_bind *mb__set_upper_limit;
		godot_method_bind *mb_get_flag;
		godot_method_bind *mb_get_param;
		godot_method_bind *mb_set_flag;
		godot_method_bind *mb_set_param;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "HingeJoint"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Param {
		PARAM_BIAS = 0,
		PARAM_LIMIT_UPPER = 1,
		PARAM_LIMIT_LOWER = 2,
		PARAM_LIMIT_BIAS = 3,
		PARAM_LIMIT_SOFTNESS = 4,
		PARAM_LIMIT_RELAXATION = 5,
		PARAM_MOTOR_TARGET_VELOCITY = 6,
		PARAM_MOTOR_MAX_IMPULSE = 7,
		PARAM_MAX = 8,
	};
	enum Flag {
		FLAG_USE_LIMIT = 0,
		FLAG_ENABLE_MOTOR = 1,
		FLAG_MAX = 2,
	};

	// constants


	static HingeJoint *_new();

	// methods
	real_t _get_lower_limit() const;
	real_t _get_upper_limit() const;
	void _set_lower_limit(const real_t lower_limit);
	void _set_upper_limit(const real_t upper_limit);
	bool get_flag(const int64_t flag) const;
	real_t get_param(const int64_t param) const;
	void set_flag(const int64_t flag, const bool enabled);
	void set_param(const int64_t param, const real_t value);

};

}

#endif