#include "PackedDataContainerRef.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PackedDataContainerRef::___method_bindings PackedDataContainerRef::___mb = {};

void PackedDataContainerRef::___init_method_bindings() {
	___mb.mb__is_dictionary = godot::api->godot_method_bind_get_method("PackedDataContainerRef", "_is_dictionary");
	___mb.mb__iter_get = godot::api->godot_method_bind_get_method("PackedDataContainerRef", "_iter_get");
	___mb.mb__iter_init = godot::api->godot_method_bind_get_method("PackedDataContainerRef", "_iter_init");
	___mb.mb__iter_next = godot::api->godot_method_bind_get_method("PackedDataContainerRef", "_iter_next");
	___mb.mb_size = godot::api->godot_method_bind_get_method("PackedDataContainerRef", "size");
}

bool PackedDataContainerRef::_is_dictionary() const {
	return ___godot_icall_bool(___mb.mb__is_dictionary, (const Object *) this);
}

Variant PackedDataContainerRef::_iter_get(const Variant arg0) {
	return ___godot_icall_Variant_Variant(___mb.mb__iter_get, (const Object *) this, arg0);
}

Variant PackedDataContainerRef::_iter_init(const Array arg0) {
	return ___godot_icall_Variant_Array(___mb.mb__iter_init, (const Object *) this, arg0);
}

Variant PackedDataContainerRef::_iter_next(const Array arg0) {
	return ___godot_icall_Variant_Array(___mb.mb__iter_next, (const Object *) this, arg0);
}

int64_t PackedDataContainerRef::size() const {
	return ___godot_icall_int(___mb.mb_size, (const Object *) this);
}

}