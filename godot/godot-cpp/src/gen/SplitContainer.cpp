#include "SplitContainer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"


namespace godot {


SplitContainer::___method_bindings SplitContainer::___mb = {};

void SplitContainer::___init_method_bindings() {
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("SplitContainer", "_gui_input");
	___mb.mb_clamp_split_offset = godot::api->godot_method_bind_get_method("SplitContainer", "clamp_split_offset");
	___mb.mb_get_dragger_visibility = godot::api->godot_method_bind_get_method("SplitContainer", "get_dragger_visibility");
	___mb.mb_get_split_offset = godot::api->godot_method_bind_get_method("SplitContainer", "get_split_offset");
	___mb.mb_is_collapsed = godot::api->godot_method_bind_get_method("SplitContainer", "is_collapsed");
	___mb.mb_set_collapsed = godot::api->godot_method_bind_get_method("SplitContainer", "set_collapsed");
	___mb.mb_set_dragger_visibility = godot::api->godot_method_bind_get_method("SplitContainer", "set_dragger_visibility");
	___mb.mb_set_split_offset = godot::api->godot_method_bind_get_method("SplitContainer", "set_split_offset");
}

void SplitContainer::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

void SplitContainer::clamp_split_offset() {
	___godot_icall_void(___mb.mb_clamp_split_offset, (const Object *) this);
}

SplitContainer::DraggerVisibility SplitContainer::get_dragger_visibility() const {
	return (SplitContainer::DraggerVisibility) ___godot_icall_int(___mb.mb_get_dragger_visibility, (const Object *) this);
}

int64_t SplitContainer::get_split_offset() const {
	return ___godot_icall_int(___mb.mb_get_split_offset, (const Object *) this);
}

bool SplitContainer::is_collapsed() const {
	return ___godot_icall_bool(___mb.mb_is_collapsed, (const Object *) this);
}

void SplitContainer::set_collapsed(const bool collapsed) {
	___godot_icall_void_bool(___mb.mb_set_collapsed, (const Object *) this, collapsed);
}

void SplitContainer::set_dragger_visibility(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_dragger_visibility, (const Object *) this, mode);
}

void SplitContainer::set_split_offset(const int64_t offset) {
	___godot_icall_void_int(___mb.mb_set_split_offset, (const Object *) this, offset);
}

}