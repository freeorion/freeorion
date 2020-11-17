#ifndef GODOT_CPP_ANIMATIONTREE_HPP
#define GODOT_CPP_ANIMATIONTREE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "AnimationTree.hpp"

#include "Node.hpp"
namespace godot {

class Node;
class AnimationNode;

class AnimationTree : public Node {
	struct ___method_bindings {
		godot_method_bind *mb__clear_caches;
		godot_method_bind *mb__node_removed;
		godot_method_bind *mb__tree_changed;
		godot_method_bind *mb__update_properties;
		godot_method_bind *mb_advance;
		godot_method_bind *mb_get_animation_player;
		godot_method_bind *mb_get_process_mode;
		godot_method_bind *mb_get_root_motion_track;
		godot_method_bind *mb_get_root_motion_transform;
		godot_method_bind *mb_get_tree_root;
		godot_method_bind *mb_is_active;
		godot_method_bind *mb_rename_parameter;
		godot_method_bind *mb_set_active;
		godot_method_bind *mb_set_animation_player;
		godot_method_bind *mb_set_process_mode;
		godot_method_bind *mb_set_root_motion_track;
		godot_method_bind *mb_set_tree_root;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AnimationTree"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum AnimationProcessMode {
		ANIMATION_PROCESS_PHYSICS = 0,
		ANIMATION_PROCESS_IDLE = 1,
		ANIMATION_PROCESS_MANUAL = 2,
	};

	// constants


	static AnimationTree *_new();

	// methods
	void _clear_caches();
	void _node_removed(const Node *arg0);
	void _tree_changed();
	void _update_properties();
	void advance(const real_t delta);
	NodePath get_animation_player() const;
	AnimationTree::AnimationProcessMode get_process_mode() const;
	NodePath get_root_motion_track() const;
	Transform get_root_motion_transform() const;
	Ref<AnimationNode> get_tree_root() const;
	bool is_active() const;
	void rename_parameter(const String old_name, const String new_name);
	void set_active(const bool active);
	void set_animation_player(const NodePath root);
	void set_process_mode(const int64_t mode);
	void set_root_motion_track(const NodePath path);
	void set_tree_root(const Ref<AnimationNode> root);

};

}

#endif