#ifndef GODOT_CPP_VISUALINSTANCE_HPP
#define GODOT_CPP_VISUALINSTANCE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Spatial.hpp"
namespace godot {


class VisualInstance : public Spatial {
	struct ___method_bindings {
		godot_method_bind *mb__get_visual_instance_rid;
		godot_method_bind *mb_get_aabb;
		godot_method_bind *mb_get_base;
		godot_method_bind *mb_get_instance;
		godot_method_bind *mb_get_layer_mask;
		godot_method_bind *mb_get_layer_mask_bit;
		godot_method_bind *mb_get_transformed_aabb;
		godot_method_bind *mb_set_base;
		godot_method_bind *mb_set_layer_mask;
		godot_method_bind *mb_set_layer_mask_bit;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualInstance"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	RID _get_visual_instance_rid() const;
	AABB get_aabb() const;
	RID get_base() const;
	RID get_instance() const;
	int64_t get_layer_mask() const;
	bool get_layer_mask_bit(const int64_t layer) const;
	AABB get_transformed_aabb() const;
	void set_base(const RID base);
	void set_layer_mask(const int64_t mask);
	void set_layer_mask_bit(const int64_t layer, const bool enabled);

};

}

#endif