#ifndef GODOT_CPP_REFLECTIONPROBE_HPP
#define GODOT_CPP_REFLECTIONPROBE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "ReflectionProbe.hpp"

#include "VisualInstance.hpp"
namespace godot {


class ReflectionProbe : public VisualInstance {
	struct ___method_bindings {
		godot_method_bind *mb_are_shadows_enabled;
		godot_method_bind *mb_get_cull_mask;
		godot_method_bind *mb_get_extents;
		godot_method_bind *mb_get_intensity;
		godot_method_bind *mb_get_interior_ambient;
		godot_method_bind *mb_get_interior_ambient_energy;
		godot_method_bind *mb_get_interior_ambient_probe_contribution;
		godot_method_bind *mb_get_max_distance;
		godot_method_bind *mb_get_origin_offset;
		godot_method_bind *mb_get_update_mode;
		godot_method_bind *mb_is_box_projection_enabled;
		godot_method_bind *mb_is_set_as_interior;
		godot_method_bind *mb_set_as_interior;
		godot_method_bind *mb_set_cull_mask;
		godot_method_bind *mb_set_enable_box_projection;
		godot_method_bind *mb_set_enable_shadows;
		godot_method_bind *mb_set_extents;
		godot_method_bind *mb_set_intensity;
		godot_method_bind *mb_set_interior_ambient;
		godot_method_bind *mb_set_interior_ambient_energy;
		godot_method_bind *mb_set_interior_ambient_probe_contribution;
		godot_method_bind *mb_set_max_distance;
		godot_method_bind *mb_set_origin_offset;
		godot_method_bind *mb_set_update_mode;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ReflectionProbe"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum UpdateMode {
		UPDATE_ONCE = 0,
		UPDATE_ALWAYS = 1,
	};

	// constants


	static ReflectionProbe *_new();

	// methods
	bool are_shadows_enabled() const;
	int64_t get_cull_mask() const;
	Vector3 get_extents() const;
	real_t get_intensity() const;
	Color get_interior_ambient() const;
	real_t get_interior_ambient_energy() const;
	real_t get_interior_ambient_probe_contribution() const;
	real_t get_max_distance() const;
	Vector3 get_origin_offset() const;
	ReflectionProbe::UpdateMode get_update_mode() const;
	bool is_box_projection_enabled() const;
	bool is_set_as_interior() const;
	void set_as_interior(const bool enable);
	void set_cull_mask(const int64_t layers);
	void set_enable_box_projection(const bool enable);
	void set_enable_shadows(const bool enable);
	void set_extents(const Vector3 extents);
	void set_intensity(const real_t intensity);
	void set_interior_ambient(const Color ambient);
	void set_interior_ambient_energy(const real_t ambient_energy);
	void set_interior_ambient_probe_contribution(const real_t ambient_probe_contribution);
	void set_max_distance(const real_t max_distance);
	void set_origin_offset(const Vector3 origin_offset);
	void set_update_mode(const int64_t mode);

};

}

#endif