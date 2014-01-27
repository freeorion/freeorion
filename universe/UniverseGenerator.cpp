#include "UniverseGenerator.h"

#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../util/Random.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/MultiplayerCommon.h"
#include "../parse/Parse.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include "Building.h"
#include "Fleet.h"
#include "Planet.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "System.h"
#include "Field.h"
#include "UniverseObject.h"
#include "Effect.h"
#include "Predicates.h"
#include "Special.h"
#include "Species.h"
#include "ValueRef.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem/fstream.hpp>


DataTableMap& UniverseDataTables() {
    static DataTableMap map;
    if (map.empty())
        LoadDataTables((GetResourceDir() / "universe_tables.txt").string(), map);
    return map;
}

//////////////////////////////////////////
//  Universe Setup Functions            //
//////////////////////////////////////////

namespace {
    const double        PI                          = 3.141592653589793;
    SmallIntDistType    g_hundred_dist              = SmallIntDist(1, 100);         // a linear distribution [1, 100] used in most universe generation
    const int           MAX_ATTEMPTS_PLACE_SYSTEM   = 100;

    double CalcNewPosNearestNeighbour(double x, double y, const std::vector<SystemPosition>& positions)
    {
        if (positions.size() == 0) // means that we are placing the first star, return a max val to not reject placement
            return 1e6;

        unsigned int j;
        double lowest_dist=  (positions[0].x  - x ) * (positions[0].x  - x ) 
            + (positions[0].y - y) * (positions[0].y - y),distance=0.0;

        for (j = 1; j < positions.size(); ++j) {
            distance =  (positions[j].x  - x ) * (positions[j].x  - x ) 
                + (positions[j].y - y) * (positions[j].y - y);
            if(lowest_dist>distance)
                lowest_dist = distance;
        }
        return lowest_dist;
    }
}

double CalcTypicalUniverseWidth(int size)
{ return (1000.0 / std::sqrt(150.0)) * std::sqrt(static_cast<double>(size)); }

void SpiralGalaxyCalcPositions(std::vector<SystemPosition>& positions,
                               unsigned int arms, unsigned int stars, double width, double height)
{
    double arm_offset     = RandDouble(0.0,2.0*PI);
    double arm_angle      = 2.0*PI / arms;
    double arm_spread     = 0.3 * PI / arms;
    double arm_length     = 1.5 * PI;
    double center         = 0.25;
    double x,y;
    
    int i, attempts;
    
    GaussianDistType  random_gaussian = GaussianDist(0.0,arm_spread);
    SmallIntDistType  random_arm      = SmallIntDist(0  ,arms);
    DoubleDistType    random_angle    = DoubleDist  (0.0,2.0*PI);
    DoubleDistType    random_radius   = DoubleDist  (0.0,  1.0);
    
    for (i = 0, attempts = 0; i < static_cast<int>(stars) && attempts < MAX_ATTEMPTS_PLACE_SYSTEM; ++i, ++attempts) {
        double radius = random_radius();
        
        if (radius < center) {
            double angle = random_angle();
            x = radius * cos( arm_offset + angle );
            y = radius * sin( arm_offset + angle );
        } else {
            double arm    = static_cast<double>(random_arm()) * arm_angle;
            double angle  = random_gaussian();
            
            x = radius * cos( arm_offset + arm + angle + radius * arm_length );
            y = radius * sin( arm_offset + arm + angle + radius * arm_length );
        }
        
        x = (x + 1) * width / 2.0;
        y = (y + 1) * height / 2.0;
        
        if (x < 0 || width <= x || y < 0 || height <= y)
            continue;
        
        // See if new star is too close to any existing star.
        double lowest_dist = CalcNewPosNearestNeighbour(x, y, positions);
        
        // If so, we try again or give up.
        if (lowest_dist < MIN_SYSTEM_SEPARATION * MIN_SYSTEM_SEPARATION) {
            if (attempts < MAX_ATTEMPTS_PLACE_SYSTEM - 1) {
                --i; //try again for same star
            } else {
                attempts=0; //give up on this star, move on to next
                Logger().debugStream() << "Giving up on placing star "<< i <<" with lowest dist squared " << lowest_dist;
            }
            continue;
        }
        
        // Add the new star location.
        positions.push_back(SystemPosition(x, y));
        
        // Note that attempts is reset for every star.
        attempts = 0;
    }
}

void EllipticalGalaxyCalcPositions(std::vector<SystemPosition>& positions,
                                   unsigned int stars, double width, double height)
{
    const double ellipse_width_vs_height = RandDouble(0.4, 0.6);
    const double rotation = RandDouble(0.0, PI),
    rotation_sin = std::sin(rotation),
    rotation_cos = std::cos(rotation);
    const double gap_constant = .95;
    const double gap_size = 1.0 - gap_constant * gap_constant * gap_constant;
    
    // Random number generators.
    DoubleDistType radius_dist = DoubleDist(0.0, gap_constant);
    DoubleDistType random_angle  = DoubleDist(0.0, 2.0 * PI);
    
    // Used to give up when failing to place a star too often.
    int attempts = 0;
    
    // For each attempt to place a star...
    for (unsigned int i = 0; i < stars && attempts < MAX_ATTEMPTS_PLACE_SYSTEM; ++i, ++attempts){
        double radius = radius_dist();
        // Adjust for bigger density near center and create gap.
        radius = radius * radius * radius + gap_size;
        double angle  = random_angle();
        
        // Rotate for individual angle and apply elliptical shape.
        double x1 = radius * std::cos(angle);
        double y1 = radius * std::sin(angle) * ellipse_width_vs_height;
        
        // Rotate for ellipse angle.
        double x = x1 * rotation_cos - y1 * rotation_sin;
        double y = x1 * rotation_sin + y1 * rotation_cos;
        
        // Move from [-1.0, 1.0] universe coordinates.
        x = (x + 1.0) * width / 2.0;
        y = (y + 1.0) * height / 2.0;
        
        // Discard stars that are outside boundaries (due to possible rounding errors).
        if (x < 0 || x >= width || y < 0 || y >= height)
            continue;
        
        // See if new star is too close to any existing star.
        double lowest_dist = CalcNewPosNearestNeighbour(x, y, positions);
        
        // If so, we try again or give up.
        if (lowest_dist < MIN_SYSTEM_SEPARATION * MIN_SYSTEM_SEPARATION) {
            if (attempts < MAX_ATTEMPTS_PLACE_SYSTEM - 1) {
                --i; //try again for same star
            } else {
                attempts=0; //give up on this star, move on to next
                Logger().debugStream() << "Giving up on placing star "<< i <<" with lowest dist squared " << lowest_dist;
            }
            continue;
        }
        
        // Add the new star location.
        positions.push_back(SystemPosition(x, y));
        
        // Note that attempts is reset for every star.
        attempts = 0;
    }
}

void ClusterGalaxyCalcPositions(std::vector<SystemPosition>& positions, unsigned int clusters,
                                unsigned int stars, double width, double height)
{
    if (stars < 1) {
        Logger().errorStream() << "ClusterGalaxyCalcPositions requested for 0 stars";
        return;
    }
    if (clusters < 1) {
        Logger().errorStream() << "ClusterGalaxyCalcPositions requested for 0 clusters. defaulting to 1";
        clusters = 1;
    }
    
    
    // probability of systems which don't belong to a cluster
    const double system_noise = 0.15;
    double ellipse_width_vs_height = RandDouble(0.2,0.5);
    // first innermost pair hold cluster position, second innermost pair stores help values for cluster rotation (sin,cos)
    std::vector<std::pair<std::pair<double, double>, std::pair<double, double> > > clusters_position;
    unsigned int i,j,attempts;
    
    DoubleDistType random_zero_to_one = DoubleDist(0.0, 1.0);
    DoubleDistType random_angle = DoubleDist(0.0, 2.0*PI);
    
    for (i = 0, attempts = 0;
         i < clusters && static_cast<int>(attempts) < MAX_ATTEMPTS_PLACE_SYSTEM;
         i++, attempts++)
    {
        // prevent cluster position near borders (and on border)
        double x = ((random_zero_to_one()*2.0 - 1.0) / (clusters + 1.0))*clusters;
        double y = ((random_zero_to_one()*2.0 - 1.0) / (clusters + 1.0))*clusters;
        
        
        // ensure all clusters have a min separation to each other (search isn't opimized, not worth the effort)
        for (j = 0; j < clusters_position.size(); j++) {
            if ((clusters_position[j].first.first - x)*(clusters_position[j].first.first - x)+ (clusters_position[j].first.second - y)*(clusters_position[j].first.second - y)
                < (2.0/clusters))
                break;
        }
        if (j < clusters_position.size()) {
            i--;
            continue;
        }
        
        attempts = 0;
        double rotation = RandDouble(0.0,PI);
        clusters_position.push_back(std::pair<std::pair<double,double>,std::pair<double,double> >(std::pair<double,double>(x,y),std::pair<double,double>(sin(rotation),cos(rotation))));
    }
    
    for (i = 0, attempts = 0; i < stars && attempts<100; i++, attempts++) {
        double x,y;
        if (random_zero_to_one() < system_noise) {
            x = random_zero_to_one() * 2.0 - 1.0;
            y = random_zero_to_one() * 2.0 - 1.0;
        } else {
            short  cluster = i % clusters_position.size();
            double radius  = random_zero_to_one();
            double angle   = random_angle();
            double x1,y1;
            
            x1 = radius * cos(angle);
            y1 = radius * sin(angle)*ellipse_width_vs_height;
            
            x = x1*clusters_position[cluster].second.second + y1*clusters_position[cluster].second.first;
            y =-x1*clusters_position[cluster].second.first  + y1*clusters_position[cluster].second.second;
            
            x = x/sqrt((double)clusters) + clusters_position[cluster].first.first;
            y = y/sqrt((double)clusters) + clusters_position[cluster].first.second;
        }
        x = (x+1)*width /2.0;
        y = (y+1)*height/2.0;
        
        if (x<0 || width<=x || y<0 || height<=y)
            continue;
        
        // See if new star is too close to any existing star.
        double lowest_dist = CalcNewPosNearestNeighbour(x, y, positions);
        
        // If so, we try again or give up.
        if (lowest_dist < MIN_SYSTEM_SEPARATION * MIN_SYSTEM_SEPARATION) {
            if (attempts < MAX_ATTEMPTS_PLACE_SYSTEM - 1) {
                --i; //try again for same star
            } else {
                attempts=0; //give up on this star, move on to next
                Logger().debugStream() << "Giving up on placing star "<< i <<" with lowest dist squared " << lowest_dist;
            }
            continue;
        }
        
        // Add the new star location.
        positions.push_back(SystemPosition(x, y));
        
        // Note that attempts is reset for every star.
        attempts = 0;
    }
}

void RingGalaxyCalcPositions(std::vector<SystemPosition>& positions, unsigned int stars,
                             double width, double height)
{
    double RING_WIDTH = width / 4.0;
    double RING_RADIUS = (width - RING_WIDTH) / 2.0;
    
    DoubleDistType   theta_dist = DoubleDist(0.0, 2.0 * PI);
    GaussianDistType radius_dist = GaussianDist(RING_RADIUS, RING_WIDTH / 3.0);
    
    for (unsigned int i = 0, attempts = 0; i < stars && static_cast<int>(attempts) < MAX_ATTEMPTS_PLACE_SYSTEM; ++i, ++attempts) {
        double theta = theta_dist();
        double radius = radius_dist();
        
        double x = width / 2.0 + radius * std::cos(theta);
        double y = height / 2.0 + radius * std::sin(theta);
        
        if (x < 0 || width <= x || y < 0 || height <= y)
            continue;
        
        // See if new star is too close to any existing star.
        double lowest_dist=CalcNewPosNearestNeighbour(x, y,positions);
        
        // If so, we try again or give up.
        if (lowest_dist < MIN_SYSTEM_SEPARATION * MIN_SYSTEM_SEPARATION) {
            if (attempts < MAX_ATTEMPTS_PLACE_SYSTEM - 1) {
                --i; //try again for same star
            } else {
                attempts=0; //give up on this star, move on to next
                Logger().debugStream() << "Giving up on placing star "<< i <<" with lowest dist squared " << lowest_dist;
            }
            continue;
        }
        
        // Add the new star location.
        positions.push_back(SystemPosition(x, y));
        
        // Note that attempts is reset for every star.
        attempts = 0;
    }
}

void IrregularGalaxyPositions(std::vector<SystemPosition>& positions, unsigned int stars,
                              double width, double height)
{
    Logger().debugStream() << "IrregularGalaxyPositions";

    unsigned int positions_placed = 0;
    for (unsigned int i = 0, attempts = 0; i < stars && static_cast<int>(attempts) < MAX_ATTEMPTS_PLACE_SYSTEM; ++i, ++attempts) {

        double x = width * RandZeroToOne();
        double y = height * RandZeroToOne();

        Logger().debugStream() << "... potential position: (" << x << ", " << y << ")";

        // reject positions outside of galaxy: minimum 0, maximum height or width.  shouldn't be a problem,
        // but I'm copying this from one of the other generation functions and figure it might as well remain
        if (x < 0 || width <= x || y < 0 || height <= y)
            continue;

        // See if new star is too close to any existing star.
        double lowest_dist = CalcNewPosNearestNeighbour(x, y, positions);

        // If so, we try again or give up.
        if (lowest_dist < MIN_SYSTEM_SEPARATION * MIN_SYSTEM_SEPARATION) {
            if (attempts < MAX_ATTEMPTS_PLACE_SYSTEM - 1) {
                --i; //try again for same star
            } else {
                attempts=0; //give up on this star, move on to next
                Logger().debugStream() << "Giving up on placing star "<< i <<" with lowest dist squared " << lowest_dist;
            }
            continue;
        }

        // Add the new star location.
        positions.push_back(SystemPosition(x, y));

        Logger().debugStream() << "... added system at (" << x << ", " << y << ") after " << attempts << " attempts";

        positions_placed++;

        // Note that attempts is reset for every star.
        attempts = 0;
    }
    Logger().debugStream() << "... placed " << positions_placed << " systems";
}


namespace Delauney {
    /** simple 2D point.  would have used array of systems, but System
      * class has limits on the range of positions that would interfere
      * with the triangulation algorithm (need a single large covering
      * triangle that overlaps all actual points being triangulated) */
    class DTPoint {
    public:
        DTPoint() :
            x(0.0),
            y(0.0)
        {}
        DTPoint(double x_, double y_) :
            x(x_),
            y(y_)
        {}
        double  x;
        double  y;
    };

    /* simple class for an integer that has an associated "sorting value",
     * so the integer can be stored in a list sorted by something other than
     * the value of the integer */
    class SortValInt {
    public:
        SortValInt(int num_, double sort_val_) :
            num(num_),
            sortVal(sort_val_)
        {}
        int     num;
        double  sortVal;
    };


    /** list of three interger array indices, and some additional info about
      * the triangle that the corresponding points make up, such as the
      * circumcentre and radius, and a function to find if another point is in
      * the circumcircle */
    class DTTriangle {
    public:
        DTTriangle();
        DTTriangle(int vert1, int vert2, int vert3, std::vector<Delauney::DTPoint> &points);

        bool                    PointInCircumCircle(Delauney::DTPoint &p);  ///< determines whether a specified point is within the circumcircle of the triangle
        const std::vector<int>& Verts() {return verts;}

    private:
        std::vector<int>    verts;      ///< indices of vertices of triangle
        Delauney::DTPoint   centre;     ///< location of circumcentre of triangle
        double              radius2;    ///< radius of circumcircle squared
    };

    DTTriangle::DTTriangle(int vert1, int vert2, int vert3, std::vector<Delauney::DTPoint>& points) {
        double a, Sx, Sy, b;
        double x1, x2, x3, y1, y2, y3;

        if (vert1 == vert2 || vert1 == vert3 || vert2 == vert3)
            throw std::runtime_error("Attempted to create Triangle with two of the same vertex indices.");

        verts = std::vector<int>(3);

        // record indices of vertices of triangle
        verts[0] = vert1;
        verts[1] = vert2;
        verts[2] = vert3;

        // extract position info for vertices
        x1 = points[vert1].x;
        x2 = points[vert2].x;
        x3 = points[vert3].x;
        y1 = points[vert1].y;
        y2 = points[vert2].y;
        y3 = points[vert3].y;

        // calculate circumcircle and circumcentre of triangle
        a = x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);

        Sx = 0.5 * ((x1 * x1 + y1 * y1) * (y2 - y3) +
                    (x2 * x2 + y2 * y2) * (y3 - y1) +
                    (x3 * x3 + y3 * y3) * (y1 - y2));

        Sy = -0.5* ((x1 * x1 + y1 * y1) * (x2 - x3) +
                    (x2 * x2 + y2 * y2) * (x3 - x1) +
                    (x3 * x3 + y3 * y3) * (x1 - x2));

        b =        ((x1 * x1 + y1 * y1) * (x2 * y3 - x3 * y2) +
                    (x2 * x2 + y2 * y2) * (x3 * y1 - x1 * y3) +
                    (x3 * x3 + y3 * y3) * (x1 * y2 - x2 * y1));

        // make sure nothing funky's going on...
        if (std::abs(a) < 0.01)
            throw std::runtime_error("Attempted to find circumcircle for a triangle with vertices in a line.");

        // finish!
        centre.x = Sx / a;
        centre.y = Sy / a;
        radius2 = (Sx*Sx + Sy*Sy)/(a*a) + b/a;
    };

    DTTriangle::DTTriangle() :
        verts(3, 0),
        centre(0.0, 0.0),
        radius2(0.0)
    {};

    bool DTTriangle::PointInCircumCircle(Delauney::DTPoint &p) {
        double vectX, vectY;

        vectX = p.x - centre.x;
        vectY = p.y - centre.y;

        if (vectX*vectX + vectY*vectY < radius2)
            return true;
        return false;
    };



    /** runs a Delauney Triangulation routine on a set of 2D points extracted
      * from an array of systems returns the list of triangles produced */
    std::list<Delauney::DTTriangle>* DelauneyTriangulate(std::vector<TemporaryPtr<System> > &systems) {

        int n, c, theSize, num, num2; // loop counters, storage for retreived size of a vector, temp storage
        std::list<Delauney::DTTriangle>::iterator itCur, itEnd;
        std::list<Delauney::SortValInt>::iterator itCur2, itEnd2;
        // vector of x and y positions of stars
        std::vector<Delauney::DTPoint> points;
        // pointer to main list of triangles algorithm works with.
        std::list<Delauney::DTTriangle> *triList;
        // list of indices in vector of points extracted from removed triangles that need to be retriangulated
        std::list<Delauney::SortValInt> pointNumList;
        double vx, vy, mag;  // vector components, magnitude

        // ensure a useful list of systems was passed...
        if (systems.empty())
            throw std::runtime_error("Attempted to run Delauney Triangulation on empty array of systems");

        // extract systems positions, and store in vector.  Can't use actual systems data since
        // systems have position limitations which would interfere with algorithm
        theSize = static_cast<int>(systems.size());
        for (n = 0; n < theSize; n++) {
            points.push_back(Delauney::DTPoint(systems[n]->X(), systems[n]->Y()));
        }

        // add points for covering triangle.  the point positions should be big enough to form a triangle
        // that encloses all the systems of the galaxy (or at least one whose circumcircle covers all points)
        points.push_back(Delauney::DTPoint(-1.0, -1.0));
        points.push_back(Delauney::DTPoint(2.0 * (GetUniverse().UniverseWidth() + 1.0), -1.0));
        points.push_back(Delauney::DTPoint(-1.0, 2.0 * (GetUniverse().UniverseWidth() + 1.0)));

        // initialize triList.  algorithm adds and removes triangles from this list, and the resulting
        // list is returned (so should be deleted externally)
        triList = new std::list<Delauney::DTTriangle>;

        // add last three points into the first triangle, the "covering triangle"
        theSize = static_cast<int>(points.size());
        triList->push_front(Delauney::DTTriangle(theSize-1, theSize-2, theSize-3, points));

        // loop through "real" points (from systems, not the last three added to make the covering triangle)
        for (n = 0; n < theSize - 3; n++) {
            pointNumList.clear();

            // check each triangle in list, to see if the new point lies in its circumcircle.  if so, delete
            // the triangle and add its vertices to a list 
            itCur = triList->begin();
            itEnd = triList->end();
            while (itCur != itEnd) {
                // get current triangle
                Delauney::DTTriangle& tri = *itCur;

                // check if point to be added to triangulation is within the circumcircle for the current triangle
                if (tri.PointInCircumCircle(points[n])) {
                    // if so, insert the triangle's vertices' indices into the list.  add in sorted position
                    // based on angle of direction to current point n being inserted.  don't add if doing
                    // so would duplicate an index already in the list
                    for (c = 0; c < 3; c++) {
                        num = (tri.Verts())[c];  // store "current point"

                        // get sorting value to order points clockwise circumferentially around point n
                        // vector from point n to current point
                        vx = points[num].x - points[n].x;
                        vy = points[num].y - points[n].y;
                        mag = sqrt(vx*vx + vy*vy);
                        // normalize
                        vx /= mag;
                        vy /= mag;
                        // dot product with (0, 1) is vy, magnitude of cross product is vx
                        // this gives a range of "sortValue" from -2 to 2, around the circle
                        if (vx >= 0) mag = vy + 1; else mag = -vy - 1;
                        // sorting value in "mag"

                        // iterate through list, finding insert spot and verifying uniqueness (or add if list is empty)
                        itCur2 = pointNumList.begin();
                        itEnd2 = pointNumList.end();
                        if (itCur2 == itEnd2) {
                            // list is empty
                            pointNumList.push_back(Delauney::SortValInt(num, mag));
                        } else {
                            while (itCur2 != itEnd2) {
                                if (itCur2->num == num) 
                                    break;
                                if (itCur2->sortVal > mag) {
                                    pointNumList.insert(itCur2, Delauney::SortValInt(num, mag));
                                    break;
                                }
                                itCur2++;
                            }
                            if (itCur2 == itEnd2) {
                                // point wasn't added, so should go at end
                                pointNumList.push_back(Delauney::SortValInt(num, mag));
                            }
                        }
                    } // end for c

                    // remove current triangle from list of triangles
                    itCur = triList->erase(itCur);
                } else {
                    // point not in circumcircle for this triangle
                    // to go next triangle in list
                    ++itCur;
                }
            } // end while

            // go through list of points, making new triangles out of them
            itCur2 = pointNumList.begin();
            itEnd2 = pointNumList.end();
            assert(itCur2 != itEnd2);

            // add triangle for last and first points and n
            triList->push_front(Delauney::DTTriangle(n, (pointNumList.front()).num, (pointNumList.back()).num, points));

            num = itCur2->num;
            ++itCur2;
            while (itCur2 != itEnd2) {
                num2 = num;
                num = itCur2->num;

                triList->push_front(Delauney::DTTriangle(n, num2, num, points));

                ++itCur2;
            } // end while

        } // end for
        return triList;
    } // end function
}

////////////////////////////////////////
// FleetPlan                          //
////////////////////////////////////////
const std::string& FleetPlan::Name() const {
    if (m_name_in_stringtable)
        return UserString(m_name);
    else
        return m_name;
}

//////////////////////
// FleetPlanManager //
//////////////////////
namespace {
    class FleetPlanManager {
    public:
        typedef std::vector<FleetPlan*>::const_iterator iterator;
        virtual ~FleetPlanManager();
        iterator    begin() const   { return m_plans.begin(); }
        iterator    end() const     { return m_plans.end(); }
        /** returns the instance of this singleton class; you should use the
          * free function GetFleetPlanManager() instead */
        static const FleetPlanManager& GetFleetPlanManager();
    private:
        FleetPlanManager();
        std::vector<FleetPlan*>     m_plans;
        static FleetPlanManager*    s_instance;
    };
    // static(s)
    FleetPlanManager* FleetPlanManager::s_instance = 0;

    const FleetPlanManager& FleetPlanManager::GetFleetPlanManager() {
        static FleetPlanManager manager;
        return manager;
    }

    FleetPlanManager::FleetPlanManager() {
        if (s_instance)
            throw std::runtime_error("Attempted to create more than one FleetPlanManager.");

        s_instance = this;

        Logger().debugStream() << "Initializing FleetPlanManager";

        parse::fleet_plans(GetResourceDir() / "starting_fleets.txt", m_plans);

#ifdef OUTPUT_PLANS_LIST
        Logger().debugStream() << "Starting Fleet Plans:";
        for (iterator it = begin(); it != end(); ++it)
            Logger().debugStream() << " ... " << (*it)->Name();
#endif
    }

    FleetPlanManager::~FleetPlanManager() {
        for (std::vector<FleetPlan*>::iterator it = m_plans.begin(); it != m_plans.end(); ++it)
            delete *it;
        m_plans.clear();
    }

    /** returns the singleton fleet plan manager */
    const FleetPlanManager& GetFleetPlanManager()
    { return FleetPlanManager::GetFleetPlanManager(); }
};

/////////////////////////////
// MonsterFleetPlanManager //
/////////////////////////////
namespace {
    class MonsterFleetPlanManager {
    public:
        typedef std::vector<MonsterFleetPlan*>::const_iterator iterator;
        virtual ~MonsterFleetPlanManager();
        iterator    begin() const       { return m_plans.begin(); }
        iterator    end() const         { return m_plans.end(); }
        int         NumMonsters() const { return m_plans.size(); }
        /** returns the instance of this singleton class; you should use the
          * free function MonsterFleetPlanManager() instead */
        static const MonsterFleetPlanManager& GetMonsterFleetPlanManager();
    private:
        MonsterFleetPlanManager();
        std::vector<MonsterFleetPlan*>     m_plans;
        static MonsterFleetPlanManager*    s_instance;
    };
    // static(s)
    MonsterFleetPlanManager* MonsterFleetPlanManager::s_instance = 0;

    const MonsterFleetPlanManager& MonsterFleetPlanManager::GetMonsterFleetPlanManager() {
        static MonsterFleetPlanManager manager;
        return manager;
    }

    MonsterFleetPlanManager::MonsterFleetPlanManager() {
        if (s_instance)
            throw std::runtime_error("Attempted to create more than one MonsterFleetPlanManager.");

        s_instance = this;

        Logger().debugStream() << "Initializing MonsterFleetPlanManager";

        parse::monster_fleet_plans(GetResourceDir() / "space_monster_spawn_fleets.txt", m_plans);

//#ifdef OUTPUT_PLANS_LIST
        Logger().debugStream() << "Starting Monster Fleet Plans:";
        for (iterator it = begin(); it != end(); ++it)
            Logger().debugStream() << " ... " << (*it)->Name() << " spawn rate: " << (*it)->SpawnRate() << " spawn limit: " << (*it)->SpawnLimit();
//#endif
    }

    MonsterFleetPlanManager::~MonsterFleetPlanManager() {
        for (std::vector<MonsterFleetPlan*>::iterator it = m_plans.begin(); it != m_plans.end(); ++it)
            delete *it;
        m_plans.clear();
    }

    /** returns the singleton fleet plan manager */
    const MonsterFleetPlanManager& GetMonsterFleetPlanManager()
    { return MonsterFleetPlanManager::GetMonsterFleetPlanManager(); }
};

/////////////////////////////
// ItemSpecManager         //
/////////////////////////////
namespace {
    class ItemSpecManager {
    public:
        typedef std::vector<ItemSpec>::const_iterator iterator;
        iterator    begin() const   { return m_items.begin(); }
        iterator    end() const     { return m_items.end(); }
        /** Adds unlocked items in this manager to the specified \a empire
          * using that Empire's UnlockItem function. */
        void        AddItemsToEmpire(Empire* empire) const;
        /** returns the instance of this singleton class; you should use the
          * free function GetItemSpecManager() instead */
        static const ItemSpecManager& GetItemSpecManager();
    private:
        ItemSpecManager();
        std::vector<ItemSpec>   m_items;
        static ItemSpecManager* s_instance;
    };
    // static(s)
    ItemSpecManager* ItemSpecManager::s_instance = 0;

    const ItemSpecManager& ItemSpecManager::GetItemSpecManager() {
        static ItemSpecManager manager;
        return manager;
    }

    ItemSpecManager::ItemSpecManager() {
        if (s_instance)
            throw std::runtime_error("Attempted to create more than one ItemSpecManager.");

        s_instance = this;

        Logger().debugStream() << "Initializing ItemSpecManager";

        parse::items(GetResourceDir() / "preunlocked_items.txt", m_items);

#ifdef OUTPUT_ITEM_SPECS_LIST
        Logger().debugStream() << "Starting Unlocked Item Specs:";
        for (iterator it = begin(); it != end(); ++it) {
            const ItemSpec& item = *it;
            Logger().debugStream() << " ... " << boost::lexical_cast<std::string>(item.type) << " : " << item.name;
        }
#endif
    }

    void ItemSpecManager::AddItemsToEmpire(Empire* empire) const {
        if (!empire)
            return;
        for (iterator it = begin(); it != end(); ++it)
            empire->UnlockItem(*it);
    }

    /** returns the singleton item spec manager */
    const ItemSpecManager& GetItemSpecManager()
    { return ItemSpecManager::GetItemSpecManager(); }
};

namespace {
    /** Used by GenerateStarlanes.  Determines if two systems are connected by
      * maxLaneJumps or less edges on graph. */
    bool ConnectedWithin(int system1, int system2, int maxLaneJumps, std::vector<std::set<int> >& laneSetArray) {
        // list of indices of systems that are accessible from previously visited systems.
        // when a new system is found to be accessible, it is added to the back of the
        // list.  the list is iterated through from front to back to find systems
        // to examine
        std::list<int> accessibleSystemsList;
        std::list<int>::iterator sysListIter, sysListEnd;

        // map using star index number as the key, and also storing the number of starlane
        // jumps away from system1 a given system is.  this is used to determine if a
        // system has already been added to the accessibleSystemsList without needing
        // to iterate through the list.  it also provides some indication of the
        // current depth of the search, which allows the serch to terminate after searching
        // to the depth of maxLaneJumps without finding system2
        // (considered using a vector for this, but felt that for large galaxies, the
        // size of the vector and the time to intialize would be too much)
        std::map<int, int> accessibleSystemsMap;

        // system currently being investigated, destination of a starlane origination at curSys
        int curSys, curLaneDest;
        // "depth" level in tree of system currently being investigated
        int curDepth;

        // iterators to set of starlanes, in graph, for the current system    
        std::set<int>::iterator curSysLanesSetIter, curSysLanesSetEnd;

        // check for simple cases for quick termination
        if (system1 == system2) return true; // system is always connected to itself
        if (0 == maxLaneJumps) return false; // no system is connected to any other system by less than 1 jump
        if (0 == (laneSetArray[system1]).size()) return false; // no lanes out of start system
        if (0 == (laneSetArray[system2]).size()) return false; // no lanes into destination system
        if (system1 >= static_cast<int>(laneSetArray.size()) || system2 >= static_cast<int>(laneSetArray.size())) return false; // out of range
        if (system1 < 0 || system2 < 0) return false; // out of range

        // add starting system to list and set of accessible systems
        accessibleSystemsList.push_back(system1);
        accessibleSystemsMap.insert(std::pair<int, int>(system1, 0));

        // loop through visited systems
        sysListIter = accessibleSystemsList.begin();
        sysListEnd = accessibleSystemsList.end();
        while (sysListIter != sysListEnd) {
            curSys = *sysListIter;

            // check that iteration hasn't reached maxLaneJumps levels deep, which would 
            // mean that system2 isn't within maxLaneJumps starlane jumps of system1
            curDepth = (*accessibleSystemsMap.find(curSys)).second;

            if (curDepth >= maxLaneJumps) return false;

            // get set of starlanes for this system
            curSysLanesSetIter = (laneSetArray[curSys]).begin();
            curSysLanesSetEnd = (laneSetArray[curSys]).end();

            // add starlanes accessible from this system to list and set of accessible starlanes
            // (and check for the goal starlane)
            while (curSysLanesSetIter != curSysLanesSetEnd) {
                curLaneDest = *curSysLanesSetIter;

                // check if curLaneDest has been added to the map of accessible systems
                if (0 == accessibleSystemsMap.count(curLaneDest)) {

                    // check for goal
                    if (curLaneDest == system2) return true;

                    // add curLaneDest to accessible systems list and map
                    accessibleSystemsList.push_back(curLaneDest);
                    accessibleSystemsMap.insert(std::pair<int, int>(curLaneDest, curDepth + 1));
                   }

                curSysLanesSetIter++;
            }

            sysListIter++;
        }
        return false; // default
    }

    /** Removes lanes from passed graph that are angularly too close to
      * each other. */
    void CullAngularlyTooCloseLanes(double maxLaneUVectDotProd, std::vector<std::set<int> >& laneSetArray,
                                    std::vector<TemporaryPtr<System> > &systems)
    {
        // start and end systems of a new lane being considered, and end points of lanes that already exist with that
        // start at the start or destination of the new lane
        int curSys, dest1, dest2;

        // geometry stuff... points componenets, vector componenets dot product & magnitudes of vectors
        double startX, startY, vectX1, vectX2, vectY1, vectY2, dotProd, mag1, mag2;
        // 2 component vector and vect + magnitude typedefs

        typedef std::pair<double, double> VectTypeQQ;
        typedef std::pair<VectTypeQQ, double> VectAndMagTypeQQ;
        typedef std::pair<int, VectAndMagTypeQQ> MapInsertableTypeQQ;

        std::map<int, VectAndMagTypeQQ> laneVectsMap;  // componenets of vectors of lanes of current system, indexed by destination system number
        std::map<int, VectAndMagTypeQQ>::iterator laneVectsMapIter;

        VectTypeQQ tempVect;
        VectAndMagTypeQQ tempVectAndMag;

        // iterators to go through sets of lanes in array
        std::set<int>::iterator laneSetIter1, laneSetIter2, laneSetEnd;

        std::set<std::pair<int, int> > lanesToRemoveSet;  // start and end stars of lanes to be removed in final step...
        std::set<std::pair<int, int> >::iterator lanesToRemoveIter, lanesToRemoveEnd;
        std::pair<int, int> lane1, lane2;

        int curNumLanes;

        int numSys = systems.size();
        // make sure data is consistent
        if (static_cast<int>(laneSetArray.size()) != numSys) {
            Logger().errorStream() << "CullAngularlyTooCloseLanes got different size vectors of lane sets and systems.  Doing nothing.";
            return;
        }
        
        if (numSys < 3) return;  // nothing worth doing for less than three systems

        //Logger().debugStream() << "Culling Too Close Angularly Lanes";

        // loop through systems
        for (curSys = 0; curSys < numSys; curSys++) {
            // get position of current system (for use in calculated vectors)
            startX = systems[curSys]->X();
            startY = systems[curSys]->Y();

            // get number of starlanes current system has
            curNumLanes = laneSetArray[curSys].size();

            // can't have pairs of lanes with less than two lanes...
            if (curNumLanes > 1) {

                // remove any old lane Vector Data
                laneVectsMap.clear();

                // get unit vectors for all lanes of this system
                laneSetIter1 = laneSetArray[curSys].begin();
                laneSetEnd = laneSetArray[curSys].end();
                while (laneSetIter1 != laneSetEnd) {
                    // get destination for this lane
                    dest1 = *laneSetIter1;
                    // get vector to this lane destination
                    vectX1 = systems[dest1]->X() - startX;
                    vectY1 = systems[dest1]->Y() - startY;
                    // normalize
                    mag1 = std::sqrt(vectX1 * vectX1 + vectY1 * vectY1);
                    vectX1 /= mag1;
                    vectY1 /= mag1;

                    // store lane in map of lane vectors
                    tempVect = VectTypeQQ(vectX1, vectY1);
                    tempVectAndMag = VectAndMagTypeQQ(tempVect, mag1);
                    laneVectsMap.insert( MapInsertableTypeQQ(dest1, tempVectAndMag) );

                    laneSetIter1++;
                }

                // iterate through lanes of curSys
                laneSetIter1 = laneSetArray[curSys].begin();
                laneSetIter1++;  // start at second, since iterators are used in pairs, and starting both at the first wouldn't be a valid pair
                while (laneSetIter1 != laneSetEnd) {
                    // get destination of current starlane
                    dest1 = *laneSetIter1;

                    if (curSys < dest1) 
                        lane1 = std::pair<int, int>(curSys, dest1);
                    else
                        lane1 = std::pair<int, int>(dest1, curSys);

                    // check if this lane has already been added to the set of lanes to remove
                    if (0 == lanesToRemoveSet.count(lane1)) {

                        // extract data on starlane vector...
                        laneVectsMapIter = laneVectsMap.find(dest1);
                        assert(laneVectsMapIter != laneVectsMap.end());
                        tempVectAndMag = laneVectsMapIter->second;
                        tempVect = tempVectAndMag.first;
                        vectX1 = tempVect.first;
                        vectY1 = tempVect.second;
                        mag1 = tempVectAndMag.second;

                        // iterate through other lanes of curSys, in order to get all possible pairs of lanes
                        laneSetIter2 = laneSetArray[curSys].begin();
                        while (laneSetIter2 != laneSetIter1) {
                            dest2 = *laneSetIter2;

                            if (curSys < dest2) 
                                lane2 = std::pair<int, int>(curSys, dest2);
                            else
                                lane2 = std::pair<int, int>(dest2, curSys);

                            // check if this lane has already been added to the set of lanes to remove
                            if (0 == lanesToRemoveSet.count(lane2)) {

                                // extract data on starlane vector...
                                laneVectsMapIter = laneVectsMap.find(dest2);
                                assert(laneVectsMapIter != laneVectsMap.end());
                                tempVectAndMag = laneVectsMapIter->second;
                                tempVect = tempVectAndMag.first;
                                vectX2 = tempVect.first;
                                vectY2 = tempVect.second;
                                mag2 = tempVectAndMag.second;

                                // find dot product
                                dotProd = vectX1 * vectX2 + vectY1 * vectY2;

                                // if dotProd is big enough, then lanes are too close angularly
                                // thus one needs to be removed.
                                if (dotProd > maxLaneUVectDotProd) {

                                     // preferentially remove the longer lane
                                    if (mag1 > mag2) {
                                        lanesToRemoveSet.insert(lane1);
                                        break;  // don't need to check any more lanes against lane1, since lane1 has been removed
                                    }
                                    else {
                                        lanesToRemoveSet.insert(lane2);
                                    }
                                }
                            }

                            laneSetIter2++;
                        }
                    }

                    laneSetIter1++;
                }
            }
        }

        // iterate through set of lanes to remove, and remove them in turn...
        lanesToRemoveIter = lanesToRemoveSet.begin();
        lanesToRemoveEnd = lanesToRemoveSet.end();
        while (lanesToRemoveIter != lanesToRemoveEnd) {
            lane1 = *lanesToRemoveIter;

            laneSetArray[lane1.first].erase(lane1.second);
            laneSetArray[lane1.second].erase(lane1.first);

            // check that removing lane hasn't disconnected systems
            if (!ConnectedWithin(lane1.first, lane1.second, numSys, laneSetArray)) {
                // they aren't connected... reconnect them
                laneSetArray[lane1.first].insert(lane1.second);
                laneSetArray[lane1.second].insert(lane1.first);
            }

            lanesToRemoveIter++;
        }
    }

    /** Removes lanes from passed graph that are angularly too close to
      * each other. */
    void CullTooLongLanes(double maxLaneLength, std::vector<std::set<int> >& laneSetArray,
                          std::vector<TemporaryPtr<System> > &systems)
    {
        // start and end systems of a new lane being considered, and end points of lanes that already exist with that start
        // at the start or destination of the new lane
        int curSys, dest;

        // geometry stuff... points components, vector componenets
        double startX, startY, vectX, vectY;

        // iterators to go through sets of lanes in array
        std::set<int>::iterator laneSetIter, laneSetEnd;

        // map, indexed by lane length, of start and end stars of lanes to be removed
        std::multimap<double, std::pair<int, int>, std::greater<double> > lanesToRemoveMap;
        std::multimap<double, std::pair<int, int>, std::greater<double> >::iterator lanesToRemoveIter, lanesToRemoveEnd;
        std::pair<int, int> lane;
        typedef std::pair<double, std::pair<int, int> > MapInsertableTypeQQ;

        int numSys = systems.size();
        // make sure data is consistent
        if (static_cast<int>(laneSetArray.size()) != numSys) {
            return;
        }

        if (numSys < 2) return;  // nothing worth doing for less than two systems (no lanes!)

        // get squared max lane lenth, so as to eliminate the need to take square roots of lane lenths...
        double maxLaneLength2 = maxLaneLength*maxLaneLength;

        // loop through systems
        for (curSys = 0; curSys < numSys; curSys++) {
            // get position of current system (for use in calculating vector)
            startX = systems[curSys]->X();
            startY = systems[curSys]->Y();

            // iterate through all lanes in system, checking lengths and marking to be removed if necessary
            laneSetIter = laneSetArray[curSys].begin();
            laneSetEnd = laneSetArray[curSys].end();
            while (laneSetIter != laneSetEnd) {
                // get destination for this lane
                dest = *laneSetIter;
                // convert start and end into ordered pair to represent lane
                if (curSys < dest) 
                    lane = std::pair<int, int>(curSys, dest);
                else
                    lane = std::pair<int, int>(dest, curSys);

                // get vector to this lane destination
                vectX = systems[dest]->X() - startX;
                vectY = systems[dest]->Y() - startY;

                // compare magnitude of vector to max allowed
                double laneLength2 = vectX*vectX + vectY*vectY;
                if (laneLength2 > maxLaneLength2) {
                    // lane is too long!  mark it to be removed
                    lanesToRemoveMap.insert( MapInsertableTypeQQ(laneLength2, lane) );
                } 

                laneSetIter++;
            }
        }

        // Iterate through set of lanes to remove, and remove them in turn.  Since lanes were inserted in the map indexed by
        // their length, iteration starting with begin starts with the longest lane first, then moves through the lanes as
        // they get shorter, ensuring that the longest lanes are removed first.
        lanesToRemoveIter = lanesToRemoveMap.begin();
        lanesToRemoveEnd = lanesToRemoveMap.end();
        while (lanesToRemoveIter != lanesToRemoveEnd) {
            lane = lanesToRemoveIter->second;

            // ensure the lane still exists
            if (laneSetArray[lane.first].count(lane.second) > 0 &&
                laneSetArray[lane.second].count(lane.first) > 0) {

                // remove lane
                laneSetArray[lane.first].erase(lane.second);
                laneSetArray[lane.second].erase(lane.first);

                // if removing lane has disconnected systems, reconnect them
                if (!ConnectedWithin(lane.first, lane.second, numSys, laneSetArray)) {
                    laneSetArray[lane.first].insert(lane.second);
                    laneSetArray[lane.second].insert(lane.first);
                }
            }
            lanesToRemoveIter++;
        }
    }

    /** Grows trees to connect stars...  takes an array of sets of potential
      * starlanes for each star, and puts the starlanes of the tree into
      * another set. */
    void GrowSpanningTrees(std::vector<int> roots, std::vector<std::set<int> >& potentialLaneSetArray,
                           std::vector<std::set<int> >& laneSetArray)
    {
        // array to keep track of whether a given system (index #) has been connected to by growing tree algorithm
        std::vector<int> treeOfSystemArray; // which growing tree a particular system has been assigned to

        //  map index by tree number, containing a list for each tree, each of which contains the systems in a particular tree
        std::map<int, std::list<int> > treeSysListsMap;
        std::map<int, std::list<int> >::iterator treeSysListsMapIter, treeSysListsMapEnd;
        std::pair<int, std::list<int> > mapInsertable;
        std::list<int> treeSysList, *pTreeSysList, *pTreeToMergeSysList;
        std::list<int>::iterator sysListIter;
        std::set<int>::iterator lanesSetIter, lanesSetEnd;

        int n, q, d, curTree, destTree, curSys, destSys, mergeSys;

        int numSys = potentialLaneSetArray.size();
        int numTrees = roots.size();

        // number of new connections to make from each connected node that is processed.  
        // could be made a parameter, possibly a function of the starlane frequency

        // make sure data is consistent
        if (static_cast<int>(laneSetArray.size()) != numSys) {
            Logger().errorStream() << "GrowSpanningTrees got different size vectors of potential lane set(s) and systems.  Doing nothing.";
            return;
        }
        if ((numTrees < 1) || (numTrees > numSys)) {
            Logger().errorStream() << "GrowSpanningTrees was asked to grow too many or too few trees simultaneously.  Doing nothing.";
            return;
        }
        if (static_cast<int>(roots.size()) > numSys) {
            Logger().errorStream() << "GrowSpanningTrees was asked to grow more separate trees than there are systems to grow from.  Doing nothing.";
            return;
        }

        laneSetArray.resize(numSys);

        // set up data structures...
        treeOfSystemArray.resize(numSys);
        for (n = 0; n < numSys; n++) 
            treeOfSystemArray[n] = -1;  // sentinel value for not connected to any tree

        treeSysListsMap.clear();
        for (n = 0; n < numTrees; n++) {
            // check that next root is within valid range...
            q = roots[n];
            if ((q >= numSys) || (q < 0)) {
                //Logger().debugStream() << "GrowSpanningTrees was asked to grow to grow a tree from a system that doesn't exist.";
                return;
            }

            // make new tree to put into map
            treeSysList.clear();        
            treeSysList.push_front(q);

            // put new list into into map (for tree n), indexed by tree number
            mapInsertable = std::pair<int, std::list<int> >(n, treeSysList);
            treeSysListsMap.insert(mapInsertable);

            // record the tree to which root system of tree n, roots[n], belongs (tree n)
            treeOfSystemArray[q] = n;
        }

        //Logger().debugStream() << "Growing Trees Algorithm Starting...";

        // loop through map (indexed by tree number) of lists of systems, until map (and all lists) are empty...
        treeSysListsMapIter = treeSysListsMap.begin();
        treeSysListsMapEnd = treeSysListsMap.end();
        while (treeSysListsMapIter != treeSysListsMapEnd) {
            // extract number and list of tree
            curTree = treeSysListsMapIter->first;
            pTreeSysList = &(treeSysListsMapIter->second);

            if (pTreeSysList->empty()) {
                // no systems left for tree to grow.  Remove it from map of growing trees.
                treeSysListsMap.erase(curTree);
                //Logger().debugStream() << "Tree " << curTree << " was empty, so was removed from map of trees.";

                // check if set is empty...
                if (treeSysListsMap.empty()) break;  // and stop loop if it is
                // (iterator invalidated by erasing, so set to first tree remaining in map)
                treeSysListsMapIter = treeSysListsMap.begin();
            }
            else {
                //Logger().debugStream() << "Tree " << curTree << " contains " << pTreeSysList->size() << " systems.";
                // tree has systems left to grow.

                // extract and remove a random system from the list

                // iterate to the position of the random system
                sysListIter = pTreeSysList->begin();
                for (d = RandSmallInt(0, pTreeSysList->size() - 1); d > 0; --d) // RandSmallInt(int min, int max);
                    sysListIter++;

                curSys = *sysListIter; // extract
                pTreeSysList->erase(sysListIter); // erase

                //Logger().debugStream() << "Processing system " << curSys << " from tree " << curTree;

                // iterate through list of potential lanes for current system
                lanesSetIter = potentialLaneSetArray[curSys].begin();
                lanesSetEnd = potentialLaneSetArray[curSys].end();
                while (lanesSetIter != lanesSetEnd) {
                    // get destination system of potential lane
                    destSys = *lanesSetIter;

                    // get which, if any, tree the destination system belongs to currently
                    destTree = treeOfSystemArray[destSys];

                    //Logger().debugStream() << "Considering lane from system " << curSys << " to system " << destSys << " of tree " << destTree;

                    // check if the destination system already belongs to the current tree.
                    if (curTree != destTree) {
                        // destination system is either in no tree, or is in a tree other than the current tree

                        // add lane between current and destination systems
                        laneSetArray[curSys].insert(destSys);
                        laneSetArray[destSys].insert(curSys);

                        // mark destination system as part of this tree
                        treeOfSystemArray[destSys] = curTree;

                        //Logger().debugStream() << "Added lane from " << curSys << " to " << destSys << ", and added " << destSys << " to list of systems to process in tree " << curTree;
                    }
                    //else
                    //    Logger().debugStream() << "Both systems were already part of the same tree, so no lane was added";

                    // check what, if any, tree the destination system was before being added to the current tree
                    if (-1 == destTree) {
                        // destination system was not yet part of any tree.
                        // add system to list of systems to consider for this tree in future
                        pTreeSysList->push_back(destSys);

                        //Logger().debugStream() << "System was not yet part of an tree, so was added to the list of systems to process for tree " << curTree;
                    }
                    else if (destTree != curTree) {
                        // tree was already part of another tree
                        // merge the two trees.

                        //Logger().debugStream() << "Merging tree " << destTree << " into current tree " << curTree;

                        pTreeToMergeSysList = &((treeSysListsMap.find(destTree))->second);

                        //Logger().debugStream() << "...got pointer to systems list for tree to merge into current tree";
                        //Logger().debugStream() << "...list to merge has " << pTreeToMergeSysList->size() << " systems.";

                        // extract systems from tree to be merged into current tree
                        while (!pTreeToMergeSysList->empty()) {
                            // get system from list
                            mergeSys = pTreeToMergeSysList->front();
                            pTreeToMergeSysList->pop_front();
                            // add to current list
                            pTreeSysList->push_back(mergeSys);

                            //Logger().debugStream() << "Adding system " << mergeSys << " to current tree " << curTree << " from old tree " << destTree;
                        }

                        // reassign all systems from destination tree to current tree (gets systems even after they're removed
                        // from the list of systems for the dest tree)
                        for (q = 0; q < numSys; q++) 
                            if (treeOfSystemArray[q] == destTree)
                                treeOfSystemArray[q] = curTree;

                        treeSysListsMap.erase(destTree);
                    }

                    lanesSetIter++;
                }
            }

            //Logger().debugStream() << "Moving to next tree...";

            treeSysListsMapIter++;
            treeSysListsMapEnd = treeSysListsMap.end();  // incase deleting or merging trees messed things up
            if (treeSysListsMapIter == treeSysListsMapEnd)
                treeSysListsMapIter = treeSysListsMap.begin();
        }
    }
}

void SetActiveMetersToTargetMaxCurrentValues(ObjectMap& object_map) {
    std::map<MeterType, MeterType> meters;
    meters[METER_POPULATION] =   METER_TARGET_POPULATION;
    meters[METER_INDUSTRY] =     METER_TARGET_INDUSTRY;
    meters[METER_RESEARCH] =     METER_TARGET_RESEARCH;
    meters[METER_TRADE] =        METER_TARGET_TRADE;
    meters[METER_CONSTRUCTION] = METER_TARGET_CONSTRUCTION;
    meters[METER_FUEL] =         METER_MAX_FUEL;
    meters[METER_SHIELD] =       METER_MAX_SHIELD;
    meters[METER_STRUCTURE] =    METER_MAX_STRUCTURE;
    meters[METER_DEFENSE] =      METER_MAX_DEFENSE;
    meters[METER_TROOPS] =       METER_MAX_TROOPS;

    // check for each pair of meter types.  if both exist, set active
    // meter current value equal to target meter current value.
    for (ObjectMap::iterator<> it = object_map.begin(); it != object_map.end(); ++it) {
        for (std::map<MeterType, MeterType>::const_iterator meter_it = meters.begin(); meter_it != meters.end(); ++meter_it)
            if (Meter* meter = it->GetMeter(meter_it->first))
                if (Meter* targetmax_meter = it->GetMeter(meter_it->second))
                    meter->SetCurrent(targetmax_meter->Current());
    }
}

void SetNativePopulationValues(ObjectMap& object_map) {
    for (ObjectMap::iterator<> it = object_map.begin(); it != object_map.end(); ++it) {
        Meter* meter = it->GetMeter(METER_POPULATION);
        Meter* targetmax_meter = it->GetMeter(METER_TARGET_POPULATION);
        // only applies to unowned planets
        if (meter && targetmax_meter && it->Unowned()) {
            double r = RandZeroToOne();
            double factor = (0.1<r)?r:0.1;
            meter->SetCurrent(targetmax_meter->Current() * factor);
        }
    }
}

void AddStartingSpecials(Universe& universe, GalaxySetupOption specials_freq) {
    Logger().debugStream() << "AddStartingSpecials";

    double special_chance = UniverseDataTables()["SpecialsFrequency"][0][specials_freq] / 10000.0;
    if (special_chance == 0.0)
        return;

    const std::vector<std::string> special_names = SpecialNames();
    if (special_names.empty())
        return;

    // initialize count of how many of each special has been added
    std::map<std::string, int> specials_added;
    for (std::vector<std::string>::const_iterator special_it = special_names.begin();
         special_it != special_names.end(); ++special_it)
    {
        specials_added[*special_it] = 0;
    }

    // attempt to apply a special to every object by finding a special that can
    // be applied to it and hasn't been added too many times, and then attempt
    // to add that special by testing its spawn rate
    std::vector<std::string>::const_iterator special_name_it = special_names.begin();
    for (ObjectMap::iterator<> obj_it = universe.Objects().begin(); obj_it != universe.Objects().end(); ++obj_it) {
        // for this object, find a suitable special
        std::vector<std::string>::const_iterator initial_special_name_it = special_name_it;
        while (true) {
            const std::string& special_name = *special_name_it;
            const Special* special = GetSpecial(special_name);

            bool special_add_attempted = false;

            // test if too many of this special have already been added
            if (special && specials_added[special_name] < special->SpawnLimit()) {
                // test if this special can be spawned on this object.  If
                // there is no location condition, assume this special can't
                // be spawned automatically
                const Condition::ConditionBase* location_test = special->Location();
                if (location_test && location_test->Eval(*obj_it)) {
                    // special can be placed here

                    // test random chance to place this special
                    double this_special_chance = std::max(0.0, special_chance * special->SpawnRate());
                    if (RandZeroToOne() < this_special_chance) {
                        // spawn special
                        specials_added[special_name]++;
                        obj_it->AddSpecial(special_name);
                        Logger().debugStream() << "Special " << special_name << " added to " << obj_it->Name();

                        // kludges for planet appearance changes for particular specials
                        if (special_name == "TIDAL_LOCK_SPECIAL") {
                            if (TemporaryPtr<Planet> planet = boost::dynamic_pointer_cast<Planet>(*obj_it))
                                planet->SetRotationalPeriod(Day(planet->OrbitalPeriod()));
                        } else if (special_name == "SLOW_ROTATION_SPECIAL") {
                            if (TemporaryPtr<Planet> planet = boost::dynamic_pointer_cast<Planet>(*obj_it))
                                planet->SetRotationalPeriod(planet->RotationalPeriod() * 10.0);
                        } else if (special_name == "HIGH_AXIAL_TILT_SPECIAL") {
                            if (TemporaryPtr<Planet> planet = boost::dynamic_pointer_cast<Planet>(*obj_it))
                                planet->SetHighAxialTilt();
                        }
                    }
                    special_add_attempted = true;
                } else {
                    //Logger().debugStream() << "... ... cannot be added here";
                }
            } else {
                //Logger().debugStream() << "... ... has been added " << specials_added[special_name] << " times, which is >= the limit of " << special->SpawnLimit();
            }

            // increment special name iterator
            special_name_it++;
            if (special_name_it == special_names.end())
                special_name_it = special_names.begin();

            // stop attempting to add specials here?
            if (special_name_it == initial_special_name_it || special_add_attempted)
                break;
        }
    }
}

namespace {
    static const Condition::Not homeworld_jumps_filter(
        new Condition::WithinStarlaneJumps(
            new ValueRef::Constant<int>(1),
            new Condition::Homeworld()
        )
    );
}

void GenerateNatives(Universe& universe, GalaxySetupOption freq) {
    Logger().debugStream() << "GenerateNatives";

    int inverse_native_chance = UniverseDataTables()["NativeFrequency"][0][freq];
    double native_chance(0.0);
    if (inverse_native_chance > 0)
        native_chance = 1.0 / static_cast<double>(inverse_native_chance);
    else
        return;

    SpeciesManager& species_manager = GetSpeciesManager();

    std::vector<TemporaryPtr<Planet> > planet_vec = Objects().FindObjects<Planet>();
    Condition::ObjectSet planet_set(planet_vec.size());
    for (std::vector<TemporaryPtr<Planet> >::iterator it = planet_vec.begin();
         it != planet_vec.end(); ++it)
    { planet_set.push_back(*it); }

    // select only planets far away from player homeworlds
    Condition::ObjectSet native_safe_planet_set;
    native_safe_planet_set.reserve(planet_set.size());
    homeworld_jumps_filter.Eval(native_safe_planet_set, planet_set);
    Logger().debugStream() << "Number of planets far enough from players for natives to be allowed: " << native_safe_planet_set.size();
    if (native_safe_planet_set.empty())
        return;

    Logger().debugStream() << "Species that can be added as natives:";
    for (SpeciesManager::native_iterator species_it = species_manager.native_begin();
        species_it != species_manager.native_end(); ++species_it)
    { Logger().debugStream() << "... " << species_it->first; }

    std::vector<TemporaryPtr<Planet> > native_safe_planets;
    for (Condition::ObjectSet::iterator it = native_safe_planet_set.begin(); it != native_safe_planet_set.end(); ++it)
        if (TemporaryPtr<const Planet> planet = boost::dynamic_pointer_cast<const Planet>(*it))
            native_safe_planets.push_back(boost::const_pointer_cast<Planet>(planet));

    // randomly add species to planets
    for (std::vector<TemporaryPtr<Planet> >::iterator it = native_safe_planets.begin(); it != native_safe_planets.end(); ++it) {
        if (RandZeroToOne() > native_chance)
            continue;

        TemporaryPtr<Planet> planet = *it;
        PlanetType planet_type = planet->Type();
        Logger().debugStream() << "Attempting to add natives to planet " << planet->Name() << " of type " << planet_type;

        // find species that like this planet type
        std::vector<std::string> suitable_species;
        for (SpeciesManager::native_iterator species_it = species_manager.native_begin();
             species_it != species_manager.native_end(); ++species_it)
        {
            if (species_it->second->GetPlanetEnvironment(planet_type) == PE_GOOD)
                suitable_species.push_back(species_it->first);
        }
        if (suitable_species.empty()) {
            Logger().debugStream() << "... no suitable native species found (with good environment on this planet)";
            continue;
        }
        Logger().debugStream() << " ... " << suitable_species.size() << " native species are appropriate for this planet";

        // pick a species and assign to the planet
        int species_idx = RandSmallInt(0, suitable_species.size() - 1);
        const std::string& species_name = suitable_species.at(species_idx);
        planet->SetSpecies(species_name);

        // set planet as a homeworld for that species
        if (Species* species = species_manager.GetSpecies(species_name))
            species->AddHomeworld(planet->ID());

        // find a focus to give planets by default.  use first defined available focus.
        std::vector<std::string> available_foci = planet->AvailableFoci();
        if (!available_foci.empty()) {
            planet->SetFocus(*available_foci.begin());
            Logger().debugStream() << "Set focus to " << *available_foci.begin();
        } else {
            Logger().debugStream() << "No foci available for this planet! (" << planet->ObjectType() << ")";
        }

        Logger().debugStream() << "Added native " << species_name << " to planet " << planet->Name();
    }
}

void GenerateSpaceMonsters(Universe& universe, GalaxySetupOption freq) {
    Logger().debugStream() << "GenerateSpaceMonsters";

    // get overall universe chance for monster generation in a system
    int inverse_monster_chance = UniverseDataTables()["MonsterFrequency"][0][freq];
    Logger().debugStream() << "Universe::GenerateSpaceMonsters(" << boost::lexical_cast<std::string>(freq)
                           << ") inverse monster chance: " << inverse_monster_chance;
    double monster_chance(0.0);
    if (inverse_monster_chance > 0)
        monster_chance = 1.0 / static_cast<double>(inverse_monster_chance);
    else
        return;
    Logger().debugStream() << "Default monster spawn chance: " << monster_chance;

    // sets of monsters to generate
    const MonsterFleetPlanManager& monster_manager = GetMonsterFleetPlanManager();
    if (monster_manager.NumMonsters() < 1)
        return;

    // ship designs (including monsters)
    const PredefinedShipDesignManager& predefined_design_manager = GetPredefinedShipDesignManager();

    // possible locations to generate monsters
    std::vector<TemporaryPtr<System> > system_vec = Objects().FindObjects<System>();
    if (system_vec.empty())
        return;

    // initialize count of how many of each monster fleet plan has been created
    std::map<MonsterFleetPlanManager::iterator, int> monster_fleets_created;
    for (MonsterFleetPlanManager::iterator monster_plan_it = monster_manager.begin();
         monster_plan_it != monster_manager.end(); ++monster_plan_it)
    { monster_fleets_created[monster_plan_it] = 0; }

    // for each system, find a monster whose location condition allows the
    // system, which hasn't already been added too many times, and then attempt
    // to add that monster by testing the spawn rate chance
    MonsterFleetPlanManager::iterator monster_plan_it = monster_manager.begin();
    for (std::vector<TemporaryPtr<System> >::iterator sys_it = system_vec.begin();
         sys_it != system_vec.end(); ++sys_it)
    {
        //Logger().debugStream() << "Attempting to add monster at system " << system->Name();
        TemporaryPtr<System> system = *sys_it;

        // for this system, find a suitable monster fleet plan
        const MonsterFleetPlanManager::iterator initial_monster_plan_it = monster_plan_it;
        while (true) {
            const MonsterFleetPlan* plan = *monster_plan_it;
            //Logger().debugStream() << "... considering monster plan " << plan->Name();

            bool monster_add_attempted = false;

            // test if too many of this fleet have already been created
            if (monster_fleets_created[monster_plan_it] < plan->SpawnLimit()) {
                // test if this monster fleet plan can be spawned at this
                // location.  if there is no location condition, assume there
                // is no restriction on this monster's spawn location.
                const Condition::ConditionBase* location_test = plan->Location();
                if (!location_test || location_test->Eval(system)) {
                    //Logger().debugStream() << "... ... can be placed here";
                    // monster can be placed here.

                    // test random chance to place this monster fleet
                    double this_monster_fleet_chance = std::max(0.0, monster_chance * plan->SpawnRate());
                    //Logger().debugStream() << "... ... chance: " << this_monster_fleet_chance;
                    if (RandZeroToOne() < this_monster_fleet_chance) {
                        //Logger().debugStream() << "... ... passed random chance test";

                        // spawn monster fleet
                        monster_fleets_created[monster_plan_it]++;

                        const std::vector<std::string>& monsters = plan->ShipDesigns();
                        if (monsters.empty())
                            break;

                        // create fleet for monsters
                        const std::string& fleet_name = UserString("MONSTERS");
                        TemporaryPtr<Fleet> fleet = universe.CreateFleet(fleet_name, system->X(), system->Y(), ALL_EMPIRES);
                        if (!fleet) {
                            Logger().errorStream() << "unable to create new fleet for monsters!";
                            return;
                        }
                        system->Insert(fleet);

                        // create ships and add to fleet
                        for (std::vector<std::string>::const_iterator monster_it = monsters.begin();
                             monster_it != monsters.end(); ++monster_it)
                        {
                            int design_id = predefined_design_manager.GetDesignID(*monster_it);
                            if (design_id == ShipDesign::INVALID_DESIGN_ID) {
                                Logger().errorStream() << "Couldn't find space monster with name " << *monster_it;
                                continue;
                            }

                            // create new monster ship
                            TemporaryPtr<Ship> ship = universe.CreateShip(ALL_EMPIRES, design_id, "", ALL_EMPIRES);
                            system->Insert(ship);

                            ship->Rename(NewMonsterName());

                            fleet->AddShip(ship->ID());
                            ship->SetFleetID(fleet->ID());
                        }
                    } else {
                        //Logger().debugStream() << "... ... failed random chance test. skipping system.";
                        // monster was acceptable for this location, but
                        // failed chance test.  no monsters here.
                    }
                    monster_add_attempted = true;
                } else {
                    //Logger().debugStream() << "... ... cannot be placed here";
                }
            } else {
                //Logger().debugStream() << "... ... has been placed " << monster_fleets_created[monster_plan_it] << " times, which is >= the limit of " << plan->SpawnLimit();
            }

            // increment monster plan iterator
            monster_plan_it++;
            if (monster_plan_it == monster_manager.end())
                monster_plan_it = monster_manager.begin();

            // stop attempting to add monsters here?
            if (monster_plan_it == initial_monster_plan_it || monster_add_attempted)
                break;
        }
    }
}

void GenerateStarlanes(GalaxySetupOption freq) {
    if (freq == GALAXY_SETUP_NONE)
        return;

    int numSys, s1, s2, s3; // numbers of systems, indices in vec_sys
    int n; // loop counter

    std::vector<int> triVerts;  // indices of stars that form vertices of a triangle

    // array of set to store final, included starlanes for each star
    std::vector<std::set<int> > laneSetArray;

    // array of set to store possible starlanes for each star, as extracted form triangulation
    std::vector<std::set<int> > potentialLaneSetArray;

    // iterators for traversing lists of starlanes
    std::set<int>::iterator laneSetIter, laneSetEnd, laneSetIter2, laneSetEnd2;

    // get systems
    std::vector<TemporaryPtr<System> > sys_vec = Objects().FindObjects<System>();
    numSys = sys_vec.size();  // (actually = number of systems + 1)
    
    // pass systems to Delauney Triangulation routine, getting array of triangles back
    std::list<Delauney::DTTriangle>* triList = Delauney::DelauneyTriangulate(sys_vec);
    if (!triList ||triList->empty()) {
        Logger().errorStream() << "Got no list or blank list of triangles from Triangulation.";
        return;
    }

    Delauney::DTTriangle tri;

    // convert passed StarlaneFrequency freq into maximum number of starlane jumps between systems that are
    // "adjacent" in the delauney triangulation.  (separated by a single potential starlane).
    // these numbers can be tweaked
    int maxJumpsBetweenSystems = UniverseDataTables()["MaxJumpsBetweenSystems"][0][freq];

    // initialize arrays...
    potentialLaneSetArray.resize(numSys);
    for (n = 0; n < numSys; n++) {
        potentialLaneSetArray[n].clear();
    }
    laneSetArray.resize(numSys);
    for (n = 0; n < numSys; n++) {
        laneSetArray[n].clear();
    }

    // extract triangles from list, add edges to sets of potential starlanes for each star (in array)
    while (!triList->empty()) {
        tri = triList->front();

        triVerts = tri.Verts();
        s1 = triVerts[0];
        s2 = triVerts[1];
        s3 = triVerts[2];

        // add starlanes to list of potential starlanes for each star, making sure each pair involves
        // only stars that actually exist.  triangle generation uses three extra points which don't
        // represent actual systems and which need to be weeded out here.
        if ((s1 >= 0) && (s2 >= 0) && (s3 >= 0)) {
            if ((s1 < numSys) && (s2 < numSys)) {
                potentialLaneSetArray[s1].insert(s2);
                potentialLaneSetArray[s2].insert(s1);
            }
            if ((s1 < numSys) && (s3 < numSys)) {
                potentialLaneSetArray[s1].insert(s3);
                potentialLaneSetArray[s3].insert(s1);
            }
            if ((s2 < numSys) && (s3 < numSys)) {
                potentialLaneSetArray[s2].insert(s3);
                potentialLaneSetArray[s3].insert(s2);
            }
        }

        triList->pop_front();
    }

    // cleanup
    delete triList;

    //Logger().debugStream() << "Extracted Potential Starlanes from Triangulation";

    double maxStarlaneLength = UniverseDataTables()["MaxStarlaneLength"][0][0];
    CullTooLongLanes(maxStarlaneLength, potentialLaneSetArray, sys_vec);

    CullAngularlyTooCloseLanes(0.98, potentialLaneSetArray, sys_vec);

    //Logger().debugStream() << "Culled Agularly Too Close Lanes";

    laneSetArray = potentialLaneSetArray;

    // attempt removing lanes, but don't do so if it would make the systems
    // the lane connects too far apart
    for (n = 0; n < numSys; ++n) {
        laneSetIter = potentialLaneSetArray[n].begin();

        while (laneSetIter != potentialLaneSetArray[n].end()) {
            s1 = *laneSetIter;

            // try removing lane
            laneSetArray[n].erase(s1);
            laneSetArray[s1].erase(n);

            if (!ConnectedWithin(n, s1, maxJumpsBetweenSystems, laneSetArray)) {
                // lane removal was a bad idea.  restore it
                laneSetArray[n].insert(s1);
                laneSetArray[s1].insert(n);
            }

            laneSetIter++;
        } // end while
    }

     // add the starlane to the stars
    for (n = 0; n < numSys; ++n) {
        const std::set<int>& lanes = laneSetArray[n];
        for (std::set<int>::const_iterator it = lanes.begin(); it != lanes.end(); ++it)
            sys_vec[n]->AddStarlane(sys_vec[*it]->ID()); // System::AddStarlane() expects a system ID
    }

    Logger().debugStream() << "Initializing System Graph";
    GetUniverse().InitializeSystemGraph();


    //for (n = 0; n < numSys; n++)
    //    laneSetArray[n].clear();

    //// array of indices of systems from which to start growing spanning tree(s).  This can later be replaced with
    //// some sort of user input.  It can also be ommited entirely, so just the ConnectedWithin loop below is used.
    //std::vector<int> roots(4);
    //roots[0] = 0;  roots[1] = 1;  roots[2] = 2;  roots[3] = 3;
    //GrowSpanningTrees(roots, potentialLaneSetArray, laneSetArray);
    ////Logger().debugStream() << "Constructed initial spanning trees.";

    //// add starlanes of spanning tree to stars
    //for (n = 0; n < numSys; n++) {
    //    laneSetIter = laneSetArray[n].begin();
    //    laneSetEnd = laneSetArray[n].end();
    //    while (laneSetIter != laneSetEnd) {
    //        s1 = *laneSetIter;
    //        // add the starlane to the stars
    //        sys_vec[n]->AddStarlane(s1);
    //        sys_vec[s1]->AddStarlane(n);
    //        laneSetIter++;
    //    } // end while
    //} // end for n


    //// loop through stars, seeing if any are too far away from stars they could be connected to by a
    //// potential starlane.  If so, add the potential starlane to the stars to directly connect them
    //for (n = 0; n < numSys; n++) {
    //    laneSetIter = potentialLaneSetArray[n].begin();
    //    laneSetEnd = potentialLaneSetArray[n].end();

    //    while (laneSetIter != laneSetEnd) {
    //        s1 = *laneSetIter;

    //        if (!ConnectedWithin(n, s1, maxJumpsBetweenSystems, laneSetArray)) {

    //            // add the starlane to the sets of starlanes for each star
    //            laneSetArray[n].insert(s1);
    //            laneSetArray[s1].insert(n);
    //            // add the starlane to the stars
    //            sys_vec[n]->AddStarlane(s1);
    //            sys_vec[s1]->AddStarlane(n);
    //        }

    //        laneSetIter++;
    //    } // end while
    //} // end for n
}

void GenerateHomeworlds(Universe& universe, int players, std::vector<int>& homeworld_planet_ids) {
    homeworld_planet_ids.clear();

    std::vector<TemporaryPtr<System> > sys_vec = Objects().FindObjects<System>();
    //Logger().debugStream() << "Universe::GenerateHomeworlds sys_vec:";
    //for (std::vector<TemporaryPtr<System> >::const_iterator it = sys_vec.begin(); it != sys_vec.end(); ++it) {
    //    TemporaryPtr<const System> sys = *it;
    //    Logger().debugStream() << "... sys ptr: " << sys << " name: " << (sys ? sys->Name() : "no system!?") << " id: " << (sys ? boost::lexical_cast<std::string>(sys->ID()) : "none?!");
    //}

    if (sys_vec.empty())
        throw std::runtime_error("Attempted to generate homeworlds in an empty galaxy.");

    for (int i = 0; i < players; ++i) {
        int system_index;
        TemporaryPtr<System> system;

        // make sure it has planets and it's not too close to the other homeworlds
        bool too_close = true;
        int attempts = 0;
        do {
            too_close = false;
            system_index = RandSmallInt(0, static_cast<int>(sys_vec.size()) - 1);
            //Logger().debugStream() << "Universe::GenerateHomeworlds trying to put homeworld on system with index: " << system_index;
            system = sys_vec[system_index];
            //Logger().debugStream() << "... system name: " << (system ? system->Name() : "no system!?") << " id: " << (system ? boost::lexical_cast<std::string>(system->ID()) : "none?!");

            for (unsigned int j = 0; j < homeworld_planet_ids.size(); ++j) {
                //Logger().debugStream() << "Universe::GenerateHomeworlds checking previously-existing homeworld with id " << homeworld_planet_ids[j];
                TemporaryPtr<Planet> homeworld = GetPlanet(homeworld_planet_ids[j]);
                if (!homeworld) {
                    //Logger().errorStream() << "couldn't find homeworld!";
                    continue;
                }

                TemporaryPtr<System> existing_system = GetSystem(homeworld->SystemID());
                if (!existing_system) {
                    //Logger().errorStream() << "couldn't find existing system!";
                    continue;
                }

                double x_dist = existing_system->X() - system->X();
                double y_dist = existing_system->Y() - system->Y();
                if (x_dist * x_dist + y_dist * y_dist < MIN_HOME_SYSTEM_SEPARATION * MIN_HOME_SYSTEM_SEPARATION) {
                    too_close = true;
                    break;
                }
            }
        } while ((!system->Orbits() || system->PlanetIDs().empty() || too_close) && ++attempts < 50);

        sys_vec.erase(sys_vec.begin() + system_index);

        // find a place to put the homeworld, and replace whatever planet is there already
        int home_orbit = -1;

        if (system->PlanetIDs().empty()) {
            home_orbit = 0;
        } else {
            // find empty orbit?
            for (int o = 0; o < system->Orbits(); ++o) {
                if (system->PlanetInOrbit(o) == INVALID_OBJECT_ID) {
                    home_orbit = o;
                    break;
                }
            }
            if (home_orbit == -1) {
                // no free orbit, so need to clear one.
                int sacrificial_planet = system->PlanetInOrbit(0);
                system->Remove(sacrificial_planet);
                universe.Delete(sacrificial_planet);
                home_orbit = 0;
            }
        }

        TemporaryPtr<Planet> planet = universe.CreatePlanet(PT_TERRAN, SZ_MEDIUM);
        std::string planet_name = system->Name() + " " + RomanNumber(home_orbit + 1);
        planet->Rename(planet_name);
        system->Insert(TemporaryPtr<UniverseObject>(planet), home_orbit);

        homeworld_planet_ids.push_back(planet->ID());
    }
}

void NamePlanets(Universe& universe) {
    std::vector<TemporaryPtr<System> > sys_vec = Objects().FindObjects<System>();
    for (ObjectMap::iterator<System> sys_it = Objects().begin<System>();
         sys_it != Objects().end<System>(); ++sys_it)
    {
        TemporaryPtr<System> system = *sys_it;
        std::vector<TemporaryPtr<Planet> > planets = Objects().FindObjects<Planet>(system->PlanetIDs());

        for (std::vector<TemporaryPtr<Planet> >::iterator planet_it = planets.begin();
             planet_it != planets.end(); ++planet_it)
        {
            TemporaryPtr<Planet> planet = *planet_it;

            if (planet->Type() == PT_ASTEROIDS) {
                std::string name = boost::io::str(FlexibleFormat(UserString("PL_ASTEROID_BELT_OF_SYSTEM")) %
                                                  system->Name());
                planet->Rename(name);
            } else {
                int orbit = system->OrbitOfPlanet(planet->ID());
                planet->Rename(system->Name() + " " + RomanNumber(orbit+1));
            }
        }
    }
}

void GenerateFields(Universe& universe, GalaxySetupOption freq) {
    //const FieldType* storm_type = GetFieldType("ION_STORM");
    //if (!storm_type) {
    //    Logger().errorStream() << "AddFields couldn't find storm field type!";
    //    return;
    //}
    //TemporaryPtr<Field> field = new Field(storm_type->Name(), 0.0, 0.0, 100.0);
    //Insert(field);
}

namespace {
    /** Reads list of strings from file, surrounded by enclosing quotes. */
    void LoadNames(std::vector<std::string>& names, const std::string& file_name) {
        names.clear();
        std::string input;
        boost::filesystem::ifstream ifs(GetResourceDir() / file_name);
        if (ifs) {
            std::getline(ifs, input, '\0');
            ifs.close();
        } else {
            Logger().errorStream() << "Unable to open data file " << file_name;
        }
        using namespace boost::algorithm;
        split(names, input, is_any_of("\"\n"), token_compress_on);
        for (std::size_t i = 0; i < names.size(); ) {
            if (names[i].empty())
                names.erase(names.begin() + i);
            else
                ++i;
        }
    }
};

bool SetEmpireHomeworld(Empire* empire, int planet_id, std::string species_name) {
    // get home planet and system, check if they exist
    TemporaryPtr<Planet> home_planet = GetPlanet(planet_id);
    TemporaryPtr<System> home_system;
    if (home_planet)
        home_system = GetSystem(home_planet->SystemID());
    if (!home_planet || !home_system) {
        Logger().errorStream() << "UniverseGenerator::SetEmpireHomeworld: couldn't get homeworld or system for empire" << empire->EmpireID();
        return false;
    }

    Logger().debugStream() << "UniverseGenerator::SetEmpireHomeworld: setting " << home_system->Name()
                           << " (planet " <<  home_planet->ID()
                           << ") to be home system for empire " << empire->EmpireID();

    // get species, check if it exists
    Species* species = GetSpeciesManager().GetSpecies(species_name);
    if (!species) {
        Logger().errorStream() << "UniverseGenerator::SetEmpireHomeworld: couldn't get species \""
                               << species_name << "\" to set with homeworld id " << home_planet->ID();
        return false;
    }

    // set homeword's planet type to the preferred type for this species
    const std::map<PlanetType, PlanetEnvironment>& spte = species->PlanetEnvironments();
    if (!spte.empty()) {
        // invert map from planet type to environments to map from
        // environments to type, sorted by environment
        std::map<PlanetEnvironment, PlanetType> sept;
        for (std::map<PlanetType, PlanetEnvironment>::const_iterator it = spte.begin(); it != spte.end(); ++it)
            sept[it->second] = it->first;
        // assuming enum values are ordered in increasing goodness...
        PlanetType preferred_planet_type = sept.rbegin()->second;

        home_planet->SetType(preferred_planet_type);
        if (preferred_planet_type == PT_ASTEROIDS)
            home_planet->SetSize(SZ_ASTEROIDS);
        else if (preferred_planet_type == PT_GASGIANT)
            home_planet->SetSize(SZ_GASGIANT);
        else
            home_planet->SetSize(SZ_MEDIUM);
    }

    home_planet->Colonize(empire->EmpireID(), species_name, Meter::LARGE_VALUE);
    species->AddHomeworld(home_planet->ID());
    empire->SetCapitalID(home_planet->ID());
    empire->AddExploredSystem(home_planet->SystemID());

    return true;
}

void InitEmpires(const std::map<int, PlayerSetupData>& player_setup_data)
{
    Logger().debugStream() << "Initializing " << player_setup_data.size() << " empires";

    // copy empire colour table, so that individual colours can be removed after they're used
    std::vector<GG::Clr> colors = EmpireColors();

    // create empire objects and do some basic initilization for each player
    int player_i = 0;
    for (std::map<int, PlayerSetupData>::const_iterator setup_data_it = player_setup_data.begin();
         setup_data_it != player_setup_data.end(); ++setup_data_it, ++player_i)
    {
        int         player_id =     setup_data_it->first;
        if (player_id == Networking::INVALID_PLAYER_ID)
            Logger().errorStream() << "Universe::InitEmpires player id (" << player_id << ") is invalid";
        // use player ID for empire ID so that the calling code can get the
        // correct empire for each player ID  in player_setup_data
        int         empire_id =     player_id;
        std::string player_name =   setup_data_it->second.m_player_name;
        GG::Clr     empire_colour = setup_data_it->second.m_empire_color;

        // validate or generate empire colour
        // ensure no other empire gets auto-assigned this colour automatically
        std::vector<GG::Clr>::iterator color_it = std::find(colors.begin(), colors.end(), empire_colour);
        if (color_it != colors.end())
            colors.erase(color_it);

        // if no colour already set, do so automatically
        if (empire_colour == GG::Clr(0, 0, 0, 0)) {
            if (!colors.empty()) {
                // take next colour from list
                empire_colour = colors[0];
                colors.erase(colors.begin());
            } else {
                // as a last resort, make up a colour
                empire_colour = GG::FloatClr(static_cast<float>(RandZeroToOne()), static_cast<float>(RandZeroToOne()),
                                             static_cast<float>(RandZeroToOne()), 1.0f);
            }
        }

        // set generic default empire name
        std::string empire_name = UserString("EMPIRE") + boost::lexical_cast<std::string>(empire_id);

        Logger().debugStream() << "Universe::InitEmpires creating new empire" << " with ID: " << empire_id
                               << " for player: " << player_name << " (with player id: " << player_id << ")";

        // create new Empire object through empire manager
        Empires().CreateEmpire(empire_id, empire_name, player_name, empire_colour);
    }

    Empires().ResetDiplomacy();
}
