#ifndef _GodotI18n_h_
#define _GodotI18n_h_

#include <godot_cpp/classes/translation.hpp>

class GodotI18n : public godot::Translation {
    GDCLASS(GodotI18n, Translation)
public:
    godot::StringName _get_plural_message(const godot::StringName &p_src_message,
                                          const godot::StringName &p_src_plural_message,
                                          int32_t p_n,
                                          const godot::StringName &p_context) const override;
    godot::StringName _get_message(const godot::StringName &p_src_message, const godot::StringName &p_context) const override;
protected:
    static void _bind_methods();
};

#endif

