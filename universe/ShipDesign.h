// -*- C++ -*-
#ifndef _ShipDesign_h_
#define _ShipDesign_h_

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include <string>

class XMLElement;

/** a class representing a ship design */
struct ShipDesign
{
    /** \name Structors */ //@{
    ShipDesign(); ///< default ctor
    ShipDesign(const XMLElement& elem); ///< ctor that constructs a ShipDesign object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a ShipDesign object
    //@}

    int         empire;      ///< the empire that designed this ship
    std::string name;        ///< the name of the design
    std::string description; ///< short description of the design, will maybe last after v.2
    double      cost;        ///< the cost of the design, in PP
    double      speed;       ///< the number of map-units the ship can move in one turn
    bool        colonize;    ///< whether or not the ship is capable of creating a new colony

    /////////////////////////////////////////////////////////////////////////////
    // V0.3 ONLY!!!!
    int         attack;      ///< the attack value of the design
    int         defense;     ///< the defense value of the design
    std::string graphic;     ///< the name of the grapic file for this ship design
    // V0.3 ONLY!!!!
    /////////////////////////////////////////////////////////////////////////////

    /** \name Accessors */ //@{
    XMLElement XMLEncode() const; ///< constructs an XMLElement from a ShipDesign object
    //@}

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

const ShipDesign* GetShipDesign(int empire_id, const std::string& name);

// template implementations
template <class Archive>
void ShipDesign::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(empire)
        & BOOST_SERIALIZATION_NVP(name)
        & BOOST_SERIALIZATION_NVP(description)
        & BOOST_SERIALIZATION_NVP(cost)
        & BOOST_SERIALIZATION_NVP(speed)
        & BOOST_SERIALIZATION_NVP(colonize)
        & BOOST_SERIALIZATION_NVP(attack)
        & BOOST_SERIALIZATION_NVP(defense)
        & BOOST_SERIALIZATION_NVP(graphic);
}

#endif // _ShipDesign_h_


