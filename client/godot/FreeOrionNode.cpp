#include "FreeOrionNode.h"

void FreeOrionNode::_register_methods() {
    register_method("_exit_tree", &FreeOrionNode::_exit_tree);
}

FreeOrionNode::FreeOrionNode()
{ }

FreeOrionNode::~FreeOrionNode()
{ }

void FreeOrionNode::_init()
{ }

void FreeOrionNode::_exit_tree()
{ }

