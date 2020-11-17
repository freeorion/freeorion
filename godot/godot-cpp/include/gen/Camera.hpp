#ifndef GODOT_CPP_CAMERA_HPP
#define GODOT_CPP_CAMERA_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Camera.hpp"

#include "Spatial.hpp"
namespace godot {

class Environment;

class Camera : public Spatial {
	struct ___method_bindings {
		godot_method_bind *mb_clear_current;
		godot_method_bind *mb_get_camera_rid;
		godot_method_bind *mb_get_camera_transform;
		godot_method_bind *mb_get_cull_mask;
		godot_method_bind *mb_get_cull_mask_bit;
		godot_method_bind *mb_get_doppler_tracking;
		godot_method_bind *mb_get_environment;
		godot_method_bind *mb_get_fov;
		godot_method_bind *mb_get_frustum;
		godot_method_bind *mb_get_frustum_offset;
		godot_method_bind *mb_get_h_offset;
		godot_method_bind *mb_get_keep_aspect_mode;
		godot_method_bind *mb_get_projection;
		godot_method_bind *mb_get_size;
		godot_method_bind *mb_get_v_offset;
		godot_method_bind *mb_get_zfar;
		godot_method_bind *mb_get_znear;
		godot_method_bind *mb_is_current;
		godot_method_bind *mb_is_position_behind;
		godot_method_bind *mb_make_current;
		godot_method_bind *mb_project_local_ray_normal;
		godot_method_bind *mb_project_position;
		godot_method_bind *mb_project_ray_normal;
		godot_method_bind *mb_project_ray_origin;
		godot_method_bind *mb_set_cull_mask;
		godot_method_bind *mb_set_cull_mask_bit;
		godot_method_bind *mb_set_current;
		godot_method_bind *mb_set_doppler_tracking;
		godot_method_bind *mb_set_environment;
		godot_method_bind *mb_set_fov;
		godot_method_bind *mb_set_frustum;
		godot_method_bind *mb_set_frustum_offset;
		godot_method_bind *mb_set_h_offset;
		godot_method_bind *mb_set_keep_aspect_mode;
		godot_method_bind *mb_set_orthogonal;
		godot_method_bind *mb_set_perspective;
		godot_method_bind *mb_set_projection;
		godot_method_bind *mb_set_size;
		godot_method_bind *mb_set_v_offset;
		godot_method_bind *mb_set_zfar;
		godot_method_bind *mb_set_znear;
		godot_method_bind *mb_unproject_position;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Camera"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum KeepAspect {
		KEEP_WIDTH = 0,
		KEEP_HEIGHT = 1,
	};
	enum Projection {
		PROJECTION_PERSPECTIVE = 0,
		PROJECTION_ORTHOGONAL = 1,
		PROJECTION_FRUSTUM = 2,
	};
	enum DopplerTracking {
		DOPPLER_TRACKING_DISABLED = 0,
		DOPPLER_TRACKING_IDLE_STEP = 1,
		DOPPLER_TRACKING_PHYSICS_STEP = 2,
	};

	// constants


	static Camera *_new();

	// methods
	void clear_current(const bool enable_next = true);
	RID get_camera_rid() const;
	Transform get_camera_transform() const;
	int64_t get_cull_mask() const;
	bool get_cull_mask_bit(const int64_t layer) const;
	Camera::DopplerTracking get_doppler_tracking() const;
	Ref<Environment> get_environment() const;
	real_t get_fov() const;
	Array get_frustum() const;
	Vector2 get_frustum_offset() const;
	real_t get_h_offset() const;
	Camera::KeepAspect get_keep_aspect_mode() const;
	Camera::Projection get_projection() const;
	real_t get_size() const;
	real_t get_v_offset() const;
	real_t get_zfar() const;
	real_t get_znear() const;
	bool is_current() const;
	bool is_position_behind(const Vector3 world_point) const;
	void make_current();
	Vector3 project_local_ray_normal(const Vector2 screen_point) const;
	Vector3 project_position(const Vector2 screen_point, const real_t z_depth) const;
	Vector3 project_ray_normal(const Vector2 screen_point) const;
	Vector3 project_ray_origin(const Vector2 screen_point) const;
	void set_cull_mask(const int64_t mask);
	void set_cull_mask_bit(const int64_t layer, const bool enable);
	void set_current(const bool arg0);
	void set_doppler_tracking(const int64_t mode);
	void set_environment(const Ref<Environment> env);
	void set_fov(const real_t arg0);
	void set_frustum(const real_t size, const Vector2 offset, const real_t z_near, const real_t z_far);
	void set_frustum_offset(const Vector2 arg0);
	void set_h_offset(const real_t ofs);
	void set_keep_aspect_mode(const int64_t mode);
	void set_orthogonal(const real_t size, const real_t z_near, const real_t z_far);
	void set_perspective(const real_t fov, const real_t z_near, const real_t z_far);
	void set_projection(const int64_t arg0);
	void set_size(const real_t arg0);
	void set_v_offset(const real_t ofs);
	void set_zfar(const real_t arg0);
	void set_znear(const real_t arg0);
	Vector2 unproject_position(const Vector3 world_point) const;

};

}

#endif