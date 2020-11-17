#ifndef GODOT_CPP_STYLEBOX_HPP
#define GODOT_CPP_STYLEBOX_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class CanvasItem;

class StyleBox : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_draw;
		godot_method_bind *mb_get_center_size;
		godot_method_bind *mb_get_current_item_drawn;
		godot_method_bind *mb_get_default_margin;
		godot_method_bind *mb_get_margin;
		godot_method_bind *mb_get_minimum_size;
		godot_method_bind *mb_get_offset;
		godot_method_bind *mb_set_default_margin;
		godot_method_bind *mb_test_mask;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "StyleBox"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void draw(const RID canvas_item, const Rect2 rect) const;
	Vector2 get_center_size() const;
	CanvasItem *get_current_item_drawn() const;
	real_t get_default_margin(const int64_t margin) const;
	real_t get_margin(const int64_t margin) const;
	Vector2 get_minimum_size() const;
	Vector2 get_offset() const;
	void set_default_margin(const int64_t margin, const real_t offset);
	bool test_mask(const Vector2 point, const Rect2 rect) const;

};

}

#endif