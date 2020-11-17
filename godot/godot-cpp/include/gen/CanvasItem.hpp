#ifndef GODOT_CPP_CANVASITEM_HPP
#define GODOT_CPP_CANVASITEM_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node.hpp"
namespace godot {

class Font;
class Texture;
class Mesh;
class MultiMesh;
class StyleBox;
class Material;
class World2D;
class InputEvent;

class CanvasItem : public Node {
	struct ___method_bindings {
		godot_method_bind *mb__draw;
		godot_method_bind *mb__edit_get_pivot;
		godot_method_bind *mb__edit_get_position;
		godot_method_bind *mb__edit_get_rect;
		godot_method_bind *mb__edit_get_rotation;
		godot_method_bind *mb__edit_get_scale;
		godot_method_bind *mb__edit_get_state;
		godot_method_bind *mb__edit_get_transform;
		godot_method_bind *mb__edit_set_pivot;
		godot_method_bind *mb__edit_set_position;
		godot_method_bind *mb__edit_set_rect;
		godot_method_bind *mb__edit_set_rotation;
		godot_method_bind *mb__edit_set_scale;
		godot_method_bind *mb__edit_set_state;
		godot_method_bind *mb__edit_use_pivot;
		godot_method_bind *mb__edit_use_rect;
		godot_method_bind *mb__edit_use_rotation;
		godot_method_bind *mb__is_on_top;
		godot_method_bind *mb__set_on_top;
		godot_method_bind *mb__toplevel_raise_self;
		godot_method_bind *mb__update_callback;
		godot_method_bind *mb_draw_arc;
		godot_method_bind *mb_draw_char;
		godot_method_bind *mb_draw_circle;
		godot_method_bind *mb_draw_colored_polygon;
		godot_method_bind *mb_draw_line;
		godot_method_bind *mb_draw_mesh;
		godot_method_bind *mb_draw_multiline;
		godot_method_bind *mb_draw_multiline_colors;
		godot_method_bind *mb_draw_multimesh;
		godot_method_bind *mb_draw_polygon;
		godot_method_bind *mb_draw_polyline;
		godot_method_bind *mb_draw_polyline_colors;
		godot_method_bind *mb_draw_primitive;
		godot_method_bind *mb_draw_rect;
		godot_method_bind *mb_draw_set_transform;
		godot_method_bind *mb_draw_set_transform_matrix;
		godot_method_bind *mb_draw_string;
		godot_method_bind *mb_draw_style_box;
		godot_method_bind *mb_draw_texture;
		godot_method_bind *mb_draw_texture_rect;
		godot_method_bind *mb_draw_texture_rect_region;
		godot_method_bind *mb_force_update_transform;
		godot_method_bind *mb_get_canvas;
		godot_method_bind *mb_get_canvas_item;
		godot_method_bind *mb_get_canvas_transform;
		godot_method_bind *mb_get_global_mouse_position;
		godot_method_bind *mb_get_global_transform;
		godot_method_bind *mb_get_global_transform_with_canvas;
		godot_method_bind *mb_get_light_mask;
		godot_method_bind *mb_get_local_mouse_position;
		godot_method_bind *mb_get_material;
		godot_method_bind *mb_get_modulate;
		godot_method_bind *mb_get_self_modulate;
		godot_method_bind *mb_get_transform;
		godot_method_bind *mb_get_use_parent_material;
		godot_method_bind *mb_get_viewport_rect;
		godot_method_bind *mb_get_viewport_transform;
		godot_method_bind *mb_get_world_2d;
		godot_method_bind *mb_hide;
		godot_method_bind *mb_is_draw_behind_parent_enabled;
		godot_method_bind *mb_is_local_transform_notification_enabled;
		godot_method_bind *mb_is_set_as_toplevel;
		godot_method_bind *mb_is_transform_notification_enabled;
		godot_method_bind *mb_is_visible;
		godot_method_bind *mb_is_visible_in_tree;
		godot_method_bind *mb_make_canvas_position_local;
		godot_method_bind *mb_make_input_local;
		godot_method_bind *mb_set_as_toplevel;
		godot_method_bind *mb_set_draw_behind_parent;
		godot_method_bind *mb_set_light_mask;
		godot_method_bind *mb_set_material;
		godot_method_bind *mb_set_modulate;
		godot_method_bind *mb_set_notify_local_transform;
		godot_method_bind *mb_set_notify_transform;
		godot_method_bind *mb_set_self_modulate;
		godot_method_bind *mb_set_use_parent_material;
		godot_method_bind *mb_set_visible;
		godot_method_bind *mb_show;
		godot_method_bind *mb_update;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CanvasItem"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum BlendMode {
		BLEND_MODE_MIX = 0,
		BLEND_MODE_ADD = 1,
		BLEND_MODE_SUB = 2,
		BLEND_MODE_MUL = 3,
		BLEND_MODE_PREMULT_ALPHA = 4,
		BLEND_MODE_DISABLED = 5,
	};

	// constants
	const static int NOTIFICATION_DRAW = 30;
	const static int NOTIFICATION_ENTER_CANVAS = 32;
	const static int NOTIFICATION_EXIT_CANVAS = 33;
	const static int NOTIFICATION_TRANSFORM_CHANGED = 2000;
	const static int NOTIFICATION_VISIBILITY_CHANGED = 31;

	// methods
	void _draw();
	Vector2 _edit_get_pivot() const;
	Vector2 _edit_get_position() const;
	Rect2 _edit_get_rect() const;
	real_t _edit_get_rotation() const;
	Vector2 _edit_get_scale() const;
	Dictionary _edit_get_state() const;
	Transform2D _edit_get_transform() const;
	void _edit_set_pivot(const Vector2 pivot);
	void _edit_set_position(const Vector2 position);
	void _edit_set_rect(const Rect2 rect);
	void _edit_set_rotation(const real_t degrees);
	void _edit_set_scale(const Vector2 scale);
	void _edit_set_state(const Dictionary state);
	bool _edit_use_pivot() const;
	bool _edit_use_rect() const;
	bool _edit_use_rotation() const;
	bool _is_on_top() const;
	void _set_on_top(const bool on_top);
	void _toplevel_raise_self();
	void _update_callback();
	void draw_arc(const Vector2 center, const real_t radius, const real_t start_angle, const real_t end_angle, const int64_t point_count, const Color color, const real_t width = 1, const bool antialiased = false);
	real_t draw_char(const Ref<Font> font, const Vector2 position, const String _char, const String next, const Color modulate = Color(1,1,1,1));
	void draw_circle(const Vector2 position, const real_t radius, const Color color);
	void draw_colored_polygon(const PoolVector2Array points, const Color color, const PoolVector2Array uvs = PoolVector2Array(), const Ref<Texture> texture = nullptr, const Ref<Texture> normal_map = nullptr, const bool antialiased = false);
	void draw_line(const Vector2 from, const Vector2 to, const Color color, const real_t width = 1, const bool antialiased = false);
	void draw_mesh(const Ref<Mesh> mesh, const Ref<Texture> texture, const Ref<Texture> normal_map = nullptr, const Transform2D transform = Transform2D(), const Color modulate = Color(1,1,1,1));
	void draw_multiline(const PoolVector2Array points, const Color color, const real_t width = 1, const bool antialiased = false);
	void draw_multiline_colors(const PoolVector2Array points, const PoolColorArray colors, const real_t width = 1, const bool antialiased = false);
	void draw_multimesh(const Ref<MultiMesh> multimesh, const Ref<Texture> texture, const Ref<Texture> normal_map = nullptr);
	void draw_polygon(const PoolVector2Array points, const PoolColorArray colors, const PoolVector2Array uvs = PoolVector2Array(), const Ref<Texture> texture = nullptr, const Ref<Texture> normal_map = nullptr, const bool antialiased = false);
	void draw_polyline(const PoolVector2Array points, const Color color, const real_t width = 1, const bool antialiased = false);
	void draw_polyline_colors(const PoolVector2Array points, const PoolColorArray colors, const real_t width = 1, const bool antialiased = false);
	void draw_primitive(const PoolVector2Array points, const PoolColorArray colors, const PoolVector2Array uvs, const Ref<Texture> texture = nullptr, const real_t width = 1, const Ref<Texture> normal_map = nullptr);
	void draw_rect(const Rect2 rect, const Color color, const bool filled = true, const real_t width = 1, const bool antialiased = false);
	void draw_set_transform(const Vector2 position, const real_t rotation, const Vector2 scale);
	void draw_set_transform_matrix(const Transform2D xform);
	void draw_string(const Ref<Font> font, const Vector2 position, const String text, const Color modulate = Color(1,1,1,1), const int64_t clip_w = -1);
	void draw_style_box(const Ref<StyleBox> style_box, const Rect2 rect);
	void draw_texture(const Ref<Texture> texture, const Vector2 position, const Color modulate = Color(1,1,1,1), const Ref<Texture> normal_map = nullptr);
	void draw_texture_rect(const Ref<Texture> texture, const Rect2 rect, const bool tile, const Color modulate = Color(1,1,1,1), const bool transpose = false, const Ref<Texture> normal_map = nullptr);
	void draw_texture_rect_region(const Ref<Texture> texture, const Rect2 rect, const Rect2 src_rect, const Color modulate = Color(1,1,1,1), const bool transpose = false, const Ref<Texture> normal_map = nullptr, const bool clip_uv = true);
	void force_update_transform();
	RID get_canvas() const;
	RID get_canvas_item() const;
	Transform2D get_canvas_transform() const;
	Vector2 get_global_mouse_position() const;
	Transform2D get_global_transform() const;
	Transform2D get_global_transform_with_canvas() const;
	int64_t get_light_mask() const;
	Vector2 get_local_mouse_position() const;
	Ref<Material> get_material() const;
	Color get_modulate() const;
	Color get_self_modulate() const;
	Transform2D get_transform() const;
	bool get_use_parent_material() const;
	Rect2 get_viewport_rect() const;
	Transform2D get_viewport_transform() const;
	Ref<World2D> get_world_2d() const;
	void hide();
	bool is_draw_behind_parent_enabled() const;
	bool is_local_transform_notification_enabled() const;
	bool is_set_as_toplevel() const;
	bool is_transform_notification_enabled() const;
	bool is_visible() const;
	bool is_visible_in_tree() const;
	Vector2 make_canvas_position_local(const Vector2 screen_point) const;
	Ref<InputEvent> make_input_local(const Ref<InputEvent> event) const;
	void set_as_toplevel(const bool enable);
	void set_draw_behind_parent(const bool enable);
	void set_light_mask(const int64_t light_mask);
	void set_material(const Ref<Material> material);
	void set_modulate(const Color modulate);
	void set_notify_local_transform(const bool enable);
	void set_notify_transform(const bool enable);
	void set_self_modulate(const Color self_modulate);
	void set_use_parent_material(const bool enable);
	void set_visible(const bool visible);
	void show();
	void update();

};

}

#endif