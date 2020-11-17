#ifndef GODOT_CPP_ENVIRONMENT_HPP
#define GODOT_CPP_ENVIRONMENT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Environment.hpp"

#include "Resource.hpp"
namespace godot {

class Texture;
class Sky;

class Environment : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_get_adjustment_brightness;
		godot_method_bind *mb_get_adjustment_color_correction;
		godot_method_bind *mb_get_adjustment_contrast;
		godot_method_bind *mb_get_adjustment_saturation;
		godot_method_bind *mb_get_ambient_light_color;
		godot_method_bind *mb_get_ambient_light_energy;
		godot_method_bind *mb_get_ambient_light_sky_contribution;
		godot_method_bind *mb_get_background;
		godot_method_bind *mb_get_bg_color;
		godot_method_bind *mb_get_bg_energy;
		godot_method_bind *mb_get_camera_feed_id;
		godot_method_bind *mb_get_canvas_max_layer;
		godot_method_bind *mb_get_dof_blur_far_amount;
		godot_method_bind *mb_get_dof_blur_far_distance;
		godot_method_bind *mb_get_dof_blur_far_quality;
		godot_method_bind *mb_get_dof_blur_far_transition;
		godot_method_bind *mb_get_dof_blur_near_amount;
		godot_method_bind *mb_get_dof_blur_near_distance;
		godot_method_bind *mb_get_dof_blur_near_quality;
		godot_method_bind *mb_get_dof_blur_near_transition;
		godot_method_bind *mb_get_fog_color;
		godot_method_bind *mb_get_fog_depth_begin;
		godot_method_bind *mb_get_fog_depth_curve;
		godot_method_bind *mb_get_fog_depth_end;
		godot_method_bind *mb_get_fog_height_curve;
		godot_method_bind *mb_get_fog_height_max;
		godot_method_bind *mb_get_fog_height_min;
		godot_method_bind *mb_get_fog_sun_amount;
		godot_method_bind *mb_get_fog_sun_color;
		godot_method_bind *mb_get_fog_transmit_curve;
		godot_method_bind *mb_get_glow_blend_mode;
		godot_method_bind *mb_get_glow_bloom;
		godot_method_bind *mb_get_glow_hdr_bleed_scale;
		godot_method_bind *mb_get_glow_hdr_bleed_threshold;
		godot_method_bind *mb_get_glow_hdr_luminance_cap;
		godot_method_bind *mb_get_glow_intensity;
		godot_method_bind *mb_get_glow_strength;
		godot_method_bind *mb_get_sky;
		godot_method_bind *mb_get_sky_custom_fov;
		godot_method_bind *mb_get_sky_orientation;
		godot_method_bind *mb_get_sky_rotation;
		godot_method_bind *mb_get_sky_rotation_degrees;
		godot_method_bind *mb_get_ssao_ao_channel_affect;
		godot_method_bind *mb_get_ssao_bias;
		godot_method_bind *mb_get_ssao_blur;
		godot_method_bind *mb_get_ssao_color;
		godot_method_bind *mb_get_ssao_direct_light_affect;
		godot_method_bind *mb_get_ssao_edge_sharpness;
		godot_method_bind *mb_get_ssao_intensity;
		godot_method_bind *mb_get_ssao_intensity2;
		godot_method_bind *mb_get_ssao_quality;
		godot_method_bind *mb_get_ssao_radius;
		godot_method_bind *mb_get_ssao_radius2;
		godot_method_bind *mb_get_ssr_depth_tolerance;
		godot_method_bind *mb_get_ssr_fade_in;
		godot_method_bind *mb_get_ssr_fade_out;
		godot_method_bind *mb_get_ssr_max_steps;
		godot_method_bind *mb_get_tonemap_auto_exposure;
		godot_method_bind *mb_get_tonemap_auto_exposure_grey;
		godot_method_bind *mb_get_tonemap_auto_exposure_max;
		godot_method_bind *mb_get_tonemap_auto_exposure_min;
		godot_method_bind *mb_get_tonemap_auto_exposure_speed;
		godot_method_bind *mb_get_tonemap_exposure;
		godot_method_bind *mb_get_tonemap_white;
		godot_method_bind *mb_get_tonemapper;
		godot_method_bind *mb_is_adjustment_enabled;
		godot_method_bind *mb_is_dof_blur_far_enabled;
		godot_method_bind *mb_is_dof_blur_near_enabled;
		godot_method_bind *mb_is_fog_depth_enabled;
		godot_method_bind *mb_is_fog_enabled;
		godot_method_bind *mb_is_fog_height_enabled;
		godot_method_bind *mb_is_fog_transmit_enabled;
		godot_method_bind *mb_is_glow_bicubic_upscale_enabled;
		godot_method_bind *mb_is_glow_enabled;
		godot_method_bind *mb_is_glow_level_enabled;
		godot_method_bind *mb_is_ssao_enabled;
		godot_method_bind *mb_is_ssr_enabled;
		godot_method_bind *mb_is_ssr_rough;
		godot_method_bind *mb_set_adjustment_brightness;
		godot_method_bind *mb_set_adjustment_color_correction;
		godot_method_bind *mb_set_adjustment_contrast;
		godot_method_bind *mb_set_adjustment_enable;
		godot_method_bind *mb_set_adjustment_saturation;
		godot_method_bind *mb_set_ambient_light_color;
		godot_method_bind *mb_set_ambient_light_energy;
		godot_method_bind *mb_set_ambient_light_sky_contribution;
		godot_method_bind *mb_set_background;
		godot_method_bind *mb_set_bg_color;
		godot_method_bind *mb_set_bg_energy;
		godot_method_bind *mb_set_camera_feed_id;
		godot_method_bind *mb_set_canvas_max_layer;
		godot_method_bind *mb_set_dof_blur_far_amount;
		godot_method_bind *mb_set_dof_blur_far_distance;
		godot_method_bind *mb_set_dof_blur_far_enabled;
		godot_method_bind *mb_set_dof_blur_far_quality;
		godot_method_bind *mb_set_dof_blur_far_transition;
		godot_method_bind *mb_set_dof_blur_near_amount;
		godot_method_bind *mb_set_dof_blur_near_distance;
		godot_method_bind *mb_set_dof_blur_near_enabled;
		godot_method_bind *mb_set_dof_blur_near_quality;
		godot_method_bind *mb_set_dof_blur_near_transition;
		godot_method_bind *mb_set_fog_color;
		godot_method_bind *mb_set_fog_depth_begin;
		godot_method_bind *mb_set_fog_depth_curve;
		godot_method_bind *mb_set_fog_depth_enabled;
		godot_method_bind *mb_set_fog_depth_end;
		godot_method_bind *mb_set_fog_enabled;
		godot_method_bind *mb_set_fog_height_curve;
		godot_method_bind *mb_set_fog_height_enabled;
		godot_method_bind *mb_set_fog_height_max;
		godot_method_bind *mb_set_fog_height_min;
		godot_method_bind *mb_set_fog_sun_amount;
		godot_method_bind *mb_set_fog_sun_color;
		godot_method_bind *mb_set_fog_transmit_curve;
		godot_method_bind *mb_set_fog_transmit_enabled;
		godot_method_bind *mb_set_glow_bicubic_upscale;
		godot_method_bind *mb_set_glow_blend_mode;
		godot_method_bind *mb_set_glow_bloom;
		godot_method_bind *mb_set_glow_enabled;
		godot_method_bind *mb_set_glow_hdr_bleed_scale;
		godot_method_bind *mb_set_glow_hdr_bleed_threshold;
		godot_method_bind *mb_set_glow_hdr_luminance_cap;
		godot_method_bind *mb_set_glow_intensity;
		godot_method_bind *mb_set_glow_level;
		godot_method_bind *mb_set_glow_strength;
		godot_method_bind *mb_set_sky;
		godot_method_bind *mb_set_sky_custom_fov;
		godot_method_bind *mb_set_sky_orientation;
		godot_method_bind *mb_set_sky_rotation;
		godot_method_bind *mb_set_sky_rotation_degrees;
		godot_method_bind *mb_set_ssao_ao_channel_affect;
		godot_method_bind *mb_set_ssao_bias;
		godot_method_bind *mb_set_ssao_blur;
		godot_method_bind *mb_set_ssao_color;
		godot_method_bind *mb_set_ssao_direct_light_affect;
		godot_method_bind *mb_set_ssao_edge_sharpness;
		godot_method_bind *mb_set_ssao_enabled;
		godot_method_bind *mb_set_ssao_intensity;
		godot_method_bind *mb_set_ssao_intensity2;
		godot_method_bind *mb_set_ssao_quality;
		godot_method_bind *mb_set_ssao_radius;
		godot_method_bind *mb_set_ssao_radius2;
		godot_method_bind *mb_set_ssr_depth_tolerance;
		godot_method_bind *mb_set_ssr_enabled;
		godot_method_bind *mb_set_ssr_fade_in;
		godot_method_bind *mb_set_ssr_fade_out;
		godot_method_bind *mb_set_ssr_max_steps;
		godot_method_bind *mb_set_ssr_rough;
		godot_method_bind *mb_set_tonemap_auto_exposure;
		godot_method_bind *mb_set_tonemap_auto_exposure_grey;
		godot_method_bind *mb_set_tonemap_auto_exposure_max;
		godot_method_bind *mb_set_tonemap_auto_exposure_min;
		godot_method_bind *mb_set_tonemap_auto_exposure_speed;
		godot_method_bind *mb_set_tonemap_exposure;
		godot_method_bind *mb_set_tonemap_white;
		godot_method_bind *mb_set_tonemapper;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Environment"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum SSAOBlur {
		SSAO_BLUR_DISABLED = 0,
		SSAO_BLUR_1x1 = 1,
		SSAO_BLUR_2x2 = 2,
		SSAO_BLUR_3x3 = 3,
	};
	enum ToneMapper {
		TONE_MAPPER_LINEAR = 0,
		TONE_MAPPER_REINHARDT = 1,
		TONE_MAPPER_FILMIC = 2,
		TONE_MAPPER_ACES = 3,
	};
	enum GlowBlendMode {
		GLOW_BLEND_MODE_ADDITIVE = 0,
		GLOW_BLEND_MODE_SCREEN = 1,
		GLOW_BLEND_MODE_SOFTLIGHT = 2,
		GLOW_BLEND_MODE_REPLACE = 3,
	};
	enum BGMode {
		BG_CLEAR_COLOR = 0,
		BG_COLOR = 1,
		BG_SKY = 2,
		BG_COLOR_SKY = 3,
		BG_CANVAS = 4,
		BG_KEEP = 5,
		BG_CAMERA_FEED = 6,
		BG_MAX = 7,
	};
	enum SSAOQuality {
		SSAO_QUALITY_LOW = 0,
		SSAO_QUALITY_MEDIUM = 1,
		SSAO_QUALITY_HIGH = 2,
	};
	enum DOFBlurQuality {
		DOF_BLUR_QUALITY_LOW = 0,
		DOF_BLUR_QUALITY_MEDIUM = 1,
		DOF_BLUR_QUALITY_HIGH = 2,
	};

	// constants


	static Environment *_new();

	// methods
	real_t get_adjustment_brightness() const;
	Ref<Texture> get_adjustment_color_correction() const;
	real_t get_adjustment_contrast() const;
	real_t get_adjustment_saturation() const;
	Color get_ambient_light_color() const;
	real_t get_ambient_light_energy() const;
	real_t get_ambient_light_sky_contribution() const;
	Environment::BGMode get_background() const;
	Color get_bg_color() const;
	real_t get_bg_energy() const;
	int64_t get_camera_feed_id() const;
	int64_t get_canvas_max_layer() const;
	real_t get_dof_blur_far_amount() const;
	real_t get_dof_blur_far_distance() const;
	Environment::DOFBlurQuality get_dof_blur_far_quality() const;
	real_t get_dof_blur_far_transition() const;
	real_t get_dof_blur_near_amount() const;
	real_t get_dof_blur_near_distance() const;
	Environment::DOFBlurQuality get_dof_blur_near_quality() const;
	real_t get_dof_blur_near_transition() const;
	Color get_fog_color() const;
	real_t get_fog_depth_begin() const;
	real_t get_fog_depth_curve() const;
	real_t get_fog_depth_end() const;
	real_t get_fog_height_curve() const;
	real_t get_fog_height_max() const;
	real_t get_fog_height_min() const;
	real_t get_fog_sun_amount() const;
	Color get_fog_sun_color() const;
	real_t get_fog_transmit_curve() const;
	Environment::GlowBlendMode get_glow_blend_mode() const;
	real_t get_glow_bloom() const;
	real_t get_glow_hdr_bleed_scale() const;
	real_t get_glow_hdr_bleed_threshold() const;
	real_t get_glow_hdr_luminance_cap() const;
	real_t get_glow_intensity() const;
	real_t get_glow_strength() const;
	Ref<Sky> get_sky() const;
	real_t get_sky_custom_fov() const;
	Basis get_sky_orientation() const;
	Vector3 get_sky_rotation() const;
	Vector3 get_sky_rotation_degrees() const;
	real_t get_ssao_ao_channel_affect() const;
	real_t get_ssao_bias() const;
	Environment::SSAOBlur get_ssao_blur() const;
	Color get_ssao_color() const;
	real_t get_ssao_direct_light_affect() const;
	real_t get_ssao_edge_sharpness() const;
	real_t get_ssao_intensity() const;
	real_t get_ssao_intensity2() const;
	Environment::SSAOQuality get_ssao_quality() const;
	real_t get_ssao_radius() const;
	real_t get_ssao_radius2() const;
	real_t get_ssr_depth_tolerance() const;
	real_t get_ssr_fade_in() const;
	real_t get_ssr_fade_out() const;
	int64_t get_ssr_max_steps() const;
	bool get_tonemap_auto_exposure() const;
	real_t get_tonemap_auto_exposure_grey() const;
	real_t get_tonemap_auto_exposure_max() const;
	real_t get_tonemap_auto_exposure_min() const;
	real_t get_tonemap_auto_exposure_speed() const;
	real_t get_tonemap_exposure() const;
	real_t get_tonemap_white() const;
	Environment::ToneMapper get_tonemapper() const;
	bool is_adjustment_enabled() const;
	bool is_dof_blur_far_enabled() const;
	bool is_dof_blur_near_enabled() const;
	bool is_fog_depth_enabled() const;
	bool is_fog_enabled() const;
	bool is_fog_height_enabled() const;
	bool is_fog_transmit_enabled() const;
	bool is_glow_bicubic_upscale_enabled() const;
	bool is_glow_enabled() const;
	bool is_glow_level_enabled(const int64_t idx) const;
	bool is_ssao_enabled() const;
	bool is_ssr_enabled() const;
	bool is_ssr_rough() const;
	void set_adjustment_brightness(const real_t brightness);
	void set_adjustment_color_correction(const Ref<Texture> color_correction);
	void set_adjustment_contrast(const real_t contrast);
	void set_adjustment_enable(const bool enabled);
	void set_adjustment_saturation(const real_t saturation);
	void set_ambient_light_color(const Color color);
	void set_ambient_light_energy(const real_t energy);
	void set_ambient_light_sky_contribution(const real_t energy);
	void set_background(const int64_t mode);
	void set_bg_color(const Color color);
	void set_bg_energy(const real_t energy);
	void set_camera_feed_id(const int64_t camera_feed_id);
	void set_canvas_max_layer(const int64_t layer);
	void set_dof_blur_far_amount(const real_t intensity);
	void set_dof_blur_far_distance(const real_t intensity);
	void set_dof_blur_far_enabled(const bool enabled);
	void set_dof_blur_far_quality(const int64_t intensity);
	void set_dof_blur_far_transition(const real_t intensity);
	void set_dof_blur_near_amount(const real_t intensity);
	void set_dof_blur_near_distance(const real_t intensity);
	void set_dof_blur_near_enabled(const bool enabled);
	void set_dof_blur_near_quality(const int64_t level);
	void set_dof_blur_near_transition(const real_t intensity);
	void set_fog_color(const Color color);
	void set_fog_depth_begin(const real_t distance);
	void set_fog_depth_curve(const real_t curve);
	void set_fog_depth_enabled(const bool enabled);
	void set_fog_depth_end(const real_t distance);
	void set_fog_enabled(const bool enabled);
	void set_fog_height_curve(const real_t curve);
	void set_fog_height_enabled(const bool enabled);
	void set_fog_height_max(const real_t height);
	void set_fog_height_min(const real_t height);
	void set_fog_sun_amount(const real_t amount);
	void set_fog_sun_color(const Color color);
	void set_fog_transmit_curve(const real_t curve);
	void set_fog_transmit_enabled(const bool enabled);
	void set_glow_bicubic_upscale(const bool enabled);
	void set_glow_blend_mode(const int64_t mode);
	void set_glow_bloom(const real_t amount);
	void set_glow_enabled(const bool enabled);
	void set_glow_hdr_bleed_scale(const real_t scale);
	void set_glow_hdr_bleed_threshold(const real_t threshold);
	void set_glow_hdr_luminance_cap(const real_t amount);
	void set_glow_intensity(const real_t intensity);
	void set_glow_level(const int64_t idx, const bool enabled);
	void set_glow_strength(const real_t strength);
	void set_sky(const Ref<Sky> sky);
	void set_sky_custom_fov(const real_t scale);
	void set_sky_orientation(const Basis orientation);
	void set_sky_rotation(const Vector3 euler_radians);
	void set_sky_rotation_degrees(const Vector3 euler_degrees);
	void set_ssao_ao_channel_affect(const real_t amount);
	void set_ssao_bias(const real_t bias);
	void set_ssao_blur(const int64_t mode);
	void set_ssao_color(const Color color);
	void set_ssao_direct_light_affect(const real_t amount);
	void set_ssao_edge_sharpness(const real_t edge_sharpness);
	void set_ssao_enabled(const bool enabled);
	void set_ssao_intensity(const real_t intensity);
	void set_ssao_intensity2(const real_t intensity);
	void set_ssao_quality(const int64_t quality);
	void set_ssao_radius(const real_t radius);
	void set_ssao_radius2(const real_t radius);
	void set_ssr_depth_tolerance(const real_t depth_tolerance);
	void set_ssr_enabled(const bool enabled);
	void set_ssr_fade_in(const real_t fade_in);
	void set_ssr_fade_out(const real_t fade_out);
	void set_ssr_max_steps(const int64_t max_steps);
	void set_ssr_rough(const bool rough);
	void set_tonemap_auto_exposure(const bool auto_exposure);
	void set_tonemap_auto_exposure_grey(const real_t exposure_grey);
	void set_tonemap_auto_exposure_max(const real_t exposure_max);
	void set_tonemap_auto_exposure_min(const real_t exposure_min);
	void set_tonemap_auto_exposure_speed(const real_t exposure_speed);
	void set_tonemap_exposure(const real_t exposure);
	void set_tonemap_white(const real_t white);
	void set_tonemapper(const int64_t mode);

};

}

#endif