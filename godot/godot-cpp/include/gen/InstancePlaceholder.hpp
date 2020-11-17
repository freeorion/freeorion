#ifndef GODOT_CPP_INSTANCEPLACEHOLDER_HPP
#define GODOT_CPP_INSTANCEPLACEHOLDER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node.hpp"
namespace godot {

class Node;
class PackedScene;

class InstancePlaceholder : public Node {
	struct ___method_bindings {
		godot_method_bind *mb_create_instance;
		godot_method_bind *mb_get_instance_path;
		godot_method_bind *mb_get_stored_values;
		godot_method_bind *mb_replace_by_instance;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "InstancePlaceholder"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	Node *create_instance(const bool replace = false, const Ref<PackedScene> custom_scene = nullptr);
	String get_instance_path() const;
	Dictionary get_stored_values(const bool with_order = false);
	void replace_by_instance(const Ref<PackedScene> custom_scene = nullptr);

};

}

#endif