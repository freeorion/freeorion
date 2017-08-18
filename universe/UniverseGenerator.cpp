#include "UniverseGenerator.h"

#include "../util/Directories.h"
#include "../util/Random.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../parse/Parse.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include "Planet.h"
#include "System.h"
#include "Species.h"
#include "ValueRef.h"
#include "Enums.h"

//////////////////////////////////////////
//  Universe Setup Functions            //
//////////////////////////////////////////

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
    std::list<Delauney::DTTriangle>* DelauneyTriangulate(std::vector<std::shared_ptr<System>> &systems) {

        int n, c, theSize, num, num2; // loop counters, storage for retreived size of a vector, temp storage
        std::list<Delauney::DTTriangle>::iterator itCur, itEnd;
        std::list<Delauney::SortValInt>::iterator itCur2, itEnd2;
        // vector of x and y positions of stars
        std::vector<Delauney::DTPoint> points;
        // pointer to main list of triangles algorithm works with.
        std::list<Delauney::DTTriangle> *triangle_list;
        // list of indices in vector of points extracted from removed triangles that need to be retriangulated
        std::list<Delauney::SortValInt> pointNumList;
        double vx, vy, mag;  // vector components, magnitude

        // ensure a useful list of systems was passed...
        if (systems.empty())
            throw std::runtime_error("Attempted to run Delauney Triangulation on empty array of systems");

        // extract systems positions, and store in vector.  Can't use actual systems data since
        // systems have position limitations which would interfere with algorithm
        for (auto& system : systems) {
            points.push_back(Delauney::DTPoint(system->X(), system->Y()));
        }

        // add points for covering triangle.  the point positions should be big enough to form a triangle
        // that encloses all the systems of the galaxy (or at least one whose circumcircle covers all points)
        points.push_back(Delauney::DTPoint(-1.0, -1.0));
        points.push_back(Delauney::DTPoint(2.0 * (GetUniverse().UniverseWidth() + 1.0), -1.0));
        points.push_back(Delauney::DTPoint(-1.0, 2.0 * (GetUniverse().UniverseWidth() + 1.0)));

        // initialize triangle_list.  algorithm adds and removes triangles from this list, and the resulting
        // list is returned (so should be deleted externally)
        triangle_list = new std::list<Delauney::DTTriangle>;

        // add last three points into the first triangle, the "covering triangle"
        theSize = static_cast<int>(points.size());
        triangle_list->push_front(Delauney::DTTriangle(theSize-1, theSize-2, theSize-3, points));

        // loop through "real" points (from systems, not the last three added to make the covering triangle)
        for (n = 0; n < theSize - 3; n++) {
            pointNumList.clear();

            // check each triangle in list, to see if the new point lies in its circumcircle.  if so, delete
            // the triangle and add its vertices to a list 
            itCur = triangle_list->begin();
            itEnd = triangle_list->end();
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
                        mag = std::sqrt(vx*vx + vy*vy);
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
                                ++itCur2;
                            }
                            if (itCur2 == itEnd2) {
                                // point wasn't added, so should go at end
                                pointNumList.push_back(Delauney::SortValInt(num, mag));
                            }
                        }
                    } // end for c

                    // remove current triangle from list of triangles
                    itCur = triangle_list->erase(itCur);
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
            triangle_list->push_front(Delauney::DTTriangle(n, (pointNumList.front()).num, (pointNumList.back()).num, points));

            num = itCur2->num;
            ++itCur2;
            while (itCur2 != itEnd2) {
                num2 = num;
                num = itCur2->num;

                triangle_list->push_front(Delauney::DTTriangle(n, num2, num, points));

                ++itCur2;
            } // end while

        } // end for
        return triangle_list;
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

namespace {
    /** Used by GenerateStarlanes.  Determines if two systems are connected by
      * maxLaneJumps or less edges on graph. */
    bool ConnectedWithin(int system1, int system2, int maxLaneJumps, std::vector<std::set<int>>& laneSetArray) {
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

                ++curSysLanesSetIter;
            }

            ++sysListIter;
        }
        return false; // default
    }

    /** Removes lanes from passed graph that are angularly too close to
      * each other. */
    void CullAngularlyTooCloseLanes(double maxLaneUVectDotProd, std::vector<std::set<int>>& laneSetArray,
                                    std::vector<std::shared_ptr<System>> &systems)
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

        std::set<std::pair<int, int>> lanesToRemoveSet;  // start and end stars of lanes to be removed in final step...
        std::set<std::pair<int, int>>::iterator lanesToRemoveIter, lanesToRemoveEnd;
        std::pair<int, int> lane1, lane2;

        int curNumLanes;

        int num_systems = systems.size();
        // make sure data is consistent
        if (static_cast<int>(laneSetArray.size()) != num_systems) {
            ErrorLogger() << "CullAngularlyTooCloseLanes got different size vectors of lane sets and systems.  Doing nothing.";
            return;
        }

        if (num_systems < 3) return;  // nothing worth doing for less than three systems

        //DebugLogger() << "Culling Too Close Angularly Lanes";

        // loop through systems
        for (curSys = 0; curSys < num_systems; curSys++) {
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

                    ++laneSetIter1;
                }

                // iterate through lanes of curSys
                laneSetIter1 = laneSetArray[curSys].begin();
                ++laneSetIter1;  // start at second, since iterators are used in pairs, and starting both at the first wouldn't be a valid pair
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
                                    } else {
                                        lanesToRemoveSet.insert(lane2);
                                    }
                                }
                            }

                            ++laneSetIter2;
                        }
                    }

                    ++laneSetIter1;
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
            if (!ConnectedWithin(lane1.first, lane1.second, num_systems, laneSetArray)) {
                // they aren't connected... reconnect them
                laneSetArray[lane1.first].insert(lane1.second);
                laneSetArray[lane1.second].insert(lane1.first);
            }

            ++lanesToRemoveIter;
        }
    }

    /** Removes lanes from passed graph that are angularly too close to
      * each other. */
    void CullTooLongLanes(double maxLaneLength, std::vector<std::set<int>>& laneSetArray,
                          std::vector<std::shared_ptr<System>> &systems)
    {
        // start and end systems of a new lane being considered, and end points of lanes that already exist with that start
        // at the start or destination of the new lane
        int curSys, dest;

        // geometry stuff... points components, vector componenets
        double startX, startY, vectX, vectY;

        // iterators to go through sets of lanes in array
        std::set<int>::iterator laneSetIter, laneSetEnd;

        // map, indexed by lane length, of start and end stars of lanes to be removed
        std::multimap<double, std::pair<int, int>, std::greater<double>> lanesToRemoveMap;
        std::multimap<double, std::pair<int, int>, std::greater<double>>::iterator lanesToRemoveIter, lanesToRemoveEnd;
        std::pair<int, int> lane;
        typedef std::pair<double, std::pair<int, int>> MapInsertableTypeQQ;

        int num_systems = systems.size();
        // make sure data is consistent
        if (static_cast<int>(laneSetArray.size()) != num_systems) {
            return;
        }

        if (num_systems < 2) return;  // nothing worth doing for less than two systems (no lanes!)

        // get squared max lane lenth, so as to eliminate the need to take square roots of lane lenths...
        double maxLaneLength2 = maxLaneLength*maxLaneLength;

        // loop through systems
        for (curSys = 0; curSys < num_systems; curSys++) {
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

                ++laneSetIter;
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
                if (!ConnectedWithin(lane.first, lane.second, num_systems, laneSetArray)) {
                    laneSetArray[lane.first].insert(lane.second);
                    laneSetArray[lane.second].insert(lane.first);
                }
            }
            ++lanesToRemoveIter;
        }
    }
}

void GenerateStarlanes(int max_jumps_between_systems, int max_starlane_length) {
    int num_systems, s1, s2, s3; // numbers of systems, indices in vec_sys
    int n; // loop counter

    std::vector<int> triVerts;  // indices of stars that form vertices of a triangle

    // array of set to store final, included starlanes for each star
    std::vector<std::set<int>> laneSetArray;

    // array of set to store possible starlanes for each star, as extracted form triangulation
    std::vector<std::set<int>> potential_lane_set_array;

    // iterators for traversing lists of starlanes
    std::set<int>::iterator laneSetIter, laneSetEnd, laneSetIter2, laneSetEnd2;

    // get systems
    std::vector<std::shared_ptr<System>> sys_vec = Objects().FindObjects<System>();
    num_systems = sys_vec.size();  // (actually = number of systems + 1)

    // pass systems to Delauney Triangulation routine, getting array of triangles back
    std::list<Delauney::DTTriangle>* triangle_list = Delauney::DelauneyTriangulate(sys_vec);
    if (!triangle_list ||triangle_list->empty()) {
        ErrorLogger() << "Got no list or blank list of triangles from Triangulation.";
        return;
    }

    Delauney::DTTriangle tri;
    // initialize arrays...
    potential_lane_set_array.resize(num_systems);
    laneSetArray.resize(num_systems);

    // extract triangles from list, add edges to sets of potential starlanes for each star (in array)
    while (!triangle_list->empty()) {
        tri = triangle_list->front();

        triVerts = tri.Verts();
        s1 = triVerts[0];
        s2 = triVerts[1];
        s3 = triVerts[2];

        // add starlanes to list of potential starlanes for each star, making sure each pair involves
        // only stars that actually exist.  triangle generation uses three extra points which don't
        // represent actual systems and which need to be weeded out here.
        if ((s1 >= 0) && (s2 >= 0) && (s3 >= 0)) {
            if ((s1 < num_systems) && (s2 < num_systems)) {
                potential_lane_set_array[s1].insert(s2);
                potential_lane_set_array[s2].insert(s1);
            }
            if ((s1 < num_systems) && (s3 < num_systems)) {
                potential_lane_set_array[s1].insert(s3);
                potential_lane_set_array[s3].insert(s1);
            }
            if ((s2 < num_systems) && (s3 < num_systems)) {
                potential_lane_set_array[s2].insert(s3);
                potential_lane_set_array[s3].insert(s2);
            }
        }

        triangle_list->pop_front();
    }

    // cleanup
    delete triangle_list;

    //DebugLogger() << "Extracted Potential Starlanes from Triangulation";

    CullTooLongLanes(max_starlane_length, potential_lane_set_array, sys_vec);

    CullAngularlyTooCloseLanes(0.98, potential_lane_set_array, sys_vec);

    //DebugLogger() << "Culled Agularly Too Close Lanes";

    laneSetArray = potential_lane_set_array;

    // attempt removing lanes, but don't do so if it would make the systems
    // the lane connects too far apart
    for (n = 0; n < num_systems; ++n) {
        laneSetIter = potential_lane_set_array[n].begin();

        while (laneSetIter != potential_lane_set_array[n].end()) {
            s1 = *laneSetIter;

            // try removing lane
            laneSetArray[n].erase(s1);
            laneSetArray[s1].erase(n);

            if (!ConnectedWithin(n, s1, max_jumps_between_systems, laneSetArray)) {
                // lane removal was a bad idea.  restore it
                laneSetArray[n].insert(s1);
                laneSetArray[s1].insert(n);
            }

            ++laneSetIter;
        } // end while
    }

    // add the starlane to the stars
    for (n = 0; n < num_systems; ++n) {
        for (int system_idx : laneSetArray[n])
            sys_vec[n]->AddStarlane(sys_vec[system_idx]->ID()); // System::AddStarlane() expects a system ID
    }

    DebugLogger() << "Initializing System Graph";
    GetUniverse().InitializeSystemGraph();
}

void SetActiveMetersToTargetMaxCurrentValues(ObjectMap& object_map) {
    std::map<MeterType, MeterType> meters;
    meters[METER_POPULATION] =   METER_TARGET_POPULATION;
    meters[METER_INDUSTRY] =     METER_TARGET_INDUSTRY;
    meters[METER_RESEARCH] =     METER_TARGET_RESEARCH;
    meters[METER_TRADE] =        METER_TARGET_TRADE;
    meters[METER_CONSTRUCTION] = METER_TARGET_CONSTRUCTION;
    meters[METER_HAPPINESS] =    METER_TARGET_HAPPINESS;
    meters[METER_FUEL] =         METER_MAX_FUEL;
    meters[METER_SHIELD] =       METER_MAX_SHIELD;
    meters[METER_STRUCTURE] =    METER_MAX_STRUCTURE;
    meters[METER_DEFENSE] =      METER_MAX_DEFENSE;
    meters[METER_TROOPS] =       METER_MAX_TROOPS;
    meters[METER_SUPPLY] =       METER_MAX_SUPPLY;

    // check for each pair of meter types.  if both exist, set active
    // meter current value equal to target meter current value.
    for (const auto& object : object_map) {
        for (auto& entry : meters)
            if (Meter* meter = object->GetMeter(entry.first))
                if (Meter* targetmax_meter = object->GetMeter(entry.second))
                    meter->SetCurrent(targetmax_meter->Current());
    }
}

void SetNativePopulationValues(ObjectMap& object_map) {
    for (const auto& object : object_map) {
        Meter* meter = object->GetMeter(METER_POPULATION);
        Meter* targetmax_meter = object->GetMeter(METER_TARGET_POPULATION);
        // only applies to unowned planets
        if (meter && targetmax_meter && object->Unowned()) {
            double r = RandZeroToOne();
            double factor = (0.1 < r) ? r : 0.1;
            meter->SetCurrent(targetmax_meter->Current() * factor);
        }
    }
}

bool SetEmpireHomeworld(Empire* empire, int planet_id, std::string species_name) {
    // get home planet and system, check if they exist
    std::shared_ptr<Planet> home_planet = GetPlanet(planet_id);
    std::shared_ptr<System> home_system;
    if (home_planet)
        home_system = GetSystem(home_planet->SystemID());
    if (!home_planet || !home_system) {
        ErrorLogger() << "SetEmpireHomeworld: couldn't get homeworld or system for empire" << empire->EmpireID();
        return false;
    }

    DebugLogger() << "SetEmpireHomeworld: setting system " << home_system->ID()
                  << " (planet " <<  home_planet->ID() << ") to be home system for empire " << empire->EmpireID();

    // get species, check if it exists
    Species* species = GetSpeciesManager().GetSpecies(species_name);
    if (!species) {
        ErrorLogger() << "SetEmpireHomeworld: couldn't get species \""
                      << species_name << "\" to set with homeworld id " << home_planet->ID();
        return false;
    }

    // set homeword's planet type to the preferred type for this species
    const std::map<PlanetType, PlanetEnvironment>& spte = species->PlanetEnvironments();
    if (!spte.empty()) {
        // invert map from planet type to environments to map from
        // environments to type, sorted by environment
        std::map<PlanetEnvironment, PlanetType> sept;
        for (const auto& entry : spte)
            sept[entry.second] = entry.first;
        // assuming enum values are ordered in increasing goodness...
        PlanetType preferred_planet_type = sept.rbegin()->second;

        // both the current as well as the original type need to be set to the preferred type
        home_planet->SetType(preferred_planet_type);
        home_planet->SetOriginalType(preferred_planet_type);
        // set planet size according to planet type
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
    DebugLogger() << "Initializing " << player_setup_data.size() << " empires";

    // copy empire colour table, so that individual colours can be removed after they're used
    auto colors = EmpireColors();

    // create empire objects and do some basic initilization for each player
    for (const auto& entry : player_setup_data) {
        int         player_id =     entry.first;
        if (player_id == Networking::INVALID_PLAYER_ID)
            ErrorLogger() << "InitEmpires player id (" << player_id << ") is invalid";

        // use player ID for empire ID so that the calling code can get the
        // correct empire for each player ID  in player_setup_data
        int         empire_id =     player_id;
        std::string player_name =   entry.second.m_player_name;
        GG::Clr     empire_colour = entry.second.m_empire_color;

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
        std::string empire_name = UserString("EMPIRE") + std::to_string(empire_id);

        DebugLogger() << "Universe::InitEmpires creating new empire" << " with ID: " << empire_id
                      << " for player: " << player_name << " (with player id: " << player_id << ")";

        // create new Empire object through empire manager
        Empires().CreateEmpire(empire_id, empire_name, player_name, empire_colour);
    }

    Empires().ResetDiplomacy();
}
