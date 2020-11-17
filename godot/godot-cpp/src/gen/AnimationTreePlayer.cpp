#include "AnimationTreePlayer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Animation.hpp"


namespace godot {


AnimationTreePlayer::___method_bindings AnimationTreePlayer::___mb = {};

void AnimationTreePlayer::___init_method_bindings() {
	___mb.mb_add_node = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "add_node");
	___mb.mb_advance = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "advance");
	___mb.mb_animation_node_get_animation = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "animation_node_get_animation");
	___mb.mb_animation_node_get_master_animation = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "animation_node_get_master_animation");
	___mb.mb_animation_node_get_position = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "animation_node_get_position");
	___mb.mb_animation_node_set_animation = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "animation_node_set_animation");
	___mb.mb_animation_node_set_filter_path = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "animation_node_set_filter_path");
	___mb.mb_animation_node_set_master_animation = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "animation_node_set_master_animation");
	___mb.mb_are_nodes_connected = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "are_nodes_connected");
	___mb.mb_blend2_node_get_amount = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "blend2_node_get_amount");
	___mb.mb_blend2_node_set_amount = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "blend2_node_set_amount");
	___mb.mb_blend2_node_set_filter_path = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "blend2_node_set_filter_path");
	___mb.mb_blend3_node_get_amount = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "blend3_node_get_amount");
	___mb.mb_blend3_node_set_amount = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "blend3_node_set_amount");
	___mb.mb_blend4_node_get_amount = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "blend4_node_get_amount");
	___mb.mb_blend4_node_set_amount = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "blend4_node_set_amount");
	___mb.mb_connect_nodes = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "connect_nodes");
	___mb.mb_disconnect_nodes = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "disconnect_nodes");
	___mb.mb_get_animation_process_mode = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "get_animation_process_mode");
	___mb.mb_get_base_path = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "get_base_path");
	___mb.mb_get_master_player = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "get_master_player");
	___mb.mb_get_node_list = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "get_node_list");
	___mb.mb_is_active = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "is_active");
	___mb.mb_mix_node_get_amount = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "mix_node_get_amount");
	___mb.mb_mix_node_set_amount = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "mix_node_set_amount");
	___mb.mb_node_exists = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "node_exists");
	___mb.mb_node_get_input_count = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "node_get_input_count");
	___mb.mb_node_get_input_source = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "node_get_input_source");
	___mb.mb_node_get_position = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "node_get_position");
	___mb.mb_node_get_type = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "node_get_type");
	___mb.mb_node_rename = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "node_rename");
	___mb.mb_node_set_position = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "node_set_position");
	___mb.mb_oneshot_node_get_autorestart_delay = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "oneshot_node_get_autorestart_delay");
	___mb.mb_oneshot_node_get_autorestart_random_delay = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "oneshot_node_get_autorestart_random_delay");
	___mb.mb_oneshot_node_get_fadein_time = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "oneshot_node_get_fadein_time");
	___mb.mb_oneshot_node_get_fadeout_time = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "oneshot_node_get_fadeout_time");
	___mb.mb_oneshot_node_has_autorestart = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "oneshot_node_has_autorestart");
	___mb.mb_oneshot_node_is_active = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "oneshot_node_is_active");
	___mb.mb_oneshot_node_set_autorestart = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "oneshot_node_set_autorestart");
	___mb.mb_oneshot_node_set_autorestart_delay = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "oneshot_node_set_autorestart_delay");
	___mb.mb_oneshot_node_set_autorestart_random_delay = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "oneshot_node_set_autorestart_random_delay");
	___mb.mb_oneshot_node_set_fadein_time = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "oneshot_node_set_fadein_time");
	___mb.mb_oneshot_node_set_fadeout_time = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "oneshot_node_set_fadeout_time");
	___mb.mb_oneshot_node_set_filter_path = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "oneshot_node_set_filter_path");
	___mb.mb_oneshot_node_start = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "oneshot_node_start");
	___mb.mb_oneshot_node_stop = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "oneshot_node_stop");
	___mb.mb_recompute_caches = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "recompute_caches");
	___mb.mb_remove_node = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "remove_node");
	___mb.mb_reset = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "reset");
	___mb.mb_set_active = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "set_active");
	___mb.mb_set_animation_process_mode = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "set_animation_process_mode");
	___mb.mb_set_base_path = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "set_base_path");
	___mb.mb_set_master_player = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "set_master_player");
	___mb.mb_timescale_node_get_scale = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "timescale_node_get_scale");
	___mb.mb_timescale_node_set_scale = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "timescale_node_set_scale");
	___mb.mb_timeseek_node_seek = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "timeseek_node_seek");
	___mb.mb_transition_node_delete_input = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "transition_node_delete_input");
	___mb.mb_transition_node_get_current = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "transition_node_get_current");
	___mb.mb_transition_node_get_input_count = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "transition_node_get_input_count");
	___mb.mb_transition_node_get_xfade_time = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "transition_node_get_xfade_time");
	___mb.mb_transition_node_has_input_auto_advance = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "transition_node_has_input_auto_advance");
	___mb.mb_transition_node_set_current = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "transition_node_set_current");
	___mb.mb_transition_node_set_input_auto_advance = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "transition_node_set_input_auto_advance");
	___mb.mb_transition_node_set_input_count = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "transition_node_set_input_count");
	___mb.mb_transition_node_set_xfade_time = godot::api->godot_method_bind_get_method("AnimationTreePlayer", "transition_node_set_xfade_time");
}

AnimationTreePlayer *AnimationTreePlayer::_new()
{
	return (AnimationTreePlayer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimationTreePlayer")());
}
void AnimationTreePlayer::add_node(const int64_t type, const String id) {
	___godot_icall_void_int_String(___mb.mb_add_node, (const Object *) this, type, id);
}

void AnimationTreePlayer::advance(const real_t delta) {
	___godot_icall_void_float(___mb.mb_advance, (const Object *) this, delta);
}

Ref<Animation> AnimationTreePlayer::animation_node_get_animation(const String id) const {
	return Ref<Animation>::__internal_constructor(___godot_icall_Object_String(___mb.mb_animation_node_get_animation, (const Object *) this, id));
}

String AnimationTreePlayer::animation_node_get_master_animation(const String id) const {
	return ___godot_icall_String_String(___mb.mb_animation_node_get_master_animation, (const Object *) this, id);
}

real_t AnimationTreePlayer::animation_node_get_position(const String id) const {
	return ___godot_icall_float_String(___mb.mb_animation_node_get_position, (const Object *) this, id);
}

void AnimationTreePlayer::animation_node_set_animation(const String id, const Ref<Animation> animation) {
	___godot_icall_void_String_Object(___mb.mb_animation_node_set_animation, (const Object *) this, id, animation.ptr());
}

void AnimationTreePlayer::animation_node_set_filter_path(const String id, const NodePath path, const bool enable) {
	___godot_icall_void_String_NodePath_bool(___mb.mb_animation_node_set_filter_path, (const Object *) this, id, path, enable);
}

void AnimationTreePlayer::animation_node_set_master_animation(const String id, const String source) {
	___godot_icall_void_String_String(___mb.mb_animation_node_set_master_animation, (const Object *) this, id, source);
}

bool AnimationTreePlayer::are_nodes_connected(const String id, const String dst_id, const int64_t dst_input_idx) const {
	return ___godot_icall_bool_String_String_int(___mb.mb_are_nodes_connected, (const Object *) this, id, dst_id, dst_input_idx);
}

real_t AnimationTreePlayer::blend2_node_get_amount(const String id) const {
	return ___godot_icall_float_String(___mb.mb_blend2_node_get_amount, (const Object *) this, id);
}

void AnimationTreePlayer::blend2_node_set_amount(const String id, const real_t blend) {
	___godot_icall_void_String_float(___mb.mb_blend2_node_set_amount, (const Object *) this, id, blend);
}

void AnimationTreePlayer::blend2_node_set_filter_path(const String id, const NodePath path, const bool enable) {
	___godot_icall_void_String_NodePath_bool(___mb.mb_blend2_node_set_filter_path, (const Object *) this, id, path, enable);
}

real_t AnimationTreePlayer::blend3_node_get_amount(const String id) const {
	return ___godot_icall_float_String(___mb.mb_blend3_node_get_amount, (const Object *) this, id);
}

void AnimationTreePlayer::blend3_node_set_amount(const String id, const real_t blend) {
	___godot_icall_void_String_float(___mb.mb_blend3_node_set_amount, (const Object *) this, id, blend);
}

Vector2 AnimationTreePlayer::blend4_node_get_amount(const String id) const {
	return ___godot_icall_Vector2_String(___mb.mb_blend4_node_get_amount, (const Object *) this, id);
}

void AnimationTreePlayer::blend4_node_set_amount(const String id, const Vector2 blend) {
	___godot_icall_void_String_Vector2(___mb.mb_blend4_node_set_amount, (const Object *) this, id, blend);
}

Error AnimationTreePlayer::connect_nodes(const String id, const String dst_id, const int64_t dst_input_idx) {
	return (Error) ___godot_icall_int_String_String_int(___mb.mb_connect_nodes, (const Object *) this, id, dst_id, dst_input_idx);
}

void AnimationTreePlayer::disconnect_nodes(const String id, const int64_t dst_input_idx) {
	___godot_icall_void_String_int(___mb.mb_disconnect_nodes, (const Object *) this, id, dst_input_idx);
}

AnimationTreePlayer::AnimationProcessMode AnimationTreePlayer::get_animation_process_mode() const {
	return (AnimationTreePlayer::AnimationProcessMode) ___godot_icall_int(___mb.mb_get_animation_process_mode, (const Object *) this);
}

NodePath AnimationTreePlayer::get_base_path() const {
	return ___godot_icall_NodePath(___mb.mb_get_base_path, (const Object *) this);
}

NodePath AnimationTreePlayer::get_master_player() const {
	return ___godot_icall_NodePath(___mb.mb_get_master_player, (const Object *) this);
}

PoolStringArray AnimationTreePlayer::get_node_list() {
	return ___godot_icall_PoolStringArray(___mb.mb_get_node_list, (const Object *) this);
}

bool AnimationTreePlayer::is_active() const {
	return ___godot_icall_bool(___mb.mb_is_active, (const Object *) this);
}

real_t AnimationTreePlayer::mix_node_get_amount(const String id) const {
	return ___godot_icall_float_String(___mb.mb_mix_node_get_amount, (const Object *) this, id);
}

void AnimationTreePlayer::mix_node_set_amount(const String id, const real_t ratio) {
	___godot_icall_void_String_float(___mb.mb_mix_node_set_amount, (const Object *) this, id, ratio);
}

bool AnimationTreePlayer::node_exists(const String node) const {
	return ___godot_icall_bool_String(___mb.mb_node_exists, (const Object *) this, node);
}

int64_t AnimationTreePlayer::node_get_input_count(const String id) const {
	return ___godot_icall_int_String(___mb.mb_node_get_input_count, (const Object *) this, id);
}

String AnimationTreePlayer::node_get_input_source(const String id, const int64_t idx) const {
	return ___godot_icall_String_String_int(___mb.mb_node_get_input_source, (const Object *) this, id, idx);
}

Vector2 AnimationTreePlayer::node_get_position(const String id) const {
	return ___godot_icall_Vector2_String(___mb.mb_node_get_position, (const Object *) this, id);
}

AnimationTreePlayer::NodeType AnimationTreePlayer::node_get_type(const String id) const {
	return (AnimationTreePlayer::NodeType) ___godot_icall_int_String(___mb.mb_node_get_type, (const Object *) this, id);
}

Error AnimationTreePlayer::node_rename(const String node, const String new_name) {
	return (Error) ___godot_icall_int_String_String(___mb.mb_node_rename, (const Object *) this, node, new_name);
}

void AnimationTreePlayer::node_set_position(const String id, const Vector2 screen_position) {
	___godot_icall_void_String_Vector2(___mb.mb_node_set_position, (const Object *) this, id, screen_position);
}

real_t AnimationTreePlayer::oneshot_node_get_autorestart_delay(const String id) const {
	return ___godot_icall_float_String(___mb.mb_oneshot_node_get_autorestart_delay, (const Object *) this, id);
}

real_t AnimationTreePlayer::oneshot_node_get_autorestart_random_delay(const String id) const {
	return ___godot_icall_float_String(___mb.mb_oneshot_node_get_autorestart_random_delay, (const Object *) this, id);
}

real_t AnimationTreePlayer::oneshot_node_get_fadein_time(const String id) const {
	return ___godot_icall_float_String(___mb.mb_oneshot_node_get_fadein_time, (const Object *) this, id);
}

real_t AnimationTreePlayer::oneshot_node_get_fadeout_time(const String id) const {
	return ___godot_icall_float_String(___mb.mb_oneshot_node_get_fadeout_time, (const Object *) this, id);
}

bool AnimationTreePlayer::oneshot_node_has_autorestart(const String id) const {
	return ___godot_icall_bool_String(___mb.mb_oneshot_node_has_autorestart, (const Object *) this, id);
}

bool AnimationTreePlayer::oneshot_node_is_active(const String id) const {
	return ___godot_icall_bool_String(___mb.mb_oneshot_node_is_active, (const Object *) this, id);
}

void AnimationTreePlayer::oneshot_node_set_autorestart(const String id, const bool enable) {
	___godot_icall_void_String_bool(___mb.mb_oneshot_node_set_autorestart, (const Object *) this, id, enable);
}

void AnimationTreePlayer::oneshot_node_set_autorestart_delay(const String id, const real_t delay_sec) {
	___godot_icall_void_String_float(___mb.mb_oneshot_node_set_autorestart_delay, (const Object *) this, id, delay_sec);
}

void AnimationTreePlayer::oneshot_node_set_autorestart_random_delay(const String id, const real_t rand_sec) {
	___godot_icall_void_String_float(___mb.mb_oneshot_node_set_autorestart_random_delay, (const Object *) this, id, rand_sec);
}

void AnimationTreePlayer::oneshot_node_set_fadein_time(const String id, const real_t time_sec) {
	___godot_icall_void_String_float(___mb.mb_oneshot_node_set_fadein_time, (const Object *) this, id, time_sec);
}

void AnimationTreePlayer::oneshot_node_set_fadeout_time(const String id, const real_t time_sec) {
	___godot_icall_void_String_float(___mb.mb_oneshot_node_set_fadeout_time, (const Object *) this, id, time_sec);
}

void AnimationTreePlayer::oneshot_node_set_filter_path(const String id, const NodePath path, const bool enable) {
	___godot_icall_void_String_NodePath_bool(___mb.mb_oneshot_node_set_filter_path, (const Object *) this, id, path, enable);
}

void AnimationTreePlayer::oneshot_node_start(const String id) {
	___godot_icall_void_String(___mb.mb_oneshot_node_start, (const Object *) this, id);
}

void AnimationTreePlayer::oneshot_node_stop(const String id) {
	___godot_icall_void_String(___mb.mb_oneshot_node_stop, (const Object *) this, id);
}

void AnimationTreePlayer::recompute_caches() {
	___godot_icall_void(___mb.mb_recompute_caches, (const Object *) this);
}

void AnimationTreePlayer::remove_node(const String id) {
	___godot_icall_void_String(___mb.mb_remove_node, (const Object *) this, id);
}

void AnimationTreePlayer::reset() {
	___godot_icall_void(___mb.mb_reset, (const Object *) this);
}

void AnimationTreePlayer::set_active(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_active, (const Object *) this, enabled);
}

void AnimationTreePlayer::set_animation_process_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_animation_process_mode, (const Object *) this, mode);
}

void AnimationTreePlayer::set_base_path(const NodePath path) {
	___godot_icall_void_NodePath(___mb.mb_set_base_path, (const Object *) this, path);
}

void AnimationTreePlayer::set_master_player(const NodePath nodepath) {
	___godot_icall_void_NodePath(___mb.mb_set_master_player, (const Object *) this, nodepath);
}

real_t AnimationTreePlayer::timescale_node_get_scale(const String id) const {
	return ___godot_icall_float_String(___mb.mb_timescale_node_get_scale, (const Object *) this, id);
}

void AnimationTreePlayer::timescale_node_set_scale(const String id, const real_t scale) {
	___godot_icall_void_String_float(___mb.mb_timescale_node_set_scale, (const Object *) this, id, scale);
}

void AnimationTreePlayer::timeseek_node_seek(const String id, const real_t seconds) {
	___godot_icall_void_String_float(___mb.mb_timeseek_node_seek, (const Object *) this, id, seconds);
}

void AnimationTreePlayer::transition_node_delete_input(const String id, const int64_t input_idx) {
	___godot_icall_void_String_int(___mb.mb_transition_node_delete_input, (const Object *) this, id, input_idx);
}

int64_t AnimationTreePlayer::transition_node_get_current(const String id) const {
	return ___godot_icall_int_String(___mb.mb_transition_node_get_current, (const Object *) this, id);
}

int64_t AnimationTreePlayer::transition_node_get_input_count(const String id) const {
	return ___godot_icall_int_String(___mb.mb_transition_node_get_input_count, (const Object *) this, id);
}

real_t AnimationTreePlayer::transition_node_get_xfade_time(const String id) const {
	return ___godot_icall_float_String(___mb.mb_transition_node_get_xfade_time, (const Object *) this, id);
}

bool AnimationTreePlayer::transition_node_has_input_auto_advance(const String id, const int64_t input_idx) const {
	return ___godot_icall_bool_String_int(___mb.mb_transition_node_has_input_auto_advance, (const Object *) this, id, input_idx);
}

void AnimationTreePlayer::transition_node_set_current(const String id, const int64_t input_idx) {
	___godot_icall_void_String_int(___mb.mb_transition_node_set_current, (const Object *) this, id, input_idx);
}

void AnimationTreePlayer::transition_node_set_input_auto_advance(const String id, const int64_t input_idx, const bool enable) {
	___godot_icall_void_String_int_bool(___mb.mb_transition_node_set_input_auto_advance, (const Object *) this, id, input_idx, enable);
}

void AnimationTreePlayer::transition_node_set_input_count(const String id, const int64_t count) {
	___godot_icall_void_String_int(___mb.mb_transition_node_set_input_count, (const Object *) this, id, count);
}

void AnimationTreePlayer::transition_node_set_xfade_time(const String id, const real_t time_sec) {
	___godot_icall_void_String_float(___mb.mb_transition_node_set_xfade_time, (const Object *) this, id, time_sec);
}

}