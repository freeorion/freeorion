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
    SetEnvGrowthMod(PlanetDataTables()["PlanetMaxPop"][0][Environment()] / 100.0);
}

Planet::Planet(const GG::XMLElement& elem) :
   UniverseObject(elem.Child("UniverseObject")),
   PopCenter(elem.Child("PopCenter")),
   ProdCenter(elem.Child("ProdCenter"))
{
    using GG::XMLElement;

    if (elem.Tag().find( "Planet" ) == std::string::npos )
        throw std::invalid_argument("Attempted to construct a Planet from an XMLElement that had a tag other than \"Planet\"");

    m_type = (PlanetType) lexical_cast<int> ( elem.Child("m_type").Attribute("value") );
    m_size = (PlanetSize) lexical_cast<int> ( elem.Child("m_size").Attribute("value") );
    m_def_bases = lexical_cast<int> ( elem.Child("m_def_bases").Attribute("value") );
    m_just_conquered = lexical_cast<int> (elem.Child("m_just_conquered").Attribute("value") );
}

UniverseObject::Visibility Planet::Visible(int empire_id) const
{
    // use the containing system's visibility
    return GetSystem()->Visible(empire_id);
}


GG::XMLElement Planet::XMLEncode() const
{
    using GG::XMLElement;
    using boost::lexical_cast;
    using std::string;

    string planet_name( "Planet" );
    planet_name += boost::lexical_cast<std::string>( ID()  );
    XMLElement element( planet_name );

    element.AppendChild( UniverseObject::XMLEncode() );

    element.AppendChild( PopCenter::XMLEncode() );

    element.AppendChild( ProdCenter::XMLEncode() );

    XMLElement type("m_type");
    type.SetAttribute( "value", lexical_cast<std::string>(m_type) );
    element.AppendChild(type);

    XMLElement size("m_size");
    size.SetAttribute( "value", lexical_cast<std::string>(m_size) );
    element.AppendChild(size);

    XMLElement def_bases("m_def_bases");
    def_bases.SetAttribute( "value", lexical_cast<std::string>(m_def_bases) );
    element.AppendChild(def_bases);

    XMLElement just_conquered("m_just_conquered");
    just_conquered.SetAttribute( "value", lexical_cast<std::string>(m_just_conquered));
    element.AppendChild(just_conquered);

    return element;
}

GG::XMLElement Planet::XMLEncode(int empire_id) const
{
    // Partial encoding of Planet for limited visibility

    using GG::XMLElement;
    using boost::lexical_cast;
    using std::string;

    string planet_name( "Planet" );
    planet_name += boost::lexical_cast<std::string>( ID() );
    XMLElement element( planet_name );

    // full encode of UniverseObject since owner list should be visible
    element.AppendChild( UniverseObject::XMLEncode() );

    // full encode for PopCenter, nothing is hidden there
    element.AppendChild( PopCenter::XMLEncode() );

    // partial encode of ProdCenter to hide the current build option info
    element.AppendChild( ProdCenter::XMLEncode(empire_id) );

    XMLElement type("m_type");
    type.SetAttribute( "value", lexical_cast<std::string>(m_type) );
    element.AppendChild(type);

    XMLElement size("m_size");
    size.SetAttribute( "value", lexical_cast<std::string>(m_size) );
    element.AppendChild(size);

    XMLElement def_bases("m_def_bases");
    def_bases.SetAttribute( "value", lexical_cast<std::string>(m_def_bases) );
    element.AppendChild(def_bases);

    XMLElement just_conquered("m_just_conquered");
    just_conquered.SetAttribute( "value", lexical_cast<std::string>(m_just_conquered));
    element.AppendChild(just_conquered);

    return element;
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
    // as per 0.1 requirements doc.
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



