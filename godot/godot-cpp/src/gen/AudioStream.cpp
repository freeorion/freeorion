#include "AudioStream.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioStream::___method_bindings AudioStream::___mb = {};

void AudioStream::___init_method_bindings() {
	___mb.mb_get_length = godot::api->godot_method_bind_get_method("AudioStream", "get_length");
}

real_t AudioStream::get_length() const {
	return ___godot_icall_float(___mb.mb_get_length, (const Object *) this);
}

}