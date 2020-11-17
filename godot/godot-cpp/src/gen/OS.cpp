#include "OS.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Image.hpp"


namespace godot {


OS *OS::_singleton = NULL;


OS::OS() {
	_owner = godot::api->godot_global_get_singleton((char *) "OS");
}


OS::___method_bindings OS::___mb = {};

void OS::___init_method_bindings() {
	___mb.mb_alert = godot::api->godot_method_bind_get_method("_OS", "alert");
	___mb.mb_can_draw = godot::api->godot_method_bind_get_method("_OS", "can_draw");
	___mb.mb_can_use_threads = godot::api->godot_method_bind_get_method("_OS", "can_use_threads");
	___mb.mb_center_window = godot::api->godot_method_bind_get_method("_OS", "center_window");
	___mb.mb_close_midi_inputs = godot::api->godot_method_bind_get_method("_OS", "close_midi_inputs");
	___mb.mb_delay_msec = godot::api->godot_method_bind_get_method("_OS", "delay_msec");
	___mb.mb_delay_usec = godot::api->godot_method_bind_get_method("_OS", "delay_usec");
	___mb.mb_dump_memory_to_file = godot::api->godot_method_bind_get_method("_OS", "dump_memory_to_file");
	___mb.mb_dump_resources_to_file = godot::api->godot_method_bind_get_method("_OS", "dump_resources_to_file");
	___mb.mb_execute = godot::api->godot_method_bind_get_method("_OS", "execute");
	___mb.mb_find_scancode_from_string = godot::api->godot_method_bind_get_method("_OS", "find_scancode_from_string");
	___mb.mb_get_audio_driver_count = godot::api->godot_method_bind_get_method("_OS", "get_audio_driver_count");
	___mb.mb_get_audio_driver_name = godot::api->godot_method_bind_get_method("_OS", "get_audio_driver_name");
	___mb.mb_get_borderless_window = godot::api->godot_method_bind_get_method("_OS", "get_borderless_window");
	___mb.mb_get_clipboard = godot::api->godot_method_bind_get_method("_OS", "get_clipboard");
	___mb.mb_get_cmdline_args = godot::api->godot_method_bind_get_method("_OS", "get_cmdline_args");
	___mb.mb_get_connected_midi_inputs = godot::api->godot_method_bind_get_method("_OS", "get_connected_midi_inputs");
	___mb.mb_get_current_screen = godot::api->godot_method_bind_get_method("_OS", "get_current_screen");
	___mb.mb_get_current_video_driver = godot::api->godot_method_bind_get_method("_OS", "get_current_video_driver");
	___mb.mb_get_date = godot::api->godot_method_bind_get_method("_OS", "get_date");
	___mb.mb_get_datetime = godot::api->godot_method_bind_get_method("_OS", "get_datetime");
	___mb.mb_get_datetime_from_unix_time = godot::api->godot_method_bind_get_method("_OS", "get_datetime_from_unix_time");
	___mb.mb_get_dynamic_memory_usage = godot::api->godot_method_bind_get_method("_OS", "get_dynamic_memory_usage");
	___mb.mb_get_environment = godot::api->godot_method_bind_get_method("_OS", "get_environment");
	___mb.mb_get_executable_path = godot::api->godot_method_bind_get_method("_OS", "get_executable_path");
	___mb.mb_get_exit_code = godot::api->godot_method_bind_get_method("_OS", "get_exit_code");
	___mb.mb_get_granted_permissions = godot::api->godot_method_bind_get_method("_OS", "get_granted_permissions");
	___mb.mb_get_ime_selection = godot::api->godot_method_bind_get_method("_OS", "get_ime_selection");
	___mb.mb_get_ime_text = godot::api->godot_method_bind_get_method("_OS", "get_ime_text");
	___mb.mb_get_latin_keyboard_variant = godot::api->godot_method_bind_get_method("_OS", "get_latin_keyboard_variant");
	___mb.mb_get_locale = godot::api->godot_method_bind_get_method("_OS", "get_locale");
	___mb.mb_get_low_processor_usage_mode_sleep_usec = godot::api->godot_method_bind_get_method("_OS", "get_low_processor_usage_mode_sleep_usec");
	___mb.mb_get_max_window_size = godot::api->godot_method_bind_get_method("_OS", "get_max_window_size");
	___mb.mb_get_min_window_size = godot::api->godot_method_bind_get_method("_OS", "get_min_window_size");
	___mb.mb_get_model_name = godot::api->godot_method_bind_get_method("_OS", "get_model_name");
	___mb.mb_get_name = godot::api->godot_method_bind_get_method("_OS", "get_name");
	___mb.mb_get_power_percent_left = godot::api->godot_method_bind_get_method("_OS", "get_power_percent_left");
	___mb.mb_get_power_seconds_left = godot::api->godot_method_bind_get_method("_OS", "get_power_seconds_left");
	___mb.mb_get_power_state = godot::api->godot_method_bind_get_method("_OS", "get_power_state");
	___mb.mb_get_process_id = godot::api->godot_method_bind_get_method("_OS", "get_process_id");
	___mb.mb_get_processor_count = godot::api->godot_method_bind_get_method("_OS", "get_processor_count");
	___mb.mb_get_real_window_size = godot::api->godot_method_bind_get_method("_OS", "get_real_window_size");
	___mb.mb_get_scancode_string = godot::api->godot_method_bind_get_method("_OS", "get_scancode_string");
	___mb.mb_get_screen_count = godot::api->godot_method_bind_get_method("_OS", "get_screen_count");
	___mb.mb_get_screen_dpi = godot::api->godot_method_bind_get_method("_OS", "get_screen_dpi");
	___mb.mb_get_screen_orientation = godot::api->godot_method_bind_get_method("_OS", "get_screen_orientation");
	___mb.mb_get_screen_position = godot::api->godot_method_bind_get_method("_OS", "get_screen_position");
	___mb.mb_get_screen_size = godot::api->godot_method_bind_get_method("_OS", "get_screen_size");
	___mb.mb_get_splash_tick_msec = godot::api->godot_method_bind_get_method("_OS", "get_splash_tick_msec");
	___mb.mb_get_static_memory_peak_usage = godot::api->godot_method_bind_get_method("_OS", "get_static_memory_peak_usage");
	___mb.mb_get_static_memory_usage = godot::api->godot_method_bind_get_method("_OS", "get_static_memory_usage");
	___mb.mb_get_system_dir = godot::api->godot_method_bind_get_method("_OS", "get_system_dir");
	___mb.mb_get_system_time_msecs = godot::api->godot_method_bind_get_method("_OS", "get_system_time_msecs");
	___mb.mb_get_system_time_secs = godot::api->godot_method_bind_get_method("_OS", "get_system_time_secs");
	___mb.mb_get_ticks_msec = godot::api->godot_method_bind_get_method("_OS", "get_ticks_msec");
	___mb.mb_get_ticks_usec = godot::api->godot_method_bind_get_method("_OS", "get_ticks_usec");
	___mb.mb_get_time = godot::api->godot_method_bind_get_method("_OS", "get_time");
	___mb.mb_get_time_zone_info = godot::api->godot_method_bind_get_method("_OS", "get_time_zone_info");
	___mb.mb_get_unique_id = godot::api->godot_method_bind_get_method("_OS", "get_unique_id");
	___mb.mb_get_unix_time = godot::api->godot_method_bind_get_method("_OS", "get_unix_time");
	___mb.mb_get_unix_time_from_datetime = godot::api->godot_method_bind_get_method("_OS", "get_unix_time_from_datetime");
	___mb.mb_get_user_data_dir = godot::api->godot_method_bind_get_method("_OS", "get_user_data_dir");
	___mb.mb_get_video_driver_count = godot::api->godot_method_bind_get_method("_OS", "get_video_driver_count");
	___mb.mb_get_video_driver_name = godot::api->godot_method_bind_get_method("_OS", "get_video_driver_name");
	___mb.mb_get_virtual_keyboard_height = godot::api->godot_method_bind_get_method("_OS", "get_virtual_keyboard_height");
	___mb.mb_get_window_per_pixel_transparency_enabled = godot::api->godot_method_bind_get_method("_OS", "get_window_per_pixel_transparency_enabled");
	___mb.mb_get_window_position = godot::api->godot_method_bind_get_method("_OS", "get_window_position");
	___mb.mb_get_window_safe_area = godot::api->godot_method_bind_get_method("_OS", "get_window_safe_area");
	___mb.mb_get_window_size = godot::api->godot_method_bind_get_method("_OS", "get_window_size");
	___mb.mb_global_menu_add_item = godot::api->godot_method_bind_get_method("_OS", "global_menu_add_item");
	___mb.mb_global_menu_add_separator = godot::api->godot_method_bind_get_method("_OS", "global_menu_add_separator");
	___mb.mb_global_menu_clear = godot::api->godot_method_bind_get_method("_OS", "global_menu_clear");
	___mb.mb_global_menu_remove_item = godot::api->godot_method_bind_get_method("_OS", "global_menu_remove_item");
	___mb.mb_has_environment = godot::api->godot_method_bind_get_method("_OS", "has_environment");
	___mb.mb_has_feature = godot::api->godot_method_bind_get_method("_OS", "has_feature");
	___mb.mb_has_touchscreen_ui_hint = godot::api->godot_method_bind_get_method("_OS", "has_touchscreen_ui_hint");
	___mb.mb_has_virtual_keyboard = godot::api->godot_method_bind_get_method("_OS", "has_virtual_keyboard");
	___mb.mb_hide_virtual_keyboard = godot::api->godot_method_bind_get_method("_OS", "hide_virtual_keyboard");
	___mb.mb_is_debug_build = godot::api->godot_method_bind_get_method("_OS", "is_debug_build");
	___mb.mb_is_in_low_processor_usage_mode = godot::api->godot_method_bind_get_method("_OS", "is_in_low_processor_usage_mode");
	___mb.mb_is_keep_screen_on = godot::api->godot_method_bind_get_method("_OS", "is_keep_screen_on");
	___mb.mb_is_ok_left_and_cancel_right = godot::api->godot_method_bind_get_method("_OS", "is_ok_left_and_cancel_right");
	___mb.mb_is_scancode_unicode = godot::api->godot_method_bind_get_method("_OS", "is_scancode_unicode");
	___mb.mb_is_stdout_verbose = godot::api->godot_method_bind_get_method("_OS", "is_stdout_verbose");
	___mb.mb_is_userfs_persistent = godot::api->godot_method_bind_get_method("_OS", "is_userfs_persistent");
	___mb.mb_is_vsync_enabled = godot::api->godot_method_bind_get_method("_OS", "is_vsync_enabled");
	___mb.mb_is_vsync_via_compositor_enabled = godot::api->godot_method_bind_get_method("_OS", "is_vsync_via_compositor_enabled");
	___mb.mb_is_window_always_on_top = godot::api->godot_method_bind_get_method("_OS", "is_window_always_on_top");
	___mb.mb_is_window_focused = godot::api->godot_method_bind_get_method("_OS", "is_window_focused");
	___mb.mb_is_window_fullscreen = godot::api->godot_method_bind_get_method("_OS", "is_window_fullscreen");
	___mb.mb_is_window_maximized = godot::api->godot_method_bind_get_method("_OS", "is_window_maximized");
	___mb.mb_is_window_minimized = godot::api->godot_method_bind_get_method("_OS", "is_window_minimized");
	___mb.mb_is_window_resizable = godot::api->godot_method_bind_get_method("_OS", "is_window_resizable");
	___mb.mb_kill = godot::api->godot_method_bind_get_method("_OS", "kill");
	___mb.mb_move_window_to_foreground = godot::api->godot_method_bind_get_method("_OS", "move_window_to_foreground");
	___mb.mb_native_video_is_playing = godot::api->godot_method_bind_get_method("_OS", "native_video_is_playing");
	___mb.mb_native_video_pause = godot::api->godot_method_bind_get_method("_OS", "native_video_pause");
	___mb.mb_native_video_play = godot::api->godot_method_bind_get_method("_OS", "native_video_play");
	___mb.mb_native_video_stop = godot::api->godot_method_bind_get_method("_OS", "native_video_stop");
	___mb.mb_native_video_unpause = godot::api->godot_method_bind_get_method("_OS", "native_video_unpause");
	___mb.mb_open_midi_inputs = godot::api->godot_method_bind_get_method("_OS", "open_midi_inputs");
	___mb.mb_print_all_resources = godot::api->godot_method_bind_get_method("_OS", "print_all_resources");
	___mb.mb_print_all_textures_by_size = godot::api->godot_method_bind_get_method("_OS", "print_all_textures_by_size");
	___mb.mb_print_resources_by_type = godot::api->godot_method_bind_get_method("_OS", "print_resources_by_type");
	___mb.mb_print_resources_in_use = godot::api->godot_method_bind_get_method("_OS", "print_resources_in_use");
	___mb.mb_request_attention = godot::api->godot_method_bind_get_method("_OS", "request_attention");
	___mb.mb_request_permission = godot::api->godot_method_bind_get_method("_OS", "request_permission");
	___mb.mb_request_permissions = godot::api->godot_method_bind_get_method("_OS", "request_permissions");
	___mb.mb_set_borderless_window = godot::api->godot_method_bind_get_method("_OS", "set_borderless_window");
	___mb.mb_set_clipboard = godot::api->godot_method_bind_get_method("_OS", "set_clipboard");
	___mb.mb_set_current_screen = godot::api->godot_method_bind_get_method("_OS", "set_current_screen");
	___mb.mb_set_exit_code = godot::api->godot_method_bind_get_method("_OS", "set_exit_code");
	___mb.mb_set_icon = godot::api->godot_method_bind_get_method("_OS", "set_icon");
	___mb.mb_set_ime_active = godot::api->godot_method_bind_get_method("_OS", "set_ime_active");
	___mb.mb_set_ime_position = godot::api->godot_method_bind_get_method("_OS", "set_ime_position");
	___mb.mb_set_keep_screen_on = godot::api->godot_method_bind_get_method("_OS", "set_keep_screen_on");
	___mb.mb_set_low_processor_usage_mode = godot::api->godot_method_bind_get_method("_OS", "set_low_processor_usage_mode");
	___mb.mb_set_low_processor_usage_mode_sleep_usec = godot::api->godot_method_bind_get_method("_OS", "set_low_processor_usage_mode_sleep_usec");
	___mb.mb_set_max_window_size = godot::api->godot_method_bind_get_method("_OS", "set_max_window_size");
	___mb.mb_set_min_window_size = godot::api->godot_method_bind_get_method("_OS", "set_min_window_size");
	___mb.mb_set_native_icon = godot::api->godot_method_bind_get_method("_OS", "set_native_icon");
	___mb.mb_set_screen_orientation = godot::api->godot_method_bind_get_method("_OS", "set_screen_orientation");
	___mb.mb_set_thread_name = godot::api->godot_method_bind_get_method("_OS", "set_thread_name");
	___mb.mb_set_use_file_access_save_and_swap = godot::api->godot_method_bind_get_method("_OS", "set_use_file_access_save_and_swap");
	___mb.mb_set_use_vsync = godot::api->godot_method_bind_get_method("_OS", "set_use_vsync");
	___mb.mb_set_vsync_via_compositor = godot::api->godot_method_bind_get_method("_OS", "set_vsync_via_compositor");
	___mb.mb_set_window_always_on_top = godot::api->godot_method_bind_get_method("_OS", "set_window_always_on_top");
	___mb.mb_set_window_fullscreen = godot::api->godot_method_bind_get_method("_OS", "set_window_fullscreen");
	___mb.mb_set_window_maximized = godot::api->godot_method_bind_get_method("_OS", "set_window_maximized");
	___mb.mb_set_window_minimized = godot::api->godot_method_bind_get_method("_OS", "set_window_minimized");
	___mb.mb_set_window_per_pixel_transparency_enabled = godot::api->godot_method_bind_get_method("_OS", "set_window_per_pixel_transparency_enabled");
	___mb.mb_set_window_position = godot::api->godot_method_bind_get_method("_OS", "set_window_position");
	___mb.mb_set_window_resizable = godot::api->godot_method_bind_get_method("_OS", "set_window_resizable");
	___mb.mb_set_window_size = godot::api->godot_method_bind_get_method("_OS", "set_window_size");
	___mb.mb_set_window_title = godot::api->godot_method_bind_get_method("_OS", "set_window_title");
	___mb.mb_shell_open = godot::api->godot_method_bind_get_method("_OS", "shell_open");
	___mb.mb_show_virtual_keyboard = godot::api->godot_method_bind_get_method("_OS", "show_virtual_keyboard");
}

void OS::alert(const String text, const String title) {
	___godot_icall_void_String_String(___mb.mb_alert, (const Object *) this, text, title);
}

bool OS::can_draw() const {
	return ___godot_icall_bool(___mb.mb_can_draw, (const Object *) this);
}

bool OS::can_use_threads() const {
	return ___godot_icall_bool(___mb.mb_can_use_threads, (const Object *) this);
}

void OS::center_window() {
	___godot_icall_void(___mb.mb_center_window, (const Object *) this);
}

void OS::close_midi_inputs() {
	___godot_icall_void(___mb.mb_close_midi_inputs, (const Object *) this);
}

void OS::delay_msec(const int64_t msec) const {
	___godot_icall_void_int(___mb.mb_delay_msec, (const Object *) this, msec);
}

void OS::delay_usec(const int64_t usec) const {
	___godot_icall_void_int(___mb.mb_delay_usec, (const Object *) this, usec);
}

void OS::dump_memory_to_file(const String file) {
	___godot_icall_void_String(___mb.mb_dump_memory_to_file, (const Object *) this, file);
}

void OS::dump_resources_to_file(const String file) {
	___godot_icall_void_String(___mb.mb_dump_resources_to_file, (const Object *) this, file);
}

int64_t OS::execute(const String path, const PoolStringArray arguments, const bool blocking, const Array output, const bool read_stderr) {
	return ___godot_icall_int_String_PoolStringArray_bool_Array_bool(___mb.mb_execute, (const Object *) this, path, arguments, blocking, output, read_stderr);
}

int64_t OS::find_scancode_from_string(const String string) const {
	return ___godot_icall_int_String(___mb.mb_find_scancode_from_string, (const Object *) this, string);
}

int64_t OS::get_audio_driver_count() const {
	return ___godot_icall_int(___mb.mb_get_audio_driver_count, (const Object *) this);
}

String OS::get_audio_driver_name(const int64_t driver) const {
	return ___godot_icall_String_int(___mb.mb_get_audio_driver_name, (const Object *) this, driver);
}

bool OS::get_borderless_window() const {
	return ___godot_icall_bool(___mb.mb_get_borderless_window, (const Object *) this);
}

String OS::get_clipboard() const {
	return ___godot_icall_String(___mb.mb_get_clipboard, (const Object *) this);
}

PoolStringArray OS::get_cmdline_args() {
	return ___godot_icall_PoolStringArray(___mb.mb_get_cmdline_args, (const Object *) this);
}

PoolStringArray OS::get_connected_midi_inputs() {
	return ___godot_icall_PoolStringArray(___mb.mb_get_connected_midi_inputs, (const Object *) this);
}

int64_t OS::get_current_screen() const {
	return ___godot_icall_int(___mb.mb_get_current_screen, (const Object *) this);
}

OS::VideoDriver OS::get_current_video_driver() const {
	return (OS::VideoDriver) ___godot_icall_int(___mb.mb_get_current_video_driver, (const Object *) this);
}

Dictionary OS::get_date(const bool utc) const {
	return ___godot_icall_Dictionary_bool(___mb.mb_get_date, (const Object *) this, utc);
}

Dictionary OS::get_datetime(const bool utc) const {
	return ___godot_icall_Dictionary_bool(___mb.mb_get_datetime, (const Object *) this, utc);
}

Dictionary OS::get_datetime_from_unix_time(const int64_t unix_time_val) const {
	return ___godot_icall_Dictionary_int(___mb.mb_get_datetime_from_unix_time, (const Object *) this, unix_time_val);
}

int64_t OS::get_dynamic_memory_usage() const {
	return ___godot_icall_int(___mb.mb_get_dynamic_memory_usage, (const Object *) this);
}

String OS::get_environment(const String environment) const {
	return ___godot_icall_String_String(___mb.mb_get_environment, (const Object *) this, environment);
}

String OS::get_executable_path() const {
	return ___godot_icall_String(___mb.mb_get_executable_path, (const Object *) this);
}

int64_t OS::get_exit_code() const {
	return ___godot_icall_int(___mb.mb_get_exit_code, (const Object *) this);
}

PoolStringArray OS::get_granted_permissions() const {
	return ___godot_icall_PoolStringArray(___mb.mb_get_granted_permissions, (const Object *) this);
}

Vector2 OS::get_ime_selection() const {
	return ___godot_icall_Vector2(___mb.mb_get_ime_selection, (const Object *) this);
}

String OS::get_ime_text() const {
	return ___godot_icall_String(___mb.mb_get_ime_text, (const Object *) this);
}

String OS::get_latin_keyboard_variant() const {
	return ___godot_icall_String(___mb.mb_get_latin_keyboard_variant, (const Object *) this);
}

String OS::get_locale() const {
	return ___godot_icall_String(___mb.mb_get_locale, (const Object *) this);
}

int64_t OS::get_low_processor_usage_mode_sleep_usec() const {
	return ___godot_icall_int(___mb.mb_get_low_processor_usage_mode_sleep_usec, (const Object *) this);
}

Vector2 OS::get_max_window_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_max_window_size, (const Object *) this);
}

Vector2 OS::get_min_window_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_min_window_size, (const Object *) this);
}

String OS::get_model_name() const {
	return ___godot_icall_String(___mb.mb_get_model_name, (const Object *) this);
}

String OS::get_name() const {
	return ___godot_icall_String(___mb.mb_get_name, (const Object *) this);
}

int64_t OS::get_power_percent_left() {
	return ___godot_icall_int(___mb.mb_get_power_percent_left, (const Object *) this);
}

int64_t OS::get_power_seconds_left() {
	return ___godot_icall_int(___mb.mb_get_power_seconds_left, (const Object *) this);
}

OS::PowerState OS::get_power_state() {
	return (OS::PowerState) ___godot_icall_int(___mb.mb_get_power_state, (const Object *) this);
}

int64_t OS::get_process_id() const {
	return ___godot_icall_int(___mb.mb_get_process_id, (const Object *) this);
}

int64_t OS::get_processor_count() const {
	return ___godot_icall_int(___mb.mb_get_processor_count, (const Object *) this);
}

Vector2 OS::get_real_window_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_real_window_size, (const Object *) this);
}

String OS::get_scancode_string(const int64_t code) const {
	return ___godot_icall_String_int(___mb.mb_get_scancode_string, (const Object *) this, code);
}

int64_t OS::get_screen_count() const {
	return ___godot_icall_int(___mb.mb_get_screen_count, (const Object *) this);
}

int64_t OS::get_screen_dpi(const int64_t screen) const {
	return ___godot_icall_int_int(___mb.mb_get_screen_dpi, (const Object *) this, screen);
}

OS::ScreenOrientation OS::get_screen_orientation() const {
	return (OS::ScreenOrientation) ___godot_icall_int(___mb.mb_get_screen_orientation, (const Object *) this);
}

Vector2 OS::get_screen_position(const int64_t screen) const {
	return ___godot_icall_Vector2_int(___mb.mb_get_screen_position, (const Object *) this, screen);
}

Vector2 OS::get_screen_size(const int64_t screen) const {
	return ___godot_icall_Vector2_int(___mb.mb_get_screen_size, (const Object *) this, screen);
}

int64_t OS::get_splash_tick_msec() const {
	return ___godot_icall_int(___mb.mb_get_splash_tick_msec, (const Object *) this);
}

int64_t OS::get_static_memory_peak_usage() const {
	return ___godot_icall_int(___mb.mb_get_static_memory_peak_usage, (const Object *) this);
}

int64_t OS::get_static_memory_usage() const {
	return ___godot_icall_int(___mb.mb_get_static_memory_usage, (const Object *) this);
}

String OS::get_system_dir(const int64_t dir) const {
	return ___godot_icall_String_int(___mb.mb_get_system_dir, (const Object *) this, dir);
}

int64_t OS::get_system_time_msecs() const {
	return ___godot_icall_int(___mb.mb_get_system_time_msecs, (const Object *) this);
}

int64_t OS::get_system_time_secs() const {
	return ___godot_icall_int(___mb.mb_get_system_time_secs, (const Object *) this);
}

int64_t OS::get_ticks_msec() const {
	return ___godot_icall_int(___mb.mb_get_ticks_msec, (const Object *) this);
}

int64_t OS::get_ticks_usec() const {
	return ___godot_icall_int(___mb.mb_get_ticks_usec, (const Object *) this);
}

Dictionary OS::get_time(const bool utc) const {
	return ___godot_icall_Dictionary_bool(___mb.mb_get_time, (const Object *) this, utc);
}

Dictionary OS::get_time_zone_info() const {
	return ___godot_icall_Dictionary(___mb.mb_get_time_zone_info, (const Object *) this);
}

String OS::get_unique_id() const {
	return ___godot_icall_String(___mb.mb_get_unique_id, (const Object *) this);
}

int64_t OS::get_unix_time() const {
	return ___godot_icall_int(___mb.mb_get_unix_time, (const Object *) this);
}

int64_t OS::get_unix_time_from_datetime(const Dictionary datetime) const {
	return ___godot_icall_int_Dictionary(___mb.mb_get_unix_time_from_datetime, (const Object *) this, datetime);
}

String OS::get_user_data_dir() const {
	return ___godot_icall_String(___mb.mb_get_user_data_dir, (const Object *) this);
}

int64_t OS::get_video_driver_count() const {
	return ___godot_icall_int(___mb.mb_get_video_driver_count, (const Object *) this);
}

String OS::get_video_driver_name(const int64_t driver) const {
	return ___godot_icall_String_int(___mb.mb_get_video_driver_name, (const Object *) this, driver);
}

int64_t OS::get_virtual_keyboard_height() {
	return ___godot_icall_int(___mb.mb_get_virtual_keyboard_height, (const Object *) this);
}

bool OS::get_window_per_pixel_transparency_enabled() const {
	return ___godot_icall_bool(___mb.mb_get_window_per_pixel_transparency_enabled, (const Object *) this);
}

Vector2 OS::get_window_position() const {
	return ___godot_icall_Vector2(___mb.mb_get_window_position, (const Object *) this);
}

Rect2 OS::get_window_safe_area() const {
	return ___godot_icall_Rect2(___mb.mb_get_window_safe_area, (const Object *) this);
}

Vector2 OS::get_window_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_window_size, (const Object *) this);
}

void OS::global_menu_add_item(const String menu, const String label, const Variant id, const Variant meta) {
	___godot_icall_void_String_String_Variant_Variant(___mb.mb_global_menu_add_item, (const Object *) this, menu, label, id, meta);
}

void OS::global_menu_add_separator(const String menu) {
	___godot_icall_void_String(___mb.mb_global_menu_add_separator, (const Object *) this, menu);
}

void OS::global_menu_clear(const String menu) {
	___godot_icall_void_String(___mb.mb_global_menu_clear, (const Object *) this, menu);
}

void OS::global_menu_remove_item(const String menu, const int64_t idx) {
	___godot_icall_void_String_int(___mb.mb_global_menu_remove_item, (const Object *) this, menu, idx);
}

bool OS::has_environment(const String environment) const {
	return ___godot_icall_bool_String(___mb.mb_has_environment, (const Object *) this, environment);
}

bool OS::has_feature(const String tag_name) const {
	return ___godot_icall_bool_String(___mb.mb_has_feature, (const Object *) this, tag_name);
}

bool OS::has_touchscreen_ui_hint() const {
	return ___godot_icall_bool(___mb.mb_has_touchscreen_ui_hint, (const Object *) this);
}

bool OS::has_virtual_keyboard() const {
	return ___godot_icall_bool(___mb.mb_has_virtual_keyboard, (const Object *) this);
}

void OS::hide_virtual_keyboard() {
	___godot_icall_void(___mb.mb_hide_virtual_keyboard, (const Object *) this);
}

bool OS::is_debug_build() const {
	return ___godot_icall_bool(___mb.mb_is_debug_build, (const Object *) this);
}

bool OS::is_in_low_processor_usage_mode() const {
	return ___godot_icall_bool(___mb.mb_is_in_low_processor_usage_mode, (const Object *) this);
}

bool OS::is_keep_screen_on() const {
	return ___godot_icall_bool(___mb.mb_is_keep_screen_on, (const Object *) this);
}

bool OS::is_ok_left_and_cancel_right() const {
	return ___godot_icall_bool(___mb.mb_is_ok_left_and_cancel_right, (const Object *) this);
}

bool OS::is_scancode_unicode(const int64_t code) const {
	return ___godot_icall_bool_int(___mb.mb_is_scancode_unicode, (const Object *) this, code);
}

bool OS::is_stdout_verbose() const {
	return ___godot_icall_bool(___mb.mb_is_stdout_verbose, (const Object *) this);
}

bool OS::is_userfs_persistent() const {
	return ___godot_icall_bool(___mb.mb_is_userfs_persistent, (const Object *) this);
}

bool OS::is_vsync_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_vsync_enabled, (const Object *) this);
}

bool OS::is_vsync_via_compositor_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_vsync_via_compositor_enabled, (const Object *) this);
}

bool OS::is_window_always_on_top() const {
	return ___godot_icall_bool(___mb.mb_is_window_always_on_top, (const Object *) this);
}

bool OS::is_window_focused() const {
	return ___godot_icall_bool(___mb.mb_is_window_focused, (const Object *) this);
}

bool OS::is_window_fullscreen() const {
	return ___godot_icall_bool(___mb.mb_is_window_fullscreen, (const Object *) this);
}

bool OS::is_window_maximized() const {
	return ___godot_icall_bool(___mb.mb_is_window_maximized, (const Object *) this);
}

bool OS::is_window_minimized() const {
	return ___godot_icall_bool(___mb.mb_is_window_minimized, (const Object *) this);
}

bool OS::is_window_resizable() const {
	return ___godot_icall_bool(___mb.mb_is_window_resizable, (const Object *) this);
}

Error OS::kill(const int64_t pid) {
	return (Error) ___godot_icall_int_int(___mb.mb_kill, (const Object *) this, pid);
}

void OS::move_window_to_foreground() {
	___godot_icall_void(___mb.mb_move_window_to_foreground, (const Object *) this);
}

bool OS::native_video_is_playing() {
	return ___godot_icall_bool(___mb.mb_native_video_is_playing, (const Object *) this);
}

void OS::native_video_pause() {
	___godot_icall_void(___mb.mb_native_video_pause, (const Object *) this);
}

Error OS::native_video_play(const String path, const real_t volume, const String audio_track, const String subtitle_track) {
	return (Error) ___godot_icall_int_String_float_String_String(___mb.mb_native_video_play, (const Object *) this, path, volume, audio_track, subtitle_track);
}

void OS::native_video_stop() {
	___godot_icall_void(___mb.mb_native_video_stop, (const Object *) this);
}

void OS::native_video_unpause() {
	___godot_icall_void(___mb.mb_native_video_unpause, (const Object *) this);
}

void OS::open_midi_inputs() {
	___godot_icall_void(___mb.mb_open_midi_inputs, (const Object *) this);
}

void OS::print_all_resources(const String tofile) {
	___godot_icall_void_String(___mb.mb_print_all_resources, (const Object *) this, tofile);
}

void OS::print_all_textures_by_size() {
	___godot_icall_void(___mb.mb_print_all_textures_by_size, (const Object *) this);
}

void OS::print_resources_by_type(const PoolStringArray types) {
	___godot_icall_void_PoolStringArray(___mb.mb_print_resources_by_type, (const Object *) this, types);
}

void OS::print_resources_in_use(const bool _short) {
	___godot_icall_void_bool(___mb.mb_print_resources_in_use, (const Object *) this, _short);
}

void OS::request_attention() {
	___godot_icall_void(___mb.mb_request_attention, (const Object *) this);
}

bool OS::request_permission(const String name) {
	return ___godot_icall_bool_String(___mb.mb_request_permission, (const Object *) this, name);
}

bool OS::request_permissions() {
	return ___godot_icall_bool(___mb.mb_request_permissions, (const Object *) this);
}

void OS::set_borderless_window(const bool borderless) {
	___godot_icall_void_bool(___mb.mb_set_borderless_window, (const Object *) this, borderless);
}

void OS::set_clipboard(const String clipboard) {
	___godot_icall_void_String(___mb.mb_set_clipboard, (const Object *) this, clipboard);
}

void OS::set_current_screen(const int64_t screen) {
	___godot_icall_void_int(___mb.mb_set_current_screen, (const Object *) this, screen);
}

void OS::set_exit_code(const int64_t code) {
	___godot_icall_void_int(___mb.mb_set_exit_code, (const Object *) this, code);
}

void OS::set_icon(const Ref<Image> icon) {
	___godot_icall_void_Object(___mb.mb_set_icon, (const Object *) this, icon.ptr());
}

void OS::set_ime_active(const bool active) {
	___godot_icall_void_bool(___mb.mb_set_ime_active, (const Object *) this, active);
}

void OS::set_ime_position(const Vector2 position) {
	___godot_icall_void_Vector2(___mb.mb_set_ime_position, (const Object *) this, position);
}

void OS::set_keep_screen_on(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_keep_screen_on, (const Object *) this, enabled);
}

void OS::set_low_processor_usage_mode(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_low_processor_usage_mode, (const Object *) this, enable);
}

void OS::set_low_processor_usage_mode_sleep_usec(const int64_t usec) {
	___godot_icall_void_int(___mb.mb_set_low_processor_usage_mode_sleep_usec, (const Object *) this, usec);
}

void OS::set_max_window_size(const Vector2 size) {
	___godot_icall_void_Vector2(___mb.mb_set_max_window_size, (const Object *) this, size);
}

void OS::set_min_window_size(const Vector2 size) {
	___godot_icall_void_Vector2(___mb.mb_set_min_window_size, (const Object *) this, size);
}

void OS::set_native_icon(const String filename) {
	___godot_icall_void_String(___mb.mb_set_native_icon, (const Object *) this, filename);
}

void OS::set_screen_orientation(const int64_t orientation) {
	___godot_icall_void_int(___mb.mb_set_screen_orientation, (const Object *) this, orientation);
}

Error OS::set_thread_name(const String name) {
	return (Error) ___godot_icall_int_String(___mb.mb_set_thread_name, (const Object *) this, name);
}

void OS::set_use_file_access_save_and_swap(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_use_file_access_save_and_swap, (const Object *) this, enabled);
}

void OS::set_use_vsync(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_vsync, (const Object *) this, enable);
}

void OS::set_vsync_via_compositor(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_vsync_via_compositor, (const Object *) this, enable);
}

void OS::set_window_always_on_top(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_window_always_on_top, (const Object *) this, enabled);
}

void OS::set_window_fullscreen(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_window_fullscreen, (const Object *) this, enabled);
}

void OS::set_window_maximized(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_window_maximized, (const Object *) this, enabled);
}

void OS::set_window_minimized(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_window_minimized, (const Object *) this, enabled);
}

void OS::set_window_per_pixel_transparency_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_window_per_pixel_transparency_enabled, (const Object *) this, enabled);
}

void OS::set_window_position(const Vector2 position) {
	___godot_icall_void_Vector2(___mb.mb_set_window_position, (const Object *) this, position);
}

void OS::set_window_resizable(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_window_resizable, (const Object *) this, enabled);
}

void OS::set_window_size(const Vector2 size) {
	___godot_icall_void_Vector2(___mb.mb_set_window_size, (const Object *) this, size);
}

void OS::set_window_title(const String title) {
	___godot_icall_void_String(___mb.mb_set_window_title, (const Object *) this, title);
}

Error OS::shell_open(const String uri) {
	return (Error) ___godot_icall_int_String(___mb.mb_shell_open, (const Object *) this, uri);
}

void OS::show_virtual_keyboard(const String existing_text) {
	___godot_icall_void_String(___mb.mb_show_virtual_keyboard, (const Object *) this, existing_text);
}

}