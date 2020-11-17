#ifndef GODOT_CPP_PARALLAXBACKGROUND_HPP
#define GODOT_CPP_PARALLAXBACKGROUND_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "CanvasLayer.hpp"
namespace godot {


class ParallaxBackground : public CanvasLayer {
	struct ___method_bindings {
		godot_method_bind *mb__camera_moved;
		godot_method_bind *mb_get_limit_begin;
		godot_method_bind *mb_get_limit_end;
		godot_method_bind *mb_get_scroll_base_offset;
		godot_method_bind *mb_get_scroll_base_scale;
		godot_method_bind *mb_get_scroll_offset;
		godot_method_bind *mb_is_ignore_camera_zoom;
		godot_method_bind *mb_set_ignore_camera_zoom;
		godot_method_bind *mb_set_limit_begin;
		godot_method_bind *mb_set_limit_end;
		godot_method_bind *mb_set_scroll_base_offset;
		godot_method_bind *mb_set_scroll_base_scale;
		godot_method_bind *mb_set_scroll_offset;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ParallaxBackground"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ParallaxBackground *_new();

	// methods
	void _camera_moved(const Transform2D arg0, const Vector2 arg1);
	Vector2 get_limit_begin() const;
	Vector2 get_limit_end() const;
	Vector2 get_scroll_base_offset() const;
	Vector2 get_scroll_base_scale() const;
	Vector2 get_scroll_offset() const;
	bool is_ignore_camera_zoom();
	void set_ignore_camera_zoom(const bool ignore);
	void set_limit_begin(const Vector2 ofs);
	void set_limit_end(const Vector2 ofs);
	void set_scroll_base_offset(const Vector2 ofs);
	void set_scroll_base_scale(const Vector2 scale);
	void set_scroll_offset(const Vector2 ofs);

};

}

#endif