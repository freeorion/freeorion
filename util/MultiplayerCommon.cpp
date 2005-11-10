#include "MultiplayerCommon.h"

#include "md5.h"
#include "OptionsDB.h"
#include "../UI/StringTable.h"
#include "../util/Directories.h"

#include <log4cpp/Priority.hh>

#include <fstream>
#include <iostream>

namespace {
    // command-line options
    void AddOptions(OptionsDB& db)
    {
        db.Add<std::string>("settings-dir", "Sets the root directory for the settings and data files.", (GetGlobalDir() / "default/").native_directory_string());
        db.Add<std::string>("log-level", "Sets the level at or above which log messages will be output "
                            "(levels in order of decreasing verbosity: DEBUG, INFO, NOTICE, WARN, ERROR, CRIT, "
                            "ALERT, FATAL, EMERG", "WARN");
        db.Add<std::string>("stringtable-filename", "Sets the language-specific string table filename.", "eng_stringtable.txt");
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

    bool temp_header_bool = RecordHeaderFile(MultiplayerCommonRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

/////////////////////////////////////////////////////
// Free Function(s)
/////////////////////////////////////////////////////
const std::vector<GG::Clr>& EmpireColors()
{
    static std::vector<GG::Clr> colors;
    if (colors.empty()) {
        GG::XMLDoc doc;
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

GG::XMLElement ClrToXML(const GG::Clr& clr)
{
    GG::XMLElement retval("GG::Clr");
    retval.AppendChild(GG::XMLElement("red", boost::lexical_cast<std::string>(static_cast<int>(clr.r))));
    retval.AppendChild(GG::XMLElement("green", boost::lexical_cast<std::string>(static_cast<int>(clr.g))));
    retval.AppendChild(GG::XMLElement("blue", boost::lexical_cast<std::string>(static_cast<int>(clr.b))));
    retval.AppendChild(GG::XMLElement("alpha", boost::lexical_cast<std::string>(static_cast<int>(clr.a))));
    return retval;
}

GG::Clr XMLToClr(const GG::XMLElement& clr)
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

const std::vector<std::string>& VersionSensitiveSettingsFiles()
{
    static std::vector<std::string> retval;
    static bool init = false;
    if (!init) {
        retval.push_back("techs.xml");
        retval.push_back("buildings.xml");
        retval.push_back("specials.xml");
        init = true;
    }
    return retval;
}

const std::map<std::string, std::string>& SourceFiles()
{
    static std::map<std::string, std::string> source_files;
    return source_files;
}

bool RecordSourceFile(const std::string& filename, const std::string& revision)
{
    std::string::size_type filename_start = filename.find_first_of(" \t");
    filename_start = filename.find_first_not_of(" \t", filename_start);
    std::string::size_type filename_length = filename.find_last_of(',') - filename_start;
    std::string transformed_filename = filename.substr(filename_start, filename_length);

    std::string::size_type revision_start = revision.find_first_of(" \t");
    revision_start = revision.find_first_not_of(" \t", revision_start);
    std::string::size_type revision_length = revision.find_first_of(" \t", revision_start) - revision_start;
    std::string transformed_revision = revision.substr(revision_start, revision_length);

    std::map<std::string, std::string>& source_files = const_cast<std::map<std::string, std::string>&>(SourceFiles());
    std::map<std::string, std::string>::iterator it = source_files.find(transformed_filename);
    if (it == source_files.end()) {
        source_files[transformed_filename] = transformed_revision;
    } else {
        std::string message = "RecordSourceFile() : Attempted to record source file \"" +
            transformed_filename + "\" twice.";
        std::cerr << message << "\n";
        throw std::runtime_error(message.c_str());
    }
    return true;
}

bool RecordHeaderFile(const std::pair<std::string, std::string>& filename_revision_pair)
{
    return RecordSourceFile(filename_revision_pair.first, filename_revision_pair.second);
}

std::string MD5StringSum(const std::string& str)
{
    std::string retval;
    if (!str.empty()) {
        md5_state_t md5_state;
        md5_byte_t digest[16];
        md5_init(&md5_state);
        md5_append(&md5_state,
                   reinterpret_cast<const md5_byte_t*>(str.c_str()),
                   str.size());
        md5_finish(&md5_state, digest);
        if (0) {
            retval = reinterpret_cast<const char*>(digest);
        } else {
            retval.resize(32, ' ');
            for (int i = 0; i < 16; ++i) {
                sprintf(&retval[i * 2], "%02x", digest[i]);
            }
        }
    }
    return retval;
}

std::string MD5FileSum(const std::string& filename)
{
    std::string file_contents;
    std::ifstream ifs(filename.c_str());
    while (ifs) {
        file_contents += ifs.get();
    }
    return MD5StringSum(file_contents);
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
// SaveGameEmpireData
/////////////////////////////////////////////////////
SaveGameEmpireData::SaveGameEmpireData() 
{
}

SaveGameEmpireData::SaveGameEmpireData(const GG::XMLElement& elem)
{
    id = boost::lexical_cast<int>(elem.Child("id").Text());
    name = elem.Child("name").Text();
    player_name = elem.Child("player_name").Text();
    color = XMLToClr(elem.Child("color").Child("GG::Clr"));
}

GG::XMLElement SaveGameEmpireData::XMLEncode()
{
    using GG::XMLElement;
    using boost::lexical_cast;

    XMLElement retval("SaveGameEmpireData");
    retval.AppendChild(XMLElement("id", lexical_cast<std::string>(id)));
    retval.AppendChild(XMLElement("name", name));
    retval.AppendChild(XMLElement("player_name", player_name));
    retval.AppendChild(XMLElement("color", ClrToXML(color)));
    return retval;
}


/////////////////////////////////////////////////////
// PlayerSetupData
/////////////////////////////////////////////////////
PlayerSetupData::PlayerSetupData() :
    empire_name("Humans"),
    empire_color(GG::CLR_GRAY),
    save_game_empire_id(-1)
{
}

PlayerSetupData::PlayerSetupData(const GG::XMLElement& elem)
{
    empire_name = elem.Child("empire_name").Text();
    empire_color = XMLToClr(elem.Child("empire_color").Child("GG::Clr"));
    save_game_empire_id = boost::lexical_cast<int>(elem.Child("save_game_empire_id").Text());
}

GG::XMLElement PlayerSetupData::XMLEncode() const
{
    using GG::XMLElement;
    using boost::lexical_cast;

    XMLElement retval("PlayerSetupData");
    retval.AppendChild(XMLElement("empire_name", empire_name));
    retval.AppendChild(XMLElement("empire_color", ClrToXML(empire_color)));
    retval.AppendChild(XMLElement("save_game_empire_id", lexical_cast<std::string>(save_game_empire_id)));
    return retval;
}
