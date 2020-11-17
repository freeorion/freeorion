#ifndef GODOT_CPP_VISIBILITYNOTIFIER2D_HPP
#define GODOT_CPP_VISIBILITYNOTIFIER2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node2D.hpp"
namespace godot {


class VisibilityNotifier2D : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb_get_rect;
		godot_method_bind *mb_is_on_screen;
		godot_method_bind *mb_set_rect;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisibilityNotifier2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static VisibilityNotifier2D *_new();

	// methods
	Rect2 get_rect() const;
	bool is_on_screen() const;
	void set_rect(const Rect2 rect);

};

}

#endif