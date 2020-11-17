#ifndef GODOT_CPP_DYNAMICFONTDATA_HPP
#define GODOT_CPP_DYNAMICFONTDATA_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "DynamicFontData.hpp"

#include "Resource.hpp"
namespace godot {


class DynamicFontData : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_get_font_path;
		godot_method_bind *mb_get_hinting;
		godot_method_bind *mb_is_antialiased;
		godot_method_bind *mb_set_antialiased;
		godot_method_bind *mb_set_font_path;
		godot_method_bind *mb_set_hinting;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "DynamicFontData"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Hinting {
		HINTING_NONE = 0,
		HINTING_LIGHT = 1,
		HINTING_NORMAL = 2,
	};

	// constants


	static DynamicFontData *_new();

	// methods
	String get_font_path() const;
	DynamicFontData::Hinting get_hinting() const;
	bool is_antialiased() const;
	void set_antialiased(const bool antialiased);
	void set_font_path(const String path);
	void set_hinting(const int64_t mode);

};

}

#endif