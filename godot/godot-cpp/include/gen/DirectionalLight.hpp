#ifndef GODOT_CPP_DIRECTIONALLIGHT_HPP
#define GODOT_CPP_DIRECTIONALLIGHT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "DirectionalLight.hpp"

#include "Light.hpp"
namespace godot {


class DirectionalLight : public Light {
	struct ___method_bindings {
		godot_method_bind *mb_get_shadow_depth_range;
		godot_method_bind *mb_get_shadow_mode;
		godot_method_bind *mb_is_blend_splits_enabled;
		godot_method_bind *mb_set_blend_splits;
		godot_method_bind *mb_set_shadow_depth_range;
		godot_method_bind *mb_set_shadow_mode;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "DirectionalLight"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum ShadowMode {
		SHADOW_ORTHOGONAL = 0,
		SHADOW_PARALLEL_2_SPLITS = 1,
		SHADOW_PARALLEL_4_SPLITS = 2,
	};
	enum ShadowDepthRange {
		SHADOW_DEPTH_RANGE_STABLE = 0,
		SHADOW_DEPTH_RANGE_OPTIMIZED = 1,
	};

	// constants


	static DirectionalLight *_new();

	// methods
	DirectionalLight::ShadowDepthRange get_shadow_depth_range() const;
	DirectionalLight::ShadowMode get_shadow_mode() const;
	bool is_blend_splits_enabled() const;
	void set_blend_splits(const bool enabled);
	void set_shadow_depth_range(const int64_t mode);
	void set_shadow_mode(const int64_t mode);

};

}

#endif