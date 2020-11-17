#include "SpatialMaterial.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


SpatialMaterial::___method_bindings SpatialMaterial::___mb = {};

void SpatialMaterial::___init_method_bindings() {
	___mb.mb_get_albedo = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_albedo");
	___mb.mb_get_alpha_scissor_threshold = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_alpha_scissor_threshold");
	___mb.mb_get_anisotropy = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_anisotropy");
	___mb.mb_get_ao_light_affect = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_ao_light_affect");
	___mb.mb_get_ao_texture_channel = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_ao_texture_channel");
	___mb.mb_get_billboard_mode = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_billboard_mode");
	___mb.mb_get_blend_mode = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_blend_mode");
	___mb.mb_get_clearcoat = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_clearcoat");
	___mb.mb_get_clearcoat_gloss = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_clearcoat_gloss");
	___mb.mb_get_cull_mode = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_cull_mode");
	___mb.mb_get_depth_deep_parallax_flip_binormal = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_depth_deep_parallax_flip_binormal");
	___mb.mb_get_depth_deep_parallax_flip_tangent = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_depth_deep_parallax_flip_tangent");
	___mb.mb_get_depth_deep_parallax_max_layers = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_depth_deep_parallax_max_layers");
	___mb.mb_get_depth_deep_parallax_min_layers = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_depth_deep_parallax_min_layers");
	___mb.mb_get_depth_draw_mode = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_depth_draw_mode");
	___mb.mb_get_depth_scale = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_depth_scale");
	___mb.mb_get_detail_blend_mode = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_detail_blend_mode");
	___mb.mb_get_detail_uv = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_detail_uv");
	___mb.mb_get_diffuse_mode = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_diffuse_mode");
	___mb.mb_get_distance_fade = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_distance_fade");
	___mb.mb_get_distance_fade_max_distance = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_distance_fade_max_distance");
	___mb.mb_get_distance_fade_min_distance = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_distance_fade_min_distance");
	___mb.mb_get_emission = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_emission");
	___mb.mb_get_emission_energy = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_emission_energy");
	___mb.mb_get_emission_operator = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_emission_operator");
	___mb.mb_get_feature = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_feature");
	___mb.mb_get_flag = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_flag");
	___mb.mb_get_grow = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_grow");
	___mb.mb_get_line_width = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_line_width");
	___mb.mb_get_metallic = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_metallic");
	___mb.mb_get_metallic_texture_channel = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_metallic_texture_channel");
	___mb.mb_get_normal_scale = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_normal_scale");
	___mb.mb_get_particles_anim_h_frames = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_particles_anim_h_frames");
	___mb.mb_get_particles_anim_loop = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_particles_anim_loop");
	___mb.mb_get_particles_anim_v_frames = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_particles_anim_v_frames");
	___mb.mb_get_point_size = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_point_size");
	___mb.mb_get_proximity_fade_distance = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_proximity_fade_distance");
	___mb.mb_get_refraction = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_refraction");
	___mb.mb_get_refraction_texture_channel = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_refraction_texture_channel");
	___mb.mb_get_rim = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_rim");
	___mb.mb_get_rim_tint = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_rim_tint");
	___mb.mb_get_roughness = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_roughness");
	___mb.mb_get_roughness_texture_channel = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_roughness_texture_channel");
	___mb.mb_get_specular = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_specular");
	___mb.mb_get_specular_mode = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_specular_mode");
	___mb.mb_get_subsurface_scattering_strength = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_subsurface_scattering_strength");
	___mb.mb_get_texture = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_texture");
	___mb.mb_get_transmission = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_transmission");
	___mb.mb_get_uv1_offset = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_uv1_offset");
	___mb.mb_get_uv1_scale = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_uv1_scale");
	___mb.mb_get_uv1_triplanar_blend_sharpness = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_uv1_triplanar_blend_sharpness");
	___mb.mb_get_uv2_offset = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_uv2_offset");
	___mb.mb_get_uv2_scale = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_uv2_scale");
	___mb.mb_get_uv2_triplanar_blend_sharpness = godot::api->godot_method_bind_get_method("SpatialMaterial", "get_uv2_triplanar_blend_sharpness");
	___mb.mb_is_depth_deep_parallax_enabled = godot::api->godot_method_bind_get_method("SpatialMaterial", "is_depth_deep_parallax_enabled");
	___mb.mb_is_grow_enabled = godot::api->godot_method_bind_get_method("SpatialMaterial", "is_grow_enabled");
	___mb.mb_is_proximity_fade_enabled = godot::api->godot_method_bind_get_method("SpatialMaterial", "is_proximity_fade_enabled");
	___mb.mb_set_albedo = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_albedo");
	___mb.mb_set_alpha_scissor_threshold = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_alpha_scissor_threshold");
	___mb.mb_set_anisotropy = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_anisotropy");
	___mb.mb_set_ao_light_affect = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_ao_light_affect");
	___mb.mb_set_ao_texture_channel = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_ao_texture_channel");
	___mb.mb_set_billboard_mode = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_billboard_mode");
	___mb.mb_set_blend_mode = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_blend_mode");
	___mb.mb_set_clearcoat = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_clearcoat");
	___mb.mb_set_clearcoat_gloss = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_clearcoat_gloss");
	___mb.mb_set_cull_mode = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_cull_mode");
	___mb.mb_set_depth_deep_parallax = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_depth_deep_parallax");
	___mb.mb_set_depth_deep_parallax_flip_binormal = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_depth_deep_parallax_flip_binormal");
	___mb.mb_set_depth_deep_parallax_flip_tangent = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_depth_deep_parallax_flip_tangent");
	___mb.mb_set_depth_deep_parallax_max_layers = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_depth_deep_parallax_max_layers");
	___mb.mb_set_depth_deep_parallax_min_layers = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_depth_deep_parallax_min_layers");
	___mb.mb_set_depth_draw_mode = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_depth_draw_mode");
	___mb.mb_set_depth_scale = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_depth_scale");
	___mb.mb_set_detail_blend_mode = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_detail_blend_mode");
	___mb.mb_set_detail_uv = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_detail_uv");
	___mb.mb_set_diffuse_mode = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_diffuse_mode");
	___mb.mb_set_distance_fade = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_distance_fade");
	___mb.mb_set_distance_fade_max_distance = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_distance_fade_max_distance");
	___mb.mb_set_distance_fade_min_distance = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_distance_fade_min_distance");
	___mb.mb_set_emission = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_emission");
	___mb.mb_set_emission_energy = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_emission_energy");
	___mb.mb_set_emission_operator = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_emission_operator");
	___mb.mb_set_feature = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_feature");
	___mb.mb_set_flag = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_flag");
	___mb.mb_set_grow = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_grow");
	___mb.mb_set_grow_enabled = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_grow_enabled");
	___mb.mb_set_line_width = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_line_width");
	___mb.mb_set_metallic = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_metallic");
	___mb.mb_set_metallic_texture_channel = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_metallic_texture_channel");
	___mb.mb_set_normal_scale = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_normal_scale");
	___mb.mb_set_particles_anim_h_frames = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_particles_anim_h_frames");
	___mb.mb_set_particles_anim_loop = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_particles_anim_loop");
	___mb.mb_set_particles_anim_v_frames = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_particles_anim_v_frames");
	___mb.mb_set_point_size = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_point_size");
	___mb.mb_set_proximity_fade = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_proximity_fade");
	___mb.mb_set_proximity_fade_distance = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_proximity_fade_distance");
	___mb.mb_set_refraction = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_refraction");
	___mb.mb_set_refraction_texture_channel = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_refraction_texture_channel");
	___mb.mb_set_rim = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_rim");
	___mb.mb_set_rim_tint = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_rim_tint");
	___mb.mb_set_roughness = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_roughness");
	___mb.mb_set_roughness_texture_channel = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_roughness_texture_channel");
	___mb.mb_set_specular = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_specular");
	___mb.mb_set_specular_mode = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_specular_mode");
	___mb.mb_set_subsurface_scattering_strength = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_subsurface_scattering_strength");
	___mb.mb_set_texture = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_texture");
	___mb.mb_set_transmission = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_transmission");
	___mb.mb_set_uv1_offset = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_uv1_offset");
	___mb.mb_set_uv1_scale = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_uv1_scale");
	___mb.mb_set_uv1_triplanar_blend_sharpness = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_uv1_triplanar_blend_sharpness");
	___mb.mb_set_uv2_offset = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_uv2_offset");
	___mb.mb_set_uv2_scale = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_uv2_scale");
	___mb.mb_set_uv2_triplanar_blend_sharpness = godot::api->godot_method_bind_get_method("SpatialMaterial", "set_uv2_triplanar_blend_sharpness");
}

SpatialMaterial *SpatialMaterial::_new()
{
	return (SpatialMaterial *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"SpatialMaterial")());
}
Color SpatialMaterial::get_albedo() const {
	return ___godot_icall_Color(___mb.mb_get_albedo, (const Object *) this);
}

real_t SpatialMaterial::get_alpha_scissor_threshold() const {
	return ___godot_icall_float(___mb.mb_get_alpha_scissor_threshold, (const Object *) this);
}

real_t SpatialMaterial::get_anisotropy() const {
	return ___godot_icall_float(___mb.mb_get_anisotropy, (const Object *) this);
}

real_t SpatialMaterial::get_ao_light_affect() const {
	return ___godot_icall_float(___mb.mb_get_ao_light_affect, (const Object *) this);
}

SpatialMaterial::TextureChannel SpatialMaterial::get_ao_texture_channel() const {
	return (SpatialMaterial::TextureChannel) ___godot_icall_int(___mb.mb_get_ao_texture_channel, (const Object *) this);
}

SpatialMaterial::BillboardMode SpatialMaterial::get_billboard_mode() const {
	return (SpatialMaterial::BillboardMode) ___godot_icall_int(___mb.mb_get_billboard_mode, (const Object *) this);
}

SpatialMaterial::BlendMode SpatialMaterial::get_blend_mode() const {
	return (SpatialMaterial::BlendMode) ___godot_icall_int(___mb.mb_get_blend_mode, (const Object *) this);
}

real_t SpatialMaterial::get_clearcoat() const {
	return ___godot_icall_float(___mb.mb_get_clearcoat, (const Object *) this);
}

real_t SpatialMaterial::get_clearcoat_gloss() const {
	return ___godot_icall_float(___mb.mb_get_clearcoat_gloss, (const Object *) this);
}

SpatialMaterial::CullMode SpatialMaterial::get_cull_mode() const {
	return (SpatialMaterial::CullMode) ___godot_icall_int(___mb.mb_get_cull_mode, (const Object *) this);
}

bool SpatialMaterial::get_depth_deep_parallax_flip_binormal() const {
	return ___godot_icall_bool(___mb.mb_get_depth_deep_parallax_flip_binormal, (const Object *) this);
}

bool SpatialMaterial::get_depth_deep_parallax_flip_tangent() const {
	return ___godot_icall_bool(___mb.mb_get_depth_deep_parallax_flip_tangent, (const Object *) this);
}

int64_t SpatialMaterial::get_depth_deep_parallax_max_layers() const {
	return ___godot_icall_int(___mb.mb_get_depth_deep_parallax_max_layers, (const Object *) this);
}

int64_t SpatialMaterial::get_depth_deep_parallax_min_layers() const {
	return ___godot_icall_int(___mb.mb_get_depth_deep_parallax_min_layers, (const Object *) this);
}

SpatialMaterial::DepthDrawMode SpatialMaterial::get_depth_draw_mode() const {
	return (SpatialMaterial::DepthDrawMode) ___godot_icall_int(___mb.mb_get_depth_draw_mode, (const Object *) this);
}

real_t SpatialMaterial::get_depth_scale() const {
	return ___godot_icall_float(___mb.mb_get_depth_scale, (const Object *) this);
}

SpatialMaterial::BlendMode SpatialMaterial::get_detail_blend_mode() const {
	return (SpatialMaterial::BlendMode) ___godot_icall_int(___mb.mb_get_detail_blend_mode, (const Object *) this);
}

SpatialMaterial::DetailUV SpatialMaterial::get_detail_uv() const {
	return (SpatialMaterial::DetailUV) ___godot_icall_int(___mb.mb_get_detail_uv, (const Object *) this);
}

SpatialMaterial::DiffuseMode SpatialMaterial::get_diffuse_mode() const {
	return (SpatialMaterial::DiffuseMode) ___godot_icall_int(___mb.mb_get_diffuse_mode, (const Object *) this);
}

SpatialMaterial::DistanceFadeMode SpatialMaterial::get_distance_fade() const {
	return (SpatialMaterial::DistanceFadeMode) ___godot_icall_int(___mb.mb_get_distance_fade, (const Object *) this);
}

real_t SpatialMaterial::get_distance_fade_max_distance() const {
	return ___godot_icall_float(___mb.mb_get_distance_fade_max_distance, (const Object *) this);
}

real_t SpatialMaterial::get_distance_fade_min_distance() const {
	return ___godot_icall_float(___mb.mb_get_distance_fade_min_distance, (const Object *) this);
}

Color SpatialMaterial::get_emission() const {
	return ___godot_icall_Color(___mb.mb_get_emission, (const Object *) this);
}

real_t SpatialMaterial::get_emission_energy() const {
	return ___godot_icall_float(___mb.mb_get_emission_energy, (const Object *) this);
}

SpatialMaterial::EmissionOperator SpatialMaterial::get_emission_operator() const {
	return (SpatialMaterial::EmissionOperator) ___godot_icall_int(___mb.mb_get_emission_operator, (const Object *) this);
}

bool SpatialMaterial::get_feature(const int64_t feature) const {
	return ___godot_icall_bool_int(___mb.mb_get_feature, (const Object *) this, feature);
}

bool SpatialMaterial::get_flag(const int64_t flag) const {
	return ___godot_icall_bool_int(___mb.mb_get_flag, (const Object *) this, flag);
}

real_t SpatialMaterial::get_grow() const {
	return ___godot_icall_float(___mb.mb_get_grow, (const Object *) this);
}

real_t SpatialMaterial::get_line_width() const {
	return ___godot_icall_float(___mb.mb_get_line_width, (const Object *) this);
}

real_t SpatialMaterial::get_metallic() const {
	return ___godot_icall_float(___mb.mb_get_metallic, (const Object *) this);
}

SpatialMaterial::TextureChannel SpatialMaterial::get_metallic_texture_channel() const {
	return (SpatialMaterial::TextureChannel) ___godot_icall_int(___mb.mb_get_metallic_texture_channel, (const Object *) this);
}

real_t SpatialMaterial::get_normal_scale() const {
	return ___godot_icall_float(___mb.mb_get_normal_scale, (const Object *) this);
}

int64_t SpatialMaterial::get_particles_anim_h_frames() const {
	return ___godot_icall_int(___mb.mb_get_particles_anim_h_frames, (const Object *) this);
}

bool SpatialMaterial::get_particles_anim_loop() const {
	return ___godot_icall_bool(___mb.mb_get_particles_anim_loop, (const Object *) this);
}

int64_t SpatialMaterial::get_particles_anim_v_frames() const {
	return ___godot_icall_int(___mb.mb_get_particles_anim_v_frames, (const Object *) this);
}

real_t SpatialMaterial::get_point_size() const {
	return ___godot_icall_float(___mb.mb_get_point_size, (const Object *) this);
}

real_t SpatialMaterial::get_proximity_fade_distance() const {
	return ___godot_icall_float(___mb.mb_get_proximity_fade_distance, (const Object *) this);
}

real_t SpatialMaterial::get_refraction() const {
	return ___godot_icall_float(___mb.mb_get_refraction, (const Object *) this);
}

SpatialMaterial::TextureChannel SpatialMaterial::get_refraction_texture_channel() const {
	return (SpatialMaterial::TextureChannel) ___godot_icall_int(___mb.mb_get_refraction_texture_channel, (const Object *) this);
}

real_t SpatialMaterial::get_rim() const {
	return ___godot_icall_float(___mb.mb_get_rim, (const Object *) this);
}

real_t SpatialMaterial::get_rim_tint() const {
	return ___godot_icall_float(___mb.mb_get_rim_tint, (const Object *) this);
}

real_t SpatialMaterial::get_roughness() const {
	return ___godot_icall_float(___mb.mb_get_roughness, (const Object *) this);
}

SpatialMaterial::TextureChannel SpatialMaterial::get_roughness_texture_channel() const {
	return (SpatialMaterial::TextureChannel) ___godot_icall_int(___mb.mb_get_roughness_texture_channel, (const Object *) this);
}

real_t SpatialMaterial::get_specular() const {
	return ___godot_icall_float(___mb.mb_get_specular, (const Object *) this);
}

SpatialMaterial::SpecularMode SpatialMaterial::get_specular_mode() const {
	return (SpatialMaterial::SpecularMode) ___godot_icall_int(___mb.mb_get_specular_mode, (const Object *) this);
}

real_t SpatialMaterial::get_subsurface_scattering_strength() const {
	return ___godot_icall_float(___mb.mb_get_subsurface_scattering_strength, (const Object *) this);
}

Ref<Texture> SpatialMaterial::get_texture(const int64_t param) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_texture, (const Object *) this, param));
}

Color SpatialMaterial::get_transmission() const {
	return ___godot_icall_Color(___mb.mb_get_transmission, (const Object *) this);
}

Vector3 SpatialMaterial::get_uv1_offset() const {
	return ___godot_icall_Vector3(___mb.mb_get_uv1_offset, (const Object *) this);
}

Vector3 SpatialMaterial::get_uv1_scale() const {
	return ___godot_icall_Vector3(___mb.mb_get_uv1_scale, (const Object *) this);
}

real_t SpatialMaterial::get_uv1_triplanar_blend_sharpness() const {
	return ___godot_icall_float(___mb.mb_get_uv1_triplanar_blend_sharpness, (const Object *) this);
}

Vector3 SpatialMaterial::get_uv2_offset() const {
	return ___godot_icall_Vector3(___mb.mb_get_uv2_offset, (const Object *) this);
}

Vector3 SpatialMaterial::get_uv2_scale() const {
	return ___godot_icall_Vector3(___mb.mb_get_uv2_scale, (const Object *) this);
}

real_t SpatialMaterial::get_uv2_triplanar_blend_sharpness() const {
	return ___godot_icall_float(___mb.mb_get_uv2_triplanar_blend_sharpness, (const Object *) this);
}

bool SpatialMaterial::is_depth_deep_parallax_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_depth_deep_parallax_enabled, (const Object *) this);
}

bool SpatialMaterial::is_grow_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_grow_enabled, (const Object *) this);
}

bool SpatialMaterial::is_proximity_fade_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_proximity_fade_enabled, (const Object *) this);
}

void SpatialMaterial::set_albedo(const Color albedo) {
	___godot_icall_void_Color(___mb.mb_set_albedo, (const Object *) this, albedo);
}

void SpatialMaterial::set_alpha_scissor_threshold(const real_t threshold) {
	___godot_icall_void_float(___mb.mb_set_alpha_scissor_threshold, (const Object *) this, threshold);
}

void SpatialMaterial::set_anisotropy(const real_t anisotropy) {
	___godot_icall_void_float(___mb.mb_set_anisotropy, (const Object *) this, anisotropy);
}

void SpatialMaterial::set_ao_light_affect(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_ao_light_affect, (const Object *) this, amount);
}

void SpatialMaterial::set_ao_texture_channel(const int64_t channel) {
	___godot_icall_void_int(___mb.mb_set_ao_texture_channel, (const Object *) this, channel);
}

void SpatialMaterial::set_billboard_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_billboard_mode, (const Object *) this, mode);
}

void SpatialMaterial::set_blend_mode(const int64_t blend_mode) {
	___godot_icall_void_int(___mb.mb_set_blend_mode, (const Object *) this, blend_mode);
}

void SpatialMaterial::set_clearcoat(const real_t clearcoat) {
	___godot_icall_void_float(___mb.mb_set_clearcoat, (const Object *) this, clearcoat);
}

void SpatialMaterial::set_clearcoat_gloss(const real_t clearcoat_gloss) {
	___godot_icall_void_float(___mb.mb_set_clearcoat_gloss, (const Object *) this, clearcoat_gloss);
}

void SpatialMaterial::set_cull_mode(const int64_t cull_mode) {
	___godot_icall_void_int(___mb.mb_set_cull_mode, (const Object *) this, cull_mode);
}

void SpatialMaterial::set_depth_deep_parallax(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_depth_deep_parallax, (const Object *) this, enable);
}

void SpatialMaterial::set_depth_deep_parallax_flip_binormal(const bool flip) {
	___godot_icall_void_bool(___mb.mb_set_depth_deep_parallax_flip_binormal, (const Object *) this, flip);
}

void SpatialMaterial::set_depth_deep_parallax_flip_tangent(const bool flip) {
	___godot_icall_void_bool(___mb.mb_set_depth_deep_parallax_flip_tangent, (const Object *) this, flip);
}

void SpatialMaterial::set_depth_deep_parallax_max_layers(const int64_t layer) {
	___godot_icall_void_int(___mb.mb_set_depth_deep_parallax_max_layers, (const Object *) this, layer);
}

void SpatialMaterial::set_depth_deep_parallax_min_layers(const int64_t layer) {
	___godot_icall_void_int(___mb.mb_set_depth_deep_parallax_min_layers, (const Object *) this, layer);
}

void SpatialMaterial::set_depth_draw_mode(const int64_t depth_draw_mode) {
	___godot_icall_void_int(___mb.mb_set_depth_draw_mode, (const Object *) this, depth_draw_mode);
}

void SpatialMaterial::set_depth_scale(const real_t depth_scale) {
	___godot_icall_void_float(___mb.mb_set_depth_scale, (const Object *) this, depth_scale);
}

void SpatialMaterial::set_detail_blend_mode(const int64_t detail_blend_mode) {
	___godot_icall_void_int(___mb.mb_set_detail_blend_mode, (const Object *) this, detail_blend_mode);
}

void SpatialMaterial::set_detail_uv(const int64_t detail_uv) {
	___godot_icall_void_int(___mb.mb_set_detail_uv, (const Object *) this, detail_uv);
}

void SpatialMaterial::set_diffuse_mode(const int64_t diffuse_mode) {
	___godot_icall_void_int(___mb.mb_set_diffuse_mode, (const Object *) this, diffuse_mode);
}

void SpatialMaterial::set_distance_fade(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_distance_fade, (const Object *) this, mode);
}

void SpatialMaterial::set_distance_fade_max_distance(const real_t distance) {
	___godot_icall_void_float(___mb.mb_set_distance_fade_max_distance, (const Object *) this, distance);
}

void SpatialMaterial::set_distance_fade_min_distance(const real_t distance) {
	___godot_icall_void_float(___mb.mb_set_distance_fade_min_distance, (const Object *) this, distance);
}

void SpatialMaterial::set_emission(const Color emission) {
	___godot_icall_void_Color(___mb.mb_set_emission, (const Object *) this, emission);
}

void SpatialMaterial::set_emission_energy(const real_t emission_energy) {
	___godot_icall_void_float(___mb.mb_set_emission_energy, (const Object *) this, emission_energy);
}

void SpatialMaterial::set_emission_operator(const int64_t _operator) {
	___godot_icall_void_int(___mb.mb_set_emission_operator, (const Object *) this, _operator);
}

void SpatialMaterial::set_feature(const int64_t feature, const bool enable) {
	___godot_icall_void_int_bool(___mb.mb_set_feature, (const Object *) this, feature, enable);
}

void SpatialMaterial::set_flag(const int64_t flag, const bool enable) {
	___godot_icall_void_int_bool(___mb.mb_set_flag, (const Object *) this, flag, enable);
}

void SpatialMaterial::set_grow(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_grow, (const Object *) this, amount);
}

void SpatialMaterial::set_grow_enabled(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_grow_enabled, (const Object *) this, enable);
}

void SpatialMaterial::set_line_width(const real_t line_width) {
	___godot_icall_void_float(___mb.mb_set_line_width, (const Object *) this, line_width);
}

void SpatialMaterial::set_metallic(const real_t metallic) {
	___godot_icall_void_float(___mb.mb_set_metallic, (const Object *) this, metallic);
}

void SpatialMaterial::set_metallic_texture_channel(const int64_t channel) {
	___godot_icall_void_int(___mb.mb_set_metallic_texture_channel, (const Object *) this, channel);
}

void SpatialMaterial::set_normal_scale(const real_t normal_scale) {
	___godot_icall_void_float(___mb.mb_set_normal_scale, (const Object *) this, normal_scale);
}

void SpatialMaterial::set_particles_anim_h_frames(const int64_t frames) {
	___godot_icall_void_int(___mb.mb_set_particles_anim_h_frames, (const Object *) this, frames);
}

void SpatialMaterial::set_particles_anim_loop(const bool loop) {
	___godot_icall_void_bool(___mb.mb_set_particles_anim_loop, (const Object *) this, loop);
}

void SpatialMaterial::set_particles_anim_v_frames(const int64_t frames) {
	___godot_icall_void_int(___mb.mb_set_particles_anim_v_frames, (const Object *) this, frames);
}

void SpatialMaterial::set_point_size(const real_t point_size) {
	___godot_icall_void_float(___mb.mb_set_point_size, (const Object *) this, point_size);
}

void SpatialMaterial::set_proximity_fade(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_proximity_fade, (const Object *) this, enabled);
}

void SpatialMaterial::set_proximity_fade_distance(const real_t distance) {
	___godot_icall_void_float(___mb.mb_set_proximity_fade_distance, (const Object *) this, distance);
}

void SpatialMaterial::set_refraction(const real_t refraction) {
	___godot_icall_void_float(___mb.mb_set_refraction, (const Object *) this, refraction);
}

void SpatialMaterial::set_refraction_texture_channel(const int64_t channel) {
	___godot_icall_void_int(___mb.mb_set_refraction_texture_channel, (const Object *) this, channel);
}

void SpatialMaterial::set_rim(const real_t rim) {
	___godot_icall_void_float(___mb.mb_set_rim, (const Object *) this, rim);
}

void SpatialMaterial::set_rim_tint(const real_t rim_tint) {
	___godot_icall_void_float(___mb.mb_set_rim_tint, (const Object *) this, rim_tint);
}

void SpatialMaterial::set_roughness(const real_t roughness) {
	___godot_icall_void_float(___mb.mb_set_roughness, (const Object *) this, roughness);
}

void SpatialMaterial::set_roughness_texture_channel(const int64_t channel) {
	___godot_icall_void_int(___mb.mb_set_roughness_texture_channel, (const Object *) this, channel);
}

void SpatialMaterial::set_specular(const real_t specular) {
	___godot_icall_void_float(___mb.mb_set_specular, (const Object *) this, specular);
}

void SpatialMaterial::set_specular_mode(const int64_t specular_mode) {
	___godot_icall_void_int(___mb.mb_set_specular_mode, (const Object *) this, specular_mode);
}

void SpatialMaterial::set_subsurface_scattering_strength(const real_t strength) {
	___godot_icall_void_float(___mb.mb_set_subsurface_scattering_strength, (const Object *) this, strength);
}

void SpatialMaterial::set_texture(const int64_t param, const Ref<Texture> texture) {
	___godot_icall_void_int_Object(___mb.mb_set_texture, (const Object *) this, param, texture.ptr());
}

void SpatialMaterial::set_transmission(const Color transmission) {
	___godot_icall_void_Color(___mb.mb_set_transmission, (const Object *) this, transmission);
}

void SpatialMaterial::set_uv1_offset(const Vector3 offset) {
	___godot_icall_void_Vector3(___mb.mb_set_uv1_offset, (const Object *) this, offset);
}

void SpatialMaterial::set_uv1_scale(const Vector3 scale) {
	___godot_icall_void_Vector3(___mb.mb_set_uv1_scale, (const Object *) this, scale);
}

void SpatialMaterial::set_uv1_triplanar_blend_sharpness(const real_t sharpness) {
	___godot_icall_void_float(___mb.mb_set_uv1_triplanar_blend_sharpness, (const Object *) this, sharpness);
}

void SpatialMaterial::set_uv2_offset(const Vector3 offset) {
	___godot_icall_void_Vector3(___mb.mb_set_uv2_offset, (const Object *) this, offset);
}

void SpatialMaterial::set_uv2_scale(const Vector3 scale) {
	___godot_icall_void_Vector3(___mb.mb_set_uv2_scale, (const Object *) this, scale);
}

void SpatialMaterial::set_uv2_triplanar_blend_sharpness(const real_t sharpness) {
	___godot_icall_void_float(___mb.mb_set_uv2_triplanar_blend_sharpness, (const Object *) this, sharpness);
}

}