#ifndef GODOT_CPP_CANVASITEMMATERIAL_HPP
#define GODOT_CPP_CANVASITEMMATERIAL_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "CanvasItemMaterial.hpp"

#include "Material.hpp"
namespace godot {


class CanvasItemMaterial : public Material {
	struct ___method_bindings {
		godot_method_bind *mb_get_blend_mode;
		godot_method_bind *mb_get_light_mode;
		godot_method_bind *mb_get_particles_anim_h_frames;
		godot_method_bind *mb_get_particles_anim_loop;
		godot_method_bind *mb_get_particles_anim_v_frames;
		godot_method_bind *mb_get_particles_animation;
		godot_method_bind *mb_set_blend_mode;
		godot_method_bind *mb_set_light_mode;
		godot_method_bind *mb_set_particles_anim_h_frames;
		godot_method_bind *mb_set_particles_anim_loop;
		godot_method_bind *mb_set_particles_anim_v_frames;
		godot_method_bind *mb_set_particles_animation;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CanvasItemMaterial"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum LightMode {
		LIGHT_MODE_NORMAL = 0,
		LIGHT_MODE_UNSHADED = 1,
		LIGHT_MODE_LIGHT_ONLY = 2,
	};
	enum BlendMode {
		BLEND_MODE_MIX = 0,
		BLEND_MODE_ADD = 1,
		BLEND_MODE_SUB = 2,
		BLEND_MODE_MUL = 3,
		BLEND_MODE_PREMULT_ALPHA = 4,
	};

	// constants


	static CanvasItemMaterial *_new();

	// methods
	CanvasItemMaterial::BlendMode get_blend_mode() const;
	CanvasItemMaterial::LightMode get_light_mode() const;
	int64_t get_particles_anim_h_frames() const;
	bool get_particles_anim_loop() const;
	int64_t get_particles_anim_v_frames() const;
	bool get_particles_animation() const;
	void set_blend_mode(const int64_t blend_mode);
	void set_light_mode(const int64_t light_mode);
	void set_particles_anim_h_frames(const int64_t frames);
	void set_particles_anim_loop(const bool loop);
	void set_particles_anim_v_frames(const int64_t frames);
	void set_particles_animation(const bool particles_anim);

};

}

#endif