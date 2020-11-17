#ifndef GODOT_CPP_BITMAP_HPP
#define GODOT_CPP_BITMAP_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class Image;

class BitMap : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb__get_data;
		godot_method_bind *mb__set_data;
		godot_method_bind *mb_create;
		godot_method_bind *mb_create_from_image_alpha;
		godot_method_bind *mb_get_bit;
		godot_method_bind *mb_get_size;
		godot_method_bind *mb_get_true_bit_count;
		godot_method_bind *mb_grow_mask;
		godot_method_bind *mb_opaque_to_polygons;
		godot_method_bind *mb_set_bit;
		godot_method_bind *mb_set_bit_rect;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "BitMap"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static BitMap *_new();

	// methods
	Dictionary _get_data() const;
	void _set_data(const Dictionary arg0);
	void create(const Vector2 size);
	void create_from_image_alpha(const Ref<Image> image, const real_t threshold = 0.1);
	bool get_bit(const Vector2 position) const;
	Vector2 get_size() const;
	int64_t get_true_bit_count() const;
	void grow_mask(const int64_t pixels, const Rect2 rect);
	Array opaque_to_polygons(const Rect2 rect, const real_t epsilon = 2) const;
	void set_bit(const Vector2 position, const bool bit);
	void set_bit_rect(const Rect2 rect, const bool bit);

};

}

#endif