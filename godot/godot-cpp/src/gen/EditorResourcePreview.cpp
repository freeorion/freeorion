#include "EditorResourcePreview.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"
#include "EditorResourcePreviewGenerator.hpp"
#include "Resource.hpp"
#include "Object.hpp"


namespace godot {


EditorResourcePreview::___method_bindings EditorResourcePreview::___mb = {};

void EditorResourcePreview::___init_method_bindings() {
	___mb.mb__preview_ready = godot::api->godot_method_bind_get_method("EditorResourcePreview", "_preview_ready");
	___mb.mb_add_preview_generator = godot::api->godot_method_bind_get_method("EditorResourcePreview", "add_preview_generator");
	___mb.mb_check_for_invalidation = godot::api->godot_method_bind_get_method("EditorResourcePreview", "check_for_invalidation");
	___mb.mb_queue_edited_resource_preview = godot::api->godot_method_bind_get_method("EditorResourcePreview", "queue_edited_resource_preview");
	___mb.mb_queue_resource_preview = godot::api->godot_method_bind_get_method("EditorResourcePreview", "queue_resource_preview");
	___mb.mb_remove_preview_generator = godot::api->godot_method_bind_get_method("EditorResourcePreview", "remove_preview_generator");
}

void EditorResourcePreview::_preview_ready(const String arg0, const Ref<Texture> arg1, const Ref<Texture> arg2, const int64_t arg3, const String arg4, const Variant arg5) {
	___godot_icall_void_String_Object_Object_int_String_Variant(___mb.mb__preview_ready, (const Object *) this, arg0, arg1.ptr(), arg2.ptr(), arg3, arg4, arg5);
}

void EditorResourcePreview::add_preview_generator(const Ref<EditorResourcePreviewGenerator> generator) {
	___godot_icall_void_Object(___mb.mb_add_preview_generator, (const Object *) this, generator.ptr());
}

void EditorResourcePreview::check_for_invalidation(const String path) {
	___godot_icall_void_String(___mb.mb_check_for_invalidation, (const Object *) this, path);
}

void EditorResourcePreview::queue_edited_resource_preview(const Ref<Resource> resource, const Object *receiver, const String receiver_func, const Variant userdata) {
	___godot_icall_void_Object_Object_String_Variant(___mb.mb_queue_edited_resource_preview, (const Object *) this, resource.ptr(), receiver, receiver_func, userdata);
}

void EditorResourcePreview::queue_resource_preview(const String path, const Object *receiver, const String receiver_func, const Variant userdata) {
	___godot_icall_void_String_Object_String_Variant(___mb.mb_queue_resource_preview, (const Object *) this, path, receiver, receiver_func, userdata);
}

void EditorResourcePreview::remove_preview_generator(const Ref<EditorResourcePreviewGenerator> generator) {
	___godot_icall_void_Object(___mb.mb_remove_preview_generator, (const Object *) this, generator.ptr());
}

}