// -*- C++ -*-
#ifndef _SaveGameUIData_h_
#define _SaveGameUIData_h_

#include <GG/PtRect.h>

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include <string>
#include <vector>


/** Contains the UI data that must be saved in save game files in order to restore games to the users' last views. */
struct SaveGameUIData
{
    struct NebulaData
    {
        std::string filename;
        GG::Pt      center;

    private:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    GG::Pt                  map_upper_left;
    double                  map_zoom_factor;
    std::vector<NebulaData> map_nebulae;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void SaveGameUIData::NebulaData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(filename)
        & BOOST_SERIALIZATION_NVP(center);
}

template <class Archive>
void SaveGameUIData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(map_upper_left)
        & BOOST_SERIALIZATION_NVP(map_zoom_factor)
        & BOOST_SERIALIZATION_NVP(map_nebulae);
}

#endif
