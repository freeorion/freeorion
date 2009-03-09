// -*- C++ -*-
#ifndef _AsteroidBeltObstacle_h_
#define _AsteroidBeltObstacle_h_

#include "Obstacle.h"

#include <set>


class AsteroidBeltObstacle :
    public OpenSteer::Obstacle
{
public:
    AsteroidBeltObstacle(float r, float tr);

    void findIntersectionWithVehiclePath(const OpenSteer::AbstractVehicle& vehicle,
                                         PathIntersection& pi) const;

private:
    AsteroidBeltObstacle();
    void TestCylinderSides(const OpenSteer::Vec3& position, const OpenSteer::Vec3& direction,
                           float cylinder_radius, std::set<float>& solutions) const;
    void TestBetweenCylinders(const OpenSteer::Vec3& position, const OpenSteer::Vec3& direction,
                              float inner_cylinder_radius, float outer_cylinder_radius,
                              float z, std::set<float>& solutions) const;
    void InsertSolution(const OpenSteer::Vec3& position, const OpenSteer::Vec3& direction,
                        float solution, std::set<float>& solutions) const;

    float m_radius;
    float m_tube_radius;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
        {
            ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Obstacle)
                & BOOST_SERIALIZATION_NVP(m_radius)
                & BOOST_SERIALIZATION_NVP(m_tube_radius);
        }
};

#endif
