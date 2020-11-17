#ifndef GODOT_CPP_ANIMATIONNODE_HPP
#define GODOT_CPP_ANIMATIONNODE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class AnimationNode;
class Object;

class AnimationNode : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb__get_filters;
		godot_method_bind *mb__set_filters;
		godot_method_bind *mb_add_input;
		godot_method_bind *mb_blend_animation;
		godot_method_bind *mb_blend_input;
		godot_method_bind *mb_blend_node;
		godot_method_bind *mb_get_caption;
		godot_method_bind *mb_get_child_by_name;
		godot_method_bind *mb_get_child_nodes;
		godot_method_bind *mb_get_input_count;
		godot_method_bind *mb_get_input_name;
		godot_method_bind *mb_get_parameter;
		godot_method_bind *mb_get_parameter_default_value;
		godot_method_bind *mb_get_parameter_list;
		godot_method_bind *mb_has_filter;
		godot_method_bind *mb_is_filter_enabled;
		godot_method_bind *mb_is_path_filtered;
		godot_method_bind *mb_process;
		godot_method_bind *mb_remove_input;
		godot_method_bind *mb_set_filter_enabled;
		godot_method_bind *mb_set_filter_path;
		godot_method_bind *mb_set_parameter;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AnimationNode"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum FilterAction {
		FILTER_IGNORE = 0,
		FILTER_PASS = 1,
		FILTER_STOP = 2,
		FILTER_BLEND = 3,
	};

	// constants


	static AnimationNode *_new();

	// methods
	Array _get_filters() const;
	void _set_filters(const Array filters);
	void add_input(const String name);
	void blend_animation(const String animation, const real_t time, const real_t delta, const bool seeked, const real_t blend);
	real_t blend_input(const int64_t input_index, const real_t time, const bool seek, const real_t blend, const int64_t filter = 0, const bool optimize = true);
	real_t blend_node(const String name, const Ref<AnimationNode> node, const real_t time, const bool seek, const real_t blend, const int64_t filter = 0, const bool optimize = true);
	String get_caption();
	Object *get_child_by_name(const String name);
	Dictionary get_child_nodes();
	int64_t get_input_count() const;
	String get_input_name(const int64_t input);
	Variant get_parameter(const String name) const;
	Variant get_parameter_default_value(const String name);
	Array get_parameter_list();
	String has_filter();
	bool is_filter_enabled() const;
	bool is_path_filtered(const NodePath path) const;
	void process(const real_t time, const bool seek);
	void remove_input(const int64_t index);
	void set_filter_enabled(const bool enable);
	void set_filter_path(const NodePath path, const bool enable);
	void set_parameter(const String name, const Variant value);

};

}

#endif