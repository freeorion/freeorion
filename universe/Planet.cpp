#include "Planet.h"

#include "../util/AppInterface.h"
#include "../util/DataTable.h"
#include "Fleet.h"
#include "Predicates.h"
#include "../server/ServerApp.h"
#include "Ship.h"
#include "System.h"

#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include "../util/AppInterface.h"
#include <stdexcept>

namespace {
    DataTableMap& PlanetDataTables()
    {
        static DataTableMap map;
        if (map.empty()) {
            LoadDataTables("default/planet_tables.txt", map);
        }
        return map;
    }
}

Planet::Planet() :
   UniverseObject(),
   PopCenter(),
   ProdCenter(),
   m_just_conquered(0)
{
}

Planet::Planet(PlanetType type, PlanetSize size) :
   UniverseObject(),
   PopCenter(),
   ProdCenter(),
   m_just_conquered(0)
{
    m_type = type;
    m_size = size;
    m_def_bases = 0;

    SetMaxPop(PlanetDataTables()["PlanetMaxPop"][size][Environment()]);
    SetEnvGrowthMod(PlanetDataTables()["PlanetEnvEffectOnPopGrowthRate"][0][Environment()] / 100.0);
    SetPlanetType(type);
}

Planet::Planet(const GG::XMLElement& elem) :
   UniverseObject(elem.Child("UniverseObject")),
   PopCenter(elem.Child("PopCenter")),
   ProdCenter(elem.Child("ProdCenter")),
   m_def_bases(0)
{
    using GG::XMLElement;

    if (elem.Tag().find( "Planet" ) == std::string::npos )
        throw std::invalid_argument("Attempted to construct a Planet from an XMLElement that had a tag other than \"Planet\"");

    try {
        m_type = lexical_cast<PlanetType>(elem.Child("m_type").Text());
        m_size = PlanetSize(lexical_cast<int>(elem.Child("m_size").Text()));
        m_just_conquered = lexical_cast<int>(elem.Child("m_just_conquered").Text());

        Visibility vis = Visibility(lexical_cast<int>(elem.Child("UniverseObject").Child("vis").Text()));
        if (vis == FULL_VISIBILITY) {
            m_def_bases = lexical_cast<int>(elem.Child("m_def_bases").Text());
        }
    } catch (const boost::bad_lexical_cast& e) {
        Logger().debugStream() << "Caught boost::bad_lexical_cast in Planet::Planet(); bad XMLElement was:";
        std::stringstream osstream;
        elem.WriteElement(osstream);
        Logger().debugStream() << "\n" << osstream.str();
        throw;
    }
}

UniverseObject::Visibility Planet::GetVisibility(int empire_id) const
{
    // use the containing system's visibility
    return GetSystem()->GetVisibility(empire_id);
}

GG::XMLElement Planet::XMLEncode(int empire_id/* = Universe::ALL_EMPIRES*/) const
{
    // Partial encoding of Planet for limited visibility
    using GG::XMLElement;
    using boost::lexical_cast;
    using std::string;

    Visibility vis= GetVisibility(empire_id);
    if (empire_id == Universe::ALL_EMPIRES)
        vis = FULL_VISIBILITY;

    XMLElement retval("Planet" + boost::lexical_cast<std::string>(ID()));
    retval.AppendChild(UniverseObject::XMLEncode(empire_id));
    retval.AppendChild(PopCenter::XMLEncode(vis));
    retval.AppendChild(ProdCenter::XMLEncode(vis));
    retval.AppendChild(XMLElement("m_type", lexical_cast<std::string>(m_type)));
    retval.AppendChild(XMLElement("m_size", lexical_cast<std::string>(m_size)));
    retval.AppendChild(XMLElement("m_just_conquered", lexical_cast<std::string>(m_just_conquered)));
    if (vis == FULL_VISIBILITY) {
        retval.AppendChild(XMLElement("m_def_bases", lexical_cast<std::string>(m_def_bases)));
    }
    return retval;
}

void Planet::AddOwner   (int id)
{
    GetSystem()->UniverseObject::AddOwner(id);
    UniverseObject::AddOwner(id);
}

void Planet::RemoveOwner(int id)
{
    System *system=GetSystem();
    std::vector<Planet*> planets = system->FindObjects<Planet>();

    // check if Empire(id) is owner of at least one other planet
    std::vector<Planet*>::const_iterator plt_it;
    int count_planets = 0;
    for(plt_it=planets.begin();plt_it != planets.end();++plt_it)
        if((*plt_it)->Owners().find(id) != (*plt_it)->Owners().end())
            count_planets++;

    if(count_planets==1)
        system->UniverseObject::RemoveOwner(id);

    UniverseObject::RemoveOwner(id);
}


void Planet::Conquer(int conquerer)
{
    m_just_conquered = 1;

    // RemoveOwner will change owners - without temp_owner => side effect
    std::set<int> temp_owner(Owners());
    for(std::set<int>::const_iterator own_it = temp_owner.begin();own_it != temp_owner.end();++own_it)
        RemoveOwner(*own_it);

    AddOwner(conquerer);
}


void Planet::MovementPhase()
{
}

void Planet::PopGrowthProductionResearchPhase( )
{
    Empire* empire = (Empires()).Lookup( *Owners().begin() );

    // do not do production of planet was just conquered
    // as per v0.2 requirements doc.
    if (m_just_conquered == 1)
        m_just_conquered = 0;
    else
        ProdCenter::PopGrowthProductionResearchPhase( empire, SystemID(), ID() );

    PopCenter::PopGrowthProductionResearchPhase( );

    // adjust workforce for prod center
    SetWorkforce(PopPoints());

    StateChangedSignal()();
}

Planet::PlanetEnvironment Planet::Environment()
{
    switch (m_type)
    {
        case PT_ASTEROIDS:
        case PT_GASGIANT:   return PE_UNINHABITABLE;
        case PT_SWAMP:
        case PT_TOXIC:
        case PT_INFERNO:
        case PT_RADIATED:
        case PT_BARREN:
        case PT_TUNDRA:     return PE_TERRIBLE;
        case PT_DESERT:
        case PT_OCEAN:      return PE_ADEQUATE;
        case PT_TERRAN:     return PE_OPTIMAL;
        case PT_GAIA:       return PE_SUPERB;
        default:            throw std::invalid_argument("Planet::Environment::Invalid Planet type");
    }
}



