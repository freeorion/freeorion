#ifndef GODOT_CPP_LINKBUTTON_HPP
#define GODOT_CPP_LINKBUTTON_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "LinkButton.hpp"

#include "BaseButton.hpp"
namespace godot {


class LinkButton : public BaseButton {
	struct ___method_bindings {
		godot_method_bind *mb_get_text;
		godot_method_bind *mb_get_underline_mode;
		godot_method_bind *mb_set_text;
		godot_method_bind *mb_set_underline_mode;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "LinkButton"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum UnderlineMode {
		UNDERLINE_MODE_ALWAYS = 0,
		UNDERLINE_MODE_ON_HOVER = 1,
		UNDERLINE_MODE_NEVER = 2,
	};

	// constants


	static LinkButton *_new();

	// methods
	String get_text() const;
	LinkButton::UnderlineMode get_underline_mode() const;
	void set_text(const String text);
	void set_underline_mode(const int64_t underline_mode);

};

}

#endif