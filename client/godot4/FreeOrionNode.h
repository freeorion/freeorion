#ifndef _FreeOrionNode_h_
#define _FreeOrionNode_h_

#include <godot_cpp/classes/node.hpp>

class FreeOrionNode : public godot::Node {
    GDCLASS(FreeOrionNode, Node)
public:
    FreeOrionNode();
    ~FreeOrionNode();
protected:
	static void _bind_methods();
};

#endif
