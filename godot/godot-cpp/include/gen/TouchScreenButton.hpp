#ifndef GODOT_CPP_TOUCHSCREENBUTTON_HPP
#define GODOT_CPP_TOUCHSCREENBUTTON_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "TouchScreenButton.hpp"

#include "Node2D.hpp"
namespace godot {

class InputEvent;
class BitMap;
class Shape2D;
class Texture;

class TouchScreenButton : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb__input;
		godot_method_bind *mb_get_action;
		godot_method_bind *mb_get_bitmask;
		godot_method_bind *mb_get_shape;
		godot_method_bind *mb_get_texture;
		godot_method_bind *mb_get_texture_pressed;
		godot_method_bind *mb_get_visibility_mode;
		godot_method_bind *mb_is_passby_press_enabled;
		godot_method_bind *mb_is_pressed;
		godot_method_bind *mb_is_shape_centered;
		godot_method_bind *mb_is_shape_visible;
		godot_method_bind *mb_set_action;
		godot_method_bind *mb_set_bitmask;
		godot_method_bind *mb_set_passby_press;
		godot_method_bind *mb_set_shape;
		godot_method_bind *mb_set_shape_centered;
		godot_method_bind *mb_set_shape_visible;
		godot_method_bind *mb_set_texture;
		godot_method_bind *mb_set_texture_pressed;
		godot_method_bind *mb_set_visibility_mode;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "TouchScreenButton"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum VisibilityMode {
		VISIBILITY_ALWAYS = 0,
		VISIBILITY_TOUCHSCREEN_ONLY = 1,
	};

	// constants


	static TouchScreenButton *_new();

	// methods
	void _input(const Ref<InputEvent> arg0);
	String get_action() const;
	Ref<BitMap> get_bitmask() const;
	Ref<Shape2D> get_shape() const;
	Ref<Texture> get_texture() const;
	Ref<Texture> get_texture_pressed() const;
	TouchScreenButton::VisibilityMode get_visibility_mode() const;
	bool is_passby_press_enabled() const;
	bool is_pressed() const;
	bool is_shape_centered() const;
	bool is_shape_visible() const;
	void set_action(const String action);
	void set_bitmask(const Ref<BitMap> bitmask);
	void set_passby_press(const bool enabled);
	void set_shape(const Ref<Shape2D> shape);
	void set_shape_centered(const bool _bool);
	void set_shape_visible(const bool _bool);
	void set_texture(const Ref<Texture> texture);
	void set_texture_pressed(const Ref<Texture> texture_pressed);
	void set_visibility_mode(const int64_t mode);

};

}

#endif