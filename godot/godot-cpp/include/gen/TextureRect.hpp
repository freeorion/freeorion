#ifndef GODOT_CPP_TEXTURERECT_HPP
#define GODOT_CPP_TEXTURERECT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "TextureRect.hpp"

#include "Control.hpp"
namespace godot {

class Texture;

class TextureRect : public Control {
	struct ___method_bindings {
		godot_method_bind *mb__texture_changed;
		godot_method_bind *mb_get_stretch_mode;
		godot_method_bind *mb_get_texture;
		godot_method_bind *mb_has_expand;
		godot_method_bind *mb_is_flipped_h;
		godot_method_bind *mb_is_flipped_v;
		godot_method_bind *mb_set_expand;
		godot_method_bind *mb_set_flip_h;
		godot_method_bind *mb_set_flip_v;
		godot_method_bind *mb_set_stretch_mode;
		godot_method_bind *mb_set_texture;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "TextureRect"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum StretchMode {
		STRETCH_SCALE_ON_EXPAND = 0,
		STRETCH_SCALE = 1,
		STRETCH_TILE = 2,
		STRETCH_KEEP = 3,
		STRETCH_KEEP_CENTERED = 4,
		STRETCH_KEEP_ASPECT = 5,
		STRETCH_KEEP_ASPECT_CENTERED = 6,
		STRETCH_KEEP_ASPECT_COVERED = 7,
	};

	// constants


	static TextureRect *_new();

	// methods
	void _texture_changed();
	TextureRect::StretchMode get_stretch_mode() const;
	Ref<Texture> get_texture() const;
	bool has_expand() const;
	bool is_flipped_h() const;
	bool is_flipped_v() const;
	void set_expand(const bool enable);
	void set_flip_h(const bool enable);
	void set_flip_v(const bool enable);
	void set_stretch_mode(const int64_t stretch_mode);
	void set_texture(const Ref<Texture> texture);

};

}

#endif