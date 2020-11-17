#ifndef GODOT_CPP_PARTICLES_HPP
#define GODOT_CPP_PARTICLES_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Particles.hpp"

#include "GeometryInstance.hpp"
namespace godot {

class Mesh;
class Material;

class Particles : public GeometryInstance {
	struct ___method_bindings {
		godot_method_bind *mb_capture_aabb;
		godot_method_bind *mb_get_amount;
		godot_method_bind *mb_get_draw_order;
		godot_method_bind *mb_get_draw_pass_mesh;
		godot_method_bind *mb_get_draw_passes;
		godot_method_bind *mb_get_explosiveness_ratio;
		godot_method_bind *mb_get_fixed_fps;
		godot_method_bind *mb_get_fractional_delta;
		godot_method_bind *mb_get_lifetime;
		godot_method_bind *mb_get_one_shot;
		godot_method_bind *mb_get_pre_process_time;
		godot_method_bind *mb_get_process_material;
		godot_method_bind *mb_get_randomness_ratio;
		godot_method_bind *mb_get_speed_scale;
		godot_method_bind *mb_get_use_local_coordinates;
		godot_method_bind *mb_get_visibility_aabb;
		godot_method_bind *mb_is_emitting;
		godot_method_bind *mb_restart;
		godot_method_bind *mb_set_amount;
		godot_method_bind *mb_set_draw_order;
		godot_method_bind *mb_set_draw_pass_mesh;
		godot_method_bind *mb_set_draw_passes;
		godot_method_bind *mb_set_emitting;
		godot_method_bind *mb_set_explosiveness_ratio;
		godot_method_bind *mb_set_fixed_fps;
		godot_method_bind *mb_set_fractional_delta;
		godot_method_bind *mb_set_lifetime;
		godot_method_bind *mb_set_one_shot;
		godot_method_bind *mb_set_pre_process_time;
		godot_method_bind *mb_set_process_material;
		godot_method_bind *mb_set_randomness_ratio;
		godot_method_bind *mb_set_speed_scale;
		godot_method_bind *mb_set_use_local_coordinates;
		godot_method_bind *mb_set_visibility_aabb;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Particles"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum DrawOrder {
		DRAW_ORDER_INDEX = 0,
		DRAW_ORDER_LIFETIME = 1,
		DRAW_ORDER_VIEW_DEPTH = 2,
	};

	// constants
	const static int MAX_DRAW_PASSES = 4;


	static Particles *_new();

	// methods
	AABB capture_aabb() const;
	int64_t get_amount() const;
	Particles::DrawOrder get_draw_order() const;
	Ref<Mesh> get_draw_pass_mesh(const int64_t pass) const;
	int64_t get_draw_passes() const;
	real_t get_explosiveness_ratio() const;
	int64_t get_fixed_fps() const;
	bool get_fractional_delta() const;
	real_t get_lifetime() const;
	bool get_one_shot() const;
	real_t get_pre_process_time() const;
	Ref<Material> get_process_material() const;
	real_t get_randomness_ratio() const;
	real_t get_speed_scale() const;
	bool get_use_local_coordinates() const;
	AABB get_visibility_aabb() const;
	bool is_emitting() const;
	void restart();
	void set_amount(const int64_t amount);
	void set_draw_order(const int64_t order);
	void set_draw_pass_mesh(const int64_t pass, const Ref<Mesh> mesh);
	void set_draw_passes(const int64_t passes);
	void set_emitting(const bool emitting);
	void set_explosiveness_ratio(const real_t ratio);
	void set_fixed_fps(const int64_t fps);
	void set_fractional_delta(const bool enable);
	void set_lifetime(const real_t secs);
	void set_one_shot(const bool enable);
	void set_pre_process_time(const real_t secs);
	void set_process_material(const Ref<Material> material);
	void set_randomness_ratio(const real_t ratio);
	void set_speed_scale(const real_t scale);
	void set_use_local_coordinates(const bool enable);
	void set_visibility_aabb(const AABB aabb);

};

}

#endif