#include "Container.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Control.hpp"


namespace godot {


Container::___method_bindings Container::___mb = {};

void Container::___init_method_bindings() {
	___mb.mb__child_minsize_changed = godot::api->godot_method_bind_get_method("Container", "_child_minsize_changed");
	___mb.mb__sort_children = godot::api->godot_method_bind_get_method("Container", "_sort_children");
	___mb.mb_fit_child_in_rect = godot::api->godot_method_bind_get_method("Container", "fit_child_in_rect");
	___mb.mb_queue_sort = godot::api->godot_method_bind_get_method("Container", "queue_sort");
}

Container *Container::_new()
{
	return (Container *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Container")());
}
void Container::_child_minsize_changed() {
	___godot_icall_void(___mb.mb__child_minsize_changed, (const Object *) this);
}

void Container::_sort_children() {
	___godot_icall_void(___mb.mb__sort_children, (const Object *) this);
}

void Container::fit_child_in_rect(const Control *child, const Rect2 rect) {
	___godot_icall_void_Object_Rect2(___mb.mb_fit_child_in_rect, (const Object *) this, child, rect);
}

void Container::queue_sort() {
	___godot_icall_void(___mb.mb_queue_sort, (const Object *) this);
}

}