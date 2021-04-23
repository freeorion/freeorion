#ifndef _FreeOrionNode_h_
#define _FreeOrionNode_h_

#include <Godot.hpp>
#include <Node.hpp>
#include <String.hpp>

/** Node for GDNative library of FreeOrion Godot client. */
class FreeOrionNode : public godot::Node {
    GODOT_CLASS(FreeOrionNode, godot::Node)

public:
    static void _register_methods(); ///< Registers GDNative methods, properties and signals

    FreeOrionNode();
    ~FreeOrionNode();

    /** Calls when Godot initializes Node.
      * Should be used instead of constructor. */
    void _init();
private:
    /** Calls when Godot remove Node from scene at finalization.
      * Should be used instead of destructor. */
    void _exit_tree();

    godot::String get_version() const; ///< Returns FreeOrion version
};

#endif
