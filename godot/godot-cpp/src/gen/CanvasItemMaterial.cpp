#include "CanvasItemMaterial.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


CanvasItemMaterial::___method_bindings CanvasItemMaterial::___mb = {};

void CanvasItemMaterial::___init_method_bindings() {
	___mb.mb_get_blend_mode = godot::api->godot_method_bind_get_method("CanvasItemMaterial", "get_blend_mode");
	___mb.mb_get_light_mode = godot::api->godot_method_bind_get_method("CanvasItemMaterial", "get_light_mode");
	___mb.mb_get_particles_anim_h_frames = godot::api->godot_method_bind_get_method("CanvasItemMaterial", "get_particles_anim_h_frames");
	___mb.mb_get_particles_anim_loop = godot::api->godot_method_bind_get_method("CanvasItemMaterial", "get_particles_anim_loop");
	___mb.mb_get_particles_anim_v_frames = godot::api->godot_method_bind_get_method("CanvasItemMaterial", "get_particles_anim_v_frames");
	___mb.mb_get_particles_animation = godot::api->godot_method_bind_get_method("CanvasItemMaterial", "get_particles_animation");
	___mb.mb_set_blend_mode = godot::api->godot_method_bind_get_method("CanvasItemMaterial", "set_blend_mode");
	___mb.mb_set_light_mode = godot::api->godot_method_bind_get_method("CanvasItemMaterial", "set_light_mode");
	___mb.mb_set_particles_anim_h_frames = godot::api->godot_method_bind_get_method("CanvasItemMaterial", "set_particles_anim_h_frames");
	___mb.mb_set_particles_anim_loop = godot::api->godot_method_bind_get_method("CanvasItemMaterial", "set_particles_anim_loop");
	___mb.mb_set_particles_anim_v_frames = godot::api->godot_method_bind_get_method("CanvasItemMaterial", "set_particles_anim_v_frames");
	___mb.mb_set_particles_animation = godot::api->godot_method_bind_get_method("CanvasItemMaterial", "set_particles_animation");
}

CanvasItemMaterial *CanvasItemMaterial::_new()
{
	return (CanvasItemMaterial *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CanvasItemMaterial")());
}
CanvasItemMaterial::BlendMode CanvasItemMaterial::get_blend_mode() const {
	return (CanvasItemMaterial::BlendMode) ___godot_icall_int(___mb.mb_get_blend_mode, (const Object *) this);
}

CanvasItemMaterial::LightMode CanvasItemMaterial::get_light_mode() const {
	return (CanvasItemMaterial::LightMode) ___godot_icall_int(___mb.mb_get_light_mode, (const Object *) this);
}

int64_t CanvasItemMaterial::get_particles_anim_h_frames() const {
	return ___godot_icall_int(___mb.mb_get_particles_anim_h_frames, (const Object *) this);
}

bool CanvasItemMaterial::get_particles_anim_loop() const {
	return ___godot_icall_bool(___mb.mb_get_particles_anim_loop, (const Object *) this);
}

int64_t CanvasItemMaterial::get_particles_anim_v_frames() const {
	return ___godot_icall_int(___mb.mb_get_particles_anim_v_frames, (const Object *) this);
}

bool CanvasItemMaterial::get_particles_animation() const {
	return ___godot_icall_bool(___mb.mb_get_particles_animation, (const Object *) this);
}

void CanvasItemMaterial::set_blend_mode(const int64_t blend_mode) {
	___godot_icall_void_int(___mb.mb_set_blend_mode, (const Object *) this, blend_mode);
}

void CanvasItemMaterial::set_light_mode(const int64_t light_mode) {
	___godot_icall_void_int(___mb.mb_set_light_mode, (const Object *) this, light_mode);
}

void CanvasItemMaterial::set_particles_anim_h_frames(const int64_t frames) {
	___godot_icall_void_int(___mb.mb_set_particles_anim_h_frames, (const Object *) this, frames);
}

void CanvasItemMaterial::set_particles_anim_loop(const bool loop) {
	___godot_icall_void_bool(___mb.mb_set_particles_anim_loop, (const Object *) this, loop);
}

void CanvasItemMaterial::set_particles_anim_v_frames(const int64_t frames) {
	___godot_icall_void_int(___mb.mb_set_particles_anim_v_frames, (const Object *) this, frames);
}

void CanvasItemMaterial::set_particles_animation(const bool particles_anim) {
	___godot_icall_void_bool(___mb.mb_set_particles_animation, (const Object *) this, particles_anim);
}

}