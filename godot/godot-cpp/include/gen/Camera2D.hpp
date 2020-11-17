#ifndef GODOT_CPP_CAMERA2D_HPP
#define GODOT_CPP_CAMERA2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Camera2D.hpp"

#include "Node2D.hpp"
namespace godot {

class Object;
class Node;

class Camera2D : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb__make_current;
		godot_method_bind *mb__set_current;
		godot_method_bind *mb__set_old_smoothing;
		godot_method_bind *mb__update_scroll;
		godot_method_bind *mb_align;
		godot_method_bind *mb_clear_current;
		godot_method_bind *mb_force_update_scroll;
		godot_method_bind *mb_get_anchor_mode;
		godot_method_bind *mb_get_camera_position;
		godot_method_bind *mb_get_camera_screen_center;
		godot_method_bind *mb_get_custom_viewport;
		godot_method_bind *mb_get_drag_margin;
		godot_method_bind *mb_get_follow_smoothing;
		godot_method_bind *mb_get_h_offset;
		godot_method_bind *mb_get_limit;
		godot_method_bind *mb_get_offset;
		godot_method_bind *mb_get_process_mode;
		godot_method_bind *mb_get_v_offset;
		godot_method_bind *mb_get_zoom;
		godot_method_bind *mb_is_current;
		godot_method_bind *mb_is_follow_smoothing_enabled;
		godot_method_bind *mb_is_h_drag_enabled;
		godot_method_bind *mb_is_limit_drawing_enabled;
		godot_method_bind *mb_is_limit_smoothing_enabled;
		godot_method_bind *mb_is_margin_drawing_enabled;
		godot_method_bind *mb_is_rotating;
		godot_method_bind *mb_is_screen_drawing_enabled;
		godot_method_bind *mb_is_v_drag_enabled;
		godot_method_bind *mb_make_current;
		godot_method_bind *mb_reset_smoothing;
		godot_method_bind *mb_set_anchor_mode;
		godot_method_bind *mb_set_custom_viewport;
		godot_method_bind *mb_set_drag_margin;
		godot_method_bind *mb_set_enable_follow_smoothing;
		godot_method_bind *mb_set_follow_smoothing;
		godot_method_bind *mb_set_h_drag_enabled;
		godot_method_bind *mb_set_h_offset;
		godot_method_bind *mb_set_limit;
		godot_method_bind *mb_set_limit_drawing_enabled;
		godot_method_bind *mb_set_limit_smoothing_enabled;
		godot_method_bind *mb_set_margin_drawing_enabled;
		godot_method_bind *mb_set_offset;
		godot_method_bind *mb_set_process_mode;
		godot_method_bind *mb_set_rotating;
		godot_method_bind *mb_set_screen_drawing_enabled;
		godot_method_bind *mb_set_v_drag_enabled;
		godot_method_bind *mb_set_v_offset;
		godot_method_bind *mb_set_zoom;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Camera2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Camera2DProcessMode {
		CAMERA2D_PROCESS_PHYSICS = 0,
		CAMERA2D_PROCESS_IDLE = 1,
	};
	enum AnchorMode {
		ANCHOR_MODE_FIXED_TOP_LEFT = 0,
		ANCHOR_MODE_DRAG_CENTER = 1,
	};

	// constants


	static Camera2D *_new();

	// methods
	void _make_current(const Object *arg0);
	void _set_current(const bool current);
	void _set_old_smoothing(const real_t follow_smoothing);
	void _update_scroll();
	void align();
	void clear_current();
	void force_update_scroll();
	Camera2D::AnchorMode get_anchor_mode() const;
	Vector2 get_camera_position() const;
	Vector2 get_camera_screen_center() const;
	Node *get_custom_viewport() const;
	real_t get_drag_margin(const int64_t margin) const;
	real_t get_follow_smoothing() const;
	real_t get_h_offset() const;
	int64_t get_limit(const int64_t margin) const;
	Vector2 get_offset() const;
	Camera2D::Camera2DProcessMode get_process_mode() const;
	real_t get_v_offset() const;
	Vector2 get_zoom() const;
	bool is_current() const;
	bool is_follow_smoothing_enabled() const;
	bool is_h_drag_enabled() const;
	bool is_limit_drawing_enabled() const;
	bool is_limit_smoothing_enabled() const;
	bool is_margin_drawing_enabled() const;
	bool is_rotating() const;
	bool is_screen_drawing_enabled() const;
	bool is_v_drag_enabled() const;
	void make_current();
	void reset_smoothing();
	void set_anchor_mode(const int64_t anchor_mode);
	void set_custom_viewport(const Node *viewport);
	void set_drag_margin(const int64_t margin, const real_t drag_margin);
	void set_enable_follow_smoothing(const bool follow_smoothing);
	void set_follow_smoothing(const real_t follow_smoothing);
	void set_h_drag_enabled(const bool enabled);
	void set_h_offset(const real_t ofs);
	void set_limit(const int64_t margin, const int64_t limit);
	void set_limit_drawing_enabled(const bool limit_drawing_enabled);
	void set_limit_smoothing_enabled(const bool limit_smoothing_enabled);
	void set_margin_drawing_enabled(const bool margin_drawing_enabled);
	void set_offset(const Vector2 offset);
	void set_process_mode(const int64_t mode);
	void set_rotating(const bool rotating);
	void set_screen_drawing_enabled(const bool screen_drawing_enabled);
	void set_v_drag_enabled(const bool enabled);
	void set_v_offset(const real_t ofs);
	void set_zoom(const Vector2 zoom);

};

}

#endif