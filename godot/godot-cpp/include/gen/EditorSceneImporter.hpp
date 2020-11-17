#ifndef GODOT_CPP_EDITORSCENEIMPORTER_HPP
#define GODOT_CPP_EDITORSCENEIMPORTER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class Animation;
class Node;

class EditorSceneImporter : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb__get_extensions;
		godot_method_bind *mb__get_import_flags;
		godot_method_bind *mb__import_animation;
		godot_method_bind *mb__import_scene;
		godot_method_bind *mb_import_animation_from_other_importer;
		godot_method_bind *mb_import_scene_from_other_importer;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorSceneImporter"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants
	const static int IMPORT_ANIMATION = 2;
	const static int IMPORT_ANIMATION_DETECT_LOOP = 4;
	const static int IMPORT_ANIMATION_FORCE_ALL_TRACKS_IN_ALL_CLIPS = 16;
	const static int IMPORT_ANIMATION_KEEP_VALUE_TRACKS = 32;
	const static int IMPORT_ANIMATION_OPTIMIZE = 8;
	const static int IMPORT_FAIL_ON_MISSING_DEPENDENCIES = 512;
	const static int IMPORT_GENERATE_TANGENT_ARRAYS = 256;
	const static int IMPORT_MATERIALS_IN_INSTANCES = 1024;
	const static int IMPORT_SCENE = 1;
	const static int IMPORT_USE_COMPRESSION = 2048;

	// methods
	Array _get_extensions();
	int64_t _get_import_flags();
	Ref<Animation> _import_animation(const String path, const int64_t flags, const int64_t bake_fps);
	Node *_import_scene(const String path, const int64_t flags, const int64_t bake_fps);
	Ref<Animation> import_animation_from_other_importer(const String path, const int64_t flags, const int64_t bake_fps);
	Node *import_scene_from_other_importer(const String path, const int64_t flags, const int64_t bake_fps);

};

}

#endif