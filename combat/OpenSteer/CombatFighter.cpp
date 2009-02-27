#include "CombatFighter.h"

#include "CombatShip.h"
#include "PathingEngine.h"

#include <boost/cast.hpp>
#include <boost/assign/list_of.hpp>

// TODO: For prototyping only.
#include <GL/gl.h>

#include <map>


namespace {

#if 0
    inline void iDrawTriangle (const OpenSteer::Vec3& a,
                               const OpenSteer::Vec3& b,
                               const OpenSteer::Vec3& c,
                               const OpenSteer::Color& color)
    {
        OpenSteer::warnIfInUpdatePhase ("iDrawTriangle");
        glColor3f (color.r(), color.g(), color.b());
        glBegin (GL_TRIANGLES);
        {
            OpenSteer::glVertexVec3 (a);
            OpenSteer::glVertexVec3 (b);
            OpenSteer::glVertexVec3 (c);
        }
        glEnd ();
    }

    void drawFighter(const OpenSteer::AbstractVehicle& vehicle, const OpenSteer::Color& color, float aspect)
    {
        // "aspect ratio" of body (as seen from above)
        const float x = aspect;
        const float y = OpenSteer::sqrtXXX (1 - (x * x));

        // radius and position of vehicle
        const float r = vehicle.radius();
        const OpenSteer::Vec3& p = vehicle.position();

        // body shape parameters
        const OpenSteer::Vec3 f = r * vehicle.forward();
        const OpenSteer::Vec3 s = r * vehicle.side() * x;
        const OpenSteer::Vec3 u = r * vehicle.up() * x * 0.5f;
        const OpenSteer::Vec3 b = r * vehicle.forward() * -y;

        // vertex positions
        const OpenSteer::Vec3 nose   = p + f;
        const OpenSteer::Vec3 side1  = p + b - s;
        const OpenSteer::Vec3 side2  = p + b + s;
        const OpenSteer::Vec3 top    = p + b + u;
        const OpenSteer::Vec3 bottom = p + b - u;

        // colors
        const float j = +0.05f;
        const float k = -0.05f;
        const OpenSteer::Color color1 = color + OpenSteer::Color(j, j, k);
        const OpenSteer::Color color2 = color + OpenSteer::Color(j, k, j);
        const OpenSteer::Color color3 = color + OpenSteer::Color(k, j, j);
        const OpenSteer::Color color4 = color + OpenSteer::Color(k, j, k);
        const OpenSteer::Color color5 = color + OpenSteer::Color(k, k, j);

        // draw body
        iDrawTriangle (nose,  side1,  top,    color1);  // top, side 1
        iDrawTriangle (nose,  top,    side2,  color2);  // top, side 2
        iDrawTriangle (nose,  bottom, side1,  color3);  // bottom, side 1
        iDrawTriangle (nose,  side2,  bottom, color4);  // bottom, side 2
        iDrawTriangle (side1, side2,  top,    color5);  // top back
        iDrawTriangle (side2, side1,  bottom, color5);  // bottom back
    }

    void drawInterceptor(const OpenSteer::AbstractVehicle& vehicle, const OpenSteer::Color& color)
    { drawFighter(vehicle, color, 0.4f); }

    void drawBomber(const OpenSteer::AbstractVehicle& vehicle, const OpenSteer::Color& color)
    { drawFighter(vehicle, color, 1.0f); }
#endif

    ////////////////////////////////////////////////////////////////////////////////
    // TODO: BEGIN section for testing only.
    ////////////////////////////////////////////////////////////////////////////////

    const float MAX_POINT_DEFENSE_RANGE = 10.0;

    ////////////////////////////////////////////////////////////////////////////////
    // TODO: END section for testing only.
    ////////////////////////////////////////////////////////////////////////////////

#define ECHO_TOKEN(x) (x, #x)
    std::map<CombatFighter::MissionType, std::string> FIGHTER_MISSION_STRINGS =
        boost::assign::map_list_of
        ECHO_TOKEN(CombatFighter::NONE)
        ECHO_TOKEN(CombatFighter::MOVE_TO)
        ECHO_TOKEN(CombatFighter::ATTACK_THIS)
        ECHO_TOKEN(CombatFighter::DEFEND_THIS)
        ECHO_TOKEN(CombatFighter::PATROL_TO)
        ECHO_TOKEN(CombatFighter::ATTACK_FIGHTERS_BOMBERS_FIRST)
        ECHO_TOKEN(CombatFighter::ATTACK_FIGHTERS_INTERCEPTORS_FIRST)
        ECHO_TOKEN(CombatFighter::ATTACK_SHIPS_WEAKEST_FIRST)
        ECHO_TOKEN(CombatFighter::ATTACK_SHIPS_NEAREST_FIRST)
        ECHO_TOKEN(CombatFighter::RETURN_TO_BASE);
#undef ECHO_TOKEN

    const std::size_t TARGET_FPS = 60;
    const std::size_t TARGET_FIGHTER_UPDATES_PER_SEC = 2;
    const std::size_t UPDATE_SETS = TARGET_FPS / TARGET_FIGHTER_UPDATES_PER_SEC;
}


////////////////////////////////////////////////////////////////////////////////
// CombatFighterFormation
////////////////////////////////////////////////////////////////////////////////
CombatFighterFormation::CombatFighterFormation(PathingEngine& pathing_engine) :
    m_pathing_engine(pathing_engine)
{}

CombatFighterFormation::~CombatFighterFormation()
{ m_pathing_engine.RemoveObject(m_leader); }

const CombatFighter& CombatFighterFormation::Leader() const
{ return *m_leader; }

OpenSteer::Vec3 CombatFighterFormation::Centroid() const
{
    OpenSteer::Vec3 retval;
    std::size_t count = 0;
    for (const_iterator it = begin(); it != end(); ++it) {
        retval += (*it)->position();
        ++count;
    }
    retval /= count;
    return retval;
}

bool CombatFighterFormation::empty() const
{ return m_members.empty(); }

std::size_t CombatFighterFormation::size() const
{ return m_members.size(); }

CombatFighterFormation::const_iterator CombatFighterFormation::begin() const
{ return m_members.begin(); }

CombatFighterFormation::const_iterator CombatFighterFormation::end() const
{ return m_members.end(); }

void CombatFighterFormation::SetLeader(const CombatFighterPtr& fighter)
{
    assert(!m_leader);
    m_leader = fighter;
    m_pathing_engine.AddObject(m_leader);
}

void CombatFighterFormation::push_back(const CombatFighterPtr& fighter)
{
    assert(fighter);
    m_members.push_back(fighter);
}

void CombatFighterFormation::erase(const CombatFighterPtr& fighter)
{
    std::list<CombatFighterPtr>::iterator it = std::find(m_members.begin(), m_members.end(), fighter);
    if (it != m_members.end())
        m_members.erase(it);
}

void CombatFighterFormation::erase(CombatFighter* fighter)
{
    std::list<CombatFighterPtr>::iterator it = m_members.begin();
    for (; it != m_members.end(); ++it) {
        if (it->get() == fighter)
            break;
    }
    if (it != m_members.end())
        m_members.erase(it);
}


////////////////////////////////////////////////////////////////////////////////
// CombatFighter::Mission
////////////////////////////////////////////////////////////////////////////////
CombatFighter::Mission::Mission(CombatFighter::MissionType type) :
    m_type(type),
    m_destination(),
    m_target()
{}

CombatFighter::Mission::Mission(CombatFighter::MissionType type, const OpenSteer::Vec3& destination) :
    m_type(type),
    m_destination(destination),
    m_target()
{}

CombatFighter::Mission::Mission(CombatFighter::MissionType type, const CombatObjectPtr& target) :
    m_type(type),
    m_destination(),
    m_target(target)
{}


////////////////////////////////////////////////////////////////////////////////
// CombatFighter
////////////////////////////////////////////////////////////////////////////////
CombatFighter::CombatFighter(CombatObjectPtr base, CombatFighterType type, int empire_id,
                             PathingEngine& pathing_engine, const CombatFighterFormationPtr& formation,
                             int formation_position) :
    m_proximity_token(0),
    m_leader(false),
    m_type(type),
    m_empire_id(empire_id),
    m_mission_queue(),
    m_mission_weight(0.0),
    m_base(base),
    m_formation_position(formation_position),
    m_formation(formation),
    m_pathing_engine(pathing_engine)
    ,m_instrument(false)
    ,m_last_mission(NONE)
{ Init(); }

CombatFighter::CombatFighter(CombatObjectPtr base, CombatFighterType type, int empire_id,
                             PathingEngine& pathing_engine, const CombatFighterFormationPtr& formation) :
    m_proximity_token(0),
    m_leader(true),
    m_type(type),
    m_empire_id(empire_id),
    m_mission_queue(),
    m_mission_weight(0.0),
    m_base(base),
    m_formation_position(-1),
    m_formation(formation),
    m_pathing_engine(pathing_engine)
    ,m_instrument(false)
    ,m_last_mission(NONE)
{ Init(); }

CombatFighter::~CombatFighter()
{
    delete m_proximity_token;
    if (m_formation)
        m_formation->erase(this);
}

float CombatFighter::maxForce() const
{
    float retval = SimpleVehicle::maxForce();
    if (!m_leader) {
        // Use sigmoid function to smooth force diminution when moving into
        // position in formation.  Without cutting the max force, the fighters
        // wiggle wildly side-to-side when they are very close to their
        // formation positions.
        const float MAX_FORCE_DISTANCE = 12.0;
        const float MIN_FORCE = retval / 5.0;
        const float MAX_FORCE = retval * 1.1;
        float sig =
            1.0 / (1.0 + std::exp(-(m_out_of_formation.length() - MAX_FORCE_DISTANCE / 2.0f)));
        retval = MIN_FORCE + (MAX_FORCE - MIN_FORCE) * sig;
    }
    return retval;
}

float CombatFighter::maxSpeed() const
{
    float retval = SimpleVehicle::maxSpeed();
    if (!m_leader) {
        // Use sigmoid function to smooth speed scaling when moving into
        // position in formation.
        const float MAX_SPEED_DISTANCE = 12.0;
        const float MIN_SPEED = m_formation->Leader().speed();
        const float MAX_SPEED = retval * 1.1;
        float sig =
            1.0 / (1.0 + std::exp(-(m_out_of_formation.length() - MAX_SPEED_DISTANCE / 2.0f)));
        retval = MIN_SPEED + (MAX_SPEED - MIN_SPEED) * sig;
    }
    return retval;
}

const CombatFighter::Mission& CombatFighter::CurrentMission() const
{ return m_mission_queue.back(); }

void CombatFighter::update(const float /*current_time*/, const float elapsed_time)
{
    OpenSteer::Vec3 steer = m_last_steer;
    if (m_pathing_engine.UpdateNumber() % UPDATE_SETS == serialNumber % UPDATE_SETS) {
        if (m_leader)
            UpdateMissionQueue();
        steer = Steer();
    }
    applySteeringForce(steer, elapsed_time);
    m_last_steer = steer;
    if (m_leader)
        m_proximity_token->UpdatePosition(position());
}

void CombatFighter::regenerateLocalSpace(const OpenSteer::Vec3& new_velocity, const float elapsed_time)
{ regenerateLocalSpaceForBanking(new_velocity, elapsed_time); }

#if 0
// TODO: for testing only
void CombatFighter::Draw()
{
    if (!m_leader) {
        if (m_type == INTERCEPTOR)
            drawInterceptor(*this, m_empire_id == 0 ? OpenSteer::gRed : OpenSteer::gBlue);
        else
            drawBomber(*this, m_empire_id == 0 ? OpenSteer::gDarkRed : OpenSteer::gDarkBlue);
    }
}
#endif

CombatFighterFormationPtr CombatFighter::Formation()
{ return m_formation; }

void CombatFighter::AppendMission(const Mission& mission)
{
    assert(!m_mission_queue.empty());
    if (m_mission_queue.back().m_type == NONE) {
        assert(m_mission_queue.size() == 1u);
        m_mission_queue.clear();
    }
    m_mission_queue.push_front(mission);
}

void CombatFighter::ClearMissions()
{
    m_mission_queue.clear();
    m_mission_queue.push_front(Mission(NONE));
}

void CombatFighter::Init()
{
    if (m_leader) {
        m_proximity_token =
            m_pathing_engine.GetProximityDB().AllocateToken(
                this,
                m_type == INTERCEPTOR ? INTERCEPTOR_FLAG : BOMBER_FLAG,
                EmpireFlag(m_empire_id));
    }

    SimpleVehicle::reset();
    SimpleVehicle::setMaxForce(27);
    SimpleVehicle::setMaxSpeed(9);

    // TODO: setMass()

    if (m_leader) {
        // TODO: These two lines are for testing only!
        regenerateOrthonormalBasisUF(OpenSteer::RandomUnitVector());
        SimpleVehicle::setPosition(OpenSteer::RandomVectorInUnitRadiusSphere() * 20);

        SimpleVehicle::setSpeed(CombatFighter::maxSpeed());
    } else {
        regenerateOrthonormalBasis(m_formation->Leader().forward(),
                                   m_formation->Leader().up());
        SimpleVehicle::setPosition(GlobalFormationPosition());
        SimpleVehicle::setSpeed(0.0);
    }

    if (m_leader)
        m_proximity_token->UpdatePosition(position());

    m_mission_queue.push_front(Mission(NONE));
}

OpenSteer::Vec3 CombatFighter::GlobalFormationPosition()
{
    OpenSteer::Vec3 retval;
    const OpenSteer::Vec3 FORMATION_POSITIONS[FORMATION_SIZE] = {
        OpenSteer::Vec3( 0.0,  0.0, -0.5),
        OpenSteer::Vec3( 3.0,  0.0, -1.5),
        OpenSteer::Vec3( 0.0,  3.0, -1.5),
        OpenSteer::Vec3(-3.0,  0.0, -1.5),
        OpenSteer::Vec3( 0.0, -3.0, -1.5)
    };
    if (!m_leader) {
        OpenSteer::Vec3 position = FORMATION_POSITIONS[m_formation_position];
        retval = m_formation->Leader().globalizePosition(position);
    }
    return retval;
}

void CombatFighter::RemoveMission()
{
    m_mission_queue.pop_back();
    if (m_mission_queue.empty())
        m_mission_queue.push_front(Mission(NONE));
}

void CombatFighter::UpdateMissionQueue()
{
    assert(m_leader);
    assert(!m_mission_queue.empty());

    const float DEFAULT_MISSION_WEIGHT = 12.0;
    const float MAX_MISSION_WEIGHT = 48.0;

    const float AT_DESTINATION = 3.0;
    const float AT_DEST_SQUARED = AT_DESTINATION * AT_DESTINATION;

    bool print_needed = false;
    if (m_instrument && m_last_mission != m_mission_queue.back().m_type) {
        std::cout << "empire=" << m_empire_id << " type=" << (m_type ? "BOMBER" : "INTERCEPTOR") << "\n"
                  << "    prev mission=" << FIGHTER_MISSION_STRINGS[m_last_mission] << "\n"
                  << "    new mission =" << FIGHTER_MISSION_STRINGS[m_mission_queue.back().m_type] << "\n";
        print_needed = true;
        m_last_mission = m_mission_queue.back().m_type;
    }

    m_mission_weight = 0.0;
    m_mission_destination = OpenSteer::Vec3();

    switch (m_mission_queue.back().m_type) {
    case NONE: {
        assert(m_mission_queue.size() == 1u);
        m_mission_queue.clear();
        if (m_type == INTERCEPTOR)
            m_mission_queue.push_front(Mission(ATTACK_FIGHTERS_BOMBERS_FIRST));
        else
            m_mission_queue.push_front(Mission(ATTACK_SHIPS_WEAKEST_FIRST));
        if (print_needed) std::cout << "    [STARTING DEFAULT MISSION]\n";
        break;
    }
    case MOVE_TO: {
        if (AT_DEST_SQUARED < (position() - m_mission_queue.back().m_destination).lengthSquared()) {
            m_mission_weight = MAX_MISSION_WEIGHT;
            m_mission_destination = m_mission_queue.back().m_destination;
        } else {
            if (print_needed) std::cout << "    [ARRIVED]\n";
            RemoveMission();
        }
        break;
    }
    case ATTACK_THIS: {
        if (CombatObjectPtr target = m_mission_queue.back().m_target.lock()) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            m_mission_destination = target->position();
            if (CombatFighterPtr f = boost::dynamic_pointer_cast<CombatFighter>(target))
                m_mission_destination = f->Formation()->Centroid();
        } else {
            if (print_needed) std::cout << "    [ATTACK TARGET GONE]\n";
            RemoveMission();
        }
        break;
    }
    case DEFEND_THIS: {
        if (CombatObjectPtr target = m_mission_queue.back().m_target.lock()) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            if (m_mission_subtarget = WeakestAttacker(target))
                m_mission_destination = m_mission_subtarget->position();
            else
                m_mission_destination = target->position();
        } else {
            if (print_needed) std::cout << "    [DEFEND TARGET GONE]\n";
            RemoveMission();
        }
        break;
    }
    case PATROL_TO: {
        // TODO: Consider making the engagement range dynamically adjustable by the user.
        const float PATROL_ENGAGEMENT_RANGE = 50.0;
        if (AT_DEST_SQUARED < (position() - m_mission_queue.back().m_destination).lengthSquared()) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            bool found_target = false;
            if (m_type == INTERCEPTOR) {
                if (CombatFighterPtr fighter =
                    m_pathing_engine.NearestHostileFighterInRange(position(), m_empire_id,
                                                                  PATROL_ENGAGEMENT_RANGE)) {
                    m_mission_destination = fighter->position();
                    m_mission_queue.push_back(Mission(ATTACK_THIS, fighter));
                    found_target = true;
                    if (print_needed) std::cout << "    [ENGAGING HOSTILE FIGHTER]\n";
                }
            } else {
                if (CombatObjectPtr object =
                    m_pathing_engine.NearestHostileNonFighterInRange(position(), m_empire_id,
                                                                     PATROL_ENGAGEMENT_RANGE)) {
                    m_mission_destination = object->position();
                    m_mission_queue.push_back(Mission(ATTACK_THIS, object));
                    found_target = true;
                    if (print_needed) std::cout << "    [ENGAGING HOSTILE SHIP]\n";
                }
            }
            if (!found_target)
                m_mission_destination = m_mission_queue.back().m_destination;
        } else {
            if (print_needed) std::cout << "    [ARRIVED]\n";
            RemoveMission();
        }
        break;
    }
    case ATTACK_FIGHTERS_BOMBERS_FIRST: {
        CombatFighterPtr fighter = m_pathing_engine.NearestHostileBomber(position(), m_empire_id);
        if (!fighter)
            fighter = m_pathing_engine.NearestHostileInterceptor(position(), m_empire_id);
        if (fighter) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            m_mission_destination = fighter->position();
            m_mission_queue.push_back(Mission(ATTACK_THIS, fighter));
            if (print_needed) std::cout << "    [ENGAGING HOSTILE FIGHTER]\n";
        } else {
            if (print_needed) std::cout << "    [NO TARGETS]\n";
            RemoveMission();
        }
        break;
    }
    case ATTACK_FIGHTERS_INTERCEPTORS_FIRST: {
        CombatFighterPtr fighter = m_pathing_engine.NearestHostileInterceptor(position(), m_empire_id);
        if (!fighter)
            fighter = m_pathing_engine.NearestHostileBomber(position(), m_empire_id);
        if (fighter) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            m_mission_destination = fighter->position();
            m_mission_queue.push_back(Mission(ATTACK_THIS, fighter));
            if (print_needed) std::cout << "    [ENGAGING HOSTILE FIGHTER]\n";
        } else {
            if (print_needed) std::cout << "    [NO TARGETS]\n";
            RemoveMission();
        }
        break;
    }
    case ATTACK_SHIPS_WEAKEST_FIRST: {
        if (CombatObjectPtr object = WeakestHostileShip()) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            m_mission_destination = object->position();
            m_mission_queue.push_back(Mission(ATTACK_THIS, object));
            if (print_needed) std::cout << "    [ENGAGING HOSTILE SHIP]\n";
        } else {
            if (print_needed) std::cout << "    [NO TARGETS]\n";
            RemoveMission();
        }
        m_mission_weight = DEFAULT_MISSION_WEIGHT;
        break;
    }
    case ATTACK_SHIPS_NEAREST_FIRST: {
        if (CombatObjectPtr object = m_pathing_engine.NearestHostileShip(position(), m_empire_id)) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            m_mission_destination = object->position();
            m_mission_queue.push_back(Mission(ATTACK_THIS, object));
            if (print_needed) std::cout << "    [ENGAGING HOSTILE SHIP]\n";
        } else {
            if (print_needed) std::cout << "    [NO TARGETS]\n";
            RemoveMission();
        }
        break;
    }
    case RETURN_TO_BASE: {
        if (!m_base.expired()) {
            if (AT_DEST_SQUARED < (position() - m_mission_queue.back().m_destination).lengthSquared()) {
                m_mission_weight = MAX_MISSION_WEIGHT;
                m_mission_destination = m_mission_queue.back().m_destination;
            } else {
                // TODO: Dock up (add entire formation back to base/carrier).
                m_pathing_engine.RemoveFighterFormation(m_formation);
                if (print_needed) std::cout << "    [ARRIVED AT BASE]\n";
                RemoveMission();
            }
        } else {
            if (print_needed) std::cout << "    [BASE GONE]\n";
            RemoveMission();
        }
        break;
    }
    }

    if (print_needed)
        std::cout << "    position   =" << position() << "\n"
                  << "    destination=" << m_mission_destination << "\n"
                  << "    mission_weight=" << m_mission_weight << "\n"
                  << std::endl;
}

OpenSteer::Vec3 CombatFighter::Steer()
{
    static bool once = true;
    if (once && m_type == BOMBER && m_leader) {
        //m_instrument = true;
        once = false;
    }

    // A note about obstacle avoidance.  Avoidance of static obstacles (planets,
    // asteroid feilds, etc.) are represented in static_obstacle_avoidance.  The
    // analogous avoidance of dynamic obstacles (ships, etc.) -- that is, an
    // overriding move away from an object that the fighter is about to collide
    // with -- is represented in dynamic_obstacle_avoidance.  Another object
    // avoidance factor exists, nonfighter_obstacle_evasion_vec, but it works a
    // bit differently from the other two.  It provides a distance-variable
    // force moving the fighter away, but does not get treated as the only
    // movement force if it exists (as static_obstacle_avoidance and
    // dynamic_obstacle_avoidance do).  Instead, it is just one of the many
    // forces moving the fighter, along with steering towards mission objectives
    // and away from PD, etc.

    const float OBSTACLE_AVOIDANCE_TIME = 1.0;

    const OpenSteer::Vec3 static_obstacle_avoidance =
        steerToAvoidObstacles(OBSTACLE_AVOIDANCE_TIME,
                              m_pathing_engine.Obstacles().begin(),
                              m_pathing_engine.Obstacles().end());

    if (static_obstacle_avoidance != OpenSteer::Vec3::zero)
        return static_obstacle_avoidance;

    const float SEPARATION_RADIUS =  5.0f;
    const float SEPARATION_ANGLE  = -0.707f;
    const float SEPARATION_WEIGHT =  6.0f;

    const float ALIGNMENT_RADIUS = 7.5f;
    const float ALIGNMENT_ANGLE  = 0.7f;
    const float ALIGNMENT_WEIGHT = 6.0f;

    const float COHESION_RADIUS =  9.0f;
    const float COHESION_ANGLE  = -0.15f;
    const float COHESION_WEIGHT =  4.0f;

    const float FORMATION_WEIGHT = 8.0f;

    const float BOMBER_INTERCEPTOR_EVASION_RADIUS =
        m_type == BOMBER ? 25.0f : SEPARATION_RADIUS;
    const float BOMBER_INTERCEPTOR_EVASION_WEIGHT = 8.0f;

    const float NONFIGHTER_OBSTACLE_AVOIDANCE_RADIUS = 10.0;
    const float NONFIGHTER_OBSTACLE_AVOIDANCE_WEIGHT = 10.0;

    const float POINT_DEFENSE_AVOIDANCE_RADIUS = MAX_POINT_DEFENSE_RANGE * 1.5;
    const float POINT_DEFENSE_AVOIDANCE_WEIGHT = 8.0;

    // The leader (a "fake" fighter that the real fighters in a formation
    // follow) takes into account all other fighters in proximity.  Followers
    // ("real", or "normal" fighters) only consider its fellow formation-mates
    // its neighbors.
    OpenSteer::AVGroup neighbors;
    OpenSteer::AVGroup nonfighters;
    if (m_leader) {
        const float FIGHTER_RADIUS = std::max(SEPARATION_RADIUS,
                                              std::max(ALIGNMENT_RADIUS,
                                                       COHESION_RADIUS));
        const float NONFIGHTER_RADIUS = std::max(NONFIGHTER_OBSTACLE_AVOIDANCE_RADIUS,
                                                 POINT_DEFENSE_AVOIDANCE_RADIUS);
        m_pathing_engine.GetProximityDB().FindInRadius(position(), FIGHTER_RADIUS, neighbors,
                                                       FIGHTER_FLAGS, EmpireFlag(m_empire_id));
        m_pathing_engine.GetProximityDB().FindInRadius(position(), NONFIGHTER_RADIUS, nonfighters,
                                                       NONFIGHTER_FLAGS);
    } else {
        for (CombatFighterFormation::const_iterator it = m_formation->begin();
             it != m_formation->end();
             ++it) {
            CombatFighterPtr fighter = *it;
            if (fighter->m_formation_position != m_formation_position)
                neighbors.push_back(fighter.get());
        }
    }

    // steer towards mission objectives
    OpenSteer::Vec3 mission_vec;
    if (m_leader && m_mission_weight)
        mission_vec = (m_mission_destination - position()).normalize();

    // steer to maintain formation
    OpenSteer::Vec3 formation_vec;
    m_out_of_formation = OpenSteer::Vec3();
    if (!m_leader) {
        m_out_of_formation = GlobalFormationPosition() - position();
        formation_vec = m_out_of_formation.normalize();
    }

    // steer to avoid interceptors (bombers only)
    OpenSteer::Vec3 bomber_interceptor_evasion_vec;
    OpenSteer::AVGroup interceptor_neighbors;
    if (m_type == BOMBER) {
        m_pathing_engine.GetProximityDB().FindInRadius(
            position(), BOMBER_INTERCEPTOR_EVASION_RADIUS, interceptor_neighbors,
            INTERCEPTOR_FLAG, NotEmpireFlag(m_empire_id));
        OpenSteer::Vec3 direction;
        for (std::size_t i = 0; i < interceptor_neighbors.size(); ++i) {
            direction += steerForFlee(interceptor_neighbors[i]->position());
        }
        bomber_interceptor_evasion_vec = direction.normalize();
    }

    // See note at top of function for partial explanation of these variables'
    // meanings.
    OpenSteer::Vec3 nonfighter_obstacle_evasion_vec;
    OpenSteer::Vec3 point_defense_evasion_vec;
    OpenSteer::Vec3 dynamic_obstacle_avoidance;
    if (m_leader) {
        for (std::size_t i = 0; i < nonfighters.size(); ++i) {
            // TODO: Add code to handle non-ships.
            CombatShip* ship = boost::polymorphic_downcast<CombatShip*>(nonfighters[i]);
            // handle PD avoidance
            OpenSteer::Vec3 away_vec = position() - ship->position();
            float away_vec_length = away_vec.length();
            away_vec /= away_vec_length;
            point_defense_evasion_vec += away_vec * ship->AntiFighterStrength();
            float collision_avoidance_scale_factor =
                std::max(0.0f, NONFIGHTER_OBSTACLE_AVOIDANCE_RADIUS - away_vec_length) /
                NONFIGHTER_OBSTACLE_AVOIDANCE_RADIUS;
            nonfighter_obstacle_evasion_vec += away_vec * collision_avoidance_scale_factor;
            if (OBSTACLE_AVOIDANCE_TIME * speed() < away_vec_length)
                dynamic_obstacle_avoidance += away_vec;
        }
        nonfighter_obstacle_evasion_vec = nonfighter_obstacle_evasion_vec.normalize();
        point_defense_evasion_vec = point_defense_evasion_vec.normalize();
        dynamic_obstacle_avoidance = dynamic_obstacle_avoidance.normalize();
    }
    if (0.0 < dynamic_obstacle_avoidance.lengthSquared())
        return dynamic_obstacle_avoidance;

    // flocking behaviors
    OpenSteer::AVGroup neighbors_to_use;
    if (m_leader) {
        neighbors_to_use.reserve(neighbors.size());
        for (std::size_t i = 0; i < neighbors.size(); ++i) {
            CombatFighter* fighter = boost::polymorphic_downcast<CombatFighter*>(neighbors[i]);
            // exclude self and formation-mates from list
            if (fighter == this || &fighter->m_formation->Leader() == this)
                continue;
            neighbors_to_use.push_back(neighbors[i]);
        }
    } else {
        std::swap(neighbors, neighbors_to_use);
    }

    const OpenSteer::Vec3 separation_vec = steerForSeparation(SEPARATION_RADIUS,
                                                              SEPARATION_ANGLE,
                                                              neighbors_to_use);
    const OpenSteer::Vec3 alignment_vec  = steerForAlignment(ALIGNMENT_RADIUS,
                                                             ALIGNMENT_ANGLE,
                                                             neighbors_to_use);
    const OpenSteer::Vec3 cohesion_vec  = steerForCohesion(COHESION_RADIUS,
                                                           COHESION_ANGLE,
                                                           neighbors_to_use);

    return
        position().normalize() * -1.0f + // TODO: Temporary only!  This is a center-force to keep the fighters moving throught the middle.
        mission_vec * m_mission_weight +
        formation_vec * FORMATION_WEIGHT +
        bomber_interceptor_evasion_vec * BOMBER_INTERCEPTOR_EVASION_WEIGHT +
        nonfighter_obstacle_evasion_vec * NONFIGHTER_OBSTACLE_AVOIDANCE_WEIGHT +
        point_defense_evasion_vec * POINT_DEFENSE_AVOIDANCE_WEIGHT +
        separation_vec * SEPARATION_WEIGHT +
        alignment_vec * ALIGNMENT_WEIGHT +
        cohesion_vec * COHESION_WEIGHT;
}

CombatObjectPtr CombatFighter::WeakestAttacker(const CombatObjectPtr& /*attackee*/)
{
    // TODO: This should act as WeakestShip(), but should include fighters.
    return CombatObjectPtr();
}

CombatShipPtr CombatFighter::WeakestHostileShip()
{
    // TODO: Note that the efficient evaluation of this mission requires a
    // single fighter-vulerability number and a single fighter-attack number to
    // be calculated per design.

    CombatShipPtr retval;
    OpenSteer::AVGroup all;
    m_pathing_engine.GetProximityDB().FindAll(all, SHIP_FLAG, NotEmpireFlag(m_empire_id));
    float weakest = FLT_MAX;
    for (std::size_t i = 0; i < all.size(); ++i) {
        CombatShip* ship = boost::polymorphic_downcast<CombatShip*>(all[i]);
        // TODO: Some kind of "weakness to fighter attacks" should be taken into
        // account here later.
        if (ship->AntiFighterStrength() < weakest) {
            retval = ship->shared_from_this();
            weakest = ship->AntiFighterStrength();
        }
    }
    return retval;
}
