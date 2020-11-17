#ifndef GODOT_CPP_GIPROBE_HPP
#define GODOT_CPP_GIPROBE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "GIProbe.hpp"

#include "VisualInstance.hpp"
namespace godot {

class Node;
class GIProbeData;

class GIProbe : public VisualInstance {
	struct ___method_bindings {
		godot_method_bind *mb_bake;
		godot_method_bind *mb_debug_bake;
		godot_method_bind *mb_get_bias;
		godot_method_bind *mb_get_dynamic_range;
		godot_method_bind *mb_get_energy;
		godot_method_bind *mb_get_extents;
		godot_method_bind *mb_get_normal_bias;
		godot_method_bind *mb_get_probe_data;
		godot_method_bind *mb_get_propagation;
		godot_method_bind *mb_get_subdiv;
		godot_method_bind *mb_is_compressed;
		godot_method_bind *mb_is_interior;
		godot_method_bind *mb_set_bias;
		godot_method_bind *mb_set_compress;
		godot_method_bind *mb_set_dynamic_range;
		godot_method_bind *mb_set_energy;
		godot_method_bind *mb_set_extents;
		godot_method_bind *mb_set_interior;
		godot_method_bind *mb_set_normal_bias;
		godot_method_bind *mb_set_probe_data;
		godot_method_bind *mb_set_propagation;
		godot_method_bind *mb_set_subdiv;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "GIProbe"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Subdiv {
		SUBDIV_64 = 0,
		SUBDIV_128 = 1,
		SUBDIV_256 = 2,
		SUBDIV_512 = 3,
		SUBDIV_MAX = 4,
	};

	// constants


	static GIProbe *_new();

	// methods
	void bake(const Node *from_node = nullptr, const bool create_visual_debug = false);
	void debug_bake();
	real_t get_bias() const;
	int64_t get_dynamic_range() const;
	real_t get_energy() const;
	Vector3 get_extents() const;
	real_t get_normal_bias() const;
	Ref<GIProbeData> get_probe_data() const;
	real_t get_propagation() const;
	GIProbe::Subdiv get_subdiv() const;
	bool is_compressed() const;
	bool is_interior() const;
	void set_bias(const real_t max);
	void set_compress(const bool enable);
	void set_dynamic_range(const int64_t max);
	void set_energy(const real_t max);
	void set_extents(const Vector3 extents);
	void set_interior(const bool enable);
	void set_normal_bias(const real_t max);
	void set_probe_data(const Ref<GIProbeData> data);
	void set_propagation(const real_t max);
	void set_subdiv(const int64_t subdiv);

};

}

#endif