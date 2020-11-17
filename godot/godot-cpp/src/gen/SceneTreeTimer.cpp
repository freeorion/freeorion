#include "SceneTreeTimer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


SceneTreeTimer::___method_bindings SceneTreeTimer::___mb = {};

void SceneTreeTimer::___init_method_bindings() {
	___mb.mb_get_time_left = godot::api->godot_method_bind_get_method("SceneTreeTimer", "get_time_left");
	___mb.mb_set_time_left = godot::api->godot_method_bind_get_method("SceneTreeTimer", "set_time_left");
}

real_t SceneTreeTimer::get_time_left() const {
	return ___godot_icall_float(___mb.mb_get_time_left, (const Object *) this);
}

void SceneTreeTimer::set_time_left(const real_t time) {
	___godot_icall_void_float(___mb.mb_set_time_left, (const Object *) this, time);
}

}