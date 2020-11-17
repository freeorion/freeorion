#ifndef GODOT_CPP_SCENESTATE_HPP
#define GODOT_CPP_SCENESTATE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class PackedScene;

class SceneState : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_get_connection_binds;
		godot_method_bind *mb_get_connection_count;
		godot_method_bind *mb_get_connection_flags;
		godot_method_bind *mb_get_connection_method;
		godot_method_bind *mb_get_connection_signal;
		godot_method_bind *mb_get_connection_source;
		godot_method_bind *mb_get_connection_target;
		godot_method_bind *mb_get_node_count;
		godot_method_bind *mb_get_node_groups;
		godot_method_bind *mb_get_node_index;
		godot_method_bind *mb_get_node_instance;
		godot_method_bind *mb_get_node_instance_placeholder;
		godot_method_bind *mb_get_node_name;
		godot_method_bind *mb_get_node_owner_path;
		godot_method_bind *mb_get_node_path;
		godot_method_bind *mb_get_node_property_count;
		godot_method_bind *mb_get_node_property_name;
		godot_method_bind *mb_get_node_property_value;
		godot_method_bind *mb_get_node_type;
		godot_method_bind *mb_is_node_instance_placeholder;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "SceneState"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum GenEditState {
		GEN_EDIT_STATE_DISABLED = 0,
		GEN_EDIT_STATE_INSTANCE = 1,
		GEN_EDIT_STATE_MAIN = 2,
	};

	// constants

	// methods
	Array get_connection_binds(const int64_t idx) const;
	int64_t get_connection_count() const;
	int64_t get_connection_flags(const int64_t idx) const;
	String get_connection_method(const int64_t idx) const;
	String get_connection_signal(const int64_t idx) const;
	NodePath get_connection_source(const int64_t idx) const;
	NodePath get_connection_target(const int64_t idx) const;
	int64_t get_node_count() const;
	PoolStringArray get_node_groups(const int64_t idx) const;
	int64_t get_node_index(const int64_t idx) const;
	Ref<PackedScene> get_node_instance(const int64_t idx) const;
	String get_node_instance_placeholder(const int64_t idx) const;
	String get_node_name(const int64_t idx) const;
	NodePath get_node_owner_path(const int64_t idx) const;
	NodePath get_node_path(const int64_t idx, const bool for_parent = false) const;
	int64_t get_node_property_count(const int64_t idx) const;
	String get_node_property_name(const int64_t idx, const int64_t prop_idx) const;
	Variant get_node_property_value(const int64_t idx, const int64_t prop_idx) const;
	String get_node_type(const int64_t idx) const;
	bool is_node_instance_placeholder(const int64_t idx) const;

};

}

#endif