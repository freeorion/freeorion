#ifndef GODOT_CPP_CUBEMAP_HPP
#define GODOT_CPP_CUBEMAP_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "CubeMap.hpp"

#include "Resource.hpp"
namespace godot {

class Image;

class CubeMap : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_get_flags;
		godot_method_bind *mb_get_height;
		godot_method_bind *mb_get_lossy_storage_quality;
		godot_method_bind *mb_get_side;
		godot_method_bind *mb_get_storage;
		godot_method_bind *mb_get_width;
		godot_method_bind *mb_set_flags;
		godot_method_bind *mb_set_lossy_storage_quality;
		godot_method_bind *mb_set_side;
		godot_method_bind *mb_set_storage;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CubeMap"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Flags {
		FLAG_MIPMAPS = 1,
		FLAG_REPEAT = 2,
		FLAG_FILTER = 4,
		FLAGS_DEFAULT = 7,
	};
	enum Side {
		SIDE_LEFT = 0,
		SIDE_RIGHT = 1,
		SIDE_BOTTOM = 2,
		SIDE_TOP = 3,
		SIDE_FRONT = 4,
		SIDE_BACK = 5,
	};
	enum Storage {
		STORAGE_RAW = 0,
		STORAGE_COMPRESS_LOSSY = 1,
		STORAGE_COMPRESS_LOSSLESS = 2,
	};

	// constants


	static CubeMap *_new();

	// methods
	int64_t get_flags() const;
	int64_t get_height() const;
	real_t get_lossy_storage_quality() const;
	Ref<Image> get_side(const int64_t side) const;
	CubeMap::Storage get_storage() const;
	int64_t get_width() const;
	void set_flags(const int64_t flags);
	void set_lossy_storage_quality(const real_t quality);
	void set_side(const int64_t side, const Ref<Image> image);
	void set_storage(const int64_t mode);

};

}

#endif