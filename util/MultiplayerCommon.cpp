#include "MultiplayerCommon.h"

#include "Directories.h"
#include "i18n.h"
#include "Logger.h"
#include "Math.h"
#include "OptionsDB.h"
#include "Random.h"
#include "../universe/Fleet.h"
#include "../universe/Planet.h"
#include "../universe/System.h"

#if defined(_MSC_VER) && defined(int64_t)
#undef int64_t
#endif


#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/system/system_error.hpp>
#include <GG/utf8/checked.h>


const std::string MP_SAVE_FILE_EXTENSION = ".mps";
const std::string SP_SAVE_FILE_EXTENSION = ".sav";

namespace fs = boost::filesystem;

namespace {
    // command-line options
    void AddOptions(OptionsDB& db) {
        db.Add<std::string>("resource-dir",         UserStringNop("OPTIONS_DB_RESOURCE_DIR"),          PathString(GetRootDataDir() / "default"));
        db.Add<std::string>('S', "save-dir",        UserStringNop("OPTIONS_DB_SAVE_DIR"),              PathString(GetUserDir() / "save"));
        db.Add<std::string>("log-level",            UserStringNop("OPTIONS_DB_LOG_LEVEL"),             "DEBUG");
        db.Add<std::string>("stringtable-filename", UserStringNop("OPTIONS_DB_STRINGTABLE_FILENAME"),  PathString(GetRootDataDir() / "default" / "stringtables" / "en.txt"));
        db.Add("binary-serialization",              UserStringNop("OPTIONS_DB_BINARY_SERIALIZATION"),  true);  // Consider changing to Enum to support more serialization formats

        // AI Testing options-- the following options are to facilitate AI testing and do not currently have an options page widget; 
        // they are intended to be changed via the command line and are not currently storable in the configuration file.
        db.Add<std::string>("ai-path",              UserStringNop("OPTIONS_DB_AI_FOLDER_PATH"),        "AI",     Validator<std::string>(), false);
        db.Add<std::string>("ai-config",            UserStringNop("OPTIONS_DB_AI_CONFIG"),             "",       Validator<std::string>(), false);
    }
    bool temp_bool = RegisterOptions(&AddOptions);

#ifdef DEBUG_XML_TO_CLR
    std::string ClrToString(GG::Clr clr) {
        unsigned int r = static_cast<int>(clr.r);
        unsigned int g = static_cast<int>(clr.g);
        unsigned int b = static_cast<int>(clr.b);
        unsigned int a = static_cast<int>(clr.a);
        std::string retval = "(" + boost::lexical_cast<std::string>(r) + ", " +
            boost::lexical_cast<std::string>(g) + ", " +
            boost::lexical_cast<std::string>(b) + ", " +
            boost::lexical_cast<std::string>(a) + ")";
        return retval;
    }
#endif
}

/////////////////////////////////////////////////////
// Free Function(s)
/////////////////////////////////////////////////////
XMLElement ClrToXML(const GG::Clr& clr) {
    XMLElement retval("GG::Clr");
    retval.AppendChild(XMLElement("red", boost::lexical_cast<std::string>(static_cast<int>(clr.r))));
    retval.AppendChild(XMLElement("green", boost::lexical_cast<std::string>(static_cast<int>(clr.g))));
    retval.AppendChild(XMLElement("blue", boost::lexical_cast<std::string>(static_cast<int>(clr.b))));
    retval.AppendChild(XMLElement("alpha", boost::lexical_cast<std::string>(static_cast<int>(clr.a))));
    return retval;
}

GG::Clr XMLToClr(const XMLElement& clr) {
    GG::Clr retval = GG::Clr(0, 0, 0, 255);
    if (clr.ContainsAttribute("hex")) {
        // get colour components as a single string representing three pairs of hex digits
        // from 00 to FF and an optional fourth hex digit pair for alpha
        const std::string& hex_colour = clr.Attribute("hex");
        std::istringstream iss(hex_colour);
        unsigned long rgba = 0;
        if (!(iss >> std::hex >> rgba).fail()) {
            if (hex_colour.size() == 6) {
                retval.r = (rgba >> 16) & 0xFF;
                retval.g = (rgba >> 8)  & 0xFF;
                retval.b = rgba         & 0xFF;
                retval.a = 255;
            } else {
                retval.r = (rgba >> 24) & 0xFF;
                retval.g = (rgba >> 16) & 0xFF;
                retval.b = (rgba >> 8)  & 0xFF;
                retval.a = rgba         & 0xFF;
            }
#ifdef DEBUG_XML_TO_CLR
            std::cout << "hex colour: " << hex_colour << " int: " << rgba << " RGBA: " << ClrToString(retval) << std::endl;
#endif
        } else {
            std::cerr << "XMLToClr could not interpret hex colour string \"" << hex_colour << "\"" << std::endl;
        }
    } else {
        // get colours listed as RGBA components ranging 0 to 255 as integers
        if (clr.ContainsChild("red"))
            retval.r = boost::lexical_cast<int>(clr.Child("red").Text());
        if (clr.ContainsChild("green"))
            retval.g = boost::lexical_cast<int>(clr.Child("green").Text());
        if (clr.ContainsChild("blue"))
            retval.b = boost::lexical_cast<int>(clr.Child("blue").Text());
        if (clr.ContainsChild("alpha"))
            retval.a = boost::lexical_cast<int>(clr.Child("alpha").Text());
#ifdef DEBUG_XML_TO_CLR
        std::cout << "non hex colour RGBA: " << ClrToString(retval) << std::endl;
#endif
    }
    return retval;
}


/////////////////////////////////////////////////////
// PlayerSetupData
/////////////////////////////////////////////////////
bool operator==(const PlayerSetupData& lhs, const PlayerSetupData& rhs) {
    return  lhs.m_client_type == rhs.m_client_type &&
            lhs.m_empire_color == rhs.m_empire_color &&
            lhs.m_empire_name == rhs.m_empire_name &&
            lhs.m_player_name == rhs.m_player_name &&
            lhs.m_save_game_empire_id == rhs.m_save_game_empire_id &&
            lhs.m_starting_species_name == rhs.m_starting_species_name;
}

bool operator!=(const PlayerSetupData& lhs, const PlayerSetupData& rhs)
{ return !(lhs == rhs); }


////////////////////////////////////////////////////
// MultiplayerLobbyData
/////////////////////////////////////////////////////
std::string MultiplayerLobbyData::Dump() const {
    std::stringstream stream;
    for (std::list<std::pair<int, PlayerSetupData> >::const_iterator it = m_players.begin();
         it != m_players.end(); ++it)
    {
        stream << it->first << ": " << (it->second.m_player_name.empty() ? "NO NAME" : it->second.m_player_name) << "  ";
        if (it->second.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER)
            stream << "AI PLAYER";
        else if (it->second.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER)
            stream << "HUMAN PLAYER";
        else if (it->second.m_client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER)
            stream << "HUMAN OBSERVER";
        else if (it->second.m_client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
            stream << "HUMAN MODERATOR";
        else
            stream << "UNKNOWN CLIENT TPYE";
        stream << "  " << (it->second.m_empire_name.empty() ? "NO EMPIRE NAME" : it->second.m_empire_name) << std::endl;
    }
    return stream.str();
}

