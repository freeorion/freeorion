#ifndef GODOT_CPP_EDITORRESOURCEPREVIEW_HPP
#define GODOT_CPP_EDITORRESOURCEPREVIEW_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node.hpp"
namespace godot {

class Texture;
class EditorResourcePreviewGenerator;
class Resource;
class Object;

class EditorResourcePreview : public Node {
	struct ___method_bindings {
		godot_method_bind *mb__preview_ready;
		godot_method_bind *mb_add_preview_generator;
		godot_method_bind *mb_check_for_invalidation;
		godot_method_bind *mb_queue_edited_resource_preview;
		godot_method_bind *mb_queue_resource_preview;
		godot_method_bind *mb_remove_preview_generator;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorResourcePreview"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void _preview_ready(const String arg0, const Ref<Texture> arg1, const Ref<Texture> arg2, const int64_t arg3, const String arg4, const Variant arg5);
	void add_preview_generator(const Ref<EditorResourcePreviewGenerator> generator);
	void check_for_invalidation(const String path);
	void queue_edited_resource_preview(const Ref<Resource> resource, const Object *receiver, const String receiver_func, const Variant userdata);
	void queue_resource_preview(const String path, const Object *receiver, const String receiver_func, const Variant userdata);
	void remove_preview_generator(const Ref<EditorResourcePreviewGenerator> generator);

};

}

#endif