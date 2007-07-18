// -*- C++ -*-
#ifndef _ShipDesign_h_
#define _ShipDesign_h_

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include <string>


/** a class representing a ship design */
struct ShipDesign
{
    /** \name Structors */ //@{
    ShipDesign(); ///< default ctor
    //@}

    int         empire;      ///< the empire that designed this ship
    std::string name;        ///< the name of the design
    std::string description; ///< short description of the design, will maybe last after v.2
    double      cost;        ///< the cost per turn of the design, in PP
    double      speed;       ///< the number of map-units the ship can move in one turn
    bool        colonize;    ///< whether or not the ship is capable of creating a new colony

    /////////////////////////////////////////////////////////////////////////////
    // V0.3 ONLY!!!!
    int         attack;      ///< the attack value of the design
    int         defense;     ///< the defense value of the design
    std::string graphic;     ///< the name of the grapic file for this ship design
    // V0.3 ONLY!!!!
    /////////////////////////////////////////////////////////////////////////////
    //@}

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/** Returns the ShipDesign specification object with id \a ship_design_id.  If no such ShipDesign
    is present in the Universe (because it doesn't exist, or isn't know to this client), 0 is
    returned instead. */
const ShipDesign* GetShipDesign(int ship_design_id);

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


