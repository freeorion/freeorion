#ifndef GODOT_CPP_RESOURCE_HPP
#define GODOT_CPP_RESOURCE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class Resource;
class Node;

class Resource : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb__setup_local_to_scene;
		godot_method_bind *mb_duplicate;
		godot_method_bind *mb_get_local_scene;
		godot_method_bind *mb_get_name;
		godot_method_bind *mb_get_path;
		godot_method_bind *mb_get_rid;
		godot_method_bind *mb_is_local_to_scene;
		godot_method_bind *mb_set_local_to_scene;
		godot_method_bind *mb_set_name;
		godot_method_bind *mb_set_path;
		godot_method_bind *mb_setup_local_to_scene;
		godot_method_bind *mb_take_over_path;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Resource"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Resource *_new();

	// methods
	void _setup_local_to_scene();
	Ref<Resource> duplicate(const bool subresources = false) const;
	Node *get_local_scene() const;
	String get_name() const;
	String get_path() const;
	RID get_rid() const;
	bool is_local_to_scene() const;
	void set_local_to_scene(const bool enable);
	void set_name(const String name);
	void set_path(const String path);
	void setup_local_to_scene();
	void take_over_path(const String path);

};

}

#endif