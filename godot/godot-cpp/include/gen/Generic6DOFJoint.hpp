#ifndef GODOT_CPP_GENERIC6DOFJOINT_HPP
#define GODOT_CPP_GENERIC6DOFJOINT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Joint.hpp"
namespace godot {


class Generic6DOFJoint : public Joint {
	struct ___method_bindings {
		godot_method_bind *mb__get_angular_hi_limit_x;
		godot_method_bind *mb__get_angular_hi_limit_y;
		godot_method_bind *mb__get_angular_hi_limit_z;
		godot_method_bind *mb__get_angular_lo_limit_x;
		godot_method_bind *mb__get_angular_lo_limit_y;
		godot_method_bind *mb__get_angular_lo_limit_z;
		godot_method_bind *mb__set_angular_hi_limit_x;
		godot_method_bind *mb__set_angular_hi_limit_y;
		godot_method_bind *mb__set_angular_hi_limit_z;
		godot_method_bind *mb__set_angular_lo_limit_x;
		godot_method_bind *mb__set_angular_lo_limit_y;
		godot_method_bind *mb__set_angular_lo_limit_z;
		godot_method_bind *mb_get_flag_x;
		godot_method_bind *mb_get_flag_y;
		godot_method_bind *mb_get_flag_z;
		godot_method_bind *mb_get_param_x;
		godot_method_bind *mb_get_param_y;
		godot_method_bind *mb_get_param_z;
		godot_method_bind *mb_get_precision;
		godot_method_bind *mb_set_flag_x;
		godot_method_bind *mb_set_flag_y;
		godot_method_bind *mb_set_flag_z;
		godot_method_bind *mb_set_param_x;
		godot_method_bind *mb_set_param_y;
		godot_method_bind *mb_set_param_z;
		godot_method_bind *mb_set_precision;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Generic6DOFJoint"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Param {
		PARAM_LINEAR_LOWER_LIMIT = 0,
		PARAM_LINEAR_UPPER_LIMIT = 1,
		PARAM_LINEAR_LIMIT_SOFTNESS = 2,
		PARAM_LINEAR_RESTITUTION = 3,
		PARAM_LINEAR_DAMPING = 4,
		PARAM_LINEAR_MOTOR_TARGET_VELOCITY = 5,
		PARAM_LINEAR_MOTOR_FORCE_LIMIT = 6,
		PARAM_ANGULAR_LOWER_LIMIT = 10,
		PARAM_ANGULAR_UPPER_LIMIT = 11,
		PARAM_ANGULAR_LIMIT_SOFTNESS = 12,
		PARAM_ANGULAR_DAMPING = 13,
		PARAM_ANGULAR_RESTITUTION = 14,
		PARAM_ANGULAR_FORCE_LIMIT = 15,
		PARAM_ANGULAR_ERP = 16,
		PARAM_ANGULAR_MOTOR_TARGET_VELOCITY = 17,
		PARAM_ANGULAR_MOTOR_FORCE_LIMIT = 18,
		PARAM_MAX = 22,
	};
	enum Flag {
		FLAG_ENABLE_LINEAR_LIMIT = 0,
		FLAG_ENABLE_ANGULAR_LIMIT = 1,
		FLAG_ENABLE_ANGULAR_SPRING = 2,
		FLAG_ENABLE_LINEAR_SPRING = 3,
		FLAG_ENABLE_MOTOR = 4,
		FLAG_ENABLE_LINEAR_MOTOR = 5,
		FLAG_MAX = 6,
	};

	// constants


	static Generic6DOFJoint *_new();

	// methods
	real_t _get_angular_hi_limit_x() const;
	real_t _get_angular_hi_limit_y() const;
	real_t _get_angular_hi_limit_z() const;
	real_t _get_angular_lo_limit_x() const;
	real_t _get_angular_lo_limit_y() const;
	real_t _get_angular_lo_limit_z() const;
	void _set_angular_hi_limit_x(const real_t angle);
	void _set_angular_hi_limit_y(const real_t angle);
	void _set_angular_hi_limit_z(const real_t angle);
	void _set_angular_lo_limit_x(const real_t angle);
	void _set_angular_lo_limit_y(const real_t angle);
	void _set_angular_lo_limit_z(const real_t angle);
	bool get_flag_x(const int64_t flag) const;
	bool get_flag_y(const int64_t flag) const;
	bool get_flag_z(const int64_t flag) const;
	real_t get_param_x(const int64_t param) const;
	real_t get_param_y(const int64_t param) const;
	real_t get_param_z(const int64_t param) const;
	int64_t get_precision() const;
	void set_flag_x(const int64_t flag, const bool value);
	void set_flag_y(const int64_t flag, const bool value);
	void set_flag_z(const int64_t flag, const bool value);
	void set_param_x(const int64_t param, const real_t value);
	void set_param_y(const int64_t param, const real_t value);
	void set_param_z(const int64_t param, const real_t value);
	void set_precision(const int64_t precision);

};

}

#endif