#ifndef GODOT_CPP_BAKEDLIGHTMAP_HPP
#define GODOT_CPP_BAKEDLIGHTMAP_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "BakedLightmap.hpp"

#include "VisualInstance.hpp"
namespace godot {

class Node;
class BakedLightmapData;

class BakedLightmap : public VisualInstance {
	struct ___method_bindings {
		godot_method_bind *mb_bake;
		godot_method_bind *mb_debug_bake;
		godot_method_bind *mb_get_bake_cell_size;
		godot_method_bind *mb_get_bake_default_texels_per_unit;
		godot_method_bind *mb_get_bake_mode;
		godot_method_bind *mb_get_bake_quality;
		godot_method_bind *mb_get_capture_cell_size;
		godot_method_bind *mb_get_energy;
		godot_method_bind *mb_get_extents;
		godot_method_bind *mb_get_image_path;
		godot_method_bind *mb_get_light_data;
		godot_method_bind *mb_get_propagation;
		godot_method_bind *mb_is_hdr;
		godot_method_bind *mb_set_bake_cell_size;
		godot_method_bind *mb_set_bake_default_texels_per_unit;
		godot_method_bind *mb_set_bake_mode;
		godot_method_bind *mb_set_bake_quality;
		godot_method_bind *mb_set_capture_cell_size;
		godot_method_bind *mb_set_energy;
		godot_method_bind *mb_set_extents;
		godot_method_bind *mb_set_hdr;
		godot_method_bind *mb_set_image_path;
		godot_method_bind *mb_set_light_data;
		godot_method_bind *mb_set_propagation;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "BakedLightmap"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum BakeQuality {
		BAKE_QUALITY_LOW = 0,
		BAKE_QUALITY_MEDIUM = 1,
		BAKE_QUALITY_HIGH = 2,
	};
	enum BakeError {
		BAKE_ERROR_OK = 0,
		BAKE_ERROR_NO_SAVE_PATH = 1,
		BAKE_ERROR_NO_MESHES = 2,
		BAKE_ERROR_CANT_CREATE_IMAGE = 3,
		BAKE_ERROR_USER_ABORTED = 4,
	};
	enum BakeMode {
		BAKE_MODE_CONE_TRACE = 0,
		BAKE_MODE_RAY_TRACE = 1,
	};

	// constants


	static BakedLightmap *_new();

	// methods
	BakedLightmap::BakeError bake(const Node *from_node = nullptr, const bool create_visual_debug = false);
	void debug_bake();
	real_t get_bake_cell_size() const;
	real_t get_bake_default_texels_per_unit() const;
	BakedLightmap::BakeMode get_bake_mode() const;
	BakedLightmap::BakeQuality get_bake_quality() const;
	real_t get_capture_cell_size() const;
	real_t get_energy() const;
	Vector3 get_extents() const;
	String get_image_path() const;
	Ref<BakedLightmapData> get_light_data() const;
	real_t get_propagation() const;
	bool is_hdr() const;
	void set_bake_cell_size(const real_t bake_cell_size);
	void set_bake_default_texels_per_unit(const real_t texels);
	void set_bake_mode(const int64_t bake_mode);
	void set_bake_quality(const int64_t bake_quality);
	void set_capture_cell_size(const real_t capture_cell_size);
	void set_energy(const real_t energy);
	void set_extents(const Vector3 extents);
	void set_hdr(const bool hdr);
	void set_image_path(const String image_path);
	void set_light_data(const Ref<BakedLightmapData> data);
	void set_propagation(const real_t propagation);

};

}

#endif