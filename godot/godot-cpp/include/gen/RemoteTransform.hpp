#ifndef GODOT_CPP_REMOTETRANSFORM_HPP
#define GODOT_CPP_REMOTETRANSFORM_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Spatial.hpp"
namespace godot {


class RemoteTransform : public Spatial {
	struct ___method_bindings {
		godot_method_bind *mb_force_update_cache;
		godot_method_bind *mb_get_remote_node;
		godot_method_bind *mb_get_update_position;
		godot_method_bind *mb_get_update_rotation;
		godot_method_bind *mb_get_update_scale;
		godot_method_bind *mb_get_use_global_coordinates;
		godot_method_bind *mb_set_remote_node;
		godot_method_bind *mb_set_update_position;
		godot_method_bind *mb_set_update_rotation;
		godot_method_bind *mb_set_update_scale;
		godot_method_bind *mb_set_use_global_coordinates;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "RemoteTransform"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static RemoteTransform *_new();

	// methods
	void force_update_cache();
	NodePath get_remote_node() const;
	bool get_update_position() const;
	bool get_update_rotation() const;
	bool get_update_scale() const;
	bool get_use_global_coordinates() const;
	void set_remote_node(const NodePath path);
	void set_update_position(const bool update_remote_position);
	void set_update_rotation(const bool update_remote_rotation);
	void set_update_scale(const bool update_remote_scale);
	void set_use_global_coordinates(const bool use_global_coordinates);

};

}

#endif