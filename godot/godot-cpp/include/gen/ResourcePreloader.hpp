#ifndef GODOT_CPP_RESOURCEPRELOADER_HPP
#define GODOT_CPP_RESOURCEPRELOADER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node.hpp"
namespace godot {

class Resource;

class ResourcePreloader : public Node {
	struct ___method_bindings {
		godot_method_bind *mb__get_resources;
		godot_method_bind *mb__set_resources;
		godot_method_bind *mb_add_resource;
		godot_method_bind *mb_get_resource;
		godot_method_bind *mb_get_resource_list;
		godot_method_bind *mb_has_resource;
		godot_method_bind *mb_remove_resource;
		godot_method_bind *mb_rename_resource;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ResourcePreloader"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ResourcePreloader *_new();

	// methods
	Array _get_resources() const;
	void _set_resources(const Array arg0);
	void add_resource(const String name, const Ref<Resource> resource);
	Ref<Resource> get_resource(const String name) const;
	PoolStringArray get_resource_list() const;
	bool has_resource(const String name) const;
	void remove_resource(const String name);
	void rename_resource(const String name, const String newname);

};

}

#endif