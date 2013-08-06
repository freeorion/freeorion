#include "MultiplayerCommon.h"

#include "OptionsDB.h"
#include "Obstacle.h"
#include "../combat/AsteroidBeltObstacle.h"
#include "../util/Directories.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Math.h"
#include "../util/Random.h"
#include "../universe/Fleet.h"
#include "../universe/Planet.h"
#include "../universe/System.h"

#if defined(_MSC_VER) && defined(int64_t)
#undef int64_t
#endif

#include <GG/utf8/checked.h>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/system/system_error.hpp>


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
        db.AddFlag("test-3d-combat",                UserStringNop("OPTIONS_DB_TEST_3D_COMBAT"),        false);
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    const double TWO_PI = 8.0 * std::atan(1.0);

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

    // overload for arriving fleets
    void FindPlacementAreas(const TemporaryPtr<System> system,
                            const std::map<int, std::vector<TemporaryPtr<Fleet> > >& arriving_fleets_by_starlane,
                            const std::map<int, std::set<int> >& empires_by_starlane,
                            std::vector<CombatSetupGroup>& setup_groups)
    {
        assert(!arriving_fleets_by_starlane.begin()->second.back()->Unowned());
        int owner = arriving_fleets_by_starlane.begin()->second.back()->Owner();
        for (std::map<int, std::vector<TemporaryPtr<Fleet> > >::const_iterator it = arriving_fleets_by_starlane.begin();
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
                const double SLICE_SIZE = TWO_PI / empires;
                const double START = position_among_empires * SLICE_SIZE;
                setup_group.m_regions.push_back(
                    CombatSetupRegion(x, y, StarlaneEntranceRadialAxis(), StarlaneEntranceTangentAxis(),
                                      START, START + SLICE_SIZE));
            }

            setup_group.m_allow = true;
        }
    }

    // overload for present fleets
    void FindPlacementAreas(TemporaryPtr<const System> system,
                            const std::vector<TemporaryPtr<Fleet> >& fleets,
                            CombatSetupGroup& setup_group)
    {
        for (std::size_t i = 0; i < fleets.size(); ++i) {
            setup_group.m_ships.insert(fleets[i]->begin(), fleets[i]->end());
        }

        for (System::const_orbit_iterator it = system->begin(); it != system->end(); ++it) {
            if (TemporaryPtr<const Planet> planet = GetPlanet(it->second)) {
                double orbit_r = OrbitalRadius(it->first);
                double rads = planet->OrbitalPositionOnTurn(CurrentTurn());
                float planet_r = PlanetRadius(planet->Size());
                setup_group.m_regions.push_back(
                    CombatSetupRegion(orbit_r * std::cos(rads), orbit_r * std::sin(rads), planet_r * 1.5));
            }
        }

        setup_group.m_regions.push_back(CombatSetupRegion(0.0, 0.0, StarRadius() / 2.0));

        // provide a gap between the nearest point on the ellipse and the
        // allowed placement area.
        const double FUDGE_FACTOR = 0.1;

        const double STARLANE_ZONE_START =
            StarlaneEntranceOrbitalRadius() - (1 + FUDGE_FACTOR) * StarlaneEntranceRadialAxis() / 2.0;
        setup_group.m_regions.push_back(CombatSetupRegion(STARLANE_ZONE_START, SystemRadius()));

        setup_group.m_allow = false;
    }

    void PlaceShips(TemporaryPtr<const System> system,
                    const CombatSetupGroup& setup_group,
                    const std::map<int, TemporaryPtr<UniverseObject> >& combat_universe,
                    std::map<TemporaryPtr<Ship>, std::pair<OpenSteer::Vec3, OpenSteer::Vec3> >& placements)
    {
        Logger().debugStream() << "Placing combat ships in " << system->Name();
        // TODO: Do something other than just random placement.  Take into
        // account more of the tactical situation (enemy fleets, etc.).
        for (std::set<int>::const_iterator it = setup_group.m_ships.begin();
             it != setup_group.m_ships.end(); ++it)
        {
            assert(combat_universe.find(*it) != combat_universe.end());
            TemporaryPtr<Ship> ship = universe_object_ptr_cast<Ship>(combat_universe.find(*it)->second);
#define LIMIT_ITERATIONS 1
#if LIMIT_ITERATIONS
            const unsigned int MAX_PLACEMENT_ITERATIONS = 50;
#endif
            if (setup_group.m_allow) {
#if LIMIT_ITERATIONS
                // TODO: Account for possible lack of room when placing ships.
                unsigned int iterations = 0;
                while (iterations++ < MAX_PLACEMENT_ITERATIONS) {
#else
                while (1) {
#endif
                    CombatSetupRegion region =
                        setup_group.m_regions[RandSmallInt(0, setup_group.m_regions.size() - 1)];
                    double min_radius =
                        region.m_type == CombatSetupRegion::RING ?
                        region.m_radius_begin :
                        0.0;
                    double max_radius =
                        region.m_type == CombatSetupRegion::RING ?
                        region.m_radius_end :
                        std::max(region.m_radial_axis, region.m_tangent_axis);
                    double min_theta =
                        region.m_type == CombatSetupRegion::PARTIAL_ELLIPSE ? region.m_theta_begin : 0.0;
                    double max_theta =
                        region.m_type == CombatSetupRegion::PARTIAL_ELLIPSE ? region.m_theta_end : TWO_PI;
                    double r = RandDouble(min_radius, max_radius);
                    double theta = RandDouble(min_theta, max_theta);
                    double point[2] = {
                        region.m_centroid[0] + r * std::cos(theta),
                        region.m_centroid[1] + r * std::sin(theta)
                    };
                    if (PointInRegion(point, region)) {
                        // TODO: Check for proximity to already-placed ships.
                        placements[ship].first = OpenSteer::Vec3(point[0], point[1], 0.0);
                        placements[ship].second = -placements[ship].first;
                        break;
                    }
                }
            } else {
#if LIMIT_ITERATIONS
                // TODO: Account for possible lack of room when placing ships.
                unsigned int iterations = 0;
                while (iterations++ < MAX_PLACEMENT_ITERATIONS) {
#else
                while (1) {
#endif
                    double point[2] = {
                        RandDouble(-SystemRadius(), SystemRadius()),
                        RandDouble(-SystemRadius(), SystemRadius())
                    };
                    bool valid_location = true;
                    for (std::size_t i = 0; i < setup_group.m_regions.size(); ++i) {
                        if (PointInRegion(point, setup_group.m_regions[i])) {
                            valid_location = false;
                            break;
                        }
                    }
                    if (valid_location) {
                        // TODO: Check for proximity to already-placed ships.
                        placements[ship].first = OpenSteer::Vec3(point[0], point[1], 0.0);
                        placements[ship].second = -placements[ship].first;
                        break;
                    }
                }
            }
        }
    }

    void PlaceShips(TemporaryPtr<const System> system,
                    const std::map<int, std::vector<CombatSetupGroup> >& setup_groups,
                    const std::map<int, TemporaryPtr<UniverseObject> >& combat_universe,
                    std::map<TemporaryPtr<Ship>, std::pair<OpenSteer::Vec3, OpenSteer::Vec3> >& placements)
    {
        for (std::map<int, std::vector<CombatSetupGroup> >::const_iterator it = setup_groups.begin();
             it != setup_groups.end(); ++it)
        {
            for (std::size_t i = 0; i < it->second.size(); ++i) {
                PlaceShips(system, it->second[i], combat_universe, placements);
            }
        }
    }
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
MultiplayerLobbyData::MultiplayerLobbyData(bool build_save_game_list) :
    m_new_game(true),
    m_save_file_index(-1),
    m_players(),
    m_save_games(),
    m_save_game_empire_data()
{
    if (!build_save_game_list)
        return;

    Logger().debugStream() << "MultiplayerLobbyData::MultiplayerLobbyData(true)";

    // build a list of save files
    fs::path save_dir(GetSaveDir());
    Logger().debugStream() << "MultiplayerLobbyData::MultiplayerLobbyData save dir path: " << PathString(save_dir);
    fs::directory_iterator end_it;
    for (fs::directory_iterator it(save_dir); it != end_it; ++it) {
        try {
#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
            if (fs::exists(*it) && !fs::is_directory(*it) && PathString(it->path().filename())[0] != '.') {
                std::string filename = PathString(it->path().filename());
#else
            if (fs::exists(*it) && !fs::is_directory(*it) && it->path().filename()[0] != '.') {
                std::string filename = it->path().filename();
#endif
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


////////////////////////////////////////////////
// CombatData
////////////////////////////////////////////////
CombatData::CombatData(TemporaryPtr<System> system, std::map<int, std::vector<CombatSetupGroup> >& setup_groups) :
    m_combat_turn_number(0),
    m_system(system)
{
    using OpenSteer::SphereObstacle;
    using OpenSteer::Vec3;
    m_pathing_engine.AddObstacle(new SphereObstacle(StarRadius() / 2.0, Vec3()));

    std::map<int, std::vector<TemporaryPtr<Fleet> > > present_fleets_by_empire;
    std::map<int, std::map<int, std::vector<TemporaryPtr<Fleet> > > > arriving_fleets_by_starlane_by_empire;
    std::map<int, std::set<int> > empires_by_starlane;

    for (System::const_orbit_iterator it = system->begin(); it != system->end(); ++it) {
        m_combat_universe[it->second] = GetUniverseObject(it->second);
        if (TemporaryPtr<const Planet> planet =
                universe_object_ptr_cast<const Planet>(m_combat_universe[it->second])) {
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
        } else if (TemporaryPtr<Fleet> fleet =
                   universe_object_ptr_cast<Fleet>(m_combat_universe[it->second])) {
            assert(!fleet->Unowned());
            int owner = fleet->Owner();
            if (fleet->ArrivedThisTurn()) {
                int starlane = fleet->ArrivalStarlane();
                arriving_fleets_by_starlane_by_empire[owner][starlane].push_back(fleet);
                empires_by_starlane[starlane].insert(owner);
            } else {
                present_fleets_by_empire[owner].push_back(fleet);
            }
        }
    }

    for (std::map<int, std::vector<TemporaryPtr<Fleet> > >::iterator it = present_fleets_by_empire.begin();
         it != present_fleets_by_empire.end();
         ++it)
    {
        std::vector<CombatSetupGroup>& empire_setup_groups = setup_groups[it->first];
        empire_setup_groups.push_back(CombatSetupGroup());
        FindPlacementAreas(m_system, it->second, empire_setup_groups.back());
    }

    for (std::map<int, std::map<int, std::vector<TemporaryPtr<Fleet> > > >::iterator
             it = arriving_fleets_by_starlane_by_empire.begin();
         it != arriving_fleets_by_starlane_by_empire.end();
         ++it)
    {
        FindPlacementAreas(m_system, it->second, empires_by_starlane, setup_groups[it->first]);
    }

    std::map<TemporaryPtr<Ship>, std::pair<OpenSteer::Vec3, OpenSteer::Vec3> > placements;
    PlaceShips(m_system, setup_groups, m_combat_universe, placements);

    for (std::map<int, std::vector<CombatSetupGroup> >::const_iterator it = setup_groups.begin();
         it != setup_groups.end();
         ++it) {
        for (std::size_t i = 0; i < it->second.size(); ++i) {
            for (std::set<int>::const_iterator ship_it = it->second[i].m_ships.begin();
                 ship_it != it->second[i].m_ships.end();
                 ++ship_it) {
                TemporaryPtr<Ship> ship = GetShip(*ship_it);
                assert(ship);
                const std::pair<OpenSteer::Vec3, OpenSteer::Vec3>& placement = placements[ship];
                CombatShipPtr combat_ship(
                    new CombatShip(ship, placement.first, placement.second, m_combat_universe, m_pathing_engine));
                m_pathing_engine.AddObject(combat_ship);
            }
        }
    }
}


////////////////////////////////////////////////
// CombatSetupRegion
////////////////////////////////////////////////
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
    m_radius_begin(),
    m_radius_end(),
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
    m_radius_begin(),
    m_radius_end(),
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
// PointInRegion()
////////////////////////////////////////////////
bool PointInRegion(double point[2], const CombatSetupRegion& region)
{
    bool retval = false;
    switch (region.m_type) {
    case CombatSetupRegion::RING: {
        double range = std::sqrt(point[0] * point[0] + point[1] * point[1]);
        retval = region.m_radius_begin < range && range < region.m_radius_end;
        break;
    }
    case CombatSetupRegion::ELLIPSE: {
        double theta_major = std::atan2(region.m_centroid[1], region.m_centroid[0]);
        retval = PointInEllipse(point[0], point[1],
                                region.m_centroid[0], region.m_centroid[1],
                                region.m_radial_axis, region.m_tangent_axis,
                                theta_major);
        break;
    }
    case CombatSetupRegion::PARTIAL_ELLIPSE: {
        double theta_major = std::atan2(region.m_centroid[1], region.m_centroid[0]);
        retval = PointInPartialEllipse(point[0], point[1],
                                       region.m_centroid[0], region.m_centroid[1],
                                       region.m_radial_axis, region.m_tangent_axis,
                                       theta_major,
                                       region.m_theta_begin, region.m_theta_end);
        break;
    }
    }
    return retval;
}
