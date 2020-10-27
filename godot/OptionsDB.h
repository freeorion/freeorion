#ifndef _GODOT_OPTIONS_DB_H_
#define _GODOT_OPTIONS_DB_H_

#include <Godot.hpp>
#include <Reference.hpp>

namespace godot {
    class OptionsDB : public Reference {
        GODOT_CLASS(OptionsDB, Reference)

    private:
    public:
        static void _register_methods();

        OptionsDB();
        ~OptionsDB();
    };
}

#endif

