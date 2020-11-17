#include "World2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Physics2DDirectSpaceState.hpp"


namespace godot {


World2D::___method_bindings World2D::___mb = {};

void World2D::___init_method_bindings() {
	___mb.mb_get_canvas = godot::api->godot_method_bind_get_method("World2D", "get_canvas");
	___mb.mb_get_direct_space_state = godot::api->godot_method_bind_get_method("World2D", "get_direct_space_state");
	___mb.mb_get_space = godot::api->godot_method_bind_get_method("World2D", "get_space");
}

World2D *World2D::_new()
{
	return (World2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"World2D")());
}
RID World2D::get_canvas() {
	return ___godot_icall_RID(___mb.mb_get_canvas, (const Object *) this);
}

Physics2DDirectSpaceState *World2D::get_direct_space_state() {
	return (Physics2DDirectSpaceState *) ___godot_icall_Object(___mb.mb_get_direct_space_state, (const Object *) this);
}

RID World2D::get_space() {
	return ___godot_icall_RID(___mb.mb_get_space, (const Object *) this);
}

}