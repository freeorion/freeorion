#include "EditorSceneImporter.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Animation.hpp"
#include "Node.hpp"


namespace godot {


EditorSceneImporter::___method_bindings EditorSceneImporter::___mb = {};

void EditorSceneImporter::___init_method_bindings() {
	___mb.mb__get_extensions = godot::api->godot_method_bind_get_method("EditorSceneImporter", "_get_extensions");
	___mb.mb__get_import_flags = godot::api->godot_method_bind_get_method("EditorSceneImporter", "_get_import_flags");
	___mb.mb__import_animation = godot::api->godot_method_bind_get_method("EditorSceneImporter", "_import_animation");
	___mb.mb__import_scene = godot::api->godot_method_bind_get_method("EditorSceneImporter", "_import_scene");
	___mb.mb_import_animation_from_other_importer = godot::api->godot_method_bind_get_method("EditorSceneImporter", "import_animation_from_other_importer");
	___mb.mb_import_scene_from_other_importer = godot::api->godot_method_bind_get_method("EditorSceneImporter", "import_scene_from_other_importer");
}

Array EditorSceneImporter::_get_extensions() {
	return ___godot_icall_Array(___mb.mb__get_extensions, (const Object *) this);
}

int64_t EditorSceneImporter::_get_import_flags() {
	return ___godot_icall_int(___mb.mb__get_import_flags, (const Object *) this);
}

Ref<Animation> EditorSceneImporter::_import_animation(const String path, const int64_t flags, const int64_t bake_fps) {
	return Ref<Animation>::__internal_constructor(___godot_icall_Object_String_int_int(___mb.mb__import_animation, (const Object *) this, path, flags, bake_fps));
}

Node *EditorSceneImporter::_import_scene(const String path, const int64_t flags, const int64_t bake_fps) {
	return (Node *) ___godot_icall_Object_String_int_int(___mb.mb__import_scene, (const Object *) this, path, flags, bake_fps);
}

Ref<Animation> EditorSceneImporter::import_animation_from_other_importer(const String path, const int64_t flags, const int64_t bake_fps) {
	return Ref<Animation>::__internal_constructor(___godot_icall_Object_String_int_int(___mb.mb_import_animation_from_other_importer, (const Object *) this, path, flags, bake_fps));
}

Node *EditorSceneImporter::import_scene_from_other_importer(const String path, const int64_t flags, const int64_t bake_fps) {
	return (Node *) ___godot_icall_Object_String_int_int(___mb.mb_import_scene_from_other_importer, (const Object *) this, path, flags, bake_fps);
}

}