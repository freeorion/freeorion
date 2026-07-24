#include "FreeOrionNode.h"

#include <godot_cpp/classes/os.hpp>

#include "../../util/Directories.h"

void FreeOrionNode::_bind_methods() {

}

FreeOrionNode::FreeOrionNode() {
    std::string executable_path = godot::OS::get_singleton()->get_executable_path().utf8().get_data();

    InitDirs(executable_path);
}

FreeOrionNode::~FreeOrionNode()
{ }
