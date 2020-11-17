#ifndef GODOT_CPP_TEXTUREBUTTON_HPP
#define GODOT_CPP_TEXTUREBUTTON_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "TextureButton.hpp"

#include "BaseButton.hpp"
namespace godot {

class BitMap;
class Texture;

class TextureButton : public BaseButton {
	struct ___method_bindings {
		godot_method_bind *mb_get_click_mask;
		godot_method_bind *mb_get_disabled_texture;
		godot_method_bind *mb_get_expand;
		godot_method_bind *mb_get_focused_texture;
		godot_method_bind *mb_get_hover_texture;
		godot_method_bind *mb_get_normal_texture;
		godot_method_bind *mb_get_pressed_texture;
		godot_method_bind *mb_get_stretch_mode;
		godot_method_bind *mb_set_click_mask;
		godot_method_bind *mb_set_disabled_texture;
		godot_method_bind *mb_set_expand;
		godot_method_bind *mb_set_focused_texture;
		godot_method_bind *mb_set_hover_texture;
		godot_method_bind *mb_set_normal_texture;
		godot_method_bind *mb_set_pressed_texture;
		godot_method_bind *mb_set_stretch_mode;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "TextureButton"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum StretchMode {
		STRETCH_SCALE = 0,
		STRETCH_TILE = 1,
		STRETCH_KEEP = 2,
		STRETCH_KEEP_CENTERED = 3,
		STRETCH_KEEP_ASPECT = 4,
		STRETCH_KEEP_ASPECT_CENTERED = 5,
		STRETCH_KEEP_ASPECT_COVERED = 6,
	};

	// constants


	static TextureButton *_new();

	// methods
	Ref<BitMap> get_click_mask() const;
	Ref<Texture> get_disabled_texture() const;
	bool get_expand() const;
	Ref<Texture> get_focused_texture() const;
	Ref<Texture> get_hover_texture() const;
	Ref<Texture> get_normal_texture() const;
	Ref<Texture> get_pressed_texture() const;
	TextureButton::StretchMode get_stretch_mode() const;
	void set_click_mask(const Ref<BitMap> mask);
	void set_disabled_texture(const Ref<Texture> texture);
	void set_expand(const bool p_expand);
	void set_focused_texture(const Ref<Texture> texture);
	void set_hover_texture(const Ref<Texture> texture);
	void set_normal_texture(const Ref<Texture> texture);
	void set_pressed_texture(const Ref<Texture> texture);
	void set_stretch_mode(const int64_t p_mode);

};

}

#endif