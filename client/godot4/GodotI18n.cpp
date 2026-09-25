#include "GodotI18n.h"

#include "../../util/i18n.h"

godot::StringName GodotI18n::_get_plural_message(const godot::StringName &p_src_message,
                                                 const godot::StringName &p_src_plural_message,
                                                 int32_t p_n,
                                                 const godot::StringName &p_context) const
{
    return godot::StringName(UserString(godot::String(p_src_message).utf8().get_data()).c_str());
}

godot::StringName GodotI18n::_get_message(const godot::StringName &p_src_message, const godot::StringName &p_context) const
{ return godot::StringName(UserString(godot::String(p_src_message).utf8().get_data()).c_str()); }

void GodotI18n::_bind_methods()
{ }

