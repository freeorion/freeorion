#ifndef GODOT_CPP_POPUP_HPP
#define GODOT_CPP_POPUP_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Control.hpp"
namespace godot {


class Popup : public Control {
	struct ___method_bindings {
		godot_method_bind *mb_is_exclusive;
		godot_method_bind *mb_popup;
		godot_method_bind *mb_popup_centered;
		godot_method_bind *mb_popup_centered_clamped;
		godot_method_bind *mb_popup_centered_minsize;
		godot_method_bind *mb_popup_centered_ratio;
		godot_method_bind *mb_set_as_minsize;
		godot_method_bind *mb_set_exclusive;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Popup"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants
	const static int NOTIFICATION_POPUP_HIDE = 81;
	const static int NOTIFICATION_POST_POPUP = 80;


	static Popup *_new();

	// methods
	bool is_exclusive() const;
	void popup(const Rect2 bounds = Rect2(0, 0, 0, 0));
	void popup_centered(const Vector2 size = Vector2(0, 0));
	void popup_centered_clamped(const Vector2 size = Vector2(0, 0), const real_t fallback_ratio = 0.75);
	void popup_centered_minsize(const Vector2 minsize = Vector2(0, 0));
	void popup_centered_ratio(const real_t ratio = 0.75);
	void set_as_minsize();
	void set_exclusive(const bool enable);

};

}

#endif