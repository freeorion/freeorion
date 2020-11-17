#include "ProceduralSky.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Image.hpp"


namespace godot {


ProceduralSky::___method_bindings ProceduralSky::___mb = {};

void ProceduralSky::___init_method_bindings() {
	___mb.mb__thread_done = godot::api->godot_method_bind_get_method("ProceduralSky", "_thread_done");
	___mb.mb__update_sky = godot::api->godot_method_bind_get_method("ProceduralSky", "_update_sky");
	___mb.mb_get_ground_bottom_color = godot::api->godot_method_bind_get_method("ProceduralSky", "get_ground_bottom_color");
	___mb.mb_get_ground_curve = godot::api->godot_method_bind_get_method("ProceduralSky", "get_ground_curve");
	___mb.mb_get_ground_energy = godot::api->godot_method_bind_get_method("ProceduralSky", "get_ground_energy");
	___mb.mb_get_ground_horizon_color = godot::api->godot_method_bind_get_method("ProceduralSky", "get_ground_horizon_color");
	___mb.mb_get_sky_curve = godot::api->godot_method_bind_get_method("ProceduralSky", "get_sky_curve");
	___mb.mb_get_sky_energy = godot::api->godot_method_bind_get_method("ProceduralSky", "get_sky_energy");
	___mb.mb_get_sky_horizon_color = godot::api->godot_method_bind_get_method("ProceduralSky", "get_sky_horizon_color");
	___mb.mb_get_sky_top_color = godot::api->godot_method_bind_get_method("ProceduralSky", "get_sky_top_color");
	___mb.mb_get_sun_angle_max = godot::api->godot_method_bind_get_method("ProceduralSky", "get_sun_angle_max");
	___mb.mb_get_sun_angle_min = godot::api->godot_method_bind_get_method("ProceduralSky", "get_sun_angle_min");
	___mb.mb_get_sun_color = godot::api->godot_method_bind_get_method("ProceduralSky", "get_sun_color");
	___mb.mb_get_sun_curve = godot::api->godot_method_bind_get_method("ProceduralSky", "get_sun_curve");
	___mb.mb_get_sun_energy = godot::api->godot_method_bind_get_method("ProceduralSky", "get_sun_energy");
	___mb.mb_get_sun_latitude = godot::api->godot_method_bind_get_method("ProceduralSky", "get_sun_latitude");
	___mb.mb_get_sun_longitude = godot::api->godot_method_bind_get_method("ProceduralSky", "get_sun_longitude");
	___mb.mb_get_texture_size = godot::api->godot_method_bind_get_method("ProceduralSky", "get_texture_size");
	___mb.mb_set_ground_bottom_color = godot::api->godot_method_bind_get_method("ProceduralSky", "set_ground_bottom_color");
	___mb.mb_set_ground_curve = godot::api->godot_method_bind_get_method("ProceduralSky", "set_ground_curve");
	___mb.mb_set_ground_energy = godot::api->godot_method_bind_get_method("ProceduralSky", "set_ground_energy");
	___mb.mb_set_ground_horizon_color = godot::api->godot_method_bind_get_method("ProceduralSky", "set_ground_horizon_color");
	___mb.mb_set_sky_curve = godot::api->godot_method_bind_get_method("ProceduralSky", "set_sky_curve");
	___mb.mb_set_sky_energy = godot::api->godot_method_bind_get_method("ProceduralSky", "set_sky_energy");
	___mb.mb_set_sky_horizon_color = godot::api->godot_method_bind_get_method("ProceduralSky", "set_sky_horizon_color");
	___mb.mb_set_sky_top_color = godot::api->godot_method_bind_get_method("ProceduralSky", "set_sky_top_color");
	___mb.mb_set_sun_angle_max = godot::api->godot_method_bind_get_method("ProceduralSky", "set_sun_angle_max");
	___mb.mb_set_sun_angle_min = godot::api->godot_method_bind_get_method("ProceduralSky", "set_sun_angle_min");
	___mb.mb_set_sun_color = godot::api->godot_method_bind_get_method("ProceduralSky", "set_sun_color");
	___mb.mb_set_sun_curve = godot::api->godot_method_bind_get_method("ProceduralSky", "set_sun_curve");
	___mb.mb_set_sun_energy = godot::api->godot_method_bind_get_method("ProceduralSky", "set_sun_energy");
	___mb.mb_set_sun_latitude = godot::api->godot_method_bind_get_method("ProceduralSky", "set_sun_latitude");
	___mb.mb_set_sun_longitude = godot::api->godot_method_bind_get_method("ProceduralSky", "set_sun_longitude");
	___mb.mb_set_texture_size = godot::api->godot_method_bind_get_method("ProceduralSky", "set_texture_size");
}

ProceduralSky *ProceduralSky::_new()
{
	return (ProceduralSky *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ProceduralSky")());
}
void ProceduralSky::_thread_done(const Ref<Image> image) {
	___godot_icall_void_Object(___mb.mb__thread_done, (const Object *) this, image.ptr());
}

void ProceduralSky::_update_sky() {
	___godot_icall_void(___mb.mb__update_sky, (const Object *) this);
}

Color ProceduralSky::get_ground_bottom_color() const {
	return ___godot_icall_Color(___mb.mb_get_ground_bottom_color, (const Object *) this);
}

real_t ProceduralSky::get_ground_curve() const {
	return ___godot_icall_float(___mb.mb_get_ground_curve, (const Object *) this);
}

real_t ProceduralSky::get_ground_energy() const {
	return ___godot_icall_float(___mb.mb_get_ground_energy, (const Object *) this);
}

Color ProceduralSky::get_ground_horizon_color() const {
	return ___godot_icall_Color(___mb.mb_get_ground_horizon_color, (const Object *) this);
}

real_t ProceduralSky::get_sky_curve() const {
	return ___godot_icall_float(___mb.mb_get_sky_curve, (const Object *) this);
}

real_t ProceduralSky::get_sky_energy() const {
	return ___godot_icall_float(___mb.mb_get_sky_energy, (const Object *) this);
}

Color ProceduralSky::get_sky_horizon_color() const {
	return ___godot_icall_Color(___mb.mb_get_sky_horizon_color, (const Object *) this);
}

Color ProceduralSky::get_sky_top_color() const {
	return ___godot_icall_Color(___mb.mb_get_sky_top_color, (const Object *) this);
}

real_t ProceduralSky::get_sun_angle_max() const {
	return ___godot_icall_float(___mb.mb_get_sun_angle_max, (const Object *) this);
}

real_t ProceduralSky::get_sun_angle_min() const {
	return ___godot_icall_float(___mb.mb_get_sun_angle_min, (const Object *) this);
}

Color ProceduralSky::get_sun_color() const {
	return ___godot_icall_Color(___mb.mb_get_sun_color, (const Object *) this);
}

real_t ProceduralSky::get_sun_curve() const {
	return ___godot_icall_float(___mb.mb_get_sun_curve, (const Object *) this);
}

real_t ProceduralSky::get_sun_energy() const {
	return ___godot_icall_float(___mb.mb_get_sun_energy, (const Object *) this);
}

real_t ProceduralSky::get_sun_latitude() const {
	return ___godot_icall_float(___mb.mb_get_sun_latitude, (const Object *) this);
}

real_t ProceduralSky::get_sun_longitude() const {
	return ___godot_icall_float(___mb.mb_get_sun_longitude, (const Object *) this);
}

ProceduralSky::TextureSize ProceduralSky::get_texture_size() const {
	return (ProceduralSky::TextureSize) ___godot_icall_int(___mb.mb_get_texture_size, (const Object *) this);
}

void ProceduralSky::set_ground_bottom_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_ground_bottom_color, (const Object *) this, color);
}

void ProceduralSky::set_ground_curve(const real_t curve) {
	___godot_icall_void_float(___mb.mb_set_ground_curve, (const Object *) this, curve);
}

void ProceduralSky::set_ground_energy(const real_t energy) {
	___godot_icall_void_float(___mb.mb_set_ground_energy, (const Object *) this, energy);
}

void ProceduralSky::set_ground_horizon_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_ground_horizon_color, (const Object *) this, color);
}

void ProceduralSky::set_sky_curve(const real_t curve) {
	___godot_icall_void_float(___mb.mb_set_sky_curve, (const Object *) this, curve);
}

void ProceduralSky::set_sky_energy(const real_t energy) {
	___godot_icall_void_float(___mb.mb_set_sky_energy, (const Object *) this, energy);
}

void ProceduralSky::set_sky_horizon_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_sky_horizon_color, (const Object *) this, color);
}

void ProceduralSky::set_sky_top_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_sky_top_color, (const Object *) this, color);
}

void ProceduralSky::set_sun_angle_max(const real_t degrees) {
	___godot_icall_void_float(___mb.mb_set_sun_angle_max, (const Object *) this, degrees);
}

void ProceduralSky::set_sun_angle_min(const real_t degrees) {
	___godot_icall_void_float(___mb.mb_set_sun_angle_min, (const Object *) this, degrees);
}

void ProceduralSky::set_sun_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_sun_color, (const Object *) this, color);
}

void ProceduralSky::set_sun_curve(const real_t curve) {
	___godot_icall_void_float(___mb.mb_set_sun_curve, (const Object *) this, curve);
}

void ProceduralSky::set_sun_energy(const real_t energy) {
	___godot_icall_void_float(___mb.mb_set_sun_energy, (const Object *) this, energy);
}

void ProceduralSky::set_sun_latitude(const real_t degrees) {
	___godot_icall_void_float(___mb.mb_set_sun_latitude, (const Object *) this, degrees);
}

void ProceduralSky::set_sun_longitude(const real_t degrees) {
	___godot_icall_void_float(___mb.mb_set_sun_longitude, (const Object *) this, degrees);
}

void ProceduralSky::set_texture_size(const int64_t size) {
	___godot_icall_void_int(___mb.mb_set_texture_size, (const Object *) this, size);
}

}