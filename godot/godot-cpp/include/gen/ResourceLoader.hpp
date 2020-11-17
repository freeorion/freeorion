#ifndef GODOT_CPP_RESOURCELOADER_HPP
#define GODOT_CPP_RESOURCELOADER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {

class Resource;
class ResourceInteractiveLoader;

class ResourceLoader : public Object {
	static ResourceLoader *_singleton;

	ResourceLoader();

	struct ___method_bindings {
		godot_method_bind *mb_exists;
		godot_method_bind *mb_get_dependencies;
		godot_method_bind *mb_get_recognized_extensions_for_type;
		godot_method_bind *mb_has;
		godot_method_bind *mb_has_cached;
		godot_method_bind *mb_load;
		godot_method_bind *mb_load_interactive;
		godot_method_bind *mb_set_abort_on_missing_resources;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline ResourceLoader *get_singleton()
	{
		if (!ResourceLoader::_singleton) {
			ResourceLoader::_singleton = new ResourceLoader;
		}
		return ResourceLoader::_singleton;
	}

	static inline const char *___get_class_name() { return (const char *) "ResourceLoader"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	bool exists(const String path, const String type_hint = "");
	PoolStringArray get_dependencies(const String path);
	PoolStringArray get_recognized_extensions_for_type(const String type);
	bool has(const String path);
	bool has_cached(const String path);
	Ref<Resource> load(const String path, const String type_hint = "", const bool no_cache = false);
	Ref<ResourceInteractiveLoader> load_interactive(const String path, const String type_hint = "");
	void set_abort_on_missing_resources(const bool abort);

};

}

#endif