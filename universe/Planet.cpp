#include "Planet.h"
#include "../GG/XML/XMLDoc.h"

#include <stdexcept>

Planet::Planet() : 
   UniverseObject(),
   PopCenter(),
   ProdCenter()
{
}

Planet::Planet(PlanetType type, PlanetSize size)
{
   // TODO
}

Planet::Planet(const GG::XMLElement& elem) : 
   UniverseObject(elem.Child("UniverseObject")),
   PopCenter(elem.Child("PopCenter")),
   ProdCenter(elem.Child("ProdCenter"))
{
   if (elem.Tag() != "Planet")
      throw std::invalid_argument("Attempted to construct a Planet from an XMLElement that had a tag other than \"Planet\"");
   // TODO
}

GG::XMLElement Planet::XMLEncode() const
{
   // TODO
}

void Planet::MovementPhase(std::vector<SitRepEntry>& sit_reps)
{
   // TODO
}

void Planet::PopGrowthProductionResearchPhase(std::vector<SitRepEntry>& sit_reps)
{
   // TODO
}

void Planet::XMLMerge(const GG::XMLElement& elem)
{
   // TODO
}

