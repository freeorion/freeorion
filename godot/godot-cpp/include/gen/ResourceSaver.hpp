#ifndef GODOT_CPP_RESOURCESAVER_HPP
#define GODOT_CPP_RESOURCESAVER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {

class Resource;

class ResourceSaver : public Object {
	static ResourceSaver *_singleton;

	ResourceSaver();

	struct ___method_bindings {
		godot_method_bind *mb_get_recognized_extensions;
		godot_method_bind *mb_save;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline ResourceSaver *get_singleton()
	{
		if (!ResourceSaver::_singleton) {
			ResourceSaver::_singleton = new ResourceSaver;
		}
		return ResourceSaver::_singleton;
	}

	static inline const char *___get_class_name() { return (const char *) "ResourceSaver"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum SaverFlags {
		FLAG_RELATIVE_PATHS = 1,
		FLAG_BUNDLE_RESOURCES = 2,
		FLAG_CHANGE_PATH = 4,
		FLAG_OMIT_EDITOR_PROPERTIES = 8,
		FLAG_SAVE_BIG_ENDIAN = 16,
		FLAG_COMPRESS = 32,
		FLAG_REPLACE_SUBRESOURCE_PATHS = 64,
	};

	// constants

	// methods
	PoolStringArray get_recognized_extensions(const Ref<Resource> type);
	Error save(const String path, const Ref<Resource> resource, const int64_t flags = 0);

};

}

#endif