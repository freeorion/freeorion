#include "Planet.h"
#include "XMLDoc.h"
#include "Fleet.h"
#include "Ship.h"
#include "System.h"
#include "Predicates.h"

#include "../server/ServerApp.h"


#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include "../util/AppInterface.h"
#include <stdexcept>

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

    switch(size) {
    case SZ_TINY:
        SetMaxPop(10);
        break;
    case SZ_SMALL:
        SetMaxPop(30);
        break;
    case SZ_MEDIUM:
        SetMaxPop(50);
        break;
    case SZ_LARGE:
        SetMaxPop(70);
        break;
    case SZ_HUGE:
        SetMaxPop(90);
    }

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
    // if system is visible, then planet is too. Full visibility
    // if owned by player, partial if not. 

    Empire* empire = (Empires()).Lookup(empire_id);

    if (empire->HasPlanet(ID()))
    {
        return FULL_VISIBILITY;
    }

    if (empire->HasExploredSystem(SystemID()))
    {
        return PARTIAL_VISIBILITY;
    }

    return NO_VISIBILITY;
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

void Planet::AddOwnerExt   (int id)
{
  UniverseObject::AddOwner(id);
  GetSystem()->AddOwner(id);
}

void Planet::RemoveOwnerExt(int id)
{
  UniverseObject::RemoveOwner(id);
  System *system=GetSystem();
  System::ObjectVec planets = system->FindObjects(IsPlanet);
 
  // check if Empire(id) is owner of at least one other planet
  System::ObjectVec::const_iterator plt_it;
  for(plt_it=planets.begin();plt_it != planets.end();++plt_it)
    if((*plt_it)->Owners().find(id) != (*plt_it)->Owners().end())
      break;

  if(plt_it == planets.end())
    system->RemoveOwner(id);
}


void Planet::Conquer(int conquerer)
{
#ifndef FREEORION_BUILD_SERVER
  throw std::runtime_error("call to Planet::Conquer from AI o human client");
#else
  m_just_conquered = 1;

  // RemoveOwner will change owners - without temp_owner => side effect
  std::set<int> temp_owner(Owners());
  for(std::set<int>::const_iterator own_it = temp_owner.begin();own_it != temp_owner.end();++own_it)
    RemoveOwnerExt(*own_it);

  AddOwnerExt(conquerer);
#endif
}


void Planet::MovementPhase()
{
    // TODO
    StateChangedSignal()();
}

void Planet::PopGrowthProductionResearchPhase( )
{
    Empire* empire = (Empires()).Lookup( *Owners().begin() );

    // do not do production of planet was just conquered
    // as per 0.1 requirements doc.
    if(m_just_conquered == 1)
        m_just_conquered = 0;
    else
        ProdCenter::PopGrowthProductionResearchPhase( empire, SystemID(), ID() );

    PopCenter::PopGrowthProductionResearchPhase( );

    // adjust workforce for prod center
    SetWorkforce(PopPoints());

    StateChangedSignal()();
}

