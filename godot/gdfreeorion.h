#ifndef GDFREEORION_H
#define GDFREEORION_H

#include <Godot.hpp>
#include <Node.hpp>
#include <memory>
#include <thread>

class GodotClientApp;

namespace godot {

class GDFreeOrion : public Node {
    GODOT_CLASS(GDFreeOrion, Node)

private:
    float time_passed;
    std::thread t;
    std::unique_ptr<GodotClientApp> app;
public:
    static void _register_methods();

    GDFreeOrion();
    ~GDFreeOrion();

    void _init(); // our initializer called by Godot

    void _process(float delta);
    void _exit_tree();
};

}

#endif
