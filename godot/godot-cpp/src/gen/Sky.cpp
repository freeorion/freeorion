#include "Sky.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Sky::___method_bindings Sky::___mb = {};

void Sky::___init_method_bindings() {
	___mb.mb_get_radiance_size = godot::api->godot_method_bind_get_method("Sky", "get_radiance_size");
	___mb.mb_set_radiance_size = godot::api->godot_method_bind_get_method("Sky", "set_radiance_size");
}

Sky::RadianceSize Sky::get_radiance_size() const {
	return (Sky::RadianceSize) ___godot_icall_int(___mb.mb_get_radiance_size, (const Object *) this);
}

void Sky::set_radiance_size(const int64_t size) {
	___godot_icall_void_int(___mb.mb_set_radiance_size, (const Object *) this, size);
}

}