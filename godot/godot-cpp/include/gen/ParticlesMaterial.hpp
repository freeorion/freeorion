#ifndef GODOT_CPP_PARTICLESMATERIAL_HPP
#define GODOT_CPP_PARTICLESMATERIAL_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "ParticlesMaterial.hpp"

#include "Material.hpp"
namespace godot {

class Texture;
class GradientTexture;
class CurveTexture;

class ParticlesMaterial : public Material {
	struct ___method_bindings {
		godot_method_bind *mb_get_color;
		godot_method_bind *mb_get_color_ramp;
		godot_method_bind *mb_get_direction;
		godot_method_bind *mb_get_emission_box_extents;
		godot_method_bind *mb_get_emission_color_texture;
		godot_method_bind *mb_get_emission_normal_texture;
		godot_method_bind *mb_get_emission_point_count;
		godot_method_bind *mb_get_emission_point_texture;
		godot_method_bind *mb_get_emission_shape;
		godot_method_bind *mb_get_emission_sphere_radius;
		godot_method_bind *mb_get_flag;
		godot_method_bind *mb_get_flatness;
		godot_method_bind *mb_get_gravity;
		godot_method_bind *mb_get_lifetime_randomness;
		godot_method_bind *mb_get_param;
		godot_method_bind *mb_get_param_randomness;
		godot_method_bind *mb_get_param_texture;
		godot_method_bind *mb_get_spread;
		godot_method_bind *mb_get_trail_color_modifier;
		godot_method_bind *mb_get_trail_divisor;
		godot_method_bind *mb_get_trail_size_modifier;
		godot_method_bind *mb_set_color;
		godot_method_bind *mb_set_color_ramp;
		godot_method_bind *mb_set_direction;
		godot_method_bind *mb_set_emission_box_extents;
		godot_method_bind *mb_set_emission_color_texture;
		godot_method_bind *mb_set_emission_normal_texture;
		godot_method_bind *mb_set_emission_point_count;
		godot_method_bind *mb_set_emission_point_texture;
		godot_method_bind *mb_set_emission_shape;
		godot_method_bind *mb_set_emission_sphere_radius;
		godot_method_bind *mb_set_flag;
		godot_method_bind *mb_set_flatness;
		godot_method_bind *mb_set_gravity;
		godot_method_bind *mb_set_lifetime_randomness;
		godot_method_bind *mb_set_param;
		godot_method_bind *mb_set_param_randomness;
		godot_method_bind *mb_set_param_texture;
		godot_method_bind *mb_set_spread;
		godot_method_bind *mb_set_trail_color_modifier;
		godot_method_bind *mb_set_trail_divisor;
		godot_method_bind *mb_set_trail_size_modifier;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ParticlesMaterial"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Flags {
		FLAG_ALIGN_Y_TO_VELOCITY = 0,
		FLAG_ROTATE_Y = 1,
		FLAG_DISABLE_Z = 2,
		FLAG_MAX = 3,
	};
	enum EmissionShape {
		EMISSION_SHAPE_POINT = 0,
		EMISSION_SHAPE_SPHERE = 1,
		EMISSION_SHAPE_BOX = 2,
		EMISSION_SHAPE_POINTS = 3,
		EMISSION_SHAPE_DIRECTED_POINTS = 4,
		EMISSION_SHAPE_MAX = 5,
	};
	enum Parameter {
		PARAM_INITIAL_LINEAR_VELOCITY = 0,
		PARAM_ANGULAR_VELOCITY = 1,
		PARAM_ORBIT_VELOCITY = 2,
		PARAM_LINEAR_ACCEL = 3,
		PARAM_RADIAL_ACCEL = 4,
		PARAM_TANGENTIAL_ACCEL = 5,
		PARAM_DAMPING = 6,
		PARAM_ANGLE = 7,
		PARAM_SCALE = 8,
		PARAM_HUE_VARIATION = 9,
		PARAM_ANIM_SPEED = 10,
		PARAM_ANIM_OFFSET = 11,
		PARAM_MAX = 12,
	};

	// constants


	static ParticlesMaterial *_new();

	// methods
	Color get_color() const;
	Ref<Texture> get_color_ramp() const;
	Vector3 get_direction() const;
	Vector3 get_emission_box_extents() const;
	Ref<Texture> get_emission_color_texture() const;
	Ref<Texture> get_emission_normal_texture() const;
	int64_t get_emission_point_count() const;
	Ref<Texture> get_emission_point_texture() const;
	ParticlesMaterial::EmissionShape get_emission_shape() const;
	real_t get_emission_sphere_radius() const;
	bool get_flag(const int64_t flag) const;
	real_t get_flatness() const;
	Vector3 get_gravity() const;
	real_t get_lifetime_randomness() const;
	real_t get_param(const int64_t param) const;
	real_t get_param_randomness(const int64_t param) const;
	Ref<Texture> get_param_texture(const int64_t param) const;
	real_t get_spread() const;
	Ref<GradientTexture> get_trail_color_modifier() const;
	int64_t get_trail_divisor() const;
	Ref<CurveTexture> get_trail_size_modifier() const;
	void set_color(const Color color);
	void set_color_ramp(const Ref<Texture> ramp);
	void set_direction(const Vector3 degrees);
	void set_emission_box_extents(const Vector3 extents);
	void set_emission_color_texture(const Ref<Texture> texture);
	void set_emission_normal_texture(const Ref<Texture> texture);
	void set_emission_point_count(const int64_t point_count);
	void set_emission_point_texture(const Ref<Texture> texture);
	void set_emission_shape(const int64_t shape);
	void set_emission_sphere_radius(const real_t radius);
	void set_flag(const int64_t flag, const bool enable);
	void set_flatness(const real_t amount);
	void set_gravity(const Vector3 accel_vec);
	void set_lifetime_randomness(const real_t randomness);
	void set_param(const int64_t param, const real_t value);
	void set_param_randomness(const int64_t param, const real_t randomness);
	void set_param_texture(const int64_t param, const Ref<Texture> texture);
	void set_spread(const real_t degrees);
	void set_trail_color_modifier(const Ref<GradientTexture> texture);
	void set_trail_divisor(const int64_t divisor);
	void set_trail_size_modifier(const Ref<CurveTexture> texture);

};

}

#endif