#ifndef GODOT_CPP_MOBILEVRINTERFACE_HPP
#define GODOT_CPP_MOBILEVRINTERFACE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "ARVRInterface.hpp"
namespace godot {


class MobileVRInterface : public ARVRInterface {
	struct ___method_bindings {
		godot_method_bind *mb_get_display_to_lens;
		godot_method_bind *mb_get_display_width;
		godot_method_bind *mb_get_eye_height;
		godot_method_bind *mb_get_iod;
		godot_method_bind *mb_get_k1;
		godot_method_bind *mb_get_k2;
		godot_method_bind *mb_get_oversample;
		godot_method_bind *mb_set_display_to_lens;
		godot_method_bind *mb_set_display_width;
		godot_method_bind *mb_set_eye_height;
		godot_method_bind *mb_set_iod;
		godot_method_bind *mb_set_k1;
		godot_method_bind *mb_set_k2;
		godot_method_bind *mb_set_oversample;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "MobileVRInterface"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static MobileVRInterface *_new();

	// methods
	real_t get_display_to_lens() const;
	real_t get_display_width() const;
	real_t get_eye_height() const;
	real_t get_iod() const;
	real_t get_k1() const;
	real_t get_k2() const;
	real_t get_oversample() const;
	void set_display_to_lens(const real_t display_to_lens);
	void set_display_width(const real_t display_width);
	void set_eye_height(const real_t eye_height);
	void set_iod(const real_t iod);
	void set_k1(const real_t k);
	void set_k2(const real_t k);
	void set_oversample(const real_t oversample);

};

}

#endif