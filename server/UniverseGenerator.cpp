#include "UniverseGenerator.h"

#include "../util/Random.h"
#include "../util/i18n.h"
#include "../util/MultiplayerCommon.h"
#include "../util/GameRules.h"
#include "../util/AppInterface.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Species.h"

#include "../util/ScopedTimer.h"

#include <boost/container/flat_map.hpp>

#include <limits>

namespace {
    DeclareThreadSafeLogger(effects);
}

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
        DTPoint() = default;
        DTPoint(double x_, double y_) :
            x(x_),
            y(y_)
        {}
        double x = 0.0;
        double y = 0.0;
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
        DTTriangle() = default;
        DTTriangle(int vert1, int vert2, int vert3,
                   const std::vector<Delauney::DTPoint>& points);

        ///< determines whether a specified point is within the circumcircle of the triangle
        bool PointInCircumCircle(const Delauney::DTPoint& p);
        const std::array<int, 3>& Verts() {return verts;}

    private:
        std::array<int, 3> verts{0, 0, 0};   ///< indices of vertices of triangle
        Delauney::DTPoint  centre{0.0, 0.0}; ///< location of circumcentre of triangle
        double             radius2{0.0};     ///< radius of circumcircle squared
    };

    DTTriangle::DTTriangle(int vert1, int vert2, int vert3,
                           const std::vector<Delauney::DTPoint>& points)
    {
        if (vert1 == vert2 || vert1 == vert3 || vert2 == vert3)
            throw std::runtime_error("Attempted to create Triangle with two of the same vertex indices.");

        // record indices of vertices of triangle
        verts = {vert1, vert2, vert3};

        // extract position info for vertices
        const double& x1 = points[vert1].x;
        const double& x2 = points[vert2].x;
        const double& x3 = points[vert3].x;
        const double& y1 = points[vert1].y;
        const double& y2 = points[vert2].y;
        const double& y3 = points[vert3].y;

        // calculate circumcircle and circumcentre of triangle
        double a, Sx, Sy, b;

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

    bool DTTriangle::PointInCircumCircle(const Delauney::DTPoint& p) {
        double vectX = p.x - centre.x;
        double vectY = p.y - centre.y;
        return (vectX*vectX + vectY*vectY < radius2);
    };



    /** Runs a Delauney Triangulation on a set of 2D points corresponding
      * to the locations of the systems in \a systems_vec */
    std::list<Delauney::DTTriangle> DelauneyTriangulate(
        const std::vector<std::shared_ptr<const System>>& systems_vec, double universe_width)
    {
        // ensure a useful list of systems was passed...
        if (systems_vec.empty()) {
            ErrorLogger() << "Attempted to run Delauney Triangulation on empty array of systems";
            return std::list<Delauney::DTTriangle>();
        }


        // extract systems positions from system objects.
        std::vector<Delauney::DTPoint> points_vec;
        points_vec.reserve(systems_vec.size() + 3);
        for (auto& system : systems_vec)
            points_vec.push_back({system->X(), system->Y()});

        // entries in points_vec correspond to entries in \a systems_vec
        // so that the index of an item in systems_vec will have a
        // corresponding points at that index in points_vec


        // add points for covering triangle. the point positions should be big
        // enough to form a triangle that encloses all the points in points_vec
        // (or at least one whose circumcircle covers all points)
        points_vec.push_back({-1.0, -1.0});
        points_vec.push_back({2.0 * (universe_width + 1.0), -1.0});
        points_vec.push_back({-1.0, 2.0 * (universe_width + 1.0)});


        // initialize triangle_list.
        // add last three points into the first triangle, the "covering triangle"
        std::list<Delauney::DTTriangle> triangle_list;
        int num_points_in_vec = points_vec.size();
        triangle_list.push_front({num_points_in_vec - 1, num_points_in_vec - 2,
                                  num_points_in_vec - 3, points_vec});

        if (points_vec.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            ErrorLogger() << "Attempted to run Delauney Triangulation on " << points_vec.size()
                          << " points.  The limit is " << std::numeric_limits<int>::max();
            return std::list<Delauney::DTTriangle>();
        }

        // loop through points generated from systems, excluding the final
        // 3 points added for the covering triangle
        for (std::size_t n = 0; n < points_vec.size() - 3; n++) {
            // list of indices in vector of points extracted from removed
            // triangles that need to be retriangulated
            std::list<Delauney::SortValInt> point_idx_list;

            const auto& cur_point = points_vec.at(n);

            // check each triangle to see if the current point lies in its
            // circumcircle.  if so, delete the triangle and add its vertices to a list
            auto cur_tri_it = triangle_list.begin();
            while (cur_tri_it != triangle_list.end()) {
                // get current triangle
                Delauney::DTTriangle& tri = *cur_tri_it;

                // check if point to be added to triangulation is within the
                // circumcircle for the current triangle
                if (!tri.PointInCircumCircle(cur_point)) {
                    // point not in circumcircle for this triangle.
                    // go next triangle in list
                    ++cur_tri_it;
                    continue;
                }

                // point is in circumcircle for this triangle.
                // insert the triangle's vertices' indices into the
                // list.  add in sorted position based on angle of direction
                // to current point n being inserted.  don't add if doing so
                // would duplicate an index already in the list
                for (int tri_vert_idx : tri.Verts()) {

                    // get sorting value to order points clockwise
                    // circumferentially around point n

                    // vector from point n to current point
                    double vx = points_vec[tri_vert_idx].x - points_vec[n].x;
                    double vy = points_vec[tri_vert_idx].y - points_vec[n].y;
                    double mag = std::sqrt(vx*vx + vy*vy);
                    // normalize
                    vx /= mag;
                    vy /= mag;

                    // dot product with (0, 1) is vy, magnitude of cross
                    // product is vx this gives a range of "sort value"
                    // from -2 to 2, around the circle
                    double sort_value = (vx >= 0) ? (vy + 1) : (-vy - 1);

                    // iterate through list, finding insert spot and
                    // verifying uniqueness (or add if list is empty)
                    auto idx_list_it = point_idx_list.begin();
                    if (idx_list_it == point_idx_list.end()) {
                        // list is empty
                        point_idx_list.push_back({tri_vert_idx, sort_value});

                    } else {
                        while (idx_list_it != point_idx_list.end()) {
                            if (idx_list_it->num == tri_vert_idx)
                                break;
                            if (idx_list_it->sortVal > sort_value) {
                                point_idx_list.insert(idx_list_it, {tri_vert_idx, sort_value});
                                break;
                            }
                            ++idx_list_it;
                        }
                        if (idx_list_it == point_idx_list.end()) {
                            // point wasn't added, so should go at end
                            point_idx_list.push_back({tri_vert_idx, sort_value});
                        }
                    }
                } // end for c

                // remove current triangle from list of triangles
                cur_tri_it = triangle_list.erase(cur_tri_it);
            } // end while


            // add triangle for last and first points and n
            triangle_list.push_front(
                {static_cast<int>(n), (point_idx_list.front()).num, (point_idx_list.back()).num, points_vec});


            // go through list of points, making new triangles out of them
            auto idx_list_it = point_idx_list.begin();
            int num = idx_list_it->num;
            ++idx_list_it;
            while (idx_list_it != point_idx_list.end()) {
                int num2 = num;
                num = idx_list_it->num;

                triangle_list.push_front({static_cast<int>(n), num2, num, points_vec});

                ++idx_list_it;
            } // end while

        } // end for

        DebugLogger() << "DelauneyTriangulate generated list of "
                      << triangle_list.size() << " triangles";

        return triangle_list;
    }
}

namespace {
    int IntSetMapSizeCount(const std::map<int, std::set<int>>& in) {
        int retval{0};
        for (const auto& entry : in)
            retval += entry.second.size();
        return retval;
    }

    /** Used by GenerateStarlanes.  Determines if two systems are connected by
      * maxLaneJumps or less edges on graph. */
    bool ConnectedWithin(int system1, int system2, int maxLaneJumps,
                         const std::map<int, std::set<int>>& system_lanes)
    {
        // check for simple cases for quick termination
        if (system1 == system2) return true; // system is always connected to itself
        if (0 == maxLaneJumps) return false; // no system is connected to any other system by less than 1 jump

        if (!system_lanes.contains(system1)) return false;     // start system not in lanes map
        if (!system_lanes.contains(system2)) return false;     // destination system not in lanes map

        if (system_lanes.at(system1).empty()) return false; // no lanes out of start system
        if (system_lanes.at(system2).empty()) return false; // no lanes out of destination system

        // list of indices of systems that are accessible from previously visited systems.
        // when a new system is found to be accessible, it is added to the back of the
        // list.  the list is iterated through from front to back to find systems
        // to examine
        std::vector<int> accessibleSystemsList;
        accessibleSystemsList.reserve(system_lanes.size());

        // Map from system_id to the number of starlane jumps away from system1
        // that sytsem is. This indicates the depth of the search, which allows
        // the serch to terminate after searching to the depth of maxLaneJumps
        // without finding system2
        boost::container::flat_map<int, int> accessibleSystemsMap;
        accessibleSystemsMap.reserve(system_lanes.size());
        for (const auto sys : system_lanes | range_keys)
            accessibleSystemsMap.emplace_hint(accessibleSystemsMap.end(), sys, -1);

        static constexpr int mapped_type_max = std::numeric_limits<decltype(accessibleSystemsMap)::mapped_type>::max();
        maxLaneJumps = std::min(maxLaneJumps, mapped_type_max);

        // add starting system to list and set of accessible systems
        accessibleSystemsList.push_back(system1);
        accessibleSystemsMap[system1] = 0;


        // loop through visited systems
        for (std::size_t idx = 0; idx < accessibleSystemsList.size(); ++idx) {
            auto cur_sys_id = accessibleSystemsList[idx];
            // check that iteration hasn't reached maxLaneJumps levels deep, which would
            // mean that system2 isn't within maxLaneJumps starlane jumps of system1
            auto curDepth = accessibleSystemsMap[cur_sys_id];
            if (curDepth >= maxLaneJumps) return false;

            // get set of starlanes for this system
            for (auto curLaneDest : system_lanes.at(cur_sys_id)) {
                // check for goal
                if (curLaneDest == system2) return true;

                auto& cur_lane_dest_depth = accessibleSystemsMap[curLaneDest];
                if (cur_lane_dest_depth < 0) {
                    // this lane was not yet considered, so update its depth in map and add to list ot consider
                    accessibleSystemsList.push_back(curLaneDest);
                    cur_lane_dest_depth = curDepth + 1;
                }
            }
        }
        return false; // default
    }

    /** Removes lanes from passed graph that are angularly too close to
      * each other. */
    void CullAngularlyTooCloseLanes(double max_lane_uvect_dot_product,
                                    std::map<int, std::set<int>>& system_lanes,
                                    const std::map<int, std::shared_ptr<const System>>& systems)
    {
        // 2 component vector and vect + magnitude typedefs
        typedef std::pair<std::pair<double, double>, double> VectAndMagTypeQQ;

        std::set<std::pair<int, int>> lanesToRemoveSet;  // start and end stars of lanes to be removed in final step...

        // make sure data is consistent
        if (system_lanes.size() != systems.size()) {
            ErrorLogger() << "CullAngularlyTooCloseLanes got different size vectors of lane sets and systems.  Doing nothing.";
            return;
        }

        if (systems.size() < 3) return;  // nothing worth doing for less than three systems

        //DebugLogger() << "Culling Too Close Angularly Lanes";

        // loop through systems
        for (const auto& entry : systems) {
            // can't have pairs of lanes departing from a system if that system
            // has less than two lanes
            if (system_lanes.at(entry.first).size() < 2)
                continue;

            // get position of current system (for use in calculated vectors)
            auto startX = entry.second->X();
            auto startY = entry.second->Y();
            auto cur_sys_id = entry.first;

            /** componenets of vectors of lanes of current system, indexed by destination system number */
            std::map<int, VectAndMagTypeQQ> laneVectsMap;

            // get unit vectors for all lanes of this system
            auto laneSetIter1 = system_lanes[cur_sys_id].begin();
            while (laneSetIter1 != system_lanes[cur_sys_id].end()) {
                // get destination for this lane
                auto dest1 = *laneSetIter1;
                // get vector to this lane destination
                auto vectX1 = systems.at(dest1)->X() - startX;
                auto vectY1 = systems.at(dest1)->Y() - startY;
                // normalize
                auto mag1 = std::sqrt(vectX1 * vectX1 + vectY1 * vectY1);
                vectX1 /= mag1;
                vectY1 /= mag1;

                // store lane in map of lane vectors
                laneVectsMap.emplace(dest1, VectAndMagTypeQQ{{vectX1, vectY1}, mag1});

                ++laneSetIter1;
            }

            // iterate through lanes of cur_sys_id
            laneSetIter1 = system_lanes[cur_sys_id].begin();
            ++laneSetIter1;  // start at second, since iterators are used in pairs, and starting both at the first wouldn't be a valid pair
            while (laneSetIter1 != system_lanes[cur_sys_id].end()) {
                // get destination of current starlane
                const auto& dest1 = *laneSetIter1;

                auto lane1{(cur_sys_id < dest1) ?
                    std::pair{cur_sys_id, dest1} : std::pair{dest1, cur_sys_id}};

                // check if this lane has already been added to the set of lanes to remove
                if (lanesToRemoveSet.contains(lane1)) {
                    ++laneSetIter1;
                    continue;
                }

                // extract data on starlane vector...
                auto laneVectsMapIter = laneVectsMap.find(dest1);
                assert(laneVectsMapIter != laneVectsMap.end());
                const auto& [temp_vec, mag1] = laneVectsMapIter->second;
                const auto& [vectX1, vectY1] = temp_vec;

                // iterate through other lanes of cur_sys_id, in order
                // to get all possible pairs of lanes
                auto laneSetIter2 = system_lanes[cur_sys_id].begin();
                while (laneSetIter2 != laneSetIter1) {
                    auto dest2 = *laneSetIter2;

                    std::pair<int, int> lane2;
                    if (cur_sys_id < dest2)
                        lane2 = {cur_sys_id, dest2};
                    else
                        lane2 = {dest2, cur_sys_id};

                    // check if this lane has already been added to the
                    // set of lanes to remove
                    if (lanesToRemoveSet.contains(lane2)) {
                        ++laneSetIter2;
                        continue;
                    }

                    // extract data on starlane vector...
                    laneVectsMapIter = laneVectsMap.find(dest2);
                    assert(laneVectsMapIter != laneVectsMap.end());
                    const auto& [temp_vect2, mag2] = laneVectsMapIter->second;
                    const auto& [vectX2, vectY2] = temp_vect2;

                    // find dot product
                    auto dotProd = vectX1 * vectX2 + vectY1 * vectY2;

                    // if dotProd is big enough, then lanes are too close angularly
                    // thus one needs to be removed.
                    if (dotProd > max_lane_uvect_dot_product) {
                        // preferentially remove the longer lane
                        if (mag1 > mag2) {
                            lanesToRemoveSet.insert(lane1);
                            break;  // don't need to check any more lanes against lane1, since lane1 has been removed
                        } else {
                            lanesToRemoveSet.insert(lane2);
                        }
                    }

                    ++laneSetIter2;
                }

                ++laneSetIter1;
            }
        }

        // iterate through set of lanes to remove, and remove them in turn...
        auto lanes_to_remove_it = lanesToRemoveSet.begin();
        auto lanes_to_remove_end = lanesToRemoveSet.end();
        while (lanes_to_remove_it != lanes_to_remove_end) {
            const auto& [l_start, l_end] = *lanes_to_remove_it;

            system_lanes[l_start].erase(l_end);
            system_lanes[l_end].erase(l_start);

            // check that removing lane hasn't disconnected systems
            if (!ConnectedWithin(l_start, l_end, systems.size(), system_lanes)) {
                // they aren't connected... reconnect them
                system_lanes[l_start].insert(l_end);
                system_lanes[l_end].insert(l_start);
            }

            ++lanes_to_remove_it;
        }
    }

    /** Removes lanes from passed graph that are too long. */
    void CullTooLongLanes(double max_lane_length,
                          std::map<int, std::set<int>>& system_lanes,
                          const std::map<int, std::shared_ptr<const System>>& systems)
    {
        DebugLogger() << "CullTooLongLanes max lane length: " << max_lane_length
                      << "  potential lanes: " << IntSetMapSizeCount(system_lanes)
                      << "  systems: " << systems.size();

        ScopedTimer timer("CullTooLongLanes", std::chrono::milliseconds{1});

        // map, indexed by lane length, of start and end stars of lanes to be removed
        std::multimap<double, std::pair<int, int>, std::greater<double>> lanes_to_remove;

        // make sure data is consistent
        if (system_lanes.size() != systems.size())
            return;
        // nothing worth doing for less than two systems (no lanes!)
        if (systems.size() < 2)
            return;

        // get squared max lane lenth, so as to eliminate the need to take square roots of lane lenths...
        const double max_lane_length2 = max_lane_length*max_lane_length;
        std::size_t total_lanes_count = 0;

        // loop through systems and lanes to find long lanes
        for (const auto& [cur_sys_id, cur_system] : systems) {
            // get position of current system (for use in calculating vector)
            double startX = cur_system->X();
            double startY = cur_system->Y();

            // iterate through all lanes in system, checking lengths and
            // marking to be removed if necessary
            for (const auto dest_sys_id : system_lanes[cur_sys_id]) {
                total_lanes_count++;

                // convert start and end into ordered pair to represent lane
                std::pair<int, int> lane;
                if (cur_sys_id < dest_sys_id)
                    lane = {cur_sys_id, dest_sys_id};
                else
                    lane = {dest_sys_id, cur_sys_id};

                // get vector to this lane destination
                const auto& dest_system = systems.at(dest_sys_id);
                auto vectX = dest_system->X() - startX;
                auto vectY = dest_system->Y() - startY;

                // compare magnitude of vector to max allowed
                double lane_length2 = vectX*vectX + vectY*vectY;
                if (lane_length2 > max_lane_length2) {
                    // lane is too long!  mark it to be removed
                    TraceLogger() << "CullTooLongLanes wants to remove lane of length "
                                  << std::sqrt(lane_length2)
                                  << " between systems with ids: "
                                  << cur_sys_id << " and " << dest_sys_id;
                    lanes_to_remove.emplace(lane_length2, lane);
                }
            }
        }

        DebugLogger() << "CullTooLongLanes identified " << lanes_to_remove.size()
                      << " long lanes to possibly remove of " << total_lanes_count << " lanes total";

        std::size_t total_checked_lanes = 0;
        std::size_t removable_lanes = 0;

        // TODO: Consider re-implementing this as a minimal spanning tree
        //       and immediately remove any lane not in that tree, and
        //       thus avoid having to check connectivity for each
        //       potential lane removal

        // Iterate through set of lanes to remove, and remove them in turn.
        // Lanes are sorted by length, so are processed longest-first
        for (const auto& [dist, lane] : lanes_to_remove) {
            if (!system_lanes[lane.first].contains(lane.second) ||
                !system_lanes[lane.second].contains(lane.first))
            { continue; }

            // remove lane
            system_lanes[lane.first].erase(lane.second);
            system_lanes[lane.second].erase(lane.first);

            total_checked_lanes++;

            // if removing lane has disconnected systems, reconnect them
            if (!ConnectedWithin(lane.first, lane.second, systems.size(), system_lanes)) {
                system_lanes[lane.first].insert(lane.second);
                system_lanes[lane.second].insert(lane.first);
                TraceLogger() << "CullTooLongLanes can't remove lane between systems with ids: "
                              << lane.first << " and " << lane.second
                              << " because they would then be disconnected (more than "
                              << systems.size() << " jumps apart)";
            } else {
                removable_lanes++;
                TraceLogger() << "CullTooLongLanes removing lane between systems with ids: "
                              << lane.first << " and " << lane.second;
            }
        }

        DebugLogger() << "CullTooLongLanes left with " << IntSetMapSizeCount(system_lanes)
                      << " lanes.  " << total_checked_lanes << " were checked and "
                      << removable_lanes << " were removable and "
                      << total_checked_lanes - removable_lanes << " were not removable";
    }
}

void GenerateStarlanes(int max_jumps_between_systems, int max_starlane_length,
                       Universe& universe, const EmpireManager& empires)
{
    DebugLogger() << "GenerateStarlanes  max jumps b/w sys: " << max_jumps_between_systems
                  << "  max lane length: " << max_starlane_length;

    std::vector<int> triangle_vertices;  // indices of stars that form vertices of a triangle

    // array of set to store final, included starlanes for each star
    std::map<int, std::set<int>> system_lanes;

    // array of set to store possible starlanes for each star,
    // as extracted form triangulation
    std::map<int, std::set<int>> potential_system_lanes;

    // get systems
    auto sys_rng = std::as_const(universe.Objects()).all<System>();
    std::vector<std::shared_ptr<const System>> sys_vec{sys_rng.begin(), sys_rng.end()};
    std::map<int, std::shared_ptr<const System>> sys_map;
    std::transform(sys_rng.begin(), sys_rng.end(), std::inserter(sys_map, sys_map.end()),
                   [](const std::shared_ptr<const System>& p) { return std::pair{p->ID(), p}; });

    // generate lanes
    if (GetGameRules().Get<bool>("RULE_STARLANES_EVERYWHERE")) {
        // if the lanes everywhere rules is true, add starlanes to every star
        // to every other star...
        for (const auto& sys1 : sys_vec) {
            for (const auto& sys2 : sys_vec) {
                if (sys1->ID() == sys2->ID())
                    continue;
                potential_system_lanes[sys1->ID()].insert(sys2->ID());
            }
        }
        DebugLogger() << "Generated " << IntSetMapSizeCount(potential_system_lanes) << " potential starlanes between all system pairs";
        CullTooLongLanes(max_starlane_length, potential_system_lanes, sys_map);
        DebugLogger() << "Left with " << IntSetMapSizeCount(potential_system_lanes) << " potential starlanes after length culling";
        system_lanes = potential_system_lanes;

    } else {
        // pass systems to Delauney Triangulation routine, getting array of triangles back
        auto triangle_list = Delauney::DelauneyTriangulate(sys_vec, universe.UniverseWidth());
        if (triangle_list.empty()) {
            ErrorLogger() << "Got blank list of triangles from Triangulation.";
            return;
        }

        // extract triangles from list, add edges to sets of potential starlanes
        // for each star (in array)
        while (!triangle_list.empty()) {
            // extract indices for the corners of the triangles, which should
            // correspond to indices in sys_vec, except that there can also be
            // indices up to sys_vec.size() + 2, which correspond to extra points
            // used by the algorithm
            const auto& [s1, s2, s3] = triangle_list.front().Verts();

            if (s1 < 0 || s2 < 0 || s3 < 0) {
                ErrorLogger() << "Got negative vector indices from DelauneyTriangulate!";
                triangle_list.pop_front();
                continue;
            }

            // get system ids for ends of lanes from the sys_vec indices
            int sys1_id = INVALID_OBJECT_ID;
            if (static_cast<std::size_t>(s1) < sys_vec.size())
                sys1_id = sys_vec.at(s1)->ID();
            int sys2_id = INVALID_OBJECT_ID;
            if (static_cast<std::size_t>(s2) < sys_vec.size())
                sys2_id = sys_vec.at(s2)->ID();
            int sys3_id = INVALID_OBJECT_ID;
            if (static_cast<std::size_t>(s3) < sys_vec.size())
                sys3_id = sys_vec.at(s3)->ID();


            // add starlanes to list of potential starlanes for each star,
            // making sure each pair involves only valid indices into sys_vec
            if (sys1_id != INVALID_OBJECT_ID && sys2_id != INVALID_OBJECT_ID) {
                potential_system_lanes[sys1_id].insert(sys2_id);
                potential_system_lanes[sys2_id].insert(sys1_id);
            }
            if (sys2_id != INVALID_OBJECT_ID && sys3_id != INVALID_OBJECT_ID) {
                potential_system_lanes[sys2_id].insert(sys3_id);
                potential_system_lanes[sys3_id].insert(sys2_id);
            }
            if (sys3_id != INVALID_OBJECT_ID && sys1_id != INVALID_OBJECT_ID) {
                potential_system_lanes[sys3_id].insert(sys1_id);
                potential_system_lanes[sys1_id].insert(sys3_id);
            }

            triangle_list.pop_front();
        }

        DebugLogger() << "Extracted " << IntSetMapSizeCount(potential_system_lanes) << " potential starlanes from triangulation";
        CullTooLongLanes(max_starlane_length, potential_system_lanes, sys_map);
        DebugLogger() << "Left with " << IntSetMapSizeCount(potential_system_lanes) << " potential starlanes after length culling";
        CullAngularlyTooCloseLanes(0.98, potential_system_lanes, sys_map);
        DebugLogger() << "Left with " << IntSetMapSizeCount(potential_system_lanes) << " potential starlanes after angular culling";

        system_lanes = potential_system_lanes;

        // attempt removing lanes, but don't do so if it would make the systems
        // the lane connects too far apart
        for (const auto& [sys1_id, sys1_lanes] : potential_system_lanes) {
            for (auto& sys2_id : sys1_lanes) {
                if (sys2_id <= sys1_id)
                    continue; // skip lanes that should already have been checked since lanes exist in both directions

                // try removing lane
                system_lanes[sys1_id].erase(sys2_id);
                system_lanes[sys2_id].erase(sys1_id);

                if (!ConnectedWithin(sys1_id, sys2_id, max_jumps_between_systems, system_lanes)) {
                    // lane removal was a bad idea.  restore it
                    system_lanes[sys1_id].insert(sys2_id);
                    system_lanes[sys2_id].insert(sys1_id);
                }
            }
        }
    }

    // add the starlane to the stars
    for (auto& sys : universe.Objects().all<System>()) {
        const auto& sys_lanes = system_lanes[sys->ID()];
        for (auto& lane_end_id : sys_lanes)
            sys->AddStarlane(lane_end_id);
    }

    DebugLogger() << "Initializing System Graph";
    universe.InitializeSystemGraph(empires);
}

void SetActiveMetersToTargetMaxCurrentValues(ObjectMap& object_map) {
    TraceLogger(effects) << "SetActiveMetersToTargetMaxCurrentValues";
    // check for each pair of meter types.  if both exist, set active
    // meter current value equal to target meter current value.
    for (const auto& object : object_map.all()) {
        for (auto& [meter_type, assoc_type] : AssociatedMeterTypes()) {
            Meter* meter = object->GetMeter(meter_type);
            Meter* targetmax_meter = object->GetMeter(assoc_type);
            if (meter && targetmax_meter)
                meter->SetCurrent(targetmax_meter->Current());
        }
    }
}

void SetNativePopulationValues(ObjectMap& object_map) {
    for (const auto& object : object_map.all()) {
        Meter* meter = object->GetMeter(MeterType::METER_POPULATION);
        Meter* targetmax_meter = object->GetMeter(MeterType::METER_TARGET_POPULATION);
        // only applies to unowned planets
        if (meter && targetmax_meter && object->Unowned()) {
            double r = RandZeroToOne();
            double factor = (0.1 < r) ? r : 0.1;
            meter->SetCurrent(targetmax_meter->Current() * factor);
        }
    }
}

bool SetEmpireHomeworld(Empire* empire, int planet_id, std::string species_name,
                        ScriptingContext& context)
{
    // get home planet and system, check if they exist
    auto home_planet = context.ContextObjects().get<Planet>(planet_id);
    if (!home_planet)
        return false;
    auto home_system = context.ContextObjects().get<System>(home_planet->SystemID());
    if (!home_system)
        return false;

    DebugLogger() << "SetEmpireHomeworld: setting system " << home_system->ID()
                  << " (planet " <<  home_planet->ID() << ") to be home system for empire " << empire->EmpireID();

    // get species, check if it exists
    auto species = context.species.GetSpecies(species_name);
    if (!species) {
        ErrorLogger() << "SetEmpireHomeworld: couldn't get species \""
                      << species_name << "\" to set with homeworld id " << home_planet->ID();
        return false;
    }

    // set homeword's planet type to the preferred type for this species
    const auto& spte = species->PlanetEnvironments();
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
        if (preferred_planet_type == PlanetType::PT_ASTEROIDS)
            home_planet->SetSize(PlanetSize::SZ_ASTEROIDS);
        else if (preferred_planet_type == PlanetType::PT_GASGIANT)
            home_planet->SetSize(PlanetSize::SZ_GASGIANT);
        else
            home_planet->SetSize(PlanetSize::SZ_MEDIUM);
    }

    home_planet->Colonize(empire->EmpireID(), species_name, Meter::LARGE_VALUE, context);
    context.species.AddSpeciesHomeworld(std::move(species_name), home_planet->ID());

    empire->SetCapitalID(home_planet->ID(), context.ContextObjects());
    context.Empires().RefreshCapitalIDs();
    empire->AddExploredSystem(home_planet->SystemID(), BEFORE_FIRST_TURN, context.ContextObjects());

    return true;
}

void InitEmpires(const std::map<int, PlayerSetupData>& player_setup_data, EmpireManager& empires) {
    DebugLogger() << "Initializing " << player_setup_data.size() << " empires";

    // copy empire colour table, so that individual colours can be removed after they're used
    auto colors{EmpireColors()};

    // create empire objects and do some basic initilization for each player
    for (auto& [empire_id, psd] : player_setup_data) {
        // use map key for empire ID so that the calling code can get the
        // correct empire for each player in player_setup_data
        if (empire_id == ALL_EMPIRES)
            ErrorLogger() << "InitEmpires empire id (" << empire_id << ") is invalid";

        const auto& player_name =   psd.player_name;
        auto        empire_colour = psd.empire_color;
        bool        authenticated = psd.authenticated;

        // validate or generate empire colour
        // ensure no other empire gets auto-assigned this colour automatically
        auto color_it = std::find(colors.begin(), colors.end(), empire_colour);
        if (color_it != colors.end())
            colors.erase(color_it);

        static constexpr EmpireColor CLR_ZERO{{0, 0, 0, 0}};

        // if no colour already set, do so automatically
        if (empire_colour == CLR_ZERO) {
            if (!colors.empty()) {
                // take next colour from list
                empire_colour = colors[0];
                colors.erase(colors.begin());
            } else {
                // as a last resort, make up a colour
                empire_colour = {{static_cast<uint8_t>(RandInt(0, 255)),
                                  static_cast<uint8_t>(RandInt(0, 255)),
                                  static_cast<uint8_t>(RandInt(0, 255)),
                                  255}};
            }
        }

        // set generic default empire name
        std::string empire_name = UserString("EMPIRE") + std::to_string(empire_id);

        DebugLogger() << "Universe::InitEmpires creating new empire" << " with ID: " << empire_id
                      << " for player: " << player_name << " in team: " << psd.starting_team;

        // create new Empire object through empire manager
        empires.CreateEmpire(empire_id, std::move(empire_name), player_name,
                             empire_colour, authenticated);
    }

    empires.ResetDiplomacy();

    for (auto& [player_id1, psd1] : player_setup_data) {
        if (psd1.starting_team < 0)
            continue;

        for (auto& [player_id2, psd2] : player_setup_data) {
            if (player_id1 == player_id2)
                continue;

            if (psd1.starting_team != psd2.starting_team)
                continue;

            empires.SetDiplomaticStatus(player_id1, player_id2, DiplomaticStatus::DIPLO_ALLIED);
        }
    }
}
