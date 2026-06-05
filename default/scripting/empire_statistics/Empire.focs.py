from focs._conditions import OwnedBy, Planet
from focs._effects import Sum
from focs._empire_statistics import EmpireStatistic
from focs._sources import LocalCandidate, Source
from focs._value_refs import (
    BuildingTypesProduced,
    BuildingTypesScrapped,
    EmpireShipsDestroyed,
    NumPoliciesAdopted,
    ShipDesignsLost,
    ShipDesignsProduced,
    ShipDesignsScrapped,
    SpeciesPlanetsBombed,
    SpeciesPlanetsDepoped,
    SpeciesPlanetsInvaded,
    Statistic,
)

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
