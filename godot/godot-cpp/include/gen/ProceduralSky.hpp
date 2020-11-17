#ifndef GODOT_CPP_PROCEDURALSKY_HPP
#define GODOT_CPP_PROCEDURALSKY_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "ProceduralSky.hpp"

#include "Sky.hpp"
namespace godot {

class Image;

class ProceduralSky : public Sky {
	struct ___method_bindings {
		godot_method_bind *mb__thread_done;
		godot_method_bind *mb__update_sky;
		godot_method_bind *mb_get_ground_bottom_color;
		godot_method_bind *mb_get_ground_curve;
		godot_method_bind *mb_get_ground_energy;
		godot_method_bind *mb_get_ground_horizon_color;
		godot_method_bind *mb_get_sky_curve;
		godot_method_bind *mb_get_sky_energy;
		godot_method_bind *mb_get_sky_horizon_color;
		godot_method_bind *mb_get_sky_top_color;
		godot_method_bind *mb_get_sun_angle_max;
		godot_method_bind *mb_get_sun_angle_min;
		godot_method_bind *mb_get_sun_color;
		godot_method_bind *mb_get_sun_curve;
		godot_method_bind *mb_get_sun_energy;
		godot_method_bind *mb_get_sun_latitude;
		godot_method_bind *mb_get_sun_longitude;
		godot_method_bind *mb_get_texture_size;
		godot_method_bind *mb_set_ground_bottom_color;
		godot_method_bind *mb_set_ground_curve;
		godot_method_bind *mb_set_ground_energy;
		godot_method_bind *mb_set_ground_horizon_color;
		godot_method_bind *mb_set_sky_curve;
		godot_method_bind *mb_set_sky_energy;
		godot_method_bind *mb_set_sky_horizon_color;
		godot_method_bind *mb_set_sky_top_color;
		godot_method_bind *mb_set_sun_angle_max;
		godot_method_bind *mb_set_sun_angle_min;
		godot_method_bind *mb_set_sun_color;
		godot_method_bind *mb_set_sun_curve;
		godot_method_bind *mb_set_sun_energy;
		godot_method_bind *mb_set_sun_latitude;
		godot_method_bind *mb_set_sun_longitude;
		godot_method_bind *mb_set_texture_size;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ProceduralSky"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum TextureSize {
		TEXTURE_SIZE_256 = 0,
		TEXTURE_SIZE_512 = 1,
		TEXTURE_SIZE_1024 = 2,
		TEXTURE_SIZE_2048 = 3,
		TEXTURE_SIZE_4096 = 4,
		TEXTURE_SIZE_MAX = 5,
	};

	// constants


	static ProceduralSky *_new();

	// methods
	void _thread_done(const Ref<Image> image);
	void _update_sky();
	Color get_ground_bottom_color() const;
	real_t get_ground_curve() const;
	real_t get_ground_energy() const;
	Color get_ground_horizon_color() const;
	real_t get_sky_curve() const;
	real_t get_sky_energy() const;
	Color get_sky_horizon_color() const;
	Color get_sky_top_color() const;
	real_t get_sun_angle_max() const;
	real_t get_sun_angle_min() const;
	Color get_sun_color() const;
	real_t get_sun_curve() const;
	real_t get_sun_energy() const;
	real_t get_sun_latitude() const;
	real_t get_sun_longitude() const;
	ProceduralSky::TextureSize get_texture_size() const;
	void set_ground_bottom_color(const Color color);
	void set_ground_curve(const real_t curve);
	void set_ground_energy(const real_t energy);
	void set_ground_horizon_color(const Color color);
	void set_sky_curve(const real_t curve);
	void set_sky_energy(const real_t energy);
	void set_sky_horizon_color(const Color color);
	void set_sky_top_color(const Color color);
	void set_sun_angle_max(const real_t degrees);
	void set_sun_angle_min(const real_t degrees);
	void set_sun_color(const Color color);
	void set_sun_curve(const real_t curve);
	void set_sun_energy(const real_t energy);
	void set_sun_latitude(const real_t degrees);
	void set_sun_longitude(const real_t degrees);
	void set_texture_size(const int64_t size);

};

}

#endif