#include "EditorResourcePreviewGenerator.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"
#include "Resource.hpp"


namespace godot {


EditorResourcePreviewGenerator::___method_bindings EditorResourcePreviewGenerator::___mb = {};

void EditorResourcePreviewGenerator::___init_method_bindings() {
	___mb.mb_can_generate_small_preview = godot::api->godot_method_bind_get_method("EditorResourcePreviewGenerator", "can_generate_small_preview");
	___mb.mb_generate = godot::api->godot_method_bind_get_method("EditorResourcePreviewGenerator", "generate");
	___mb.mb_generate_from_path = godot::api->godot_method_bind_get_method("EditorResourcePreviewGenerator", "generate_from_path");
	___mb.mb_generate_small_preview_automatically = godot::api->godot_method_bind_get_method("EditorResourcePreviewGenerator", "generate_small_preview_automatically");
	___mb.mb_handles = godot::api->godot_method_bind_get_method("EditorResourcePreviewGenerator", "handles");
}

bool EditorResourcePreviewGenerator::can_generate_small_preview() {
	return ___godot_icall_bool(___mb.mb_can_generate_small_preview, (const Object *) this);
}

Ref<Texture> EditorResourcePreviewGenerator::generate(const Ref<Resource> from, const Vector2 size) {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_Object_Vector2(___mb.mb_generate, (const Object *) this, from.ptr(), size));
}

Ref<Texture> EditorResourcePreviewGenerator::generate_from_path(const String path, const Vector2 size) {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_String_Vector2(___mb.mb_generate_from_path, (const Object *) this, path, size));
}

bool EditorResourcePreviewGenerator::generate_small_preview_automatically() {
	return ___godot_icall_bool(___mb.mb_generate_small_preview_automatically, (const Object *) this);
}

bool EditorResourcePreviewGenerator::handles(const String type) {
	return ___godot_icall_bool_String(___mb.mb_handles, (const Object *) this, type);
}

}