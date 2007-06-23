#include "MultiplayerCommon.h"

#include "OptionsDB.h"
#include "../UI/StringTable.h"
#include "../util/Directories.h"

#include <log4cpp/Priority.hh>

#if defined(_MSC_VER)
  // HACK! this keeps VC 7.x from barfing when it sees "typedef __int64 int64_t;"
  // in boost/cstdint.h when compiling under windows
#  if defined(int64_t)
#    undef int64_t
#  endif
#elif defined(WIN32)
  // HACK! this keeps gcc 3.x from barfing when it sees "typedef long long uint64_t;"
  // in boost/cstdint.h when compiling under windows
#  define BOOST_MSVC -1
#endif

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>

#include <fstream>
#include <iostream>


namespace fs = boost::filesystem;

namespace {
    // command-line options
    void AddOptions(OptionsDB& db)
    {
        db.Add<std::string>("settings-dir", "OPTIONS_DB_SETTINGS_DIR", (GetGlobalDir() / "default").native_directory_string());
        db.Add<std::string>("log-level", "OPTIONS_DB_LOG_LEVEL", "WARN");
        db.Add<std::string>("stringtable-filename", "OPTIONS_DB_STRINGTABLE_FILENAME", "eng_stringtable.txt");
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    std::string SettingsDir()
    {
        std::string retval = GetOptionsDB().Get<std::string>("settings-dir");
        if (retval.empty() || retval[retval.size()-1] != '/')
            retval += '/';
        return retval;
    }
    StringTable* GetStringTable()
    {
        static StringTable* string_table =
            new StringTable(SettingsDir() + GetOptionsDB().Get<std::string>("stringtable-filename"));
        return string_table;
    }
}

/////////////////////////////////////////////////////
// Free Function(s)
/////////////////////////////////////////////////////
const std::vector<GG::Clr>& EmpireColors()
{
    static std::vector<GG::Clr> colors;
    if (colors.empty()) {
        XMLDoc doc;
        std::string settings_dir = GetOptionsDB().Get<std::string>("settings-dir");
        if (!settings_dir.empty() && settings_dir[settings_dir.size() - 1] != '/')
            settings_dir += '/';
        std::ifstream ifs((settings_dir + "empire_colors.xml").c_str());
        doc.ReadDoc(ifs);
        ifs.close();
        for (int i = 0; i < doc.root_node.NumChildren(); ++i) {
            colors.push_back(XMLToClr(doc.root_node.Child(i)));
        }
    }
    return colors;
}

XMLElement ClrToXML(const GG::Clr& clr)
{
    XMLElement retval("GG::Clr");
    retval.AppendChild(XMLElement("red", boost::lexical_cast<std::string>(static_cast<int>(clr.r))));
    retval.AppendChild(XMLElement("green", boost::lexical_cast<std::string>(static_cast<int>(clr.g))));
    retval.AppendChild(XMLElement("blue", boost::lexical_cast<std::string>(static_cast<int>(clr.b))));
    retval.AppendChild(XMLElement("alpha", boost::lexical_cast<std::string>(static_cast<int>(clr.a))));
    return retval;
}

GG::Clr XMLToClr(const XMLElement& clr)
{
    GG::Clr retval;
    retval.r = boost::lexical_cast<int>(clr.Child("red").Text());
    retval.g = boost::lexical_cast<int>(clr.Child("green").Text());
    retval.b = boost::lexical_cast<int>(clr.Child("blue").Text());
    retval.a = boost::lexical_cast<int>(clr.Child("alpha").Text());
    return retval;
}

int PriorityValue(const std::string& name)
{
    static std::map<std::string, int> priority_map;
    static bool init = false;
    if (!init) {
        using namespace log4cpp;
        priority_map["FATAL"] = Priority::FATAL;
        priority_map["EMERG"] = Priority::EMERG;
        priority_map["ALERT"] = Priority::ALERT;
        priority_map["CRIT"] = Priority::CRIT;
        priority_map["ERROR"] = Priority::ERROR;
        priority_map["WARN"] = Priority::WARN;
        priority_map["NOTICE"] = Priority::NOTICE;
        priority_map["INFO"] = Priority::INFO;
        priority_map["DEBUG"] = Priority::DEBUG;
        priority_map["NOTSET"] = Priority::NOTSET;
    }
    return priority_map[name];
}

const std::string& UserString(const std::string& str)
{
    static std::string retval("ERROR");
    return GetStringTable()->String(str);
}

const std::string& Language() 
{
    static std::string retval("ERROR");
    return GetStringTable()->Language();
}


/////////////////////////////////////////////////////
// GalaxySetupData
/////////////////////////////////////////////////////
GalaxySetupData::GalaxySetupData():
    m_size(100),
    m_shape(SPIRAL_2),
    m_age(AGE_MATURE),
    m_starlane_freq(LANES_SEVERAL),
    m_planet_density(PD_AVERAGE),
    m_specials_freq(SPECIALS_UNCOMMON)
{}


/////////////////////////////////////////////////////
// SinglePlayerSetupData
/////////////////////////////////////////////////////
SinglePlayerSetupData::SinglePlayerSetupData():
    m_AIs(0)
{}


/////////////////////////////////////////////////////
// SaveGameEmpireData
/////////////////////////////////////////////////////
SaveGameEmpireData::SaveGameEmpireData():
    m_id(-1)
{}


/////////////////////////////////////////////////////
// PlayerSetupData
/////////////////////////////////////////////////////
PlayerSetupData::PlayerSetupData() :
    m_player_id(-1),
    m_empire_name("Humans"),
    m_empire_color(GG::CLR_GRAY),
    m_save_game_empire_id(-1)
{}


/////////////////////////////////////////////////////
// MultiplayerLobbyData
/////////////////////////////////////////////////////
// static(s)
const std::string MultiplayerLobbyData::MP_SAVE_FILE_EXTENSION = ".mps";

MultiplayerLobbyData::MultiplayerLobbyData() :
    m_new_game(true),
    m_save_file_index(-1),
    m_empire_colors(EmpireColors())
{}

MultiplayerLobbyData::MultiplayerLobbyData(bool build_save_game_list) :
    m_new_game(true),
    m_save_file_index(-1),
    m_empire_colors(EmpireColors())
{
    if (build_save_game_list) {
        // build a list of save files
        fs::path save_dir(GetLocalDir() / "save");
        fs::directory_iterator end_it;
        for (fs::directory_iterator it(save_dir); it != end_it; ++it) {
            try {
                if (fs::exists(*it) && !fs::is_directory(*it) && it->leaf()[0] != '.') {
                    std::string filename = it->leaf();
                    // disallow filenames that begin with a dot, and filenames with spaces in them
                    if (filename.find('.') != 0 && filename.find(' ') == std::string::npos && 
                        filename.find(MP_SAVE_FILE_EXTENSION) == filename.size() - MP_SAVE_FILE_EXTENSION.size()) {
                        m_save_games.push_back(filename);
                    }
                }
            } catch (const fs::filesystem_error& e) {
                // ignore files for which permission is denied, and rethrow other exceptions
                if (e.error() != fs::security_error)
                    throw;
            }
        }
    }
}


////////////////////////////////////////////////
// PlayerInfo
////////////////////////////////////////////////
PlayerInfo::PlayerInfo() :
    name(""),
    empire_id(-1),
    AI(false),
    host(false)
{}

PlayerInfo::PlayerInfo(const std::string& player_name_, int empire_id_, bool AI_, bool host_) :
    name(player_name_),
    empire_id(empire_id_),
    AI(AI_),
    host(host_)
{}
