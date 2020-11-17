#include "EditorScenePostImport.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"


namespace godot {


EditorScenePostImport::___method_bindings EditorScenePostImport::___mb = {};

void EditorScenePostImport::___init_method_bindings() {
	___mb.mb_get_source_file = godot::api->godot_method_bind_get_method("EditorScenePostImport", "get_source_file");
	___mb.mb_get_source_folder = godot::api->godot_method_bind_get_method("EditorScenePostImport", "get_source_folder");
	___mb.mb_post_import = godot::api->godot_method_bind_get_method("EditorScenePostImport", "post_import");
}

String EditorScenePostImport::get_source_file() const {
	return ___godot_icall_String(___mb.mb_get_source_file, (const Object *) this);
}

String EditorScenePostImport::get_source_folder() const {
	return ___godot_icall_String(___mb.mb_get_source_folder, (const Object *) this);
}

Object *EditorScenePostImport::post_import(const Object *scene) {
	return (Object *) ___godot_icall_Object_Object(___mb.mb_post_import, (const Object *) this, scene);
}

}