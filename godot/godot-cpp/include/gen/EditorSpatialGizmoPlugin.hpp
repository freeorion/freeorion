#ifndef GODOT_CPP_EDITORSPATIALGIZMOPLUGIN_HPP
#define GODOT_CPP_EDITORSPATIALGIZMOPLUGIN_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class SpatialMaterial;
class EditorSpatialGizmo;
class Spatial;
class Texture;
class Camera;

class EditorSpatialGizmoPlugin : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_add_material;
		godot_method_bind *mb_can_be_hidden;
		godot_method_bind *mb_commit_handle;
		godot_method_bind *mb_create_gizmo;
		godot_method_bind *mb_create_handle_material;
		godot_method_bind *mb_create_icon_material;
		godot_method_bind *mb_create_material;
		godot_method_bind *mb_get_handle_name;
		godot_method_bind *mb_get_handle_value;
		godot_method_bind *mb_get_material;
		godot_method_bind *mb_get_name;
		godot_method_bind *mb_get_priority;
		godot_method_bind *mb_has_gizmo;
		godot_method_bind *mb_is_handle_highlighted;
		godot_method_bind *mb_is_selectable_when_hidden;
		godot_method_bind *mb_redraw;
		godot_method_bind *mb_set_handle;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorSpatialGizmoPlugin"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void add_material(const String name, const Ref<SpatialMaterial> material);
	bool can_be_hidden();
	void commit_handle(const Ref<EditorSpatialGizmo> gizmo, const int64_t index, const Variant restore, const bool cancel);
	Ref<EditorSpatialGizmo> create_gizmo(const Spatial *spatial);
	void create_handle_material(const String name, const bool billboard = false);
	void create_icon_material(const String name, const Ref<Texture> texture, const bool on_top = false, const Color color = Color(1,1,1,1));
	void create_material(const String name, const Color color, const bool billboard = false, const bool on_top = false, const bool use_vertex_color = false);
	String get_handle_name(const Ref<EditorSpatialGizmo> gizmo, const int64_t index);
	Variant get_handle_value(const Ref<EditorSpatialGizmo> gizmo, const int64_t index);
	Ref<SpatialMaterial> get_material(const String name, const Ref<EditorSpatialGizmo> gizmo);
	String get_name();
	String get_priority();
	bool has_gizmo(const Spatial *spatial);
	bool is_handle_highlighted(const Ref<EditorSpatialGizmo> gizmo, const int64_t index);
	bool is_selectable_when_hidden();
	void redraw(const Ref<EditorSpatialGizmo> gizmo);
	void set_handle(const Ref<EditorSpatialGizmo> gizmo, const int64_t index, const Camera *camera, const Vector2 point);

};

}

#endif