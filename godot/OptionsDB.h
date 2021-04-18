#ifndef _GODOT_OPTIONS_DB_H_
#define _GODOT_OPTIONS_DB_H_

#include <Godot.hpp>
#include <Object.hpp>
#include <String.hpp>

namespace godot {
    class OptionsDB : public Object {
        GODOT_CLASS(OptionsDB, Object)

    private:
    public:
        static void _register_methods();

        OptionsDB();
        ~OptionsDB();

        void _init(); // our initializer called by Godot

        bool _exists(String opt) const;
        void _commit();
        String _get_option_str(String opt) const;
        int _get_option_int(String opt) const;
        bool _get_option_bool(String opt) const;
        double _get_option_double(String opt) const;
    };
}

#endif

