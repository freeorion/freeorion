// -*- C++ -*-
#ifndef PROXIMITY_DATABASE_H
#define PROXIMITY_DATABASE_H

#include "Vec3.h"
#include "lq.h"

#include <boost/tuple/tuple.hpp>

template <typename T>
class ProximityDatabase
{
public:
    typedef boost::tuple<T, unsigned int, unsigned int> StoredType;

    class TokenType
    {
    public:
        TokenType(const StoredType& object, ProximityDatabase& lqsd) :
            m_lq(lqsd.m_lq),
            m_object(object)
            { lqInitClientProxy(&m_proxy, &m_object); }

        ~TokenType()
            { lqRemoveFromBin(&m_proxy); }

        void UpdatePosition(const OpenSteer::Vec3& p)
            { lqUpdateForNewLocation(m_lq, &m_proxy, p.x, p.y, p.z); }

    private:
        lqClientProxy m_proxy;
        lqDB* m_lq;
        StoredType m_object;
    };

    ProximityDatabase(const OpenSteer::Vec3& center,
                      const OpenSteer::Vec3& dimensions,
                      const OpenSteer::Vec3& divisions)
        {
            const OpenSteer::Vec3 halfsize(dimensions * 0.5f);
            const OpenSteer::Vec3 origin(center - halfsize);
            m_lq = lqCreateDatabase(origin.x, origin.y, origin.z, 
                                    dimensions.x, dimensions.y, dimensions.z,  
                                    (int)round(divisions.x),
                                    (int)round(divisions.y),
                                    (int)round(divisions.z));
        }

    ~ProximityDatabase()
        { lqDeleteDatabase(m_lq); }

    TokenType* AllocateToken(T t, unsigned int type_flags = -1, unsigned int empire_ids = -1)
        { return new TokenType(StoredType(t, type_flags, empire_ids), *this); }

    void FindAll(std::vector<T>& results,
                 unsigned int type_flags = -1,
                 unsigned int empire_ids = -1)
        {
            s_type_flags = type_flags;
            s_empire_ids = empire_ids;
            lqMapOverAllObjects(m_lq,
                                ProximityDatabase::FindCallback,
                                &results);
        }

    void FindInRadius(const OpenSteer::Vec3& center,
                      const float radius,
                      std::vector<T>& results,
                      unsigned int type_flags = -1,
                      unsigned int empire_ids = -1)
        {
            s_type_flags = type_flags;
            s_empire_ids = empire_ids;
            lqMapOverAllObjectsInLocality(m_lq,
                                          center.x, center.y, center.z,
                                          radius,
                                          ProximityDatabase::FindCallback,
                                          &results);
        }

    T FindNearestInRadius(const OpenSteer::Vec3& center,
                          const float radius,
                          unsigned int type_flags = -1,
                          unsigned int empire_ids = -1)
        {
            T retval = 0;
            s_nearest_distance = FLT_MAX;
            s_type_flags = type_flags;
            s_empire_ids = empire_ids;
            lqMapOverAllObjectsInLocality(m_lq,
                                          center.x, center.y, center.z,
                                          radius,
                                          ProximityDatabase::FindNearestCallback,
                                          &retval);
            return retval;
        }

    T FindNearest(const OpenSteer::Vec3& center,
                  unsigned int type_flags = -1,
                  unsigned int empire_ids = -1)
        {
            // TODO: Consider adding an initial pass that looks in some default
            // nearby range first -- then fails over to a full search -- as an
            // optimization.
            T retval = 0;
            s_center = center;
            s_nearest_distance = FLT_MAX;
            s_type_flags = type_flags;
            s_empire_ids = empire_ids;
            lqMapOverAllObjects(m_lq,
                                ProximityDatabase::FindNearestCallback,
                                &retval);
            return retval;
        }

private:
    static void FindCallback(void* clientObject,
                             float /*distanceSquared*/,
                             void* clientQueryState)
        {
            typedef std::vector<T> ResultVec;
            ResultVec& results = *(static_cast<ResultVec*>(clientQueryState));
            const StoredType& stored_object = *static_cast<StoredType*>(clientObject);
            if (boost::get<1>(stored_object) & s_type_flags && boost::get<2>(stored_object) & s_empire_ids)
                results.push_back(boost::get<0>(stored_object));
        }

    static void FindNearestCallback(void* clientObject,
                                    float,
                                    void* clientQueryState)
        {
            typedef std::vector<T> ResultVec;
            T& result = *static_cast<T*>(clientQueryState);
            const StoredType& stored_object = *static_cast<StoredType*>(clientObject);
            T object = boost::get<0>(stored_object);
            float distance_squared = (s_center - object->position()).lengthSquared();
            if (boost::get<1>(stored_object) & s_type_flags &&
                boost::get<2>(stored_object) & s_empire_ids &&
                distance_squared < s_nearest_distance) {
                s_nearest_distance = distance_squared;
                result = object;
            }
        }

    lqDB* m_lq;

    static OpenSteer::Vec3 s_center;
    static float s_nearest_distance;
    static unsigned int s_type_flags;
    static unsigned int s_empire_ids;
};
template <typename T>
OpenSteer::Vec3 ProximityDatabase<T>::s_center;
template <typename T>
float ProximityDatabase<T>::s_nearest_distance;
template <typename T>
unsigned int ProximityDatabase<T>::s_type_flags;
template <typename T>
unsigned int ProximityDatabase<T>::s_empire_ids;

#endif
