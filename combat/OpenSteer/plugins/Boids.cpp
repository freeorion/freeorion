// ----------------------------------------------------------------------------
//
//
// OpenSteer -- Steering Behaviors for Autonomous Characters
//
// Copyright (c) 2002-2005, Sony Computer Entertainment America
// Original author: Craig Reynolds <craig_reynolds@playstation.sony.com>
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
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//
// ----------------------------------------------------------------------------
//
//
// OpenSteer Boids
// 
// 09-26-02 cwr: created 
//
//
// ----------------------------------------------------------------------------

#define private public
#include "PathingEngine.h"
#include "CombatFighter.h"
#include "CombatShip.h"
#undef private

#include "OpenSteer/SimpleVehicle.h"
#include "OpenSteer/OpenSteerDemo.h"

#include <list>
#include <map>
#include <sstream>

#ifdef WIN32
// Windows defines these as macros :(
#undef min
#undef max
#endif


namespace {

    // Include names declared in the OpenSteer namespace into the
    // namespaces to search to find names.
    using namespace OpenSteer;

    const float WORLD_RADIUS = 100.0f;

    // ----------------------------------------------------------------------------
    class BoidsPlugIn : public PlugIn
    {
    public:
        
        const char* name () {return "Boids";}

        float selectionOrderSortKey () {return 0.03f;}

        virtual ~BoidsPlugIn() {} // be more "nice" to avoid a compiler warning

        void open ()
            {
                // make the database used to accelerate proximity queries
                cyclePD = -1;
                nextPD ();

                // make default-sized flock
                population = 0;
                for (int i = 0; i < 200 / CombatFighter::FORMATION_SIZE; ++i)
                    addFlightToFlock();

                const float LARGE_SHIP_LENGTH = 15.0;
                CombatShipPtr ship(new CombatShip(0, Vec3(), 0.0, m_pathing_engine));
                ship->setRadius(LARGE_SHIP_LENGTH / 2.0);
                m_pathing_engine.AddObject(ship);

                BO* ship_rect = new BO;
                ship_rect->depth = LARGE_SHIP_LENGTH;
                ship_rect->width = LARGE_SHIP_LENGTH / 2.0;
                ship_rect->height = LARGE_SHIP_LENGTH / 2.0;
                ship_rect->setSeenFrom (Obstacle::outside);
                ship_rect->setForward (ship->forward());
                ship_rect->setSide (ship->side());
                ship_rect->setUp (ship->up());
                //m_pathing_engine.AddObstacle(ship_rect);

                initObstacles();
                m_pathing_engine.AddObstacle(insideBigSphere);

                // initialize camera
                OpenSteerDemo::init3dCamera (*OpenSteerDemo::selectedVehicle);
                OpenSteerDemo::camera.mode = Camera::cmFixed;
                OpenSteerDemo::camera.fixedDistDistance = OpenSteerDemo::cameraTargetDistance;
                OpenSteerDemo::camera.fixedDistVOffset = 0;
                OpenSteerDemo::camera.lookdownDistance = 20;
                OpenSteerDemo::camera.aimLeadTime = 0.5;
                OpenSteerDemo::camera.povOffset.set (0, 0.5, -2);
            }

        void update (const float currentTime, const float elapsedTime)
            {
                m_pathing_engine.Update(currentTime, elapsedTime);
            }

        void redraw (const float currentTime, const float elapsedTime)
            {
                // selected vehicle (user can mouse click to select another)
                AbstractVehicle& selected = *OpenSteerDemo::selectedVehicle;

                // vehicle nearest mouse (to be highlighted)
                AbstractVehicle& nearMouse = *OpenSteerDemo::vehicleNearestToMouse ();

                if (CombatFighter* f = dynamic_cast<CombatFighter*>(OpenSteerDemo::selectedVehicle)) {
                    if (f->m_leader) {
                        const CombatFighter::Mission& mission = f->CurrentMission();
                        switch (mission.m_type) {
                        case CombatFighter::MOVE_TO:
                            drawLineAlpha(f->position(), mission.m_destination, Color(0.0, 0.0, 1.0), 0.5);
                            break;
                        case CombatFighter::ATTACK_THIS:
                            if (CombatObjectPtr target = mission.m_target.lock())
                                drawLineAlpha(f->position(), target->position(), Color(1.0, 0.0, 0.0), 0.5);
                            break;
                        case CombatFighter::DEFEND_THIS:
                            if (CombatObjectPtr target = mission.m_target.lock())
                                drawLineAlpha(f->position(), target->position(), Color(0.0, 1.0, 0.0), 0.5);
                            break;
                        default:
                            break;
                        }
                    }
                }

                // update camera
                OpenSteerDemo::updateCamera (currentTime, elapsedTime, selected);

                // draw each boid in flock
                for (std::set<CombatObjectPtr>::const_iterator i = m_pathing_engine.Objects().begin();
                     i != m_pathing_engine.Objects().end(); ++i) {
                    if (CombatFighterPtr f = boost::dynamic_pointer_cast<CombatFighter>(*i))
                        f->Draw();
                    else if (CombatShipPtr f = boost::dynamic_pointer_cast<CombatShip>(*i))
                        f->Draw();
                }

                // highlight vehicle nearest mouse
                OpenSteerDemo::drawCircleHighlightOnVehicle (nearMouse, 1, gGray70);

                // highlight selected vehicle
                OpenSteerDemo::drawCircleHighlightOnVehicle (selected, 1, gGray50);

                // display status in the upper left corner of the window
                std::ostringstream status;
                status << "[F1/F2] " << population << " formations";
                status << "\n[F3]    PD type: ";
                switch (cyclePD)
                {
                case 0: status << "LQ bin lattice"; break;
                case 1: status << "brute force";    break;
                }
                status << "\n[F4]    Obstacles: ";
                switch (constraint)
                {
                case none:
                    status << "none (wrap-around at sphere boundary)" ; break;
                case insideSphere:
                    status << "inside a sphere" ; break;
                case outsideSphere:
                    status << "inside a sphere, outside another" ; break;
                case outsideSpheres:
                    status << "inside a sphere, outside several" ; break;
                case outsideSpheresNoBig:
                    status << "outside several spheres, with wrap-around" ; break;
                case rectangle:
                    status << "inside a sphere, with a rectangle" ; break;
                case rectangleNoBig:
                    status << "a rectangle, with wrap-around" ; break;
                case outsideBox:
                    status << "inside a sphere, outside a box" ; break;
                case insideBox:
                    status << "inside a box" ; break;
                }
                status << std::endl;
                const float h = drawGetWindowHeight ();
                const Vec3 screenLocation (10, h-50, 0);
                draw2dTextAt2dLocation (status, screenLocation, gGray80, drawGetWindowWidth(), drawGetWindowHeight());

                drawObstacles ();
            }

        void close ()
            {
                // delete each member of the flock
                while (population > 0) removeFlightFromFlock ();
            }

        void reset ()
            {
                // reset camera position
                OpenSteerDemo::position3dCamera (*OpenSteerDemo::selectedVehicle);

                // make camera jump immediately to new position
                OpenSteerDemo::camera.doNotSmoothNextMove ();
            }

        // for purposes of demonstration, allow cycling through various
        // types of proximity databases.  this routine is called when the
        // OpenSteerDemo user pushes a function key.
        void nextPD ()
            {
            }

        void handleFunctionKeys (int keyNumber)
            {
                switch (keyNumber)
                {
                case 1:  addFlightToFlock ();       break;
                case 2:  removeFlightFromFlock ();  break;
                case 3:  nextPD ();                 break;
                case 4:  nextBoundaryCondition ();  break;
                case 5:  printLQbinStats ();        break;
                }
            }

        void printLQbinStats ()
            {
            }
     
        void printMiniHelpForFunctionKeys ()
            {
                std::ostringstream message;
                message << "Function keys handled by ";
                message << '"' << name() << '"' << ':' << std::ends;
                OpenSteerDemo::printMessage (message);
                OpenSteerDemo::printMessage ("  F1     add 5 boids to the flock.");
                OpenSteerDemo::printMessage ("  F2     remove 5 boids from the flock.");
                OpenSteerDemo::printMessage ("  F3     use next proximity database.");
                OpenSteerDemo::printMessage ("  F4     next flock boundary condition.");
                OpenSteerDemo::printMessage ("");
            }

        void addFlightToFlock ()
            {
                m_formations.push_back(m_pathing_engine.AddFighterFormation(CombatFighter::FORMATION_SIZE));
                ++population;
            }

        void removeFlightFromFlock ()
            {
                m_pathing_engine.RemoveFighterFormation(m_formations.back());
                m_formations.pop_back();
                --population;
            }

        // return an AVGroup containing each boid of the flock
        const AVGroup& allVehicles ()
            {
                static AVGroup retval;
                retval.clear();
                for (std::set<CombatObjectPtr>::const_iterator i = m_pathing_engine.Objects().begin();
                     i != m_pathing_engine.Objects().end(); ++i) {
                    retval.push_back(i->get());
                }
                return retval;
            }

        // keep track of current flock size
        int population;

        // which of the various proximity databases is currently in use
        int cyclePD;

        // --------------------------------------------------------
        // the rest of this plug-in supports the various obstacles:
        // --------------------------------------------------------

        // enumerate demos of various constraints on the flock
        enum ConstraintType {none, insideSphere,
                             outsideSphere, outsideSpheres, outsideSpheresNoBig,
                             rectangle, rectangleNoBig,
                             outsideBox, insideBox};

        ConstraintType constraint;

        // select next "boundary condition / constraint / obstacle"
        void nextBoundaryCondition ()
            {
                constraint = (ConstraintType) ((int) constraint + 1);
                updateObstacles ();
            }

        class SO : public SphereObstacle
        {void draw (const bool filled, const Color& color, const Vec3& vp) const
                {drawSphereObstacle (*this, 10.0f, filled, color, vp);}};

        class RO : public RectangleObstacle
        {void draw (const bool, const Color& color, const Vec3&) const
                {tempDrawRectangle (*this, color);}};

        class BO : public BoxObstacle
        {void draw (const bool, const Color& color, const Vec3&) const
                {tempDrawBox (*this, color);}};

        RO* bigRectangle;
        BO *outsideBigBox, *insideBigBox;
        SO *insideBigSphere, *outsideSphere0, *outsideSphere1, *outsideSphere2,
                                                   *outsideSphere3, *outsideSphere4, *outsideSphere5, *outsideSphere6;

        PathingEngine m_pathing_engine;
        std::vector<CombatFighterFormationPtr> m_formations;

        void initObstacles ()
            {
                bigRectangle = new RO;
                outsideBigBox = new BO;
                insideBigBox = new BO;
                insideBigSphere = new SO;
                outsideSphere0 = new SO;
                outsideSphere1 = new SO;
                outsideSphere2 = new SO;
                outsideSphere3 = new SO;
                outsideSphere4 = new SO;
                outsideSphere5 = new SO;
                outsideSphere6 = new SO;

                constraint = insideSphere;

                insideBigSphere->radius = WORLD_RADIUS;
                insideBigSphere->setSeenFrom (Obstacle::inside);

                outsideSphere0->radius = WORLD_RADIUS * 0.5f;

                const float r = WORLD_RADIUS * 0.33f;
                outsideSphere1->radius = r;
                outsideSphere2->radius = r;
                outsideSphere3->radius = r;
                outsideSphere4->radius = r;
                outsideSphere5->radius = r;
                outsideSphere6->radius = r;

                const float p = WORLD_RADIUS * 0.5f;
                const float m = -p;
                const float z = 0.0f;
                outsideSphere1->center.set (p, z, z);
                outsideSphere2->center.set (m, z, z);
                outsideSphere3->center.set (z, p, z);
                outsideSphere4->center.set (z, m, z);
                outsideSphere5->center.set (z, z, p);
                outsideSphere6->center.set (z, z, m);

                const Vec3 tiltF = Vec3 (1.0f, 1.0f, 0.0f).normalize ();
                const Vec3 tiltS (0.0f, 0.0f, 1.0f);
                const Vec3 tiltU = Vec3 (-1.0f, 1.0f, 0.0f).normalize ();

                bigRectangle->width = 50.0f;
                bigRectangle->height = 80.0f;
                bigRectangle->setSeenFrom (Obstacle::both);
                bigRectangle->setForward (tiltF);
                bigRectangle->setSide (tiltS);
                bigRectangle->setUp (tiltU);

                outsideBigBox->width = 50.0f;
                outsideBigBox->height = 80.0f;
                outsideBigBox->depth = 20.0f;
                outsideBigBox->setForward (tiltF);
                outsideBigBox->setSide (tiltS);
                outsideBigBox->setUp (tiltU);

                insideBigBox = outsideBigBox;
                insideBigBox->setSeenFrom (Obstacle::inside);

                updateObstacles ();
            }


        // update obstacles list when constraint changes
        void updateObstacles ()
            {
#if 0
                // first clear out obstacle list
                m_pathing_engine.ClearObstacles();

                // add back obstacles based on mode
                switch (constraint)
                {
                default:
                    // reset for wrap-around, fall through to first case:
                    constraint = none;
                case none:
                    break;
                case insideSphere:
                    m_pathing_engine.AddObstacle(insideBigSphere);
                    break;
                case outsideSphere:
                    m_pathing_engine.AddObstacle(insideBigSphere);
                    m_pathing_engine.AddObstacle(outsideSphere0);
                    break;
                case outsideSpheres:
                    m_pathing_engine.AddObstacle(insideBigSphere);
                case outsideSpheresNoBig:
                    m_pathing_engine.AddObstacle(outsideSphere1);
                    m_pathing_engine.AddObstacle(outsideSphere2);
                    m_pathing_engine.AddObstacle(outsideSphere3);
                    m_pathing_engine.AddObstacle(outsideSphere4);
                    m_pathing_engine.AddObstacle(outsideSphere5);
                    m_pathing_engine.AddObstacle(outsideSphere6);
                    break;
                case rectangle:
                    m_pathing_engine.AddObstacle(insideBigSphere);
                    m_pathing_engine.AddObstacle(bigRectangle);
                case rectangleNoBig:
                    m_pathing_engine.AddObstacle(bigRectangle);
                    break;
                case outsideBox:
                    m_pathing_engine.AddObstacle(insideBigSphere);
                    m_pathing_engine.AddObstacle(outsideBigBox);
                    break;
                case insideBox:
                    m_pathing_engine.AddObstacle(insideBigBox);
                    break;
                }
#endif
            }


        void drawObstacles ()
            {
                for (PathingEngine::ObstacleVec::const_iterator o = m_pathing_engine.Obstacles().begin();
                     o != m_pathing_engine.Obstacles().end();
                     ++o)
                {
                    o->draw (false, // draw in wireframe
                             ((&*o == insideBigSphere) ?
                              Color (0.2f, 0.2f, 0.4f) :
                              Color (0.1f, 0.1f, 0.2f)),
                             OpenSteerDemo::camera.position ());
                }
            }


        static void tempDrawRectangle (const RectangleObstacle& rect, const Color& color)
            {
                float w = rect.width / 2;
                float h = rect.height / 2;

                Vec3 v1 = rect.globalizePosition (Vec3 ( w,  h, 0));
                Vec3 v2 = rect.globalizePosition (Vec3 (-w,  h, 0));
                Vec3 v3 = rect.globalizePosition (Vec3 (-w, -h, 0));
                Vec3 v4 = rect.globalizePosition (Vec3 ( w, -h, 0));

                drawLine (v1, v2, color);
                drawLine (v2, v3, color);
                drawLine (v3, v4, color);
                drawLine (v4, v1, color);
            }


        static void tempDrawBox (const BoxObstacle& box, const Color& color)
            {
                const float w = box.width / 2;
                const float h = box.height / 2;
                const float d = box.depth / 2;
                const Vec3 p = box.position ();
                const Vec3 s = box.side ();
                const Vec3 u = box.up ();
                const Vec3 f = box.forward ();

                const Vec3 v1 = box.globalizePosition (Vec3 ( w,  h,  d));
                const Vec3 v2 = box.globalizePosition (Vec3 (-w,  h,  d));
                const Vec3 v3 = box.globalizePosition (Vec3 (-w, -h,  d));
                const Vec3 v4 = box.globalizePosition (Vec3 ( w, -h,  d));

                const Vec3 v5 = box.globalizePosition (Vec3 ( w,  h, -d));
                const Vec3 v6 = box.globalizePosition (Vec3 (-w,  h, -d));
                const Vec3 v7 = box.globalizePosition (Vec3 (-w, -h, -d));
                const Vec3 v8 = box.globalizePosition (Vec3 ( w, -h, -d));

                drawLine (v1, v2, color);
                drawLine (v2, v3, color);
                drawLine (v3, v4, color);
                drawLine (v4, v1, color);

                drawLine (v5, v6, color);
                drawLine (v6, v7, color);
                drawLine (v7, v8, color);
                drawLine (v8, v5, color);

                drawLine (v1, v5, color);
                drawLine (v2, v6, color);
                drawLine (v3, v7, color);
                drawLine (v4, v8, color);
            }
    };

    BoidsPlugIn gBoidsPlugIn;

    // ----------------------------------------------------------------------------

} // anonymous namespace
