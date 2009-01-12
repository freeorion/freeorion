// ----------------------------------------------------------------------------
//
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//
// ----------------------------------------------------------------------------
//
// Simple soccer game by Michael Holm, IO Interactive A/S
//
// I made this to learn opensteer, and it took me four hours. The players will
// hunt the ball with no team spirit, each player acts on his own accord.
//
// I challenge the reader to change the behavour of one of the teams, to beat my
// team. It should not be too hard. If you make a better team, please share the
// source code so others get the chance to create a team that'll beat yours :)
//
// You are free to use this code for whatever you please.
//
// (contributed on July 9, 2003)
//
// ----------------------------------------------------------------------------


#include <iomanip>
#include <sstream>
#include "OpenSteer/SimpleVehicle.h"
#include "OpenSteer/OpenSteerDemo.h"
#include "OpenSteer/Draw.h"
#include "OpenSteer/Color.h"
#include "OpenSteer/UnusedParameter.h"


namespace {

    using namespace OpenSteer;


    Vec3 playerPosition[9] = {
        Vec3(4,0,0),
        Vec3(7,0,-5),
        Vec3(7,0,5),
        Vec3(10,0,-3),
        Vec3(10,0,3),
        Vec3(15,0, -8),
        Vec3(15,0,0),
        Vec3(15,0,8),
        Vec3(4,0,0)
    };

    // ----------------------------------------------------------------------------

    // a box object for the field and the goals.
    class AABBox{
    public:
        AABBox(Vec3 &min, Vec3& max): m_min(min), m_max(max){}
        AABBox(Vec3 min, Vec3 max): m_min(min), m_max(max){}
        bool	InsideX(const Vec3 p){if(p.x < m_min.x || p.x > m_max.x)	return false;return true;}
        bool	InsideZ(const Vec3 p){if(p.z < m_min.z || p.z > m_max.z)	return false;return true;}
        void	draw(){
            Vec3 b,c;
            b = Vec3(m_min.x, 0, m_max.z);
            c = Vec3(m_max.x, 0, m_min.z);
            Color color(1.0f,1.0f,0.0f);
            drawLineAlpha(m_min, b, color, 1.0f);
            drawLineAlpha(b, m_max, color, 1.0f);
            drawLineAlpha(m_max, c, color, 1.0f);
            drawLineAlpha(c,m_min, color, 1.0f);
        }
    private:
        Vec3 m_min;
        Vec3 m_max;
    };

    // The ball object
    class Ball : public SimpleVehicle{
    public:
        Ball(AABBox *bbox) : m_bbox(bbox) {reset();}

        // reset state
        void reset (void)
        {
            SimpleVehicle::reset (); // reset the vehicle 
            setSpeed (0.0f);         // speed along Forward direction.
            setMaxForce (9.0f);      // steering force is clipped to this magnitude
            setMaxSpeed (9.0f);         // velocity is clipped to this magnitude

            setPosition(0,0,0);
            clearTrailHistory ();    // prevent long streaks due to teleportation 
            setTrailParameters (100, 6000);
        }

        // per frame simulation update
        void update (const float currentTime, const float elapsedTime)
        {
            applyBrakingForce(1.5f, elapsedTime);
            applySteeringForce(velocity(), elapsedTime);
            // are we now outside the field?
            if(!m_bbox->InsideX(position()))
            {
                Vec3 d = velocity();
                regenerateOrthonormalBasis(Vec3(-d.x, d.y, d.z));
                applySteeringForce(velocity(), elapsedTime);
            }
            if(!m_bbox->InsideZ(position()))
            {
                Vec3 d = velocity();
                regenerateOrthonormalBasis(Vec3(d.x, d.y, -d.z));
                applySteeringForce(velocity(), elapsedTime);
            }
        recordTrailVertex (currentTime, position());
        }

        void kick(Vec3 dir, const float elapsedTime){
            OPENSTEER_UNUSED_PARAMETER(elapsedTime);
            
            setSpeed(dir.length());
            regenerateOrthonormalBasis(dir);
        }

        // draw this character/vehicle into the scene
        void draw (void)
        {
            drawBasic2dCircularVehicle (*this, Color(0.0f,1.0f,0.0f));
            drawTrail ();
        }

        AABBox *m_bbox;
    };

    class Player : public SimpleVehicle
    {
    public:

        // constructor
        Player (std::vector<Player*> others, std::vector<Player*> allplayers, Ball* ball, bool isTeamA, int id) : m_others(others), m_AllPlayers(allplayers), m_Ball(ball), b_ImTeamA(isTeamA), m_MyID(id) {reset ();}

        // reset state
        void reset (void)
        {
            SimpleVehicle::reset (); // reset the vehicle 
            setSpeed (0.0f);         // speed along Forward direction.
            setMaxForce (3000.7f);      // steering force is clipped to this magnitude
            setMaxSpeed (10);         // velocity is clipped to this magnitude

            // Place me on my part of the field, looking at oponnents goal
            setPosition(b_ImTeamA ? frandom01()*20 : -frandom01()*20, 0, (frandom01()-0.5f)*20);
            if(m_MyID < 9)
                {
                if(b_ImTeamA)
                    setPosition(playerPosition[m_MyID]);
                else
                    setPosition(Vec3(-playerPosition[m_MyID].x, playerPosition[m_MyID].y, playerPosition[m_MyID].z));
                }
            m_home = position();
            clearTrailHistory ();    // prevent long streaks due to teleportation 
            setTrailParameters (10, 60);
        }

        // per frame simulation update
        // (parameter names commented out to prevent compiler warning from "-W")
        void update (const float /*currentTime*/, const float elapsedTime)
        {
            // if I hit the ball, kick it.

            const float distToBall = Vec3::distance (position(), m_Ball->position());
            const float sumOfRadii = radius() + m_Ball->radius();
            if (distToBall < sumOfRadii)
                m_Ball->kick((m_Ball->position()-position())*50, elapsedTime);


            // otherwise consider avoiding collisions with others
            Vec3 collisionAvoidance = steerToAvoidNeighbors(1, (AVGroup&)m_AllPlayers);
            if(collisionAvoidance != Vec3::zero)
                applySteeringForce (collisionAvoidance, elapsedTime);
            else
                {
                float distHomeToBall = Vec3::distance (m_home, m_Ball->position());
                if( distHomeToBall < 12.0f)
                    {
                    // go for ball if I'm on the 'right' side of the ball
                        if( b_ImTeamA ? position().x > m_Ball->position().x : position().x < m_Ball->position().x)
                        {
                        Vec3 seekTarget = xxxsteerForSeek(m_Ball->position());
                        applySteeringForce (seekTarget, elapsedTime);
                        }
                    else
                        {
                        if( distHomeToBall < 12.0f)
                            {
                            float Z = m_Ball->position().z - position().z > 0 ? -1.0f : 1.0f;
                            Vec3 behindBall = m_Ball->position() + (b_ImTeamA ? Vec3(2.0f,0.0f,Z) : Vec3(-2.0f,0.0f,Z));
                            Vec3 behindBallForce = xxxsteerForSeek(behindBall);
                            annotationLine (position(), behindBall , Color(0.0f,1.0f,0.0f));
                            Vec3 evadeTarget = xxxsteerForFlee(m_Ball->position());
                            applySteeringForce (behindBallForce*10.0f + evadeTarget, elapsedTime);
                            }
                        }
                    }
                else	// Go home
                    {
                    Vec3 seekTarget = xxxsteerForSeek(m_home);
                    Vec3 seekHome = xxxsteerForSeek(m_home);
                    applySteeringForce (seekTarget+seekHome, elapsedTime);
                    }

                }
        }

        // draw this character/vehicle into the scene
        void draw (void)
        {
            drawBasic2dCircularVehicle (*this, b_ImTeamA ? Color(1.0f,0.0f,0.0f):Color(0.0f,0.0f,1.0f));
            drawTrail ();
        }
        // per-instance reference to its group
        const std::vector<Player*>	m_others;
        const std::vector<Player*>	m_AllPlayers;
        Ball*	m_Ball;
        bool	b_ImTeamA;
        int		m_MyID;
        Vec3		m_home;
    };



    // ----------------------------------------------------------------------------
    // PlugIn for OpenSteerDemo
    class MicTestPlugIn : public PlugIn
    {
    public:
        
        const char* name (void) {return "Michael's Simple Soccer";}

        // float selectionOrderSortKey (void) {return 0.06f;}

        // bool requestInitialSelection() { return true;}

        // be more "nice" to avoid a compiler warning
        virtual ~MicTestPlugIn() {}

        void open (void)
        {
            // Make a field
            m_bbox = new AABBox(Vec3(-20,0,-10), Vec3(20,0,10));
            // Red goal
            m_TeamAGoal = new AABBox(Vec3(-21,0,-7), Vec3(-19,0,7));
            // Blue Goal
            m_TeamBGoal = new AABBox(Vec3(19,0,-7), Vec3(21,0,7));
            // Make a ball
            m_Ball = new Ball(m_bbox);
            // Build team A
            m_PlayerCountA = 8;
            for(unsigned int i=0; i < m_PlayerCountA ; ++i)
            {
                Player *pMicTest = new Player(TeamA, m_AllPlayers, m_Ball, true, i);
                OpenSteerDemo::selectedVehicle = pMicTest;
                TeamA.push_back (pMicTest);
                m_AllPlayers.push_back(pMicTest);
            }
            // Build Team B
            m_PlayerCountB = 8;
            for(unsigned int i=0; i < m_PlayerCountB ; ++i)
            {
                Player *pMicTest = new Player(TeamB, m_AllPlayers, m_Ball, false, i);
                OpenSteerDemo::selectedVehicle = pMicTest;
                TeamB.push_back (pMicTest);
                m_AllPlayers.push_back(pMicTest);
            }
            // initialize camera
            OpenSteerDemo::init2dCamera (*m_Ball);
            OpenSteerDemo::camera.setPosition (10, OpenSteerDemo::camera2dElevation, 10);
            OpenSteerDemo::camera.fixedPosition.set (40, 40, 40);
            OpenSteerDemo::camera.mode = Camera::cmFixed;
            m_redScore = 0;
            m_blueScore = 0;
        }

        void update (const float currentTime, const float elapsedTime)
        {
            // update simulation of test vehicle
            for(unsigned int i=0; i < m_PlayerCountA ; ++i)
                TeamA[i]->update (currentTime, elapsedTime);
            for(unsigned int i=0; i < m_PlayerCountB ; ++i)
                TeamB[i]->update (currentTime, elapsedTime);
            m_Ball->update(currentTime, elapsedTime);

            if(m_TeamAGoal->InsideX(m_Ball->position()) && m_TeamAGoal->InsideZ(m_Ball->position()))
            {
                m_Ball->reset();	// Ball in blue teams goal, red scores
                m_redScore++;
            }
            if(m_TeamBGoal->InsideX(m_Ball->position()) && m_TeamBGoal->InsideZ(m_Ball->position()))
            {
                m_Ball->reset();	// Ball in red teams goal, blue scores
                    m_blueScore++;
            }

        }

        void redraw (const float currentTime, const float elapsedTime)
        {
            // draw test vehicle
            for(unsigned int i=0; i < m_PlayerCountA ; ++i)
                TeamA[i]->draw ();
            for(unsigned int i=0; i < m_PlayerCountB ; ++i)
                TeamB[i]->draw ();
            m_Ball->draw();
            m_bbox->draw();
            m_TeamAGoal->draw();
            m_TeamBGoal->draw();
            {
                std::ostringstream annote;
                annote << "Red: "<< m_redScore;
                draw2dTextAt3dLocation (annote, Vec3(23,0,0), Color(1.0f,0.7f,0.7f), drawGetWindowWidth(), drawGetWindowHeight());
            }
            {
                std::ostringstream annote;
                annote << "Blue: "<< m_blueScore;
                draw2dTextAt3dLocation (annote, Vec3(-23,0,0), Color(0.7f,0.7f,1.0f), drawGetWindowWidth(), drawGetWindowHeight());
            }

            // textual annotation (following the test vehicle's screen position)
    if(0)
        for(unsigned int i=0; i < m_PlayerCountA ; ++i)
            {
                std::ostringstream annote;
                annote << std::setprecision (2) << std::setiosflags (std::ios::fixed);
                annote << "      speed: " << TeamA[i]->speed() << "ID:" << i << std::ends;
                draw2dTextAt3dLocation (annote, TeamA[i]->position(), gRed, drawGetWindowWidth(), drawGetWindowHeight());
                draw2dTextAt3dLocation (*"start", Vec3::zero, gGreen, drawGetWindowWidth(), drawGetWindowHeight());
            }
            // update camera, tracking test vehicle
            OpenSteerDemo::updateCamera (currentTime, elapsedTime, *OpenSteerDemo::selectedVehicle);

            // draw "ground plane"
            OpenSteerDemo::gridUtility (Vec3(0,0,0));
        }

        void close (void)
        {
            for(unsigned int i=0; i < m_PlayerCountA ; ++i)
                delete TeamA[i];
            TeamA.clear ();
            for(unsigned int i=0; i < m_PlayerCountB ; ++i)
                delete TeamB[i];
            TeamB.clear ();
                    m_AllPlayers.clear();
        }

        void reset (void)
        {
            // reset vehicle
            for(unsigned int i=0; i < m_PlayerCountA ; ++i)
                TeamA[i]->reset ();
            for(unsigned int i=0; i < m_PlayerCountB ; ++i)
                TeamB[i]->reset ();
            m_Ball->reset();
        }

        const AVGroup& allVehicles (void) {return (const AVGroup&) TeamA;}

        unsigned int	m_PlayerCountA;
        unsigned int	m_PlayerCountB;
        std::vector<Player*> TeamA;
        std::vector<Player*> TeamB;
        std::vector<Player*> m_AllPlayers;

        Ball	*m_Ball;
        AABBox	*m_bbox;
        AABBox	*m_TeamAGoal;
        AABBox	*m_TeamBGoal;
        int junk;
        int		m_redScore;
        int		m_blueScore;
    };


    MicTestPlugIn pMicTestPlugIn;




// ----------------------------------------------------------------------------

} // anonymous namespace
