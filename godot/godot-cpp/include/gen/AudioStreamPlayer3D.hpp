#ifndef GODOT_CPP_AUDIOSTREAMPLAYER3D_HPP
#define GODOT_CPP_AUDIOSTREAMPLAYER3D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "AudioStreamPlayer3D.hpp"

#include "Spatial.hpp"
namespace godot {

class AudioStream;
class AudioStreamPlayback;

class AudioStreamPlayer3D : public Spatial {
	struct ___method_bindings {
		godot_method_bind *mb__bus_layout_changed;
		godot_method_bind *mb__is_active;
		godot_method_bind *mb__set_playing;
		godot_method_bind *mb_get_area_mask;
		godot_method_bind *mb_get_attenuation_filter_cutoff_hz;
		godot_method_bind *mb_get_attenuation_filter_db;
		godot_method_bind *mb_get_attenuation_model;
		godot_method_bind *mb_get_bus;
		godot_method_bind *mb_get_doppler_tracking;
		godot_method_bind *mb_get_emission_angle;
		godot_method_bind *mb_get_emission_angle_filter_attenuation_db;
		godot_method_bind *mb_get_max_db;
		godot_method_bind *mb_get_max_distance;
		godot_method_bind *mb_get_out_of_range_mode;
		godot_method_bind *mb_get_pitch_scale;
		godot_method_bind *mb_get_playback_position;
		godot_method_bind *mb_get_stream;
		godot_method_bind *mb_get_stream_paused;
		godot_method_bind *mb_get_stream_playback;
		godot_method_bind *mb_get_unit_db;
		godot_method_bind *mb_get_unit_size;
		godot_method_bind *mb_is_autoplay_enabled;
		godot_method_bind *mb_is_emission_angle_enabled;
		godot_method_bind *mb_is_playing;
		godot_method_bind *mb_play;
		godot_method_bind *mb_seek;
		godot_method_bind *mb_set_area_mask;
		godot_method_bind *mb_set_attenuation_filter_cutoff_hz;
		godot_method_bind *mb_set_attenuation_filter_db;
		godot_method_bind *mb_set_attenuation_model;
		godot_method_bind *mb_set_autoplay;
		godot_method_bind *mb_set_bus;
		godot_method_bind *mb_set_doppler_tracking;
		godot_method_bind *mb_set_emission_angle;
		godot_method_bind *mb_set_emission_angle_enabled;
		godot_method_bind *mb_set_emission_angle_filter_attenuation_db;
		godot_method_bind *mb_set_max_db;
		godot_method_bind *mb_set_max_distance;
		godot_method_bind *mb_set_out_of_range_mode;
		godot_method_bind *mb_set_pitch_scale;
		godot_method_bind *mb_set_stream;
		godot_method_bind *mb_set_stream_paused;
		godot_method_bind *mb_set_unit_db;
		godot_method_bind *mb_set_unit_size;
		godot_method_bind *mb_stop;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AudioStreamPlayer3D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum AttenuationModel {
		ATTENUATION_INVERSE_DISTANCE = 0,
		ATTENUATION_INVERSE_SQUARE_DISTANCE = 1,
		ATTENUATION_LOGARITHMIC = 2,
		ATTENUATION_DISABLED = 3,
	};
	enum OutOfRangeMode {
		OUT_OF_RANGE_MIX = 0,
		OUT_OF_RANGE_PAUSE = 1,
	};
	enum DopplerTracking {
		DOPPLER_TRACKING_DISABLED = 0,
		DOPPLER_TRACKING_IDLE_STEP = 1,
		DOPPLER_TRACKING_PHYSICS_STEP = 2,
	};

	// constants


	static AudioStreamPlayer3D *_new();

	// methods
	void _bus_layout_changed();
	bool _is_active() const;
	void _set_playing(const bool enable);
	int64_t get_area_mask() const;
	real_t get_attenuation_filter_cutoff_hz() const;
	real_t get_attenuation_filter_db() const;
	AudioStreamPlayer3D::AttenuationModel get_attenuation_model() const;
	String get_bus() const;
	AudioStreamPlayer3D::DopplerTracking get_doppler_tracking() const;
	real_t get_emission_angle() const;
	real_t get_emission_angle_filter_attenuation_db() const;
	real_t get_max_db() const;
	real_t get_max_distance() const;
	AudioStreamPlayer3D::OutOfRangeMode get_out_of_range_mode() const;
	real_t get_pitch_scale() const;
	real_t get_playback_position();
	Ref<AudioStream> get_stream() const;
	bool get_stream_paused() const;
	Ref<AudioStreamPlayback> get_stream_playback();
	real_t get_unit_db() const;
	real_t get_unit_size() const;
	bool is_autoplay_enabled();
	bool is_emission_angle_enabled() const;
	bool is_playing() const;
	void play(const real_t from_position = 0);
	void seek(const real_t to_position);
	void set_area_mask(const int64_t mask);
	void set_attenuation_filter_cutoff_hz(const real_t degrees);
	void set_attenuation_filter_db(const real_t db);
	void set_attenuation_model(const int64_t model);
	void set_autoplay(const bool enable);
	void set_bus(const String bus);
	void set_doppler_tracking(const int64_t mode);
	void set_emission_angle(const real_t degrees);
	void set_emission_angle_enabled(const bool enabled);
	void set_emission_angle_filter_attenuation_db(const real_t db);
	void set_max_db(const real_t max_db);
	void set_max_distance(const real_t metres);
	void set_out_of_range_mode(const int64_t mode);
	void set_pitch_scale(const real_t pitch_scale);
	void set_stream(const Ref<AudioStream> stream);
	void set_stream_paused(const bool pause);
	void set_unit_db(const real_t unit_db);
	void set_unit_size(const real_t unit_size);
	void stop();

};

}

#endif