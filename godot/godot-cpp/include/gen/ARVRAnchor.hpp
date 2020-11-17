#ifndef GODOT_CPP_ARVRANCHOR_HPP
#define GODOT_CPP_ARVRANCHOR_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Spatial.hpp"
namespace godot {

class Mesh;

class ARVRAnchor : public Spatial {
	struct ___method_bindings {
		godot_method_bind *mb_get_anchor_id;
		godot_method_bind *mb_get_anchor_name;
		godot_method_bind *mb_get_is_active;
		godot_method_bind *mb_get_mesh;
		godot_method_bind *mb_get_plane;
		godot_method_bind *mb_get_size;
		godot_method_bind *mb_set_anchor_id;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ARVRAnchor"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ARVRAnchor *_new();

	// methods
	int64_t get_anchor_id() const;
	String get_anchor_name() const;
	bool get_is_active() const;
	Ref<Mesh> get_mesh() const;
	Plane get_plane() const;
	Vector3 get_size() const;
	void set_anchor_id(const int64_t anchor_id);

};

}

#endif