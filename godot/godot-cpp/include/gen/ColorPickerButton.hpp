#ifndef GODOT_CPP_COLORPICKERBUTTON_HPP
#define GODOT_CPP_COLORPICKERBUTTON_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Button.hpp"
namespace godot {

class ColorPicker;
class PopupPanel;

class ColorPickerButton : public Button {
	struct ___method_bindings {
		godot_method_bind *mb__color_changed;
		godot_method_bind *mb__modal_closed;
		godot_method_bind *mb_get_pick_color;
		godot_method_bind *mb_get_picker;
		godot_method_bind *mb_get_popup;
		godot_method_bind *mb_is_editing_alpha;
		godot_method_bind *mb_set_edit_alpha;
		godot_method_bind *mb_set_pick_color;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ColorPickerButton"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ColorPickerButton *_new();

	// methods
	void _color_changed(const Color arg0);
	void _modal_closed();
	Color get_pick_color() const;
	ColorPicker *get_picker();
	PopupPanel *get_popup();
	bool is_editing_alpha() const;
	void set_edit_alpha(const bool show);
	void set_pick_color(const Color color);

};

}

#endif