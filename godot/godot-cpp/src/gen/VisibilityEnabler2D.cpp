#include "VisibilityEnabler2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Node.hpp"


namespace godot {


VisibilityEnabler2D::___method_bindings VisibilityEnabler2D::___mb = {};

void VisibilityEnabler2D::___init_method_bindings() {
	___mb.mb__node_removed = godot::api->godot_method_bind_get_method("VisibilityEnabler2D", "_node_removed");
	___mb.mb_is_enabler_enabled = godot::api->godot_method_bind_get_method("VisibilityEnabler2D", "is_enabler_enabled");
	___mb.mb_set_enabler = godot::api->godot_method_bind_get_method("VisibilityEnabler2D", "set_enabler");
}

VisibilityEnabler2D *VisibilityEnabler2D::_new()
{
	return (VisibilityEnabler2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisibilityEnabler2D")());
}
void VisibilityEnabler2D::_node_removed(const Node *arg0) {
	___godot_icall_void_Object(___mb.mb__node_removed, (const Object *) this, arg0);
}

bool VisibilityEnabler2D::is_enabler_enabled(const int64_t enabler) const {
	return ___godot_icall_bool_int(___mb.mb_is_enabler_enabled, (const Object *) this, enabler);
}

void VisibilityEnabler2D::set_enabler(const int64_t enabler, const bool enabled) {
	___godot_icall_void_int_bool(___mb.mb_set_enabler, (const Object *) this, enabler, enabled);
}

}