#include "MultiplayerCommon.h"

#include "md5.h"

#include <fstream>

#include "OptionsDB.h"

namespace {
    // command-line options
    void AddOptions(OptionsDB& db)
    {
        db.Add<std::string>("settings-dir", "Sets the root directory for the settings and data files.", "default/");
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    std::map<std::string, std::string> g_source_files;

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
            colors.push_back(GG::Clr(doc.root_node.Child(i)));
        }
    }
    return colors;
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
    return g_source_files;
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

    std::map<std::string, std::string>::iterator it = g_source_files.find(transformed_filename);
    if (it == g_source_files.end()) {
        g_source_files[transformed_filename] = transformed_revision;
    } else {
        throw std::runtime_error("RecordSourceFile() : Attempted to record source file \"" +
                                 transformed_filename + "\" twice.");
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
    color = GG::Clr(elem.Child("color").Child("GG::Clr"));
}

GG::XMLElement SaveGameEmpireData::XMLEncode()
{
    using GG::XMLElement;
    using boost::lexical_cast;

    XMLElement retval("SaveGameEmpireData");
    retval.AppendChild(XMLElement("id", lexical_cast<std::string>(id)));
    retval.AppendChild(XMLElement("name", name));
    retval.AppendChild(XMLElement("player_name", player_name));
    retval.AppendChild(XMLElement("color", color.XMLEncode()));
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
    empire_color = GG::Clr(elem.Child("empire_color").Child("GG::Clr"));
    save_game_empire_id = boost::lexical_cast<int>(elem.Child("save_game_empire_id").Text());
}

GG::XMLElement PlayerSetupData::XMLEncode() const
{
    using GG::XMLElement;
    using boost::lexical_cast;

    XMLElement retval("PlayerSetupData");
    retval.AppendChild(XMLElement("empire_name", empire_name));
    retval.AppendChild(XMLElement("empire_color", empire_color.XMLEncode()));
    retval.AppendChild(XMLElement("save_game_empire_id", lexical_cast<std::string>(save_game_empire_id)));
    return retval;
}
