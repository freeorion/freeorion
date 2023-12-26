from focs._effects import Source, SpeciesColoniesOwned

COLONY_UPKEEP_MULTIPLICATOR = 1 + 0.06 * SpeciesColoniesOwned(empire=Source.Owner)
