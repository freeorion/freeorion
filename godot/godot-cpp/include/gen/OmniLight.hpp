#ifndef GODOT_CPP_OMNILIGHT_HPP
#define GODOT_CPP_OMNILIGHT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "OmniLight.hpp"

#include "Light.hpp"
namespace godot {


class OmniLight : public Light {
	struct ___method_bindings {
		godot_method_bind *mb_get_shadow_detail;
		godot_method_bind *mb_get_shadow_mode;
		godot_method_bind *mb_set_shadow_detail;
		godot_method_bind *mb_set_shadow_mode;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "OmniLight"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum ShadowMode {
		SHADOW_DUAL_PARABOLOID = 0,
		SHADOW_CUBE = 1,
	};
	enum ShadowDetail {
		SHADOW_DETAIL_VERTICAL = 0,
		SHADOW_DETAIL_HORIZONTAL = 1,
	};

	// constants


	static OmniLight *_new();

	// methods
	OmniLight::ShadowDetail get_shadow_detail() const;
	OmniLight::ShadowMode get_shadow_mode() const;
	void set_shadow_detail(const int64_t detail);
	void set_shadow_mode(const int64_t mode);

};

}

#endif