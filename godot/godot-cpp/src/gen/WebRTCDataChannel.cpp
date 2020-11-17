#include "WebRTCDataChannel.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


WebRTCDataChannel::___method_bindings WebRTCDataChannel::___mb = {};

void WebRTCDataChannel::___init_method_bindings() {
	___mb.mb_close = godot::api->godot_method_bind_get_method("WebRTCDataChannel", "close");
	___mb.mb_get_id = godot::api->godot_method_bind_get_method("WebRTCDataChannel", "get_id");
	___mb.mb_get_label = godot::api->godot_method_bind_get_method("WebRTCDataChannel", "get_label");
	___mb.mb_get_max_packet_life_time = godot::api->godot_method_bind_get_method("WebRTCDataChannel", "get_max_packet_life_time");
	___mb.mb_get_max_retransmits = godot::api->godot_method_bind_get_method("WebRTCDataChannel", "get_max_retransmits");
	___mb.mb_get_protocol = godot::api->godot_method_bind_get_method("WebRTCDataChannel", "get_protocol");
	___mb.mb_get_ready_state = godot::api->godot_method_bind_get_method("WebRTCDataChannel", "get_ready_state");
	___mb.mb_get_write_mode = godot::api->godot_method_bind_get_method("WebRTCDataChannel", "get_write_mode");
	___mb.mb_is_negotiated = godot::api->godot_method_bind_get_method("WebRTCDataChannel", "is_negotiated");
	___mb.mb_is_ordered = godot::api->godot_method_bind_get_method("WebRTCDataChannel", "is_ordered");
	___mb.mb_poll = godot::api->godot_method_bind_get_method("WebRTCDataChannel", "poll");
	___mb.mb_set_write_mode = godot::api->godot_method_bind_get_method("WebRTCDataChannel", "set_write_mode");
	___mb.mb_was_string_packet = godot::api->godot_method_bind_get_method("WebRTCDataChannel", "was_string_packet");
}

void WebRTCDataChannel::close() {
	___godot_icall_void(___mb.mb_close, (const Object *) this);
}

int64_t WebRTCDataChannel::get_id() const {
	return ___godot_icall_int(___mb.mb_get_id, (const Object *) this);
}

String WebRTCDataChannel::get_label() const {
	return ___godot_icall_String(___mb.mb_get_label, (const Object *) this);
}

int64_t WebRTCDataChannel::get_max_packet_life_time() const {
	return ___godot_icall_int(___mb.mb_get_max_packet_life_time, (const Object *) this);
}

int64_t WebRTCDataChannel::get_max_retransmits() const {
	return ___godot_icall_int(___mb.mb_get_max_retransmits, (const Object *) this);
}

String WebRTCDataChannel::get_protocol() const {
	return ___godot_icall_String(___mb.mb_get_protocol, (const Object *) this);
}

WebRTCDataChannel::ChannelState WebRTCDataChannel::get_ready_state() const {
	return (WebRTCDataChannel::ChannelState) ___godot_icall_int(___mb.mb_get_ready_state, (const Object *) this);
}

WebRTCDataChannel::WriteMode WebRTCDataChannel::get_write_mode() const {
	return (WebRTCDataChannel::WriteMode) ___godot_icall_int(___mb.mb_get_write_mode, (const Object *) this);
}

bool WebRTCDataChannel::is_negotiated() const {
	return ___godot_icall_bool(___mb.mb_is_negotiated, (const Object *) this);
}

bool WebRTCDataChannel::is_ordered() const {
	return ___godot_icall_bool(___mb.mb_is_ordered, (const Object *) this);
}

Error WebRTCDataChannel::poll() {
	return (Error) ___godot_icall_int(___mb.mb_poll, (const Object *) this);
}

void WebRTCDataChannel::set_write_mode(const int64_t write_mode) {
	___godot_icall_void_int(___mb.mb_set_write_mode, (const Object *) this, write_mode);
}

bool WebRTCDataChannel::was_string_packet() const {
	return ___godot_icall_bool(___mb.mb_was_string_packet, (const Object *) this);
}

}