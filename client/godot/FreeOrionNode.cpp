#include "FreeOrionNode.h"

#include "../../util/Version.h"

void FreeOrionNode::_register_methods() {
    register_method("_exit_tree", &FreeOrionNode::_exit_tree);

    register_method("get_version", &FreeOrionNode::get_version);
}

FreeOrionNode::FreeOrionNode()
{ }

FreeOrionNode::~FreeOrionNode()
{ }

void FreeOrionNode::_init()
{ }

void FreeOrionNode::_exit_tree()
{ }

godot::String FreeOrionNode::get_version() const {
    return godot::String(FreeOrionVersionString().c_str());
}

