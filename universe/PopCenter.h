#ifndef _PopCenter_h_
#define _PopCenter_h_


#include <memory>
#include <string>
#include "EnumsFwd.h"
#include "../util/Export.h"


class Meter;
class UniverseObject;

/** The PopCenter class is an abstract base class for anything in the FreeOrion
  * gamestate that has population on or in it.  Most likely, such an object
  * will also be a subclass of UniverseObject.
  * Planet is the most obvious class to inherit PopCenter, but other classes
  * could be made from it as well (e.g., a ship that is large enough to support
  * population and still travel between systems). */
class FO_COMMON_API PopCenter : virtual public std::enable_shared_from_this<UniverseObject> {
public:
    PopCenter();
    explicit PopCenter(const std::string& species_name);
    virtual ~PopCenter();

    const std::string&  SpeciesName() const {return m_species_name;}        ///< returns the name of the species that populates this planet
    std::string         Dump(unsigned short ntabs = 0) const;
    bool                Populated() const;
    virtual Meter*      GetMeter(MeterType type) = 0;                       ///< implementation should return the requested Meter, or 0 if no such Meter of that type is found in this object
    virtual const Meter*GetMeter(MeterType type) const = 0;                 ///< implementation should return the requested Meter, or 0 if no such Meter of that type is found in this object

    void                Copy(std::shared_ptr<const PopCenter> copied_object, Visibility vis);
    void                Copy(std::shared_ptr<const PopCenter> copied_object);
    virtual void        SetSpecies(std::string species_name);               ///< sets the species of the population to \a species_name
    virtual void        Reset();                                            ///< sets all meters to 0, clears race name
    virtual void        Depopulate();                                       ///< removes population

protected:
    void Init();    ///< initialization that needs to be called by derived class after derived class is constructed
    void PopCenterResetTargetMaxUnpairedMeters();
    void PopCenterClampMeters();
    void PopCenterPopGrowthProductionResearchPhase();

private:
    virtual void AddMeter(MeterType meter_type) = 0;///< implementation should add a meter to the object so that it can be accessed with the GetMeter() functions

    std::string m_species_name;                     ///< the name of the species that occupies this planet

    template <typename Archive>
    friend void serialize(Archive&, PopCenter&, unsigned int const);
};


#endif
