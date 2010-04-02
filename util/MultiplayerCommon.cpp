#include "MultiplayerCommon.h"

#include "OptionsDB.h"
#include "../combat/OpenSteer/Obstacle.h"
#include "../combat/OpenSteer/AsteroidBeltObstacle.h"
#include "../UI/StringTable.h"
#include "../util/AppInterface.h"
#include "../util/Directories.h"
#include "../universe/Fleet.h"
#include "../universe/Planet.h"
#include "../universe/System.h"

#include <log4cpp/Priority.hh>

#if defined(_MSC_VER) && defined(int64_t)
#undef int64_t
#endif

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>


namespace fs = boost::filesystem;

namespace {
    // command-line options
    void AddOptions(OptionsDB& db) {
        db.Add<std::string>("resource-dir",         "OPTIONS_DB_RESOURCE_DIR",          (GetRootDataDir() / "default").directory_string());
        db.Add<std::string>("log-level",            "OPTIONS_DB_LOG_LEVEL",             "DEBUG");
        db.Add<std::string>("stringtable-filename", "OPTIONS_DB_STRINGTABLE_FILENAME",  "eng_stringtable.txt");
        db.AddFlag("test-3d-combat",                "OPTIONS_DB_TEST_3D_COMBAT",        false);
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    const StringTable_& GetStringTable() {
        static std::auto_ptr<StringTable_> string_table(
            new StringTable_((GetResourceDir() / GetOptionsDB().Get<std::string>("stringtable-filename")).file_string()));
        return *string_table;
    }

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

    // overload for arriving fleets
    void FindPlacementAreas(const System* system,
                            const std::map<int, std::vector<Fleet*> >& arriving_fleets_by_starlane,
                            const std::map<int, std::set<int> >& empires_by_starlane,
                            std::vector<CombatSetupGroup>& setup_groups)
    {
        assert(arriving_fleets_by_starlane.begin()->second.back()->Owners().size() == 1u);
        int owner = *arriving_fleets_by_starlane.begin()->second.back()->Owners().begin();
        for (std::map<int, std::vector<Fleet*> >::const_iterator it = arriving_fleets_by_starlane.begin();
             it != arriving_fleets_by_starlane.end();
             ++it)
        {
            setup_groups.push_back(CombatSetupGroup());
            CombatSetupGroup& setup_group = setup_groups.back();

            for (std::size_t i = 0; i < it->second.size(); ++i) {
                setup_group.m_ships.insert(it->second[i]->begin(), it->second[i]->end());
            }

            double rads = StarlaneEntranceOrbitalPosition(it->first, system->ID());
            double x = StarlaneEntranceOrbitalRadius() * std::cos(rads);
            double y = StarlaneEntranceOrbitalRadius() * std::sin(rads);
            std::map<int, std::set<int> >::const_iterator starlane_it =
                empires_by_starlane.find(it->first);
            assert(starlane_it != empires_by_starlane.end());
            if (starlane_it->second.size() == 1u) {
                setup_group.m_regions.push_back(
                    CombatSetupRegion(x, y, StarlaneEntranceRadialAxis(), StarlaneEntranceTangentAxis()));
            } else {
                std::size_t empires = starlane_it->second.size();
                assert(starlane_it->second.find(owner) != starlane_it->second.end());
                std::size_t position_among_empires =
                    std::distance(starlane_it->second.begin(), starlane_it->second.find(owner));
                const double SLICE_SIZE = 1.0 / empires;
                const double START = position_among_empires * SLICE_SIZE;
                setup_group.m_regions.push_back(
                    CombatSetupRegion(x, y, StarlaneEntranceRadialAxis(), StarlaneEntranceTangentAxis(),
                                      START, START + SLICE_SIZE));
            }

            setup_group.m_allow = true;
        }
    }

    // overload for present fleets
    void FindPlacementAreas(const System* system,
                            const std::vector<Fleet*>& fleets,
                            CombatSetupGroup& setup_group)
    {
        for (std::size_t i = 0; i < fleets.size(); ++i) {
            setup_group.m_ships.insert(fleets[i]->begin(), fleets[i]->end());
        }

        for (System::const_orbit_iterator it = system->begin(); it != system->end(); ++it) {
            if (const Planet* planet = GetObject<Planet>(it->second)) {
                double orbit_r = OrbitalRadius(it->first);
                double rads = planet->OrbitalPositionOnTurn(CurrentTurn());
                float planet_r = PlanetRadius(planet->Size());
                // TODO: Exclude planet orbital rings of non-owned planets;
                // this region just covers the planets themselves.
                setup_group.m_regions.push_back(
                    CombatSetupRegion(orbit_r * std::cos(rads), orbit_r * std::sin(rads), planet_r));
            }
        }

        setup_group.m_regions.push_back(CombatSetupRegion(0.0, 0.0, StarRadius()));

        // provide a gap between the nearest point on the ellipse and the
        // allowed placement area.
        const double FUDGE_FACTOR = 0.1;

        const double STARLANE_ZONE_START =
            StarlaneEntranceOrbitalRadius() - (1 + FUDGE_FACTOR) * StarlaneEntranceRadialAxis() / 2.0;
        setup_group.m_regions.push_back(CombatSetupRegion(STARLANE_ZONE_START, SystemRadius()));

        setup_group.m_allow = false;
    }

    // TODO: Take into account more of the tactical situation (enemy fleets, etc.)
    void PlaceShips(const System* system,
                    const CombatSetupGroup& setup_group,
                    std::map<Ship*, std::pair<OpenSteer::Vec3, OpenSteer::Vec3> >& placements)
    {
        // TODO
    }

    void PlaceShips(const System* system,
                    const std::map<int, std::vector<CombatSetupGroup> >& setup_groups,
                    std::map<Ship*, std::pair<OpenSteer::Vec3, OpenSteer::Vec3> >& placements)
    {
        for (std::map<int, std::vector<CombatSetupGroup> >::const_iterator it = setup_groups.begin();
             it != setup_groups.end();
             ++it) {
            for (std::size_t i = 0; i < it->second.size(); ++i) {
                PlaceShips(system, it->second[i], placements);
            }
        }
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

        std::string file_name = "empire_colors.xml";

        boost::filesystem::ifstream ifs(GetResourceDir() / file_name);
        if (ifs) {
            doc.ReadDoc(ifs);
            ifs.close();
        } else {
            Logger().errorStream() << "Unable to open data file " << file_name;
            return colors;
        }

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
            //std::cout << "hex colour: " << hex_colour << " int: " << rgba << " RGBA: " << ClrToString(retval) << std::endl;
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
        //std::cout << "non hex colour RGBA: " << ClrToString(retval) << std::endl;
    }
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
    return GetStringTable().String(str);
}

boost::format FlexibleFormat(const std::string &string_to_format) {
    boost::format retval(string_to_format);
    retval.exceptions(boost::io::all_error_bits ^ (boost::io::too_many_args_bit | boost::io::too_few_args_bit));
    return retval;
}

std::string RomanNumber(unsigned int n)
{
    //letter pattern (N) and the associated values (V)
    static const std::string  N[] = { "M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"};
    static const unsigned int V[] = {1000,  900, 500,  400, 100,   90,  50,   40,  10,    9,   5,    4,   1};
    unsigned int remainder = n; //remainder of the number to be written
    int i = 0;                  //pattern index
    std::string retval = "";;
    if (n == 0) return "";      //the romans didn't know there is a zero, read a book about history of the zero if you want to know more
                                //Roman numbers are written using patterns, you chosse the highest pattern lower that the number
                                //write it down, and substract it's value until you reach zero.

    // safety check to avoid very long loops
    if (n > 10000)
        return "!";

    //we start with the highest pattern and reduce the size every time it doesn't fit
    while (remainder > 0) {
        //check if number is larger than the actual pattern value
        if (remainder >= V[i]) {
            //write pattern down
            retval += N[i];
            //reduce number
            remainder -= V[i];
        } else {
            //we need the next pattern
            i++;
        }
    }
    return retval;
}

const std::string& Language() 
{
    return GetStringTable().Language();
}

#ifndef FREEORION_WIN32
void Sleep(int ms)
{
    boost::xtime t;
    boost::xtime_get(&t, boost::TIME_UTC);
    int ns_sum = t.nsec + ms * 1000000;
    const int NANOSECONDS_PER_SECOND = 1000000000;
    int delta_secs = ns_sum / NANOSECONDS_PER_SECOND;
    int nanosecs = ns_sum % NANOSECONDS_PER_SECOND;
    t.sec += delta_secs;
    t.nsec = nanosecs;
    boost::thread::sleep(t);
}
#endif


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
    m_new_game(true),
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
    m_empire_color(GG::Clr(127, 127, 127, 255)),
    m_save_game_empire_id(ALL_EMPIRES)
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
        fs::path save_dir(GetUserDir() / "save");
        fs::directory_iterator end_it;
        for (fs::directory_iterator it(save_dir); it != end_it; ++it) {
            try {
                if (fs::exists(*it) && !fs::is_directory(*it) && it->filename()[0] != '.') {
                    std::string filename = it->filename();
                    // disallow filenames that begin with a dot, and filenames with spaces in them
                    if (filename.find('.') != 0 && filename.find(' ') == std::string::npos && 
                        filename.find(MP_SAVE_FILE_EXTENSION) == filename.size() - MP_SAVE_FILE_EXTENSION.size()) {
                        m_save_games.push_back(filename);
                    }
                }
            } catch (const fs::filesystem_error& e) {
                // ignore files for which permission is denied, and rethrow other exceptions
                if (e.code() != boost::system::posix_error::permission_denied)
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
    empire_id(ALL_EMPIRES),
    AI(false),
    host(false)
{}

PlayerInfo::PlayerInfo(const std::string& player_name_, int empire_id_, bool AI_, bool host_) :
    name(player_name_),
    empire_id(empire_id_),
    AI(AI_),
    host(host_)
{}


////////////////////////////////////////////////
// CombatData
////////////////////////////////////////////////
CombatData::CombatData() :
    m_combat_turn_number(1),
    m_system(0)
{}

CombatData::CombatData(System* system, std::map<int, std::vector<CombatSetupGroup> >& setup_groups) :
    m_combat_turn_number(1),
    m_system(system)
{
    using OpenSteer::SphereObstacle;
    using OpenSteer::Vec3;
    m_pathing_engine.AddObstacle(new SphereObstacle(StarRadius(), Vec3()));

    ObjectMap& objects = GetUniverse().Objects();

    std::map<int, std::vector<Fleet*> > present_fleets_by_empire;
    std::map<int, std::map<int, std::vector<Fleet*> > > arriving_fleets_by_starlane_by_empire;
    std::map<int, std::set<int> > empires_by_starlane;

    for (System::const_orbit_iterator it = system->begin(); it != system->end(); ++it) {
        m_combat_universe[it->second] = objects.Object(it->second);
        if (const Planet* planet =
            universe_object_cast<Planet*>(m_combat_universe[it->second])) {
            double orbit_radius = OrbitalRadius(it->first);
            if (planet->Type() == PT_ASTEROIDS) {
                m_pathing_engine.AddObstacle(
                    new AsteroidBeltObstacle(orbit_radius, AsteroidBeltRadius()));
            } else {
                int game_turn = CurrentTurn();
                double rads = planet->OrbitalPositionOnTurn(game_turn);
                Vec3 position(orbit_radius * std::cos(rads),
                              orbit_radius * std::sin(rads),
                              0);
                m_pathing_engine.AddObstacle(
                    new SphereObstacle(PlanetRadius(planet->Size()), position));
            }
        } else if (Fleet* fleet =
                   universe_object_cast<Fleet*>(m_combat_universe[it->second])) {
            assert(fleet->Owners().size() == 1u);
            int owner = *fleet->Owners().begin();
            if (fleet->ArrivedThisTurn()) {
                int starlane = fleet->ArrivalStarlane();
                arriving_fleets_by_starlane_by_empire[owner][starlane].push_back(fleet);
                empires_by_starlane[starlane].insert(owner);
            } else {
                present_fleets_by_empire[owner].push_back(fleet);
            }
        }
    }

    for (std::map<int, std::vector<Fleet*> >::iterator it = present_fleets_by_empire.begin();
         it != present_fleets_by_empire.end();
         ++it)
    {
        std::vector<CombatSetupGroup>& empire_setup_groups = setup_groups[it->first];
        empire_setup_groups.push_back(CombatSetupGroup());
        FindPlacementAreas(m_system, it->second, empire_setup_groups.back());
    }

    for (std::map<int, std::map<int, std::vector<Fleet*> > >::iterator
             it = arriving_fleets_by_starlane_by_empire.begin();
         it != arriving_fleets_by_starlane_by_empire.end();
         ++it)
    {
        FindPlacementAreas(m_system, it->second, empires_by_starlane, setup_groups[it->first]);
    }

    std::map<Ship*, std::pair<OpenSteer::Vec3, OpenSteer::Vec3> > placements;
    PlaceShips(m_system, setup_groups, placements);

    for (std::map<int, std::vector<CombatSetupGroup> >::const_iterator it = setup_groups.begin();
         it != setup_groups.end();
         ++it) {
        for (std::size_t i = 0; i < it->second.size(); ++i) {
            for (std::set<int>::const_iterator ship_it = it->second[i].m_ships.begin();
                 ship_it != it->second[i].m_ships.end();
                 ++ship_it) {
                assert(universe_object_cast<Ship*>(objects.Object(*ship_it)));
                Ship* ship = static_cast<Ship*>(objects.Object(*ship_it));
                const std::pair<OpenSteer::Vec3, OpenSteer::Vec3>& placement = placements[ship];
                CombatShipPtr combat_ship(
                    new CombatShip(ship, placement.first, placement.second, m_pathing_engine));
                m_pathing_engine.AddObject(combat_ship);
            }
        }
    }
}


////////////////////////////////////////////////
// CombatSetupRegion
////////////////////////////////////////////////
CombatSetupRegion::CombatSetupRegion() :
    m_type(RING),
    m_radius_begin(),
    m_radius_end(),
    m_centroid(),
    m_radial_axis(),
    m_tangent_axis(),
    m_theta_begin(),
    m_theta_end()
{}

CombatSetupRegion::CombatSetupRegion(float radius_begin, float radius_end) :
    m_type(RING),
    m_radius_begin(radius_begin),
    m_radius_end(radius_end),
    m_centroid(),
    m_radial_axis(),
    m_tangent_axis(),
    m_theta_begin(),
    m_theta_end()
{}

CombatSetupRegion::CombatSetupRegion(float centroid_x, float centroid_y, float radius) :
    m_type(ELLIPSE),
    m_radius_begin(),
    m_radius_end(),
    m_centroid(),
    m_radial_axis(radius),
    m_tangent_axis(radius),
    m_theta_begin(),
    m_theta_end()
{
    m_centroid[0] = centroid_x;
    m_centroid[1] = centroid_y;
}

CombatSetupRegion::CombatSetupRegion(float centroid_x, float centroid_y,
                                     float radial_axis, float tangent_axis) :
    m_type(ELLIPSE),
    m_centroid(),
    m_radial_axis(radial_axis),
    m_tangent_axis(tangent_axis),
    m_theta_begin(),
    m_theta_end()
{
    m_centroid[0] = centroid_x;
    m_centroid[1] = centroid_y;
}

CombatSetupRegion::CombatSetupRegion(float centroid_x, float centroid_y,
                                     float radial_axis, float tangent_axis,
                                     float theta_begin, float theta_end) :
    m_type(PARTIAL_ELLIPSE),
    m_centroid(),
    m_radial_axis(radial_axis),
    m_tangent_axis(tangent_axis),
    m_theta_begin(theta_begin),
    m_theta_end(theta_end)
{
    m_centroid[0] = centroid_x;
    m_centroid[1] = centroid_y;
}


////////////////////////////////////////////////
// CombatSetupGroup
////////////////////////////////////////////////
CombatSetupGroup::CombatSetupGroup() :
    m_allow(false)
{}
