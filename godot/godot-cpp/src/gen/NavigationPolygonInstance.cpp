#include "NavigationPolygonInstance.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "NavigationPolygon.hpp"


namespace godot {


NavigationPolygonInstance::___method_bindings NavigationPolygonInstance::___mb = {};

void NavigationPolygonInstance::___init_method_bindings() {
	___mb.mb__navpoly_changed = godot::api->godot_method_bind_get_method("NavigationPolygonInstance", "_navpoly_changed");
	___mb.mb_get_navigation_polygon = godot::api->godot_method_bind_get_method("NavigationPolygonInstance", "get_navigation_polygon");
	___mb.mb_is_enabled = godot::api->godot_method_bind_get_method("NavigationPolygonInstance", "is_enabled");
	___mb.mb_set_enabled = godot::api->godot_method_bind_get_method("NavigationPolygonInstance", "set_enabled");
	___mb.mb_set_navigation_polygon = godot::api->godot_method_bind_get_method("NavigationPolygonInstance", "set_navigation_polygon");
}

NavigationPolygonInstance *NavigationPolygonInstance::_new()
{
	return (NavigationPolygonInstance *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"NavigationPolygonInstance")());
}
void NavigationPolygonInstance::_navpoly_changed() {
	___godot_icall_void(___mb.mb__navpoly_changed, (const Object *) this);
}

Ref<NavigationPolygon> NavigationPolygonInstance::get_navigation_polygon() const {
	return Ref<NavigationPolygon>::__internal_constructor(___godot_icall_Object(___mb.mb_get_navigation_polygon, (const Object *) this));
}

bool NavigationPolygonInstance::is_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_enabled, (const Object *) this);
}

void NavigationPolygonInstance::set_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_enabled, (const Object *) this, enabled);
}

void NavigationPolygonInstance::set_navigation_polygon(const Ref<NavigationPolygon> navpoly) {
	___godot_icall_void_Object(___mb.mb_set_navigation_polygon, (const Object *) this, navpoly.ptr());
}

}