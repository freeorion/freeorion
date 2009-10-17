// -*- C++ -*-
#ifndef _Ship_h_
#define _Ship_h_

#include "UniverseObject.h"
#include "Meter.h"

class Fighter;
class Fleet;
class ShipDesign;

/** a class representing a single FreeOrion ship*/
class Ship : public UniverseObject
{
public:
    // map from part type name to (number of parts in the design of that type,
    // number of fighters (or missiles) available of that type) pairs
    typedef std::map<std::string, std::pair<std::size_t, std::size_t> > ConsumablesMap;

    /** \name Structors */ //@{
    Ship(); ///< default ctor
    Ship(int empire_id, int design_id);     ///< general ctor taking ship's empire and design id; from this the design can be looked up and used to create the ship
    //@}

    /** \name Accessors */ //@{
    const ShipDesign*           Design() const;                     ///< returns the design of the ship, containing engine type, weapons, etc.
    int                         DesignID() const;                   ///< returns the design id of the ship
    int                         FleetID() const;                    ///< returns the ID of the fleet the ship is residing in
    Fleet*                      GetFleet() const;                   ///< returns the ID of the fleet the ship is residing in

    virtual const std::string&  PublicName(int empire_id) const;

    bool                        IsArmed() const;
    bool                        CanColonize() const;
    double                      Speed() const;

    const ConsumablesMap&       Fighters() const;
    const ConsumablesMap&       Missiles() const;

    virtual UniverseObject*     Accept(const UniverseObjectVisitor& visitor) const;

    virtual double              ProjectedCurrentMeter(MeterType type) const;    ///< returns expected value of  specified meter current value on the next turn

    bool                        OrderedScrapped() const {return m_ordered_scrapped;}
    //@}

    /** \name Mutators */ //@{
    void                        SetFleetID(int fleet_id);                       ///< sets the ID of the fleet the ship resides in

    void                        Resupply();

    void                        AddFighters(const std::string& part_name, std::size_t n);
    void                        RemoveFighters(const std::string& part_name, std::size_t n);
    void                        RemoveMissiles(const std::string& part_name, std::size_t n);

    virtual void                MoveTo(double x, double y);

    void                        SetOrderedScrapped(bool b = true);              ///< flags ship for scrapping
    //@}

private:
    int             m_design_id;
    int             m_fleet_id;
    bool            m_ordered_scrapped;
    ConsumablesMap  m_fighters;
    ConsumablesMap  m_missiles;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void Ship::serialize(Archive& ar, const unsigned int version)
{
    Visibility vis;
    bool ordered_scrapped = false;

    if (Archive::is_saving::value) {
        vis = GetVisibility(Universe::s_encoding_empire);
    }

    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(UniverseObject)
        & BOOST_SERIALIZATION_NVP(vis)
        & BOOST_SERIALIZATION_NVP(m_fleet_id);

    if (vis >= VIS_PARTIAL_VISIBILITY) {
        ar  & BOOST_SERIALIZATION_NVP(m_design_id)
            & BOOST_SERIALIZATION_NVP(m_fighters)
            & BOOST_SERIALIZATION_NVP(m_missiles);
    }

    if (vis == VIS_FULL_VISIBILITY) {
        ar  & BOOST_SERIALIZATION_NVP(ordered_scrapped);
    }

    if (Archive::is_loading::value)
        m_ordered_scrapped = ordered_scrapped;
}

#endif // _Ship_h_
