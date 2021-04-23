#ifndef _FreeOrionNode_h_
#define _FreeOrionNode_h_

#include <Godot.hpp>
#include <Node.hpp>
#include <String.hpp>

class FreeOrionNode : public godot::Node {
    GODOT_CLASS(FreeOrionNode, godot::Node)

public:
    static void _register_methods();

    FreeOrionNode();
    ~FreeOrionNode();

    void _init();
private:
    void _exit_tree();

    godot::String get_version() const;
};

#endif
