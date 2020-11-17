#ifndef GODOT_CPP_LIGHT2D_HPP
#define GODOT_CPP_LIGHT2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Light2D.hpp"

#include "Node2D.hpp"
namespace godot {

class Texture;

class Light2D : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb_get_color;
		godot_method_bind *mb_get_energy;
		godot_method_bind *mb_get_height;
		godot_method_bind *mb_get_item_cull_mask;
		godot_method_bind *mb_get_item_shadow_cull_mask;
		godot_method_bind *mb_get_layer_range_max;
		godot_method_bind *mb_get_layer_range_min;
		godot_method_bind *mb_get_mode;
		godot_method_bind *mb_get_shadow_buffer_size;
		godot_method_bind *mb_get_shadow_color;
		godot_method_bind *mb_get_shadow_filter;
		godot_method_bind *mb_get_shadow_gradient_length;
		godot_method_bind *mb_get_shadow_smooth;
		godot_method_bind *mb_get_texture;
		godot_method_bind *mb_get_texture_offset;
		godot_method_bind *mb_get_texture_scale;
		godot_method_bind *mb_get_z_range_max;
		godot_method_bind *mb_get_z_range_min;
		godot_method_bind *mb_is_editor_only;
		godot_method_bind *mb_is_enabled;
		godot_method_bind *mb_is_shadow_enabled;
		godot_method_bind *mb_set_color;
		godot_method_bind *mb_set_editor_only;
		godot_method_bind *mb_set_enabled;
		godot_method_bind *mb_set_energy;
		godot_method_bind *mb_set_height;
		godot_method_bind *mb_set_item_cull_mask;
		godot_method_bind *mb_set_item_shadow_cull_mask;
		godot_method_bind *mb_set_layer_range_max;
		godot_method_bind *mb_set_layer_range_min;
		godot_method_bind *mb_set_mode;
		godot_method_bind *mb_set_shadow_buffer_size;
		godot_method_bind *mb_set_shadow_color;
		godot_method_bind *mb_set_shadow_enabled;
		godot_method_bind *mb_set_shadow_filter;
		godot_method_bind *mb_set_shadow_gradient_length;
		godot_method_bind *mb_set_shadow_smooth;
		godot_method_bind *mb_set_texture;
		godot_method_bind *mb_set_texture_offset;
		godot_method_bind *mb_set_texture_scale;
		godot_method_bind *mb_set_z_range_max;
		godot_method_bind *mb_set_z_range_min;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Light2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum ShadowFilter {
		SHADOW_FILTER_NONE = 0,
		SHADOW_FILTER_PCF3 = 1,
		SHADOW_FILTER_PCF5 = 2,
		SHADOW_FILTER_PCF7 = 3,
		SHADOW_FILTER_PCF9 = 4,
		SHADOW_FILTER_PCF13 = 5,
	};
	enum Mode {
		MODE_ADD = 0,
		MODE_SUB = 1,
		MODE_MIX = 2,
		MODE_MASK = 3,
	};

	// constants


	static Light2D *_new();

	// methods
	Color get_color() const;
	real_t get_energy() const;
	real_t get_height() const;
	int64_t get_item_cull_mask() const;
	int64_t get_item_shadow_cull_mask() const;
	int64_t get_layer_range_max() const;
	int64_t get_layer_range_min() const;
	Light2D::Mode get_mode() const;
	int64_t get_shadow_buffer_size() const;
	Color get_shadow_color() const;
	Light2D::ShadowFilter get_shadow_filter() const;
	real_t get_shadow_gradient_length() const;
	real_t get_shadow_smooth() const;
	Ref<Texture> get_texture() const;
	Vector2 get_texture_offset() const;
	real_t get_texture_scale() const;
	int64_t get_z_range_max() const;
	int64_t get_z_range_min() const;
	bool is_editor_only() const;
	bool is_enabled() const;
	bool is_shadow_enabled() const;
	void set_color(const Color color);
	void set_editor_only(const bool editor_only);
	void set_enabled(const bool enabled);
	void set_energy(const real_t energy);
	void set_height(const real_t height);
	void set_item_cull_mask(const int64_t item_cull_mask);
	void set_item_shadow_cull_mask(const int64_t item_shadow_cull_mask);
	void set_layer_range_max(const int64_t layer);
	void set_layer_range_min(const int64_t layer);
	void set_mode(const int64_t mode);
	void set_shadow_buffer_size(const int64_t size);
	void set_shadow_color(const Color shadow_color);
	void set_shadow_enabled(const bool enabled);
	void set_shadow_filter(const int64_t filter);
	void set_shadow_gradient_length(const real_t multiplier);
	void set_shadow_smooth(const real_t smooth);
	void set_texture(const Ref<Texture> texture);
	void set_texture_offset(const Vector2 texture_offset);
	void set_texture_scale(const real_t texture_scale);
	void set_z_range_max(const int64_t z);
	void set_z_range_min(const int64_t z);

};

}

#endif