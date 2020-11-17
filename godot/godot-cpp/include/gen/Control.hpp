#ifndef GODOT_CPP_CONTROL_HPP
#define GODOT_CPP_CONTROL_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Control.hpp"

#include "CanvasItem.hpp"
namespace godot {

class InputEvent;
class Object;
class Font;
class Texture;
class Shader;
class StyleBox;
class Control;
class Theme;

class Control : public CanvasItem {
	struct ___method_bindings {
		godot_method_bind *mb__clips_input;
		godot_method_bind *mb__get_minimum_size;
		godot_method_bind *mb__get_tooltip;
		godot_method_bind *mb__gui_input;
		godot_method_bind *mb__make_custom_tooltip;
		godot_method_bind *mb__override_changed;
		godot_method_bind *mb__set_anchor;
		godot_method_bind *mb__set_global_position;
		godot_method_bind *mb__set_position;
		godot_method_bind *mb__set_size;
		godot_method_bind *mb__size_changed;
		godot_method_bind *mb__theme_changed;
		godot_method_bind *mb__update_minimum_size;
		godot_method_bind *mb_accept_event;
		godot_method_bind *mb_add_color_override;
		godot_method_bind *mb_add_constant_override;
		godot_method_bind *mb_add_font_override;
		godot_method_bind *mb_add_icon_override;
		godot_method_bind *mb_add_shader_override;
		godot_method_bind *mb_add_stylebox_override;
		godot_method_bind *mb_can_drop_data;
		godot_method_bind *mb_drop_data;
		godot_method_bind *mb_force_drag;
		godot_method_bind *mb_get_anchor;
		godot_method_bind *mb_get_begin;
		godot_method_bind *mb_get_color;
		godot_method_bind *mb_get_combined_minimum_size;
		godot_method_bind *mb_get_constant;
		godot_method_bind *mb_get_cursor_shape;
		godot_method_bind *mb_get_custom_minimum_size;
		godot_method_bind *mb_get_default_cursor_shape;
		godot_method_bind *mb_get_drag_data;
		godot_method_bind *mb_get_end;
		godot_method_bind *mb_get_focus_mode;
		godot_method_bind *mb_get_focus_neighbour;
		godot_method_bind *mb_get_focus_next;
		godot_method_bind *mb_get_focus_owner;
		godot_method_bind *mb_get_focus_previous;
		godot_method_bind *mb_get_font;
		godot_method_bind *mb_get_global_position;
		godot_method_bind *mb_get_global_rect;
		godot_method_bind *mb_get_h_grow_direction;
		godot_method_bind *mb_get_h_size_flags;
		godot_method_bind *mb_get_icon;
		godot_method_bind *mb_get_margin;
		godot_method_bind *mb_get_minimum_size;
		godot_method_bind *mb_get_mouse_filter;
		godot_method_bind *mb_get_parent_area_size;
		godot_method_bind *mb_get_parent_control;
		godot_method_bind *mb_get_pivot_offset;
		godot_method_bind *mb_get_position;
		godot_method_bind *mb_get_rect;
		godot_method_bind *mb_get_rotation;
		godot_method_bind *mb_get_rotation_degrees;
		godot_method_bind *mb_get_scale;
		godot_method_bind *mb_get_size;
		godot_method_bind *mb_get_stretch_ratio;
		godot_method_bind *mb_get_stylebox;
		godot_method_bind *mb_get_theme;
		godot_method_bind *mb_get_tooltip;
		godot_method_bind *mb_get_v_grow_direction;
		godot_method_bind *mb_get_v_size_flags;
		godot_method_bind *mb_grab_click_focus;
		godot_method_bind *mb_grab_focus;
		godot_method_bind *mb_has_color;
		godot_method_bind *mb_has_color_override;
		godot_method_bind *mb_has_constant;
		godot_method_bind *mb_has_constant_override;
		godot_method_bind *mb_has_focus;
		godot_method_bind *mb_has_font;
		godot_method_bind *mb_has_font_override;
		godot_method_bind *mb_has_icon;
		godot_method_bind *mb_has_icon_override;
		godot_method_bind *mb_has_point;
		godot_method_bind *mb_has_shader_override;
		godot_method_bind *mb_has_stylebox;
		godot_method_bind *mb_has_stylebox_override;
		godot_method_bind *mb_is_clipping_contents;
		godot_method_bind *mb_minimum_size_changed;
		godot_method_bind *mb_release_focus;
		godot_method_bind *mb_set_anchor;
		godot_method_bind *mb_set_anchor_and_margin;
		godot_method_bind *mb_set_anchors_and_margins_preset;
		godot_method_bind *mb_set_anchors_preset;
		godot_method_bind *mb_set_begin;
		godot_method_bind *mb_set_clip_contents;
		godot_method_bind *mb_set_custom_minimum_size;
		godot_method_bind *mb_set_default_cursor_shape;
		godot_method_bind *mb_set_drag_forwarding;
		godot_method_bind *mb_set_drag_preview;
		godot_method_bind *mb_set_end;
		godot_method_bind *mb_set_focus_mode;
		godot_method_bind *mb_set_focus_neighbour;
		godot_method_bind *mb_set_focus_next;
		godot_method_bind *mb_set_focus_previous;
		godot_method_bind *mb_set_global_position;
		godot_method_bind *mb_set_h_grow_direction;
		godot_method_bind *mb_set_h_size_flags;
		godot_method_bind *mb_set_margin;
		godot_method_bind *mb_set_margins_preset;
		godot_method_bind *mb_set_mouse_filter;
		godot_method_bind *mb_set_pivot_offset;
		godot_method_bind *mb_set_position;
		godot_method_bind *mb_set_rotation;
		godot_method_bind *mb_set_rotation_degrees;
		godot_method_bind *mb_set_scale;
		godot_method_bind *mb_set_size;
		godot_method_bind *mb_set_stretch_ratio;
		godot_method_bind *mb_set_theme;
		godot_method_bind *mb_set_tooltip;
		godot_method_bind *mb_set_v_grow_direction;
		godot_method_bind *mb_set_v_size_flags;
		godot_method_bind *mb_show_modal;
		godot_method_bind *mb_warp_mouse;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Control"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Anchor {
		ANCHOR_BEGIN = 0,
		ANCHOR_END = 1,
	};
	enum FocusMode {
		FOCUS_NONE = 0,
		FOCUS_CLICK = 1,
		FOCUS_ALL = 2,
	};
	enum LayoutPresetMode {
		PRESET_MODE_MINSIZE = 0,
		PRESET_MODE_KEEP_WIDTH = 1,
		PRESET_MODE_KEEP_HEIGHT = 2,
		PRESET_MODE_KEEP_SIZE = 3,
	};
	enum MouseFilter {
		MOUSE_FILTER_STOP = 0,
		MOUSE_FILTER_PASS = 1,
		MOUSE_FILTER_IGNORE = 2,
	};
	enum CursorShape {
		CURSOR_ARROW = 0,
		CURSOR_IBEAM = 1,
		CURSOR_POINTING_HAND = 2,
		CURSOR_CROSS = 3,
		CURSOR_WAIT = 4,
		CURSOR_BUSY = 5,
		CURSOR_DRAG = 6,
		CURSOR_CAN_DROP = 7,
		CURSOR_FORBIDDEN = 8,
		CURSOR_VSIZE = 9,
		CURSOR_HSIZE = 10,
		CURSOR_BDIAGSIZE = 11,
		CURSOR_FDIAGSIZE = 12,
		CURSOR_MOVE = 13,
		CURSOR_VSPLIT = 14,
		CURSOR_HSPLIT = 15,
		CURSOR_HELP = 16,
	};
	enum GrowDirection {
		GROW_DIRECTION_BEGIN = 0,
		GROW_DIRECTION_END = 1,
		GROW_DIRECTION_BOTH = 2,
	};
	enum SizeFlags {
		SIZE_FILL = 1,
		SIZE_EXPAND = 2,
		SIZE_EXPAND_FILL = 3,
		SIZE_SHRINK_CENTER = 4,
		SIZE_SHRINK_END = 8,
	};
	enum LayoutPreset {
		PRESET_TOP_LEFT = 0,
		PRESET_TOP_RIGHT = 1,
		PRESET_BOTTOM_LEFT = 2,
		PRESET_BOTTOM_RIGHT = 3,
		PRESET_CENTER_LEFT = 4,
		PRESET_CENTER_TOP = 5,
		PRESET_CENTER_RIGHT = 6,
		PRESET_CENTER_BOTTOM = 7,
		PRESET_CENTER = 8,
		PRESET_LEFT_WIDE = 9,
		PRESET_TOP_WIDE = 10,
		PRESET_RIGHT_WIDE = 11,
		PRESET_BOTTOM_WIDE = 12,
		PRESET_VCENTER_WIDE = 13,
		PRESET_HCENTER_WIDE = 14,
		PRESET_WIDE = 15,
	};

	// constants
	const static int NOTIFICATION_FOCUS_ENTER = 43;
	const static int NOTIFICATION_FOCUS_EXIT = 44;
	const static int NOTIFICATION_MODAL_CLOSE = 46;
	const static int NOTIFICATION_MOUSE_ENTER = 41;
	const static int NOTIFICATION_MOUSE_EXIT = 42;
	const static int NOTIFICATION_RESIZED = 40;
	const static int NOTIFICATION_SCROLL_BEGIN = 47;
	const static int NOTIFICATION_SCROLL_END = 48;
	const static int NOTIFICATION_THEME_CHANGED = 45;


	static Control *_new();

	// methods
	bool _clips_input();
	Vector2 _get_minimum_size();
	String _get_tooltip() const;
	void _gui_input(const Ref<InputEvent> event);
	Object *_make_custom_tooltip(const String for_text);
	void _override_changed();
	void _set_anchor(const int64_t margin, const real_t anchor);
	void _set_global_position(const Vector2 position);
	void _set_position(const Vector2 margin);
	void _set_size(const Vector2 size);
	void _size_changed();
	void _theme_changed();
	void _update_minimum_size();
	void accept_event();
	void add_color_override(const String name, const Color color);
	void add_constant_override(const String name, const int64_t constant);
	void add_font_override(const String name, const Ref<Font> font);
	void add_icon_override(const String name, const Ref<Texture> texture);
	void add_shader_override(const String name, const Ref<Shader> shader);
	void add_stylebox_override(const String name, const Ref<StyleBox> stylebox);
	bool can_drop_data(const Vector2 position, const Variant data);
	void drop_data(const Vector2 position, const Variant data);
	void force_drag(const Variant data, const Control *preview);
	real_t get_anchor(const int64_t margin) const;
	Vector2 get_begin() const;
	Color get_color(const String name, const String type = "") const;
	Vector2 get_combined_minimum_size() const;
	int64_t get_constant(const String name, const String type = "") const;
	Control::CursorShape get_cursor_shape(const Vector2 position = Vector2(0, 0)) const;
	Vector2 get_custom_minimum_size() const;
	Control::CursorShape get_default_cursor_shape() const;
	Variant get_drag_data(const Vector2 position);
	Vector2 get_end() const;
	Control::FocusMode get_focus_mode() const;
	NodePath get_focus_neighbour(const int64_t margin) const;
	NodePath get_focus_next() const;
	Control *get_focus_owner() const;
	NodePath get_focus_previous() const;
	Ref<Font> get_font(const String name, const String type = "") const;
	Vector2 get_global_position() const;
	Rect2 get_global_rect() const;
	Control::GrowDirection get_h_grow_direction() const;
	int64_t get_h_size_flags() const;
	Ref<Texture> get_icon(const String name, const String type = "") const;
	real_t get_margin(const int64_t margin) const;
	Vector2 get_minimum_size() const;
	Control::MouseFilter get_mouse_filter() const;
	Vector2 get_parent_area_size() const;
	Control *get_parent_control() const;
	Vector2 get_pivot_offset() const;
	Vector2 get_position() const;
	Rect2 get_rect() const;
	real_t get_rotation() const;
	real_t get_rotation_degrees() const;
	Vector2 get_scale() const;
	Vector2 get_size() const;
	real_t get_stretch_ratio() const;
	Ref<StyleBox> get_stylebox(const String name, const String type = "") const;
	Ref<Theme> get_theme() const;
	String get_tooltip(const Vector2 at_position = Vector2(0, 0)) const;
	Control::GrowDirection get_v_grow_direction() const;
	int64_t get_v_size_flags() const;
	void grab_click_focus();
	void grab_focus();
	bool has_color(const String name, const String type = "") const;
	bool has_color_override(const String name) const;
	bool has_constant(const String name, const String type = "") const;
	bool has_constant_override(const String name) const;
	bool has_focus() const;
	bool has_font(const String name, const String type = "") const;
	bool has_font_override(const String name) const;
	bool has_icon(const String name, const String type = "") const;
	bool has_icon_override(const String name) const;
	bool has_point(const Vector2 point);
	bool has_shader_override(const String name) const;
	bool has_stylebox(const String name, const String type = "") const;
	bool has_stylebox_override(const String name) const;
	bool is_clipping_contents();
	void minimum_size_changed();
	void release_focus();
	void set_anchor(const int64_t margin, const real_t anchor, const bool keep_margin = false, const bool push_opposite_anchor = true);
	void set_anchor_and_margin(const int64_t margin, const real_t anchor, const real_t offset, const bool push_opposite_anchor = false);
	void set_anchors_and_margins_preset(const int64_t preset, const int64_t resize_mode = 0, const int64_t margin = 0);
	void set_anchors_preset(const int64_t preset, const bool keep_margins = false);
	void set_begin(const Vector2 position);
	void set_clip_contents(const bool enable);
	void set_custom_minimum_size(const Vector2 size);
	void set_default_cursor_shape(const int64_t shape);
	void set_drag_forwarding(const Control *target);
	void set_drag_preview(const Control *control);
	void set_end(const Vector2 position);
	void set_focus_mode(const int64_t mode);
	void set_focus_neighbour(const int64_t margin, const NodePath neighbour);
	void set_focus_next(const NodePath next);
	void set_focus_previous(const NodePath previous);
	void set_global_position(const Vector2 position, const bool keep_margins = false);
	void set_h_grow_direction(const int64_t direction);
	void set_h_size_flags(const int64_t flags);
	void set_margin(const int64_t margin, const real_t offset);
	void set_margins_preset(const int64_t preset, const int64_t resize_mode = 0, const int64_t margin = 0);
	void set_mouse_filter(const int64_t filter);
	void set_pivot_offset(const Vector2 pivot_offset);
	void set_position(const Vector2 position, const bool keep_margins = false);
	void set_rotation(const real_t radians);
	void set_rotation_degrees(const real_t degrees);
	void set_scale(const Vector2 scale);
	void set_size(const Vector2 size, const bool keep_margins = false);
	void set_stretch_ratio(const real_t ratio);
	void set_theme(const Ref<Theme> theme);
	void set_tooltip(const String tooltip);
	void set_v_grow_direction(const int64_t direction);
	void set_v_size_flags(const int64_t flags);
	void show_modal(const bool exclusive = false);
	void warp_mouse(const Vector2 to_position);

};

}

#endif