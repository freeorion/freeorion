#ifndef GODOT_CPP_SKELETON_HPP
#define GODOT_CPP_SKELETON_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Spatial.hpp"
namespace godot {

class Node;
class SkinReference;
class Skin;

class Skeleton : public Spatial {
	struct ___method_bindings {
		godot_method_bind *mb_add_bone;
		godot_method_bind *mb_bind_child_node_to_bone;
		godot_method_bind *mb_clear_bones;
		godot_method_bind *mb_find_bone;
		godot_method_bind *mb_get_bone_count;
		godot_method_bind *mb_get_bone_custom_pose;
		godot_method_bind *mb_get_bone_global_pose;
		godot_method_bind *mb_get_bone_name;
		godot_method_bind *mb_get_bone_parent;
		godot_method_bind *mb_get_bone_pose;
		godot_method_bind *mb_get_bone_rest;
		godot_method_bind *mb_get_bound_child_nodes_to_bone;
		godot_method_bind *mb_is_bone_rest_disabled;
		godot_method_bind *mb_localize_rests;
		godot_method_bind *mb_physical_bones_add_collision_exception;
		godot_method_bind *mb_physical_bones_remove_collision_exception;
		godot_method_bind *mb_physical_bones_start_simulation;
		godot_method_bind *mb_physical_bones_stop_simulation;
		godot_method_bind *mb_register_skin;
		godot_method_bind *mb_set_bone_custom_pose;
		godot_method_bind *mb_set_bone_disable_rest;
		godot_method_bind *mb_set_bone_global_pose_override;
		godot_method_bind *mb_set_bone_parent;
		godot_method_bind *mb_set_bone_pose;
		godot_method_bind *mb_set_bone_rest;
		godot_method_bind *mb_unbind_child_node_from_bone;
		godot_method_bind *mb_unparent_bone_and_rest;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Skeleton"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants
	const static int NOTIFICATION_UPDATE_SKELETON = 50;


	static Skeleton *_new();

	// methods
	void add_bone(const String name);
	void bind_child_node_to_bone(const int64_t bone_idx, const Node *node);
	void clear_bones();
	int64_t find_bone(const String name) const;
	int64_t get_bone_count() const;
	Transform get_bone_custom_pose(const int64_t bone_idx) const;
	Transform get_bone_global_pose(const int64_t bone_idx) const;
	String get_bone_name(const int64_t bone_idx) const;
	int64_t get_bone_parent(const int64_t bone_idx) const;
	Transform get_bone_pose(const int64_t bone_idx) const;
	Transform get_bone_rest(const int64_t bone_idx) const;
	Array get_bound_child_nodes_to_bone(const int64_t bone_idx) const;
	bool is_bone_rest_disabled(const int64_t bone_idx) const;
	void localize_rests();
	void physical_bones_add_collision_exception(const RID exception);
	void physical_bones_remove_collision_exception(const RID exception);
	void physical_bones_start_simulation(const Array bones = Array());
	void physical_bones_stop_simulation();
	Ref<SkinReference> register_skin(const Ref<Skin> skin);
	void set_bone_custom_pose(const int64_t bone_idx, const Transform custom_pose);
	void set_bone_disable_rest(const int64_t bone_idx, const bool disable);
	void set_bone_global_pose_override(const int64_t bone_idx, const Transform pose, const real_t amount, const bool persistent = false);
	void set_bone_parent(const int64_t bone_idx, const int64_t parent_idx);
	void set_bone_pose(const int64_t bone_idx, const Transform pose);
	void set_bone_rest(const int64_t bone_idx, const Transform rest);
	void unbind_child_node_from_bone(const int64_t bone_idx, const Node *node);
	void unparent_bone_and_rest(const int64_t bone_idx);

};

}

#endif