#include "Shape.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Shape::___method_bindings Shape::___mb = {};

void Shape::___init_method_bindings() {
	___mb.mb_get_margin = godot::api->godot_method_bind_get_method("Shape", "get_margin");
	___mb.mb_set_margin = godot::api->godot_method_bind_get_method("Shape", "set_margin");
}

real_t Shape::get_margin() const {
	return ___godot_icall_float(___mb.mb_get_margin, (const Object *) this);
}

void Shape::set_margin(const real_t margin) {
	___godot_icall_void_float(___mb.mb_set_margin, (const Object *) this, margin);
}

}