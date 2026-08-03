#include "FreeOrionNode.h"

#include <godot_cpp/classes/os.hpp>

#include "../../util/Directories.h"
#include "../../util/i18n.h"
#include "../../util/OptionsDB.h"
#include "../../util/Version.h"

void FreeOrionNode::_bind_methods() {
    godot::ClassDB::bind_method(godot::D_METHOD("get_version"), &FreeOrionNode::get_version);
    godot::ClassDB::bind_method(godot::D_METHOD("get_user_data_dir"), &FreeOrionNode::get_user_data_dir);
}

FreeOrionNode::FreeOrionNode() {
    std::string executable_path = godot::OS::get_singleton()->get_executable_path().utf8().get_data();

    InitDirs(executable_path);

    if (!GetOptionsDB().OptionExists("misc.server-local-binary.path")) {
#ifdef FREEORION_WIN32
        GetOptionsDB().Add<std::string>("misc.server-local-binary.path", UserStringNop("OPTIONS_DB_FREEORIOND_PATH"),   PathToString(GetBinDir() / "freeoriond.exe"));
#else
        GetOptionsDB().Add<std::string>("misc.server-local-binary.path", UserStringNop("OPTIONS_DB_FREEORIOND_PATH"),   PathToString(GetBinDir() / "freeoriond"));
#endif
    }
}

FreeOrionNode::~FreeOrionNode()
{ }

godot::String FreeOrionNode::get_version() const
{ return godot::String(FreeOrionVersionString().c_str()); }

godot::String FreeOrionNode::get_user_data_dir() const
{ return godot::String(GetUserDataDir().native().c_str()); }
