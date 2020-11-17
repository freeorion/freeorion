#ifndef GODOT_CPP_SPATIALMATERIAL_HPP
#define GODOT_CPP_SPATIALMATERIAL_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "SpatialMaterial.hpp"

#include "Material.hpp"
namespace godot {

class Texture;

class SpatialMaterial : public Material {
	struct ___method_bindings {
		godot_method_bind *mb_get_albedo;
		godot_method_bind *mb_get_alpha_scissor_threshold;
		godot_method_bind *mb_get_anisotropy;
		godot_method_bind *mb_get_ao_light_affect;
		godot_method_bind *mb_get_ao_texture_channel;
		godot_method_bind *mb_get_billboard_mode;
		godot_method_bind *mb_get_blend_mode;
		godot_method_bind *mb_get_clearcoat;
		godot_method_bind *mb_get_clearcoat_gloss;
		godot_method_bind *mb_get_cull_mode;
		godot_method_bind *mb_get_depth_deep_parallax_flip_binormal;
		godot_method_bind *mb_get_depth_deep_parallax_flip_tangent;
		godot_method_bind *mb_get_depth_deep_parallax_max_layers;
		godot_method_bind *mb_get_depth_deep_parallax_min_layers;
		godot_method_bind *mb_get_depth_draw_mode;
		godot_method_bind *mb_get_depth_scale;
		godot_method_bind *mb_get_detail_blend_mode;
		godot_method_bind *mb_get_detail_uv;
		godot_method_bind *mb_get_diffuse_mode;
		godot_method_bind *mb_get_distance_fade;
		godot_method_bind *mb_get_distance_fade_max_distance;
		godot_method_bind *mb_get_distance_fade_min_distance;
		godot_method_bind *mb_get_emission;
		godot_method_bind *mb_get_emission_energy;
		godot_method_bind *mb_get_emission_operator;
		godot_method_bind *mb_get_feature;
		godot_method_bind *mb_get_flag;
		godot_method_bind *mb_get_grow;
		godot_method_bind *mb_get_line_width;
		godot_method_bind *mb_get_metallic;
		godot_method_bind *mb_get_metallic_texture_channel;
		godot_method_bind *mb_get_normal_scale;
		godot_method_bind *mb_get_particles_anim_h_frames;
		godot_method_bind *mb_get_particles_anim_loop;
		godot_method_bind *mb_get_particles_anim_v_frames;
		godot_method_bind *mb_get_point_size;
		godot_method_bind *mb_get_proximity_fade_distance;
		godot_method_bind *mb_get_refraction;
		godot_method_bind *mb_get_refraction_texture_channel;
		godot_method_bind *mb_get_rim;
		godot_method_bind *mb_get_rim_tint;
		godot_method_bind *mb_get_roughness;
		godot_method_bind *mb_get_roughness_texture_channel;
		godot_method_bind *mb_get_specular;
		godot_method_bind *mb_get_specular_mode;
		godot_method_bind *mb_get_subsurface_scattering_strength;
		godot_method_bind *mb_get_texture;
		godot_method_bind *mb_get_transmission;
		godot_method_bind *mb_get_uv1_offset;
		godot_method_bind *mb_get_uv1_scale;
		godot_method_bind *mb_get_uv1_triplanar_blend_sharpness;
		godot_method_bind *mb_get_uv2_offset;
		godot_method_bind *mb_get_uv2_scale;
		godot_method_bind *mb_get_uv2_triplanar_blend_sharpness;
		godot_method_bind *mb_is_depth_deep_parallax_enabled;
		godot_method_bind *mb_is_grow_enabled;
		godot_method_bind *mb_is_proximity_fade_enabled;
		godot_method_bind *mb_set_albedo;
		godot_method_bind *mb_set_alpha_scissor_threshold;
		godot_method_bind *mb_set_anisotropy;
		godot_method_bind *mb_set_ao_light_affect;
		godot_method_bind *mb_set_ao_texture_channel;
		godot_method_bind *mb_set_billboard_mode;
		godot_method_bind *mb_set_blend_mode;
		godot_method_bind *mb_set_clearcoat;
		godot_method_bind *mb_set_clearcoat_gloss;
		godot_method_bind *mb_set_cull_mode;
		godot_method_bind *mb_set_depth_deep_parallax;
		godot_method_bind *mb_set_depth_deep_parallax_flip_binormal;
		godot_method_bind *mb_set_depth_deep_parallax_flip_tangent;
		godot_method_bind *mb_set_depth_deep_parallax_max_layers;
		godot_method_bind *mb_set_depth_deep_parallax_min_layers;
		godot_method_bind *mb_set_depth_draw_mode;
		godot_method_bind *mb_set_depth_scale;
		godot_method_bind *mb_set_detail_blend_mode;
		godot_method_bind *mb_set_detail_uv;
		godot_method_bind *mb_set_diffuse_mode;
		godot_method_bind *mb_set_distance_fade;
		godot_method_bind *mb_set_distance_fade_max_distance;
		godot_method_bind *mb_set_distance_fade_min_distance;
		godot_method_bind *mb_set_emission;
		godot_method_bind *mb_set_emission_energy;
		godot_method_bind *mb_set_emission_operator;
		godot_method_bind *mb_set_feature;
		godot_method_bind *mb_set_flag;
		godot_method_bind *mb_set_grow;
		godot_method_bind *mb_set_grow_enabled;
		godot_method_bind *mb_set_line_width;
		godot_method_bind *mb_set_metallic;
		godot_method_bind *mb_set_metallic_texture_channel;
		godot_method_bind *mb_set_normal_scale;
		godot_method_bind *mb_set_particles_anim_h_frames;
		godot_method_bind *mb_set_particles_anim_loop;
		godot_method_bind *mb_set_particles_anim_v_frames;
		godot_method_bind *mb_set_point_size;
		godot_method_bind *mb_set_proximity_fade;
		godot_method_bind *mb_set_proximity_fade_distance;
		godot_method_bind *mb_set_refraction;
		godot_method_bind *mb_set_refraction_texture_channel;
		godot_method_bind *mb_set_rim;
		godot_method_bind *mb_set_rim_tint;
		godot_method_bind *mb_set_roughness;
		godot_method_bind *mb_set_roughness_texture_channel;
		godot_method_bind *mb_set_specular;
		godot_method_bind *mb_set_specular_mode;
		godot_method_bind *mb_set_subsurface_scattering_strength;
		godot_method_bind *mb_set_texture;
		godot_method_bind *mb_set_transmission;
		godot_method_bind *mb_set_uv1_offset;
		godot_method_bind *mb_set_uv1_scale;
		godot_method_bind *mb_set_uv1_triplanar_blend_sharpness;
		godot_method_bind *mb_set_uv2_offset;
		godot_method_bind *mb_set_uv2_scale;
		godot_method_bind *mb_set_uv2_triplanar_blend_sharpness;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "SpatialMaterial"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum EmissionOperator {
		EMISSION_OP_ADD = 0,
		EMISSION_OP_MULTIPLY = 1,
	};
	enum DiffuseMode {
		DIFFUSE_BURLEY = 0,
		DIFFUSE_LAMBERT = 1,
		DIFFUSE_LAMBERT_WRAP = 2,
		DIFFUSE_OREN_NAYAR = 3,
		DIFFUSE_TOON = 4,
	};
	enum SpecularMode {
		SPECULAR_SCHLICK_GGX = 0,
		SPECULAR_BLINN = 1,
		SPECULAR_PHONG = 2,
		SPECULAR_TOON = 3,
		SPECULAR_DISABLED = 4,
	};
	enum Feature {
		FEATURE_TRANSPARENT = 0,
		FEATURE_EMISSION = 1,
		FEATURE_NORMAL_MAPPING = 2,
		FEATURE_RIM = 3,
		FEATURE_CLEARCOAT = 4,
		FEATURE_ANISOTROPY = 5,
		FEATURE_AMBIENT_OCCLUSION = 6,
		FEATURE_DEPTH_MAPPING = 7,
		FEATURE_SUBSURACE_SCATTERING = 8,
		FEATURE_TRANSMISSION = 9,
		FEATURE_REFRACTION = 10,
		FEATURE_DETAIL = 11,
		FEATURE_MAX = 12,
	};
	enum Flags {
		FLAG_UNSHADED = 0,
		FLAG_USE_VERTEX_LIGHTING = 1,
		FLAG_DISABLE_DEPTH_TEST = 2,
		FLAG_ALBEDO_FROM_VERTEX_COLOR = 3,
		FLAG_SRGB_VERTEX_COLOR = 4,
		FLAG_USE_POINT_SIZE = 5,
		FLAG_FIXED_SIZE = 6,
		FLAG_BILLBOARD_KEEP_SCALE = 7,
		FLAG_UV1_USE_TRIPLANAR = 8,
		FLAG_UV2_USE_TRIPLANAR = 9,
		FLAG_TRIPLANAR_USE_WORLD = 10,
		FLAG_AO_ON_UV2 = 11,
		FLAG_EMISSION_ON_UV2 = 12,
		FLAG_USE_ALPHA_SCISSOR = 13,
		FLAG_ALBEDO_TEXTURE_FORCE_SRGB = 14,
		FLAG_DONT_RECEIVE_SHADOWS = 15,
		FLAG_ENSURE_CORRECT_NORMALS = 16,
		FLAG_DISABLE_AMBIENT_LIGHT = 17,
		FLAG_USE_SHADOW_TO_OPACITY = 18,
		FLAG_MAX = 19,
	};
	enum CullMode {
		CULL_BACK = 0,
		CULL_FRONT = 1,
		CULL_DISABLED = 2,
	};
	enum DetailUV {
		DETAIL_UV_1 = 0,
		DETAIL_UV_2 = 1,
	};
	enum DistanceFadeMode {
		DISTANCE_FADE_DISABLED = 0,
		DISTANCE_FADE_PIXEL_ALPHA = 1,
		DISTANCE_FADE_PIXEL_DITHER = 2,
		DISTANCE_FADE_OBJECT_DITHER = 3,
	};
	enum BillboardMode {
		BILLBOARD_DISABLED = 0,
		BILLBOARD_ENABLED = 1,
		BILLBOARD_FIXED_Y = 2,
		BILLBOARD_PARTICLES = 3,
	};
	enum DepthDrawMode {
		DEPTH_DRAW_OPAQUE_ONLY = 0,
		DEPTH_DRAW_ALWAYS = 1,
		DEPTH_DRAW_DISABLED = 2,
		DEPTH_DRAW_ALPHA_OPAQUE_PREPASS = 3,
	};
	enum TextureChannel {
		TEXTURE_CHANNEL_RED = 0,
		TEXTURE_CHANNEL_GREEN = 1,
		TEXTURE_CHANNEL_BLUE = 2,
		TEXTURE_CHANNEL_ALPHA = 3,
		TEXTURE_CHANNEL_GRAYSCALE = 4,
	};
	enum BlendMode {
		BLEND_MODE_MIX = 0,
		BLEND_MODE_ADD = 1,
		BLEND_MODE_SUB = 2,
		BLEND_MODE_MUL = 3,
	};
	enum TextureParam {
		TEXTURE_ALBEDO = 0,
		TEXTURE_METALLIC = 1,
		TEXTURE_ROUGHNESS = 2,
		TEXTURE_EMISSION = 3,
		TEXTURE_NORMAL = 4,
		TEXTURE_RIM = 5,
		TEXTURE_CLEARCOAT = 6,
		TEXTURE_FLOWMAP = 7,
		TEXTURE_AMBIENT_OCCLUSION = 8,
		TEXTURE_DEPTH = 9,
		TEXTURE_SUBSURFACE_SCATTERING = 10,
		TEXTURE_TRANSMISSION = 11,
		TEXTURE_REFRACTION = 12,
		TEXTURE_DETAIL_MASK = 13,
		TEXTURE_DETAIL_ALBEDO = 14,
		TEXTURE_DETAIL_NORMAL = 15,
		TEXTURE_MAX = 16,
	};

	// constants


	static SpatialMaterial *_new();

	// methods
	Color get_albedo() const;
	real_t get_alpha_scissor_threshold() const;
	real_t get_anisotropy() const;
	real_t get_ao_light_affect() const;
	SpatialMaterial::TextureChannel get_ao_texture_channel() const;
	SpatialMaterial::BillboardMode get_billboard_mode() const;
	SpatialMaterial::BlendMode get_blend_mode() const;
	real_t get_clearcoat() const;
	real_t get_clearcoat_gloss() const;
	SpatialMaterial::CullMode get_cull_mode() const;
	bool get_depth_deep_parallax_flip_binormal() const;
	bool get_depth_deep_parallax_flip_tangent() const;
	int64_t get_depth_deep_parallax_max_layers() const;
	int64_t get_depth_deep_parallax_min_layers() const;
	SpatialMaterial::DepthDrawMode get_depth_draw_mode() const;
	real_t get_depth_scale() const;
	SpatialMaterial::BlendMode get_detail_blend_mode() const;
	SpatialMaterial::DetailUV get_detail_uv() const;
	SpatialMaterial::DiffuseMode get_diffuse_mode() const;
	SpatialMaterial::DistanceFadeMode get_distance_fade() const;
	real_t get_distance_fade_max_distance() const;
	real_t get_distance_fade_min_distance() const;
	Color get_emission() const;
	real_t get_emission_energy() const;
	SpatialMaterial::EmissionOperator get_emission_operator() const;
	bool get_feature(const int64_t feature) const;
	bool get_flag(const int64_t flag) const;
	real_t get_grow() const;
	real_t get_line_width() const;
	real_t get_metallic() const;
	SpatialMaterial::TextureChannel get_metallic_texture_channel() const;
	real_t get_normal_scale() const;
	int64_t get_particles_anim_h_frames() const;
	bool get_particles_anim_loop() const;
	int64_t get_particles_anim_v_frames() const;
	real_t get_point_size() const;
	real_t get_proximity_fade_distance() const;
	real_t get_refraction() const;
	SpatialMaterial::TextureChannel get_refraction_texture_channel() const;
	real_t get_rim() const;
	real_t get_rim_tint() const;
	real_t get_roughness() const;
	SpatialMaterial::TextureChannel get_roughness_texture_channel() const;
	real_t get_specular() const;
	SpatialMaterial::SpecularMode get_specular_mode() const;
	real_t get_subsurface_scattering_strength() const;
	Ref<Texture> get_texture(const int64_t param) const;
	Color get_transmission() const;
	Vector3 get_uv1_offset() const;
	Vector3 get_uv1_scale() const;
	real_t get_uv1_triplanar_blend_sharpness() const;
	Vector3 get_uv2_offset() const;
	Vector3 get_uv2_scale() const;
	real_t get_uv2_triplanar_blend_sharpness() const;
	bool is_depth_deep_parallax_enabled() const;
	bool is_grow_enabled() const;
	bool is_proximity_fade_enabled() const;
	void set_albedo(const Color albedo);
	void set_alpha_scissor_threshold(const real_t threshold);
	void set_anisotropy(const real_t anisotropy);
	void set_ao_light_affect(const real_t amount);
	void set_ao_texture_channel(const int64_t channel);
	void set_billboard_mode(const int64_t mode);
	void set_blend_mode(const int64_t blend_mode);
	void set_clearcoat(const real_t clearcoat);
	void set_clearcoat_gloss(const real_t clearcoat_gloss);
	void set_cull_mode(const int64_t cull_mode);
	void set_depth_deep_parallax(const bool enable);
	void set_depth_deep_parallax_flip_binormal(const bool flip);
	void set_depth_deep_parallax_flip_tangent(const bool flip);
	void set_depth_deep_parallax_max_layers(const int64_t layer);
	void set_depth_deep_parallax_min_layers(const int64_t layer);
	void set_depth_draw_mode(const int64_t depth_draw_mode);
	void set_depth_scale(const real_t depth_scale);
	void set_detail_blend_mode(const int64_t detail_blend_mode);
	void set_detail_uv(const int64_t detail_uv);
	void set_diffuse_mode(const int64_t diffuse_mode);
	void set_distance_fade(const int64_t mode);
	void set_distance_fade_max_distance(const real_t distance);
	void set_distance_fade_min_distance(const real_t distance);
	void set_emission(const Color emission);
	void set_emission_energy(const real_t emission_energy);
	void set_emission_operator(const int64_t _operator);
	void set_feature(const int64_t feature, const bool enable);
	void set_flag(const int64_t flag, const bool enable);
	void set_grow(const real_t amount);
	void set_grow_enabled(const bool enable);
	void set_line_width(const real_t line_width);
	void set_metallic(const real_t metallic);
	void set_metallic_texture_channel(const int64_t channel);
	void set_normal_scale(const real_t normal_scale);
	void set_particles_anim_h_frames(const int64_t frames);
	void set_particles_anim_loop(const bool loop);
	void set_particles_anim_v_frames(const int64_t frames);
	void set_point_size(const real_t point_size);
	void set_proximity_fade(const bool enabled);
	void set_proximity_fade_distance(const real_t distance);
	void set_refraction(const real_t refraction);
	void set_refraction_texture_channel(const int64_t channel);
	void set_rim(const real_t rim);
	void set_rim_tint(const real_t rim_tint);
	void set_roughness(const real_t roughness);
	void set_roughness_texture_channel(const int64_t channel);
	void set_specular(const real_t specular);
	void set_specular_mode(const int64_t specular_mode);
	void set_subsurface_scattering_strength(const real_t strength);
	void set_texture(const int64_t param, const Ref<Texture> texture);
	void set_transmission(const Color transmission);
	void set_uv1_offset(const Vector3 offset);
	void set_uv1_scale(const Vector3 scale);
	void set_uv1_triplanar_blend_sharpness(const real_t sharpness);
	void set_uv2_offset(const Vector3 offset);
	void set_uv2_scale(const Vector3 scale);
	void set_uv2_triplanar_blend_sharpness(const real_t sharpness);

};

}

#endif