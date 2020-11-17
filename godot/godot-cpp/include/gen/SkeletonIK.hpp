#ifndef GODOT_CPP_SKELETONIK_HPP
#define GODOT_CPP_SKELETONIK_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node.hpp"
namespace godot {

class Skeleton;

class SkeletonIK : public Node {
	struct ___method_bindings {
		godot_method_bind *mb_get_interpolation;
		godot_method_bind *mb_get_magnet_position;
		godot_method_bind *mb_get_max_iterations;
		godot_method_bind *mb_get_min_distance;
		godot_method_bind *mb_get_parent_skeleton;
		godot_method_bind *mb_get_root_bone;
		godot_method_bind *mb_get_target_node;
		godot_method_bind *mb_get_target_transform;
		godot_method_bind *mb_get_tip_bone;
		godot_method_bind *mb_is_override_tip_basis;
		godot_method_bind *mb_is_running;
		godot_method_bind *mb_is_using_magnet;
		godot_method_bind *mb_set_interpolation;
		godot_method_bind *mb_set_magnet_position;
		godot_method_bind *mb_set_max_iterations;
		godot_method_bind *mb_set_min_distance;
		godot_method_bind *mb_set_override_tip_basis;
		godot_method_bind *mb_set_root_bone;
		godot_method_bind *mb_set_target_node;
		godot_method_bind *mb_set_target_transform;
		godot_method_bind *mb_set_tip_bone;
		godot_method_bind *mb_set_use_magnet;
		godot_method_bind *mb_start;
		godot_method_bind *mb_stop;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "SkeletonIK"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static SkeletonIK *_new();

	// methods
	real_t get_interpolation() const;
	Vector3 get_magnet_position() const;
	int64_t get_max_iterations() const;
	real_t get_min_distance() const;
	Skeleton *get_parent_skeleton() const;
	String get_root_bone() const;
	NodePath get_target_node();
	Transform get_target_transform() const;
	String get_tip_bone() const;
	bool is_override_tip_basis() const;
	bool is_running();
	bool is_using_magnet() const;
	void set_interpolation(const real_t interpolation);
	void set_magnet_position(const Vector3 local_position);
	void set_max_iterations(const int64_t iterations);
	void set_min_distance(const real_t min_distance);
	void set_override_tip_basis(const bool override);
	void set_root_bone(const String root_bone);
	void set_target_node(const NodePath node);
	void set_target_transform(const Transform target);
	void set_tip_bone(const String tip_bone);
	void set_use_magnet(const bool use);
	void start(const bool one_time = false);
	void stop();

};

}

#endif