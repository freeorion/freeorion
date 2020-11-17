#ifndef GODOT_CPP_WEBSOCKETMULTIPLAYERPEER_HPP
#define GODOT_CPP_WEBSOCKETMULTIPLAYERPEER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "NetworkedMultiplayerPeer.hpp"
namespace godot {

class WebSocketPeer;

class WebSocketMultiplayerPeer : public NetworkedMultiplayerPeer {
	struct ___method_bindings {
		godot_method_bind *mb_get_peer;
		godot_method_bind *mb_set_buffers;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "WebSocketMultiplayerPeer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	Ref<WebSocketPeer> get_peer(const int64_t peer_id) const;
	Error set_buffers(const int64_t input_buffer_size_kb, const int64_t input_max_packets, const int64_t output_buffer_size_kb, const int64_t output_max_packets);

};

}

#endif