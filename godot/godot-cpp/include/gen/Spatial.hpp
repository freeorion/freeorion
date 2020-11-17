#ifndef GODOT_CPP_SPATIAL_HPP
#define GODOT_CPP_SPATIAL_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node.hpp"
namespace godot {

class SpatialGizmo;
class Spatial;
class World;

class Spatial : public Node {
	struct ___method_bindings {
		godot_method_bind *mb__update_gizmo;
		godot_method_bind *mb_force_update_transform;
		godot_method_bind *mb_get_gizmo;
		godot_method_bind *mb_get_global_transform;
		godot_method_bind *mb_get_parent_spatial;
		godot_method_bind *mb_get_rotation;
		godot_method_bind *mb_get_rotation_degrees;
		godot_method_bind *mb_get_scale;
		godot_method_bind *mb_get_transform;
		godot_method_bind *mb_get_translation;
		godot_method_bind *mb_get_world;
		godot_method_bind *mb_global_rotate;
		godot_method_bind *mb_global_scale;
		godot_method_bind *mb_global_translate;
		godot_method_bind *mb_hide;
		godot_method_bind *mb_is_local_transform_notification_enabled;
		godot_method_bind *mb_is_scale_disabled;
		godot_method_bind *mb_is_set_as_toplevel;
		godot_method_bind *mb_is_transform_notification_enabled;
		godot_method_bind *mb_is_visible;
		godot_method_bind *mb_is_visible_in_tree;
		godot_method_bind *mb_look_at;
		godot_method_bind *mb_look_at_from_position;
		godot_method_bind *mb_orthonormalize;
		godot_method_bind *mb_rotate;
		godot_method_bind *mb_rotate_object_local;
		godot_method_bind *mb_rotate_x;
		godot_method_bind *mb_rotate_y;
		godot_method_bind *mb_rotate_z;
		godot_method_bind *mb_scale_object_local;
		godot_method_bind *mb_set_as_toplevel;
		godot_method_bind *mb_set_disable_scale;
		godot_method_bind *mb_set_gizmo;
		godot_method_bind *mb_set_global_transform;
		godot_method_bind *mb_set_identity;
		godot_method_bind *mb_set_ignore_transform_notification;
		godot_method_bind *mb_set_notify_local_transform;
		godot_method_bind *mb_set_notify_transform;
		godot_method_bind *mb_set_rotation;
		godot_method_bind *mb_set_rotation_degrees;
		godot_method_bind *mb_set_scale;
		godot_method_bind *mb_set_transform;
		godot_method_bind *mb_set_translation;
		godot_method_bind *mb_set_visible;
		godot_method_bind *mb_show;
		godot_method_bind *mb_to_global;
		godot_method_bind *mb_to_local;
		godot_method_bind *mb_translate;
		godot_method_bind *mb_translate_object_local;
		godot_method_bind *mb_update_gizmo;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Spatial"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants
	const static int NOTIFICATION_ENTER_WORLD = 41;
	const static int NOTIFICATION_EXIT_WORLD = 42;
	const static int NOTIFICATION_TRANSFORM_CHANGED = 2000;
	const static int NOTIFICATION_VISIBILITY_CHANGED = 43;


	static Spatial *_new();

	// methods
	void _update_gizmo();
	void force_update_transform();
	Ref<SpatialGizmo> get_gizmo() const;
	Transform get_global_transform() const;
	Spatial *get_parent_spatial() const;
	Vector3 get_rotation() const;
	Vector3 get_rotation_degrees() const;
	Vector3 get_scale() const;
	Transform get_transform() const;
	Vector3 get_translation() const;
	Ref<World> get_world() const;
	void global_rotate(const Vector3 axis, const real_t angle);
	void global_scale(const Vector3 scale);
	void global_translate(const Vector3 offset);
	void hide();
	bool is_local_transform_notification_enabled() const;
	bool is_scale_disabled() const;
	bool is_set_as_toplevel() const;
	bool is_transform_notification_enabled() const;
	bool is_visible() const;
	bool is_visible_in_tree() const;
	void look_at(const Vector3 target, const Vector3 up);
	void look_at_from_position(const Vector3 position, const Vector3 target, const Vector3 up);
	void orthonormalize();
	void rotate(const Vector3 axis, const real_t angle);
	void rotate_object_local(const Vector3 axis, const real_t angle);
	void rotate_x(const real_t angle);
	void rotate_y(const real_t angle);
	void rotate_z(const real_t angle);
	void scale_object_local(const Vector3 scale);
	void set_as_toplevel(const bool enable);
	void set_disable_scale(const bool disable);
	void set_gizmo(const Ref<SpatialGizmo> gizmo);
	void set_global_transform(const Transform global);
	void set_identity();
	void set_ignore_transform_notification(const bool enabled);
	void set_notify_local_transform(const bool enable);
	void set_notify_transform(const bool enable);
	void set_rotation(const Vector3 euler);
	void set_rotation_degrees(const Vector3 euler_degrees);
	void set_scale(const Vector3 scale);
	void set_transform(const Transform local);
	void set_translation(const Vector3 translation);
	void set_visible(const bool visible);
	void show();
	Vector3 to_global(const Vector3 local_point) const;
	Vector3 to_local(const Vector3 global_point) const;
	void translate(const Vector3 offset);
	void translate_object_local(const Vector3 offset);
	void update_gizmo();

};

}

#endif