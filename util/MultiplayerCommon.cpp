#include "MultiplayerCommon.h"


/////////////////////////////////////////////////////
// Free Function(s)
/////////////////////////////////////////////////////
const std::vector<GG::Clr>& EmpireColors()
{
    static std::vector<GG::Clr> colors;
    if (colors.empty()) {
        colors.push_back(GG::Clr(192, 192, 192, 255)); // light gray
        colors.push_back(GG::Clr(255, 0, 0, 255)); // red
        colors.push_back(GG::Clr(255, 255, 0, 255)); // yellow
        colors.push_back(GG::Clr(0, 255, 0, 255)); // green
        colors.push_back(GG::Clr(0, 255, 255, 255)); // cyan
        colors.push_back(GG::Clr(0, 128, 255, 255)); // light blue
        colors.push_back(GG::Clr(255, 128, 192, 255)); // ???
        colors.push_back(GG::Clr(255, 128, 0, 255)); // burnt orange
    }
    return colors;
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
