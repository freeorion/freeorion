#include "Environment.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"
#include "Sky.hpp"


namespace godot {


Environment::___method_bindings Environment::___mb = {};

void Environment::___init_method_bindings() {
	___mb.mb_get_adjustment_brightness = godot::api->godot_method_bind_get_method("Environment", "get_adjustment_brightness");
	___mb.mb_get_adjustment_color_correction = godot::api->godot_method_bind_get_method("Environment", "get_adjustment_color_correction");
	___mb.mb_get_adjustment_contrast = godot::api->godot_method_bind_get_method("Environment", "get_adjustment_contrast");
	___mb.mb_get_adjustment_saturation = godot::api->godot_method_bind_get_method("Environment", "get_adjustment_saturation");
	___mb.mb_get_ambient_light_color = godot::api->godot_method_bind_get_method("Environment", "get_ambient_light_color");
	___mb.mb_get_ambient_light_energy = godot::api->godot_method_bind_get_method("Environment", "get_ambient_light_energy");
	___mb.mb_get_ambient_light_sky_contribution = godot::api->godot_method_bind_get_method("Environment", "get_ambient_light_sky_contribution");
	___mb.mb_get_background = godot::api->godot_method_bind_get_method("Environment", "get_background");
	___mb.mb_get_bg_color = godot::api->godot_method_bind_get_method("Environment", "get_bg_color");
	___mb.mb_get_bg_energy = godot::api->godot_method_bind_get_method("Environment", "get_bg_energy");
	___mb.mb_get_camera_feed_id = godot::api->godot_method_bind_get_method("Environment", "get_camera_feed_id");
	___mb.mb_get_canvas_max_layer = godot::api->godot_method_bind_get_method("Environment", "get_canvas_max_layer");
	___mb.mb_get_dof_blur_far_amount = godot::api->godot_method_bind_get_method("Environment", "get_dof_blur_far_amount");
	___mb.mb_get_dof_blur_far_distance = godot::api->godot_method_bind_get_method("Environment", "get_dof_blur_far_distance");
	___mb.mb_get_dof_blur_far_quality = godot::api->godot_method_bind_get_method("Environment", "get_dof_blur_far_quality");
	___mb.mb_get_dof_blur_far_transition = godot::api->godot_method_bind_get_method("Environment", "get_dof_blur_far_transition");
	___mb.mb_get_dof_blur_near_amount = godot::api->godot_method_bind_get_method("Environment", "get_dof_blur_near_amount");
	___mb.mb_get_dof_blur_near_distance = godot::api->godot_method_bind_get_method("Environment", "get_dof_blur_near_distance");
	___mb.mb_get_dof_blur_near_quality = godot::api->godot_method_bind_get_method("Environment", "get_dof_blur_near_quality");
	___mb.mb_get_dof_blur_near_transition = godot::api->godot_method_bind_get_method("Environment", "get_dof_blur_near_transition");
	___mb.mb_get_fog_color = godot::api->godot_method_bind_get_method("Environment", "get_fog_color");
	___mb.mb_get_fog_depth_begin = godot::api->godot_method_bind_get_method("Environment", "get_fog_depth_begin");
	___mb.mb_get_fog_depth_curve = godot::api->godot_method_bind_get_method("Environment", "get_fog_depth_curve");
	___mb.mb_get_fog_depth_end = godot::api->godot_method_bind_get_method("Environment", "get_fog_depth_end");
	___mb.mb_get_fog_height_curve = godot::api->godot_method_bind_get_method("Environment", "get_fog_height_curve");
	___mb.mb_get_fog_height_max = godot::api->godot_method_bind_get_method("Environment", "get_fog_height_max");
	___mb.mb_get_fog_height_min = godot::api->godot_method_bind_get_method("Environment", "get_fog_height_min");
	___mb.mb_get_fog_sun_amount = godot::api->godot_method_bind_get_method("Environment", "get_fog_sun_amount");
	___mb.mb_get_fog_sun_color = godot::api->godot_method_bind_get_method("Environment", "get_fog_sun_color");
	___mb.mb_get_fog_transmit_curve = godot::api->godot_method_bind_get_method("Environment", "get_fog_transmit_curve");
	___mb.mb_get_glow_blend_mode = godot::api->godot_method_bind_get_method("Environment", "get_glow_blend_mode");
	___mb.mb_get_glow_bloom = godot::api->godot_method_bind_get_method("Environment", "get_glow_bloom");
	___mb.mb_get_glow_hdr_bleed_scale = godot::api->godot_method_bind_get_method("Environment", "get_glow_hdr_bleed_scale");
	___mb.mb_get_glow_hdr_bleed_threshold = godot::api->godot_method_bind_get_method("Environment", "get_glow_hdr_bleed_threshold");
	___mb.mb_get_glow_hdr_luminance_cap = godot::api->godot_method_bind_get_method("Environment", "get_glow_hdr_luminance_cap");
	___mb.mb_get_glow_intensity = godot::api->godot_method_bind_get_method("Environment", "get_glow_intensity");
	___mb.mb_get_glow_strength = godot::api->godot_method_bind_get_method("Environment", "get_glow_strength");
	___mb.mb_get_sky = godot::api->godot_method_bind_get_method("Environment", "get_sky");
	___mb.mb_get_sky_custom_fov = godot::api->godot_method_bind_get_method("Environment", "get_sky_custom_fov");
	___mb.mb_get_sky_orientation = godot::api->godot_method_bind_get_method("Environment", "get_sky_orientation");
	___mb.mb_get_sky_rotation = godot::api->godot_method_bind_get_method("Environment", "get_sky_rotation");
	___mb.mb_get_sky_rotation_degrees = godot::api->godot_method_bind_get_method("Environment", "get_sky_rotation_degrees");
	___mb.mb_get_ssao_ao_channel_affect = godot::api->godot_method_bind_get_method("Environment", "get_ssao_ao_channel_affect");
	___mb.mb_get_ssao_bias = godot::api->godot_method_bind_get_method("Environment", "get_ssao_bias");
	___mb.mb_get_ssao_blur = godot::api->godot_method_bind_get_method("Environment", "get_ssao_blur");
	___mb.mb_get_ssao_color = godot::api->godot_method_bind_get_method("Environment", "get_ssao_color");
	___mb.mb_get_ssao_direct_light_affect = godot::api->godot_method_bind_get_method("Environment", "get_ssao_direct_light_affect");
	___mb.mb_get_ssao_edge_sharpness = godot::api->godot_method_bind_get_method("Environment", "get_ssao_edge_sharpness");
	___mb.mb_get_ssao_intensity = godot::api->godot_method_bind_get_method("Environment", "get_ssao_intensity");
	___mb.mb_get_ssao_intensity2 = godot::api->godot_method_bind_get_method("Environment", "get_ssao_intensity2");
	___mb.mb_get_ssao_quality = godot::api->godot_method_bind_get_method("Environment", "get_ssao_quality");
	___mb.mb_get_ssao_radius = godot::api->godot_method_bind_get_method("Environment", "get_ssao_radius");
	___mb.mb_get_ssao_radius2 = godot::api->godot_method_bind_get_method("Environment", "get_ssao_radius2");
	___mb.mb_get_ssr_depth_tolerance = godot::api->godot_method_bind_get_method("Environment", "get_ssr_depth_tolerance");
	___mb.mb_get_ssr_fade_in = godot::api->godot_method_bind_get_method("Environment", "get_ssr_fade_in");
	___mb.mb_get_ssr_fade_out = godot::api->godot_method_bind_get_method("Environment", "get_ssr_fade_out");
	___mb.mb_get_ssr_max_steps = godot::api->godot_method_bind_get_method("Environment", "get_ssr_max_steps");
	___mb.mb_get_tonemap_auto_exposure = godot::api->godot_method_bind_get_method("Environment", "get_tonemap_auto_exposure");
	___mb.mb_get_tonemap_auto_exposure_grey = godot::api->godot_method_bind_get_method("Environment", "get_tonemap_auto_exposure_grey");
	___mb.mb_get_tonemap_auto_exposure_max = godot::api->godot_method_bind_get_method("Environment", "get_tonemap_auto_exposure_max");
	___mb.mb_get_tonemap_auto_exposure_min = godot::api->godot_method_bind_get_method("Environment", "get_tonemap_auto_exposure_min");
	___mb.mb_get_tonemap_auto_exposure_speed = godot::api->godot_method_bind_get_method("Environment", "get_tonemap_auto_exposure_speed");
	___mb.mb_get_tonemap_exposure = godot::api->godot_method_bind_get_method("Environment", "get_tonemap_exposure");
	___mb.mb_get_tonemap_white = godot::api->godot_method_bind_get_method("Environment", "get_tonemap_white");
	___mb.mb_get_tonemapper = godot::api->godot_method_bind_get_method("Environment", "get_tonemapper");
	___mb.mb_is_adjustment_enabled = godot::api->godot_method_bind_get_method("Environment", "is_adjustment_enabled");
	___mb.mb_is_dof_blur_far_enabled = godot::api->godot_method_bind_get_method("Environment", "is_dof_blur_far_enabled");
	___mb.mb_is_dof_blur_near_enabled = godot::api->godot_method_bind_get_method("Environment", "is_dof_blur_near_enabled");
	___mb.mb_is_fog_depth_enabled = godot::api->godot_method_bind_get_method("Environment", "is_fog_depth_enabled");
	___mb.mb_is_fog_enabled = godot::api->godot_method_bind_get_method("Environment", "is_fog_enabled");
	___mb.mb_is_fog_height_enabled = godot::api->godot_method_bind_get_method("Environment", "is_fog_height_enabled");
	___mb.mb_is_fog_transmit_enabled = godot::api->godot_method_bind_get_method("Environment", "is_fog_transmit_enabled");
	___mb.mb_is_glow_bicubic_upscale_enabled = godot::api->godot_method_bind_get_method("Environment", "is_glow_bicubic_upscale_enabled");
	___mb.mb_is_glow_enabled = godot::api->godot_method_bind_get_method("Environment", "is_glow_enabled");
	___mb.mb_is_glow_level_enabled = godot::api->godot_method_bind_get_method("Environment", "is_glow_level_enabled");
	___mb.mb_is_ssao_enabled = godot::api->godot_method_bind_get_method("Environment", "is_ssao_enabled");
	___mb.mb_is_ssr_enabled = godot::api->godot_method_bind_get_method("Environment", "is_ssr_enabled");
	___mb.mb_is_ssr_rough = godot::api->godot_method_bind_get_method("Environment", "is_ssr_rough");
	___mb.mb_set_adjustment_brightness = godot::api->godot_method_bind_get_method("Environment", "set_adjustment_brightness");
	___mb.mb_set_adjustment_color_correction = godot::api->godot_method_bind_get_method("Environment", "set_adjustment_color_correction");
	___mb.mb_set_adjustment_contrast = godot::api->godot_method_bind_get_method("Environment", "set_adjustment_contrast");
	___mb.mb_set_adjustment_enable = godot::api->godot_method_bind_get_method("Environment", "set_adjustment_enable");
	___mb.mb_set_adjustment_saturation = godot::api->godot_method_bind_get_method("Environment", "set_adjustment_saturation");
	___mb.mb_set_ambient_light_color = godot::api->godot_method_bind_get_method("Environment", "set_ambient_light_color");
	___mb.mb_set_ambient_light_energy = godot::api->godot_method_bind_get_method("Environment", "set_ambient_light_energy");
	___mb.mb_set_ambient_light_sky_contribution = godot::api->godot_method_bind_get_method("Environment", "set_ambient_light_sky_contribution");
	___mb.mb_set_background = godot::api->godot_method_bind_get_method("Environment", "set_background");
	___mb.mb_set_bg_color = godot::api->godot_method_bind_get_method("Environment", "set_bg_color");
	___mb.mb_set_bg_energy = godot::api->godot_method_bind_get_method("Environment", "set_bg_energy");
	___mb.mb_set_camera_feed_id = godot::api->godot_method_bind_get_method("Environment", "set_camera_feed_id");
	___mb.mb_set_canvas_max_layer = godot::api->godot_method_bind_get_method("Environment", "set_canvas_max_layer");
	___mb.mb_set_dof_blur_far_amount = godot::api->godot_method_bind_get_method("Environment", "set_dof_blur_far_amount");
	___mb.mb_set_dof_blur_far_distance = godot::api->godot_method_bind_get_method("Environment", "set_dof_blur_far_distance");
	___mb.mb_set_dof_blur_far_enabled = godot::api->godot_method_bind_get_method("Environment", "set_dof_blur_far_enabled");
	___mb.mb_set_dof_blur_far_quality = godot::api->godot_method_bind_get_method("Environment", "set_dof_blur_far_quality");
	___mb.mb_set_dof_blur_far_transition = godot::api->godot_method_bind_get_method("Environment", "set_dof_blur_far_transition");
	___mb.mb_set_dof_blur_near_amount = godot::api->godot_method_bind_get_method("Environment", "set_dof_blur_near_amount");
	___mb.mb_set_dof_blur_near_distance = godot::api->godot_method_bind_get_method("Environment", "set_dof_blur_near_distance");
	___mb.mb_set_dof_blur_near_enabled = godot::api->godot_method_bind_get_method("Environment", "set_dof_blur_near_enabled");
	___mb.mb_set_dof_blur_near_quality = godot::api->godot_method_bind_get_method("Environment", "set_dof_blur_near_quality");
	___mb.mb_set_dof_blur_near_transition = godot::api->godot_method_bind_get_method("Environment", "set_dof_blur_near_transition");
	___mb.mb_set_fog_color = godot::api->godot_method_bind_get_method("Environment", "set_fog_color");
	___mb.mb_set_fog_depth_begin = godot::api->godot_method_bind_get_method("Environment", "set_fog_depth_begin");
	___mb.mb_set_fog_depth_curve = godot::api->godot_method_bind_get_method("Environment", "set_fog_depth_curve");
	___mb.mb_set_fog_depth_enabled = godot::api->godot_method_bind_get_method("Environment", "set_fog_depth_enabled");
	___mb.mb_set_fog_depth_end = godot::api->godot_method_bind_get_method("Environment", "set_fog_depth_end");
	___mb.mb_set_fog_enabled = godot::api->godot_method_bind_get_method("Environment", "set_fog_enabled");
	___mb.mb_set_fog_height_curve = godot::api->godot_method_bind_get_method("Environment", "set_fog_height_curve");
	___mb.mb_set_fog_height_enabled = godot::api->godot_method_bind_get_method("Environment", "set_fog_height_enabled");
	___mb.mb_set_fog_height_max = godot::api->godot_method_bind_get_method("Environment", "set_fog_height_max");
	___mb.mb_set_fog_height_min = godot::api->godot_method_bind_get_method("Environment", "set_fog_height_min");
	___mb.mb_set_fog_sun_amount = godot::api->godot_method_bind_get_method("Environment", "set_fog_sun_amount");
	___mb.mb_set_fog_sun_color = godot::api->godot_method_bind_get_method("Environment", "set_fog_sun_color");
	___mb.mb_set_fog_transmit_curve = godot::api->godot_method_bind_get_method("Environment", "set_fog_transmit_curve");
	___mb.mb_set_fog_transmit_enabled = godot::api->godot_method_bind_get_method("Environment", "set_fog_transmit_enabled");
	___mb.mb_set_glow_bicubic_upscale = godot::api->godot_method_bind_get_method("Environment", "set_glow_bicubic_upscale");
	___mb.mb_set_glow_blend_mode = godot::api->godot_method_bind_get_method("Environment", "set_glow_blend_mode");
	___mb.mb_set_glow_bloom = godot::api->godot_method_bind_get_method("Environment", "set_glow_bloom");
	___mb.mb_set_glow_enabled = godot::api->godot_method_bind_get_method("Environment", "set_glow_enabled");
	___mb.mb_set_glow_hdr_bleed_scale = godot::api->godot_method_bind_get_method("Environment", "set_glow_hdr_bleed_scale");
	___mb.mb_set_glow_hdr_bleed_threshold = godot::api->godot_method_bind_get_method("Environment", "set_glow_hdr_bleed_threshold");
	___mb.mb_set_glow_hdr_luminance_cap = godot::api->godot_method_bind_get_method("Environment", "set_glow_hdr_luminance_cap");
	___mb.mb_set_glow_intensity = godot::api->godot_method_bind_get_method("Environment", "set_glow_intensity");
	___mb.mb_set_glow_level = godot::api->godot_method_bind_get_method("Environment", "set_glow_level");
	___mb.mb_set_glow_strength = godot::api->godot_method_bind_get_method("Environment", "set_glow_strength");
	___mb.mb_set_sky = godot::api->godot_method_bind_get_method("Environment", "set_sky");
	___mb.mb_set_sky_custom_fov = godot::api->godot_method_bind_get_method("Environment", "set_sky_custom_fov");
	___mb.mb_set_sky_orientation = godot::api->godot_method_bind_get_method("Environment", "set_sky_orientation");
	___mb.mb_set_sky_rotation = godot::api->godot_method_bind_get_method("Environment", "set_sky_rotation");
	___mb.mb_set_sky_rotation_degrees = godot::api->godot_method_bind_get_method("Environment", "set_sky_rotation_degrees");
	___mb.mb_set_ssao_ao_channel_affect = godot::api->godot_method_bind_get_method("Environment", "set_ssao_ao_channel_affect");
	___mb.mb_set_ssao_bias = godot::api->godot_method_bind_get_method("Environment", "set_ssao_bias");
	___mb.mb_set_ssao_blur = godot::api->godot_method_bind_get_method("Environment", "set_ssao_blur");
	___mb.mb_set_ssao_color = godot::api->godot_method_bind_get_method("Environment", "set_ssao_color");
	___mb.mb_set_ssao_direct_light_affect = godot::api->godot_method_bind_get_method("Environment", "set_ssao_direct_light_affect");
	___mb.mb_set_ssao_edge_sharpness = godot::api->godot_method_bind_get_method("Environment", "set_ssao_edge_sharpness");
	___mb.mb_set_ssao_enabled = godot::api->godot_method_bind_get_method("Environment", "set_ssao_enabled");
	___mb.mb_set_ssao_intensity = godot::api->godot_method_bind_get_method("Environment", "set_ssao_intensity");
	___mb.mb_set_ssao_intensity2 = godot::api->godot_method_bind_get_method("Environment", "set_ssao_intensity2");
	___mb.mb_set_ssao_quality = godot::api->godot_method_bind_get_method("Environment", "set_ssao_quality");
	___mb.mb_set_ssao_radius = godot::api->godot_method_bind_get_method("Environment", "set_ssao_radius");
	___mb.mb_set_ssao_radius2 = godot::api->godot_method_bind_get_method("Environment", "set_ssao_radius2");
	___mb.mb_set_ssr_depth_tolerance = godot::api->godot_method_bind_get_method("Environment", "set_ssr_depth_tolerance");
	___mb.mb_set_ssr_enabled = godot::api->godot_method_bind_get_method("Environment", "set_ssr_enabled");
	___mb.mb_set_ssr_fade_in = godot::api->godot_method_bind_get_method("Environment", "set_ssr_fade_in");
	___mb.mb_set_ssr_fade_out = godot::api->godot_method_bind_get_method("Environment", "set_ssr_fade_out");
	___mb.mb_set_ssr_max_steps = godot::api->godot_method_bind_get_method("Environment", "set_ssr_max_steps");
	___mb.mb_set_ssr_rough = godot::api->godot_method_bind_get_method("Environment", "set_ssr_rough");
	___mb.mb_set_tonemap_auto_exposure = godot::api->godot_method_bind_get_method("Environment", "set_tonemap_auto_exposure");
	___mb.mb_set_tonemap_auto_exposure_grey = godot::api->godot_method_bind_get_method("Environment", "set_tonemap_auto_exposure_grey");
	___mb.mb_set_tonemap_auto_exposure_max = godot::api->godot_method_bind_get_method("Environment", "set_tonemap_auto_exposure_max");
	___mb.mb_set_tonemap_auto_exposure_min = godot::api->godot_method_bind_get_method("Environment", "set_tonemap_auto_exposure_min");
	___mb.mb_set_tonemap_auto_exposure_speed = godot::api->godot_method_bind_get_method("Environment", "set_tonemap_auto_exposure_speed");
	___mb.mb_set_tonemap_exposure = godot::api->godot_method_bind_get_method("Environment", "set_tonemap_exposure");
	___mb.mb_set_tonemap_white = godot::api->godot_method_bind_get_method("Environment", "set_tonemap_white");
	___mb.mb_set_tonemapper = godot::api->godot_method_bind_get_method("Environment", "set_tonemapper");
}

Environment *Environment::_new()
{
	return (Environment *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Environment")());
}
real_t Environment::get_adjustment_brightness() const {
	return ___godot_icall_float(___mb.mb_get_adjustment_brightness, (const Object *) this);
}

Ref<Texture> Environment::get_adjustment_color_correction() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_adjustment_color_correction, (const Object *) this));
}

real_t Environment::get_adjustment_contrast() const {
	return ___godot_icall_float(___mb.mb_get_adjustment_contrast, (const Object *) this);
}

real_t Environment::get_adjustment_saturation() const {
	return ___godot_icall_float(___mb.mb_get_adjustment_saturation, (const Object *) this);
}

Color Environment::get_ambient_light_color() const {
	return ___godot_icall_Color(___mb.mb_get_ambient_light_color, (const Object *) this);
}

real_t Environment::get_ambient_light_energy() const {
	return ___godot_icall_float(___mb.mb_get_ambient_light_energy, (const Object *) this);
}

real_t Environment::get_ambient_light_sky_contribution() const {
	return ___godot_icall_float(___mb.mb_get_ambient_light_sky_contribution, (const Object *) this);
}

Environment::BGMode Environment::get_background() const {
	return (Environment::BGMode) ___godot_icall_int(___mb.mb_get_background, (const Object *) this);
}

Color Environment::get_bg_color() const {
	return ___godot_icall_Color(___mb.mb_get_bg_color, (const Object *) this);
}

real_t Environment::get_bg_energy() const {
	return ___godot_icall_float(___mb.mb_get_bg_energy, (const Object *) this);
}

int64_t Environment::get_camera_feed_id() const {
	return ___godot_icall_int(___mb.mb_get_camera_feed_id, (const Object *) this);
}

int64_t Environment::get_canvas_max_layer() const {
	return ___godot_icall_int(___mb.mb_get_canvas_max_layer, (const Object *) this);
}

real_t Environment::get_dof_blur_far_amount() const {
	return ___godot_icall_float(___mb.mb_get_dof_blur_far_amount, (const Object *) this);
}

real_t Environment::get_dof_blur_far_distance() const {
	return ___godot_icall_float(___mb.mb_get_dof_blur_far_distance, (const Object *) this);
}

Environment::DOFBlurQuality Environment::get_dof_blur_far_quality() const {
	return (Environment::DOFBlurQuality) ___godot_icall_int(___mb.mb_get_dof_blur_far_quality, (const Object *) this);
}

real_t Environment::get_dof_blur_far_transition() const {
	return ___godot_icall_float(___mb.mb_get_dof_blur_far_transition, (const Object *) this);
}

real_t Environment::get_dof_blur_near_amount() const {
	return ___godot_icall_float(___mb.mb_get_dof_blur_near_amount, (const Object *) this);
}

real_t Environment::get_dof_blur_near_distance() const {
	return ___godot_icall_float(___mb.mb_get_dof_blur_near_distance, (const Object *) this);
}

Environment::DOFBlurQuality Environment::get_dof_blur_near_quality() const {
	return (Environment::DOFBlurQuality) ___godot_icall_int(___mb.mb_get_dof_blur_near_quality, (const Object *) this);
}

real_t Environment::get_dof_blur_near_transition() const {
	return ___godot_icall_float(___mb.mb_get_dof_blur_near_transition, (const Object *) this);
}

Color Environment::get_fog_color() const {
	return ___godot_icall_Color(___mb.mb_get_fog_color, (const Object *) this);
}

real_t Environment::get_fog_depth_begin() const {
	return ___godot_icall_float(___mb.mb_get_fog_depth_begin, (const Object *) this);
}

real_t Environment::get_fog_depth_curve() const {
	return ___godot_icall_float(___mb.mb_get_fog_depth_curve, (const Object *) this);
}

real_t Environment::get_fog_depth_end() const {
	return ___godot_icall_float(___mb.mb_get_fog_depth_end, (const Object *) this);
}

real_t Environment::get_fog_height_curve() const {
	return ___godot_icall_float(___mb.mb_get_fog_height_curve, (const Object *) this);
}

real_t Environment::get_fog_height_max() const {
	return ___godot_icall_float(___mb.mb_get_fog_height_max, (const Object *) this);
}

real_t Environment::get_fog_height_min() const {
	return ___godot_icall_float(___mb.mb_get_fog_height_min, (const Object *) this);
}

real_t Environment::get_fog_sun_amount() const {
	return ___godot_icall_float(___mb.mb_get_fog_sun_amount, (const Object *) this);
}

Color Environment::get_fog_sun_color() const {
	return ___godot_icall_Color(___mb.mb_get_fog_sun_color, (const Object *) this);
}

real_t Environment::get_fog_transmit_curve() const {
	return ___godot_icall_float(___mb.mb_get_fog_transmit_curve, (const Object *) this);
}

Environment::GlowBlendMode Environment::get_glow_blend_mode() const {
	return (Environment::GlowBlendMode) ___godot_icall_int(___mb.mb_get_glow_blend_mode, (const Object *) this);
}

real_t Environment::get_glow_bloom() const {
	return ___godot_icall_float(___mb.mb_get_glow_bloom, (const Object *) this);
}

real_t Environment::get_glow_hdr_bleed_scale() const {
	return ___godot_icall_float(___mb.mb_get_glow_hdr_bleed_scale, (const Object *) this);
}

real_t Environment::get_glow_hdr_bleed_threshold() const {
	return ___godot_icall_float(___mb.mb_get_glow_hdr_bleed_threshold, (const Object *) this);
}

real_t Environment::get_glow_hdr_luminance_cap() const {
	return ___godot_icall_float(___mb.mb_get_glow_hdr_luminance_cap, (const Object *) this);
}

real_t Environment::get_glow_intensity() const {
	return ___godot_icall_float(___mb.mb_get_glow_intensity, (const Object *) this);
}

real_t Environment::get_glow_strength() const {
	return ___godot_icall_float(___mb.mb_get_glow_strength, (const Object *) this);
}

Ref<Sky> Environment::get_sky() const {
	return Ref<Sky>::__internal_constructor(___godot_icall_Object(___mb.mb_get_sky, (const Object *) this));
}

real_t Environment::get_sky_custom_fov() const {
	return ___godot_icall_float(___mb.mb_get_sky_custom_fov, (const Object *) this);
}

Basis Environment::get_sky_orientation() const {
	return ___godot_icall_Basis(___mb.mb_get_sky_orientation, (const Object *) this);
}

Vector3 Environment::get_sky_rotation() const {
	return ___godot_icall_Vector3(___mb.mb_get_sky_rotation, (const Object *) this);
}

Vector3 Environment::get_sky_rotation_degrees() const {
	return ___godot_icall_Vector3(___mb.mb_get_sky_rotation_degrees, (const Object *) this);
}

real_t Environment::get_ssao_ao_channel_affect() const {
	return ___godot_icall_float(___mb.mb_get_ssao_ao_channel_affect, (const Object *) this);
}

real_t Environment::get_ssao_bias() const {
	return ___godot_icall_float(___mb.mb_get_ssao_bias, (const Object *) this);
}

Environment::SSAOBlur Environment::get_ssao_blur() const {
	return (Environment::SSAOBlur) ___godot_icall_int(___mb.mb_get_ssao_blur, (const Object *) this);
}

Color Environment::get_ssao_color() const {
	return ___godot_icall_Color(___mb.mb_get_ssao_color, (const Object *) this);
}

real_t Environment::get_ssao_direct_light_affect() const {
	return ___godot_icall_float(___mb.mb_get_ssao_direct_light_affect, (const Object *) this);
}

real_t Environment::get_ssao_edge_sharpness() const {
	return ___godot_icall_float(___mb.mb_get_ssao_edge_sharpness, (const Object *) this);
}

real_t Environment::get_ssao_intensity() const {
	return ___godot_icall_float(___mb.mb_get_ssao_intensity, (const Object *) this);
}

real_t Environment::get_ssao_intensity2() const {
	return ___godot_icall_float(___mb.mb_get_ssao_intensity2, (const Object *) this);
}

Environment::SSAOQuality Environment::get_ssao_quality() const {
	return (Environment::SSAOQuality) ___godot_icall_int(___mb.mb_get_ssao_quality, (const Object *) this);
}

real_t Environment::get_ssao_radius() const {
	return ___godot_icall_float(___mb.mb_get_ssao_radius, (const Object *) this);
}

real_t Environment::get_ssao_radius2() const {
	return ___godot_icall_float(___mb.mb_get_ssao_radius2, (const Object *) this);
}

real_t Environment::get_ssr_depth_tolerance() const {
	return ___godot_icall_float(___mb.mb_get_ssr_depth_tolerance, (const Object *) this);
}

real_t Environment::get_ssr_fade_in() const {
	return ___godot_icall_float(___mb.mb_get_ssr_fade_in, (const Object *) this);
}

real_t Environment::get_ssr_fade_out() const {
	return ___godot_icall_float(___mb.mb_get_ssr_fade_out, (const Object *) this);
}

int64_t Environment::get_ssr_max_steps() const {
	return ___godot_icall_int(___mb.mb_get_ssr_max_steps, (const Object *) this);
}

bool Environment::get_tonemap_auto_exposure() const {
	return ___godot_icall_bool(___mb.mb_get_tonemap_auto_exposure, (const Object *) this);
}

real_t Environment::get_tonemap_auto_exposure_grey() const {
	return ___godot_icall_float(___mb.mb_get_tonemap_auto_exposure_grey, (const Object *) this);
}

real_t Environment::get_tonemap_auto_exposure_max() const {
	return ___godot_icall_float(___mb.mb_get_tonemap_auto_exposure_max, (const Object *) this);
}

real_t Environment::get_tonemap_auto_exposure_min() const {
	return ___godot_icall_float(___mb.mb_get_tonemap_auto_exposure_min, (const Object *) this);
}

real_t Environment::get_tonemap_auto_exposure_speed() const {
	return ___godot_icall_float(___mb.mb_get_tonemap_auto_exposure_speed, (const Object *) this);
}

real_t Environment::get_tonemap_exposure() const {
	return ___godot_icall_float(___mb.mb_get_tonemap_exposure, (const Object *) this);
}

real_t Environment::get_tonemap_white() const {
	return ___godot_icall_float(___mb.mb_get_tonemap_white, (const Object *) this);
}

Environment::ToneMapper Environment::get_tonemapper() const {
	return (Environment::ToneMapper) ___godot_icall_int(___mb.mb_get_tonemapper, (const Object *) this);
}

bool Environment::is_adjustment_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_adjustment_enabled, (const Object *) this);
}

bool Environment::is_dof_blur_far_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_dof_blur_far_enabled, (const Object *) this);
}

bool Environment::is_dof_blur_near_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_dof_blur_near_enabled, (const Object *) this);
}

bool Environment::is_fog_depth_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_fog_depth_enabled, (const Object *) this);
}

bool Environment::is_fog_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_fog_enabled, (const Object *) this);
}

bool Environment::is_fog_height_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_fog_height_enabled, (const Object *) this);
}

bool Environment::is_fog_transmit_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_fog_transmit_enabled, (const Object *) this);
}

bool Environment::is_glow_bicubic_upscale_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_glow_bicubic_upscale_enabled, (const Object *) this);
}

bool Environment::is_glow_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_glow_enabled, (const Object *) this);
}

bool Environment::is_glow_level_enabled(const int64_t idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_glow_level_enabled, (const Object *) this, idx);
}

bool Environment::is_ssao_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_ssao_enabled, (const Object *) this);
}

bool Environment::is_ssr_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_ssr_enabled, (const Object *) this);
}

bool Environment::is_ssr_rough() const {
	return ___godot_icall_bool(___mb.mb_is_ssr_rough, (const Object *) this);
}

void Environment::set_adjustment_brightness(const real_t brightness) {
	___godot_icall_void_float(___mb.mb_set_adjustment_brightness, (const Object *) this, brightness);
}

void Environment::set_adjustment_color_correction(const Ref<Texture> color_correction) {
	___godot_icall_void_Object(___mb.mb_set_adjustment_color_correction, (const Object *) this, color_correction.ptr());
}

void Environment::set_adjustment_contrast(const real_t contrast) {
	___godot_icall_void_float(___mb.mb_set_adjustment_contrast, (const Object *) this, contrast);
}

void Environment::set_adjustment_enable(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_adjustment_enable, (const Object *) this, enabled);
}

void Environment::set_adjustment_saturation(const real_t saturation) {
	___godot_icall_void_float(___mb.mb_set_adjustment_saturation, (const Object *) this, saturation);
}

void Environment::set_ambient_light_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_ambient_light_color, (const Object *) this, color);
}

void Environment::set_ambient_light_energy(const real_t energy) {
	___godot_icall_void_float(___mb.mb_set_ambient_light_energy, (const Object *) this, energy);
}

void Environment::set_ambient_light_sky_contribution(const real_t energy) {
	___godot_icall_void_float(___mb.mb_set_ambient_light_sky_contribution, (const Object *) this, energy);
}

void Environment::set_background(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_background, (const Object *) this, mode);
}

void Environment::set_bg_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_bg_color, (const Object *) this, color);
}

void Environment::set_bg_energy(const real_t energy) {
	___godot_icall_void_float(___mb.mb_set_bg_energy, (const Object *) this, energy);
}

void Environment::set_camera_feed_id(const int64_t camera_feed_id) {
	___godot_icall_void_int(___mb.mb_set_camera_feed_id, (const Object *) this, camera_feed_id);
}

void Environment::set_canvas_max_layer(const int64_t layer) {
	___godot_icall_void_int(___mb.mb_set_canvas_max_layer, (const Object *) this, layer);
}

void Environment::set_dof_blur_far_amount(const real_t intensity) {
	___godot_icall_void_float(___mb.mb_set_dof_blur_far_amount, (const Object *) this, intensity);
}

void Environment::set_dof_blur_far_distance(const real_t intensity) {
	___godot_icall_void_float(___mb.mb_set_dof_blur_far_distance, (const Object *) this, intensity);
}

void Environment::set_dof_blur_far_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_dof_blur_far_enabled, (const Object *) this, enabled);
}

void Environment::set_dof_blur_far_quality(const int64_t intensity) {
	___godot_icall_void_int(___mb.mb_set_dof_blur_far_quality, (const Object *) this, intensity);
}

void Environment::set_dof_blur_far_transition(const real_t intensity) {
	___godot_icall_void_float(___mb.mb_set_dof_blur_far_transition, (const Object *) this, intensity);
}

void Environment::set_dof_blur_near_amount(const real_t intensity) {
	___godot_icall_void_float(___mb.mb_set_dof_blur_near_amount, (const Object *) this, intensity);
}

void Environment::set_dof_blur_near_distance(const real_t intensity) {
	___godot_icall_void_float(___mb.mb_set_dof_blur_near_distance, (const Object *) this, intensity);
}

void Environment::set_dof_blur_near_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_dof_blur_near_enabled, (const Object *) this, enabled);
}

void Environment::set_dof_blur_near_quality(const int64_t level) {
	___godot_icall_void_int(___mb.mb_set_dof_blur_near_quality, (const Object *) this, level);
}

void Environment::set_dof_blur_near_transition(const real_t intensity) {
	___godot_icall_void_float(___mb.mb_set_dof_blur_near_transition, (const Object *) this, intensity);
}

void Environment::set_fog_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_fog_color, (const Object *) this, color);
}

void Environment::set_fog_depth_begin(const real_t distance) {
	___godot_icall_void_float(___mb.mb_set_fog_depth_begin, (const Object *) this, distance);
}

void Environment::set_fog_depth_curve(const real_t curve) {
	___godot_icall_void_float(___mb.mb_set_fog_depth_curve, (const Object *) this, curve);
}

void Environment::set_fog_depth_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_fog_depth_enabled, (const Object *) this, enabled);
}

void Environment::set_fog_depth_end(const real_t distance) {
	___godot_icall_void_float(___mb.mb_set_fog_depth_end, (const Object *) this, distance);
}

void Environment::set_fog_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_fog_enabled, (const Object *) this, enabled);
}

void Environment::set_fog_height_curve(const real_t curve) {
	___godot_icall_void_float(___mb.mb_set_fog_height_curve, (const Object *) this, curve);
}

void Environment::set_fog_height_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_fog_height_enabled, (const Object *) this, enabled);
}

void Environment::set_fog_height_max(const real_t height) {
	___godot_icall_void_float(___mb.mb_set_fog_height_max, (const Object *) this, height);
}

void Environment::set_fog_height_min(const real_t height) {
	___godot_icall_void_float(___mb.mb_set_fog_height_min, (const Object *) this, height);
}

void Environment::set_fog_sun_amount(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_fog_sun_amount, (const Object *) this, amount);
}

void Environment::set_fog_sun_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_fog_sun_color, (const Object *) this, color);
}

void Environment::set_fog_transmit_curve(const real_t curve) {
	___godot_icall_void_float(___mb.mb_set_fog_transmit_curve, (const Object *) this, curve);
}

void Environment::set_fog_transmit_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_fog_transmit_enabled, (const Object *) this, enabled);
}

void Environment::set_glow_bicubic_upscale(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_glow_bicubic_upscale, (const Object *) this, enabled);
}

void Environment::set_glow_blend_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_glow_blend_mode, (const Object *) this, mode);
}

void Environment::set_glow_bloom(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_glow_bloom, (const Object *) this, amount);
}

void Environment::set_glow_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_glow_enabled, (const Object *) this, enabled);
}

void Environment::set_glow_hdr_bleed_scale(const real_t scale) {
	___godot_icall_void_float(___mb.mb_set_glow_hdr_bleed_scale, (const Object *) this, scale);
}

void Environment::set_glow_hdr_bleed_threshold(const real_t threshold) {
	___godot_icall_void_float(___mb.mb_set_glow_hdr_bleed_threshold, (const Object *) this, threshold);
}

void Environment::set_glow_hdr_luminance_cap(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_glow_hdr_luminance_cap, (const Object *) this, amount);
}

void Environment::set_glow_intensity(const real_t intensity) {
	___godot_icall_void_float(___mb.mb_set_glow_intensity, (const Object *) this, intensity);
}

void Environment::set_glow_level(const int64_t idx, const bool enabled) {
	___godot_icall_void_int_bool(___mb.mb_set_glow_level, (const Object *) this, idx, enabled);
}

void Environment::set_glow_strength(const real_t strength) {
	___godot_icall_void_float(___mb.mb_set_glow_strength, (const Object *) this, strength);
}

void Environment::set_sky(const Ref<Sky> sky) {
	___godot_icall_void_Object(___mb.mb_set_sky, (const Object *) this, sky.ptr());
}

void Environment::set_sky_custom_fov(const real_t scale) {
	___godot_icall_void_float(___mb.mb_set_sky_custom_fov, (const Object *) this, scale);
}

void Environment::set_sky_orientation(const Basis orientation) {
	___godot_icall_void_Basis(___mb.mb_set_sky_orientation, (const Object *) this, orientation);
}

void Environment::set_sky_rotation(const Vector3 euler_radians) {
	___godot_icall_void_Vector3(___mb.mb_set_sky_rotation, (const Object *) this, euler_radians);
}

void Environment::set_sky_rotation_degrees(const Vector3 euler_degrees) {
	___godot_icall_void_Vector3(___mb.mb_set_sky_rotation_degrees, (const Object *) this, euler_degrees);
}

void Environment::set_ssao_ao_channel_affect(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_ssao_ao_channel_affect, (const Object *) this, amount);
}

void Environment::set_ssao_bias(const real_t bias) {
	___godot_icall_void_float(___mb.mb_set_ssao_bias, (const Object *) this, bias);
}

void Environment::set_ssao_blur(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_ssao_blur, (const Object *) this, mode);
}

void Environment::set_ssao_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_ssao_color, (const Object *) this, color);
}

void Environment::set_ssao_direct_light_affect(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_ssao_direct_light_affect, (const Object *) this, amount);
}

void Environment::set_ssao_edge_sharpness(const real_t edge_sharpness) {
	___godot_icall_void_float(___mb.mb_set_ssao_edge_sharpness, (const Object *) this, edge_sharpness);
}

void Environment::set_ssao_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_ssao_enabled, (const Object *) this, enabled);
}

void Environment::set_ssao_intensity(const real_t intensity) {
	___godot_icall_void_float(___mb.mb_set_ssao_intensity, (const Object *) this, intensity);
}

void Environment::set_ssao_intensity2(const real_t intensity) {
	___godot_icall_void_float(___mb.mb_set_ssao_intensity2, (const Object *) this, intensity);
}

void Environment::set_ssao_quality(const int64_t quality) {
	___godot_icall_void_int(___mb.mb_set_ssao_quality, (const Object *) this, quality);
}

void Environment::set_ssao_radius(const real_t radius) {
	___godot_icall_void_float(___mb.mb_set_ssao_radius, (const Object *) this, radius);
}

void Environment::set_ssao_radius2(const real_t radius) {
	___godot_icall_void_float(___mb.mb_set_ssao_radius2, (const Object *) this, radius);
}

void Environment::set_ssr_depth_tolerance(const real_t depth_tolerance) {
	___godot_icall_void_float(___mb.mb_set_ssr_depth_tolerance, (const Object *) this, depth_tolerance);
}

void Environment::set_ssr_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_ssr_enabled, (const Object *) this, enabled);
}

void Environment::set_ssr_fade_in(const real_t fade_in) {
	___godot_icall_void_float(___mb.mb_set_ssr_fade_in, (const Object *) this, fade_in);
}

void Environment::set_ssr_fade_out(const real_t fade_out) {
	___godot_icall_void_float(___mb.mb_set_ssr_fade_out, (const Object *) this, fade_out);
}

void Environment::set_ssr_max_steps(const int64_t max_steps) {
	___godot_icall_void_int(___mb.mb_set_ssr_max_steps, (const Object *) this, max_steps);
}

void Environment::set_ssr_rough(const bool rough) {
	___godot_icall_void_bool(___mb.mb_set_ssr_rough, (const Object *) this, rough);
}

void Environment::set_tonemap_auto_exposure(const bool auto_exposure) {
	___godot_icall_void_bool(___mb.mb_set_tonemap_auto_exposure, (const Object *) this, auto_exposure);
}

void Environment::set_tonemap_auto_exposure_grey(const real_t exposure_grey) {
	___godot_icall_void_float(___mb.mb_set_tonemap_auto_exposure_grey, (const Object *) this, exposure_grey);
}

void Environment::set_tonemap_auto_exposure_max(const real_t exposure_max) {
	___godot_icall_void_float(___mb.mb_set_tonemap_auto_exposure_max, (const Object *) this, exposure_max);
}

void Environment::set_tonemap_auto_exposure_min(const real_t exposure_min) {
	___godot_icall_void_float(___mb.mb_set_tonemap_auto_exposure_min, (const Object *) this, exposure_min);
}

void Environment::set_tonemap_auto_exposure_speed(const real_t exposure_speed) {
	___godot_icall_void_float(___mb.mb_set_tonemap_auto_exposure_speed, (const Object *) this, exposure_speed);
}

void Environment::set_tonemap_exposure(const real_t exposure) {
	___godot_icall_void_float(___mb.mb_set_tonemap_exposure, (const Object *) this, exposure);
}

void Environment::set_tonemap_white(const real_t white) {
	___godot_icall_void_float(___mb.mb_set_tonemap_white, (const Object *) this, white);
}

void Environment::set_tonemapper(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_tonemapper, (const Object *) this, mode);
}

}