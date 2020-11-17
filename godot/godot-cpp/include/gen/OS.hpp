#ifndef GODOT_CPP_OS_HPP
#define GODOT_CPP_OS_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "OS.hpp"

#include "Object.hpp"
namespace godot {

class Image;

class OS : public Object {
	static OS *_singleton;

	OS();

	struct ___method_bindings {
		godot_method_bind *mb_alert;
		godot_method_bind *mb_can_draw;
		godot_method_bind *mb_can_use_threads;
		godot_method_bind *mb_center_window;
		godot_method_bind *mb_close_midi_inputs;
		godot_method_bind *mb_delay_msec;
		godot_method_bind *mb_delay_usec;
		godot_method_bind *mb_dump_memory_to_file;
		godot_method_bind *mb_dump_resources_to_file;
		godot_method_bind *mb_execute;
		godot_method_bind *mb_find_scancode_from_string;
		godot_method_bind *mb_get_audio_driver_count;
		godot_method_bind *mb_get_audio_driver_name;
		godot_method_bind *mb_get_borderless_window;
		godot_method_bind *mb_get_clipboard;
		godot_method_bind *mb_get_cmdline_args;
		godot_method_bind *mb_get_connected_midi_inputs;
		godot_method_bind *mb_get_current_screen;
		godot_method_bind *mb_get_current_video_driver;
		godot_method_bind *mb_get_date;
		godot_method_bind *mb_get_datetime;
		godot_method_bind *mb_get_datetime_from_unix_time;
		godot_method_bind *mb_get_dynamic_memory_usage;
		godot_method_bind *mb_get_environment;
		godot_method_bind *mb_get_executable_path;
		godot_method_bind *mb_get_exit_code;
		godot_method_bind *mb_get_granted_permissions;
		godot_method_bind *mb_get_ime_selection;
		godot_method_bind *mb_get_ime_text;
		godot_method_bind *mb_get_latin_keyboard_variant;
		godot_method_bind *mb_get_locale;
		godot_method_bind *mb_get_low_processor_usage_mode_sleep_usec;
		godot_method_bind *mb_get_max_window_size;
		godot_method_bind *mb_get_min_window_size;
		godot_method_bind *mb_get_model_name;
		godot_method_bind *mb_get_name;
		godot_method_bind *mb_get_power_percent_left;
		godot_method_bind *mb_get_power_seconds_left;
		godot_method_bind *mb_get_power_state;
		godot_method_bind *mb_get_process_id;
		godot_method_bind *mb_get_processor_count;
		godot_method_bind *mb_get_real_window_size;
		godot_method_bind *mb_get_scancode_string;
		godot_method_bind *mb_get_screen_count;
		godot_method_bind *mb_get_screen_dpi;
		godot_method_bind *mb_get_screen_orientation;
		godot_method_bind *mb_get_screen_position;
		godot_method_bind *mb_get_screen_size;
		godot_method_bind *mb_get_splash_tick_msec;
		godot_method_bind *mb_get_static_memory_peak_usage;
		godot_method_bind *mb_get_static_memory_usage;
		godot_method_bind *mb_get_system_dir;
		godot_method_bind *mb_get_system_time_msecs;
		godot_method_bind *mb_get_system_time_secs;
		godot_method_bind *mb_get_ticks_msec;
		godot_method_bind *mb_get_ticks_usec;
		godot_method_bind *mb_get_time;
		godot_method_bind *mb_get_time_zone_info;
		godot_method_bind *mb_get_unique_id;
		godot_method_bind *mb_get_unix_time;
		godot_method_bind *mb_get_unix_time_from_datetime;
		godot_method_bind *mb_get_user_data_dir;
		godot_method_bind *mb_get_video_driver_count;
		godot_method_bind *mb_get_video_driver_name;
		godot_method_bind *mb_get_virtual_keyboard_height;
		godot_method_bind *mb_get_window_per_pixel_transparency_enabled;
		godot_method_bind *mb_get_window_position;
		godot_method_bind *mb_get_window_safe_area;
		godot_method_bind *mb_get_window_size;
		godot_method_bind *mb_global_menu_add_item;
		godot_method_bind *mb_global_menu_add_separator;
		godot_method_bind *mb_global_menu_clear;
		godot_method_bind *mb_global_menu_remove_item;
		godot_method_bind *mb_has_environment;
		godot_method_bind *mb_has_feature;
		godot_method_bind *mb_has_touchscreen_ui_hint;
		godot_method_bind *mb_has_virtual_keyboard;
		godot_method_bind *mb_hide_virtual_keyboard;
		godot_method_bind *mb_is_debug_build;
		godot_method_bind *mb_is_in_low_processor_usage_mode;
		godot_method_bind *mb_is_keep_screen_on;
		godot_method_bind *mb_is_ok_left_and_cancel_right;
		godot_method_bind *mb_is_scancode_unicode;
		godot_method_bind *mb_is_stdout_verbose;
		godot_method_bind *mb_is_userfs_persistent;
		godot_method_bind *mb_is_vsync_enabled;
		godot_method_bind *mb_is_vsync_via_compositor_enabled;
		godot_method_bind *mb_is_window_always_on_top;
		godot_method_bind *mb_is_window_focused;
		godot_method_bind *mb_is_window_fullscreen;
		godot_method_bind *mb_is_window_maximized;
		godot_method_bind *mb_is_window_minimized;
		godot_method_bind *mb_is_window_resizable;
		godot_method_bind *mb_kill;
		godot_method_bind *mb_move_window_to_foreground;
		godot_method_bind *mb_native_video_is_playing;
		godot_method_bind *mb_native_video_pause;
		godot_method_bind *mb_native_video_play;
		godot_method_bind *mb_native_video_stop;
		godot_method_bind *mb_native_video_unpause;
		godot_method_bind *mb_open_midi_inputs;
		godot_method_bind *mb_print_all_resources;
		godot_method_bind *mb_print_all_textures_by_size;
		godot_method_bind *mb_print_resources_by_type;
		godot_method_bind *mb_print_resources_in_use;
		godot_method_bind *mb_request_attention;
		godot_method_bind *mb_request_permission;
		godot_method_bind *mb_request_permissions;
		godot_method_bind *mb_set_borderless_window;
		godot_method_bind *mb_set_clipboard;
		godot_method_bind *mb_set_current_screen;
		godot_method_bind *mb_set_exit_code;
		godot_method_bind *mb_set_icon;
		godot_method_bind *mb_set_ime_active;
		godot_method_bind *mb_set_ime_position;
		godot_method_bind *mb_set_keep_screen_on;
		godot_method_bind *mb_set_low_processor_usage_mode;
		godot_method_bind *mb_set_low_processor_usage_mode_sleep_usec;
		godot_method_bind *mb_set_max_window_size;
		godot_method_bind *mb_set_min_window_size;
		godot_method_bind *mb_set_native_icon;
		godot_method_bind *mb_set_screen_orientation;
		godot_method_bind *mb_set_thread_name;
		godot_method_bind *mb_set_use_file_access_save_and_swap;
		godot_method_bind *mb_set_use_vsync;
		godot_method_bind *mb_set_vsync_via_compositor;
		godot_method_bind *mb_set_window_always_on_top;
		godot_method_bind *mb_set_window_fullscreen;
		godot_method_bind *mb_set_window_maximized;
		godot_method_bind *mb_set_window_minimized;
		godot_method_bind *mb_set_window_per_pixel_transparency_enabled;
		godot_method_bind *mb_set_window_position;
		godot_method_bind *mb_set_window_resizable;
		godot_method_bind *mb_set_window_size;
		godot_method_bind *mb_set_window_title;
		godot_method_bind *mb_shell_open;
		godot_method_bind *mb_show_virtual_keyboard;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline OS *get_singleton()
	{
		if (!OS::_singleton) {
			OS::_singleton = new OS;
		}
		return OS::_singleton;
	}

	static inline const char *___get_class_name() { return (const char *) "OS"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum VideoDriver {
		VIDEO_DRIVER_GLES3 = 0,
		VIDEO_DRIVER_GLES2 = 1,
	};
	enum SystemDir {
		SYSTEM_DIR_DESKTOP = 0,
		SYSTEM_DIR_DCIM = 1,
		SYSTEM_DIR_DOCUMENTS = 2,
		SYSTEM_DIR_DOWNLOADS = 3,
		SYSTEM_DIR_MOVIES = 4,
		SYSTEM_DIR_MUSIC = 5,
		SYSTEM_DIR_PICTURES = 6,
		SYSTEM_DIR_RINGTONES = 7,
	};
	enum ScreenOrientation {
		SCREEN_ORIENTATION_LANDSCAPE = 0,
		SCREEN_ORIENTATION_PORTRAIT = 1,
		SCREEN_ORIENTATION_REVERSE_LANDSCAPE = 2,
		SCREEN_ORIENTATION_REVERSE_PORTRAIT = 3,
		SCREEN_ORIENTATION_SENSOR_LANDSCAPE = 4,
		SCREEN_ORIENTATION_SENSOR_PORTRAIT = 5,
		SCREEN_ORIENTATION_SENSOR = 6,
	};
	enum PowerState {
		POWERSTATE_UNKNOWN = 0,
		POWERSTATE_ON_BATTERY = 1,
		POWERSTATE_NO_BATTERY = 2,
		POWERSTATE_CHARGING = 3,
		POWERSTATE_CHARGED = 4,
	};
	enum Month {
		MONTH_JANUARY = 1,
		MONTH_FEBRUARY = 2,
		MONTH_MARCH = 3,
		MONTH_APRIL = 4,
		MONTH_MAY = 5,
		MONTH_JUNE = 6,
		MONTH_JULY = 7,
		MONTH_AUGUST = 8,
		MONTH_SEPTEMBER = 9,
		MONTH_OCTOBER = 10,
		MONTH_NOVEMBER = 11,
		MONTH_DECEMBER = 12,
	};
	enum Weekday {
		DAY_SUNDAY = 0,
		DAY_MONDAY = 1,
		DAY_TUESDAY = 2,
		DAY_WEDNESDAY = 3,
		DAY_THURSDAY = 4,
		DAY_FRIDAY = 5,
		DAY_SATURDAY = 6,
	};

	// constants

	// methods
	void alert(const String text, const String title = "Alert!");
	bool can_draw() const;
	bool can_use_threads() const;
	void center_window();
	void close_midi_inputs();
	void delay_msec(const int64_t msec) const;
	void delay_usec(const int64_t usec) const;
	void dump_memory_to_file(const String file);
	void dump_resources_to_file(const String file);
	int64_t execute(const String path, const PoolStringArray arguments, const bool blocking = true, const Array output = Array(), const bool read_stderr = false);
	int64_t find_scancode_from_string(const String string) const;
	int64_t get_audio_driver_count() const;
	String get_audio_driver_name(const int64_t driver) const;
	bool get_borderless_window() const;
	String get_clipboard() const;
	PoolStringArray get_cmdline_args();
	PoolStringArray get_connected_midi_inputs();
	int64_t get_current_screen() const;
	OS::VideoDriver get_current_video_driver() const;
	Dictionary get_date(const bool utc = false) const;
	Dictionary get_datetime(const bool utc = false) const;
	Dictionary get_datetime_from_unix_time(const int64_t unix_time_val) const;
	int64_t get_dynamic_memory_usage() const;
	String get_environment(const String environment) const;
	String get_executable_path() const;
	int64_t get_exit_code() const;
	PoolStringArray get_granted_permissions() const;
	Vector2 get_ime_selection() const;
	String get_ime_text() const;
	String get_latin_keyboard_variant() const;
	String get_locale() const;
	int64_t get_low_processor_usage_mode_sleep_usec() const;
	Vector2 get_max_window_size() const;
	Vector2 get_min_window_size() const;
	String get_model_name() const;
	String get_name() const;
	int64_t get_power_percent_left();
	int64_t get_power_seconds_left();
	OS::PowerState get_power_state();
	int64_t get_process_id() const;
	int64_t get_processor_count() const;
	Vector2 get_real_window_size() const;
	String get_scancode_string(const int64_t code) const;
	int64_t get_screen_count() const;
	int64_t get_screen_dpi(const int64_t screen = -1) const;
	OS::ScreenOrientation get_screen_orientation() const;
	Vector2 get_screen_position(const int64_t screen = -1) const;
	Vector2 get_screen_size(const int64_t screen = -1) const;
	int64_t get_splash_tick_msec() const;
	int64_t get_static_memory_peak_usage() const;
	int64_t get_static_memory_usage() const;
	String get_system_dir(const int64_t dir) const;
	int64_t get_system_time_msecs() const;
	int64_t get_system_time_secs() const;
	int64_t get_ticks_msec() const;
	int64_t get_ticks_usec() const;
	Dictionary get_time(const bool utc = false) const;
	Dictionary get_time_zone_info() const;
	String get_unique_id() const;
	int64_t get_unix_time() const;
	int64_t get_unix_time_from_datetime(const Dictionary datetime) const;
	String get_user_data_dir() const;
	int64_t get_video_driver_count() const;
	String get_video_driver_name(const int64_t driver) const;
	int64_t get_virtual_keyboard_height();
	bool get_window_per_pixel_transparency_enabled() const;
	Vector2 get_window_position() const;
	Rect2 get_window_safe_area() const;
	Vector2 get_window_size() const;
	void global_menu_add_item(const String menu, const String label, const Variant id, const Variant meta);
	void global_menu_add_separator(const String menu);
	void global_menu_clear(const String menu);
	void global_menu_remove_item(const String menu, const int64_t idx);
	bool has_environment(const String environment) const;
	bool has_feature(const String tag_name) const;
	bool has_touchscreen_ui_hint() const;
	bool has_virtual_keyboard() const;
	void hide_virtual_keyboard();
	bool is_debug_build() const;
	bool is_in_low_processor_usage_mode() const;
	bool is_keep_screen_on() const;
	bool is_ok_left_and_cancel_right() const;
	bool is_scancode_unicode(const int64_t code) const;
	bool is_stdout_verbose() const;
	bool is_userfs_persistent() const;
	bool is_vsync_enabled() const;
	bool is_vsync_via_compositor_enabled() const;
	bool is_window_always_on_top() const;
	bool is_window_focused() const;
	bool is_window_fullscreen() const;
	bool is_window_maximized() const;
	bool is_window_minimized() const;
	bool is_window_resizable() const;
	Error kill(const int64_t pid);
	void move_window_to_foreground();
	bool native_video_is_playing();
	void native_video_pause();
	Error native_video_play(const String path, const real_t volume, const String audio_track, const String subtitle_track);
	void native_video_stop();
	void native_video_unpause();
	void open_midi_inputs();
	void print_all_resources(const String tofile = "");
	void print_all_textures_by_size();
	void print_resources_by_type(const PoolStringArray types);
	void print_resources_in_use(const bool _short = false);
	void request_attention();
	bool request_permission(const String name);
	bool request_permissions();
	void set_borderless_window(const bool borderless);
	void set_clipboard(const String clipboard);
	void set_current_screen(const int64_t screen);
	void set_exit_code(const int64_t code);
	void set_icon(const Ref<Image> icon);
	void set_ime_active(const bool active);
	void set_ime_position(const Vector2 position);
	void set_keep_screen_on(const bool enabled);
	void set_low_processor_usage_mode(const bool enable);
	void set_low_processor_usage_mode_sleep_usec(const int64_t usec);
	void set_max_window_size(const Vector2 size);
	void set_min_window_size(const Vector2 size);
	void set_native_icon(const String filename);
	void set_screen_orientation(const int64_t orientation);
	Error set_thread_name(const String name);
	void set_use_file_access_save_and_swap(const bool enabled);
	void set_use_vsync(const bool enable);
	void set_vsync_via_compositor(const bool enable);
	void set_window_always_on_top(const bool enabled);
	void set_window_fullscreen(const bool enabled);
	void set_window_maximized(const bool enabled);
	void set_window_minimized(const bool enabled);
	void set_window_per_pixel_transparency_enabled(const bool enabled);
	void set_window_position(const Vector2 position);
	void set_window_resizable(const bool enabled);
	void set_window_size(const Vector2 size);
	void set_window_title(const String title);
	Error shell_open(const String uri);
	void show_virtual_keyboard(const String existing_text = "");

};

}

#endif