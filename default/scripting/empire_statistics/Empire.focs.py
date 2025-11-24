from focs._effects import (
    BuildingTypesProduced,
    BuildingTypesScrapped,
    LocalCandidate,
    NumPoliciesAdopted,
    OwnedBy,
    Planet,
    Source,
    SpeciesPlanetsBombed,
    SpeciesPlanetsDepoped,
    SpeciesPlanetsInvaded,
    Statistic,
    Sum,
)
from focs._empire_statistics import EmpireStatistic
from focs._value_refs import EmpireShipsDestroyed, ShipDesignsLost, ShipDesignsProduced, ShipDesignsScrapped

EmpireStatistic(name="POLICIES_ADOPTED", value=NumPoliciesAdopted(empire=Source.Owner))

EmpireStatistic(name="BUILDINGS_PRODUCED", value=BuildingTypesProduced(empire=Source.Owner))

EmpireStatistic(name="BUILDINGS_SCRAPPED", value=BuildingTypesScrapped(empire=Source.Owner))

EmpireStatistic(name="SHIPS_DESTROYED", value=EmpireShipsDestroyed(empire=Source.Owner))

EmpireStatistic(name="SHIPS_LOST", value=ShipDesignsLost(empire=Source.Owner))

EmpireStatistic(name="SHIPS_PRODUCED", value=ShipDesignsProduced(empire=Source.Owner))

EmpireStatistic(name="SHIPS_SCRAPPED", value=ShipDesignsScrapped(empire=Source.Owner))

EmpireStatistic(name="PLANETS_BOMBED", value=SpeciesPlanetsBombed(empire=Source.Owner))

EmpireStatistic(name="PLANETS_DEPOPULATED", value=SpeciesPlanetsDepoped(empire=Source.Owner))

EmpireStatistic(name="PLANETS_INVADED", value=SpeciesPlanetsInvaded(empire=Source.Owner))

EmpireStatistic(
    name="TOTAL_POPULATION_STAT",
    value=Statistic(float, Sum, value=LocalCandidate.Population, condition=Planet() & OwnedBy(empire=Source.Owner)),
)
