"""
This module contain code that used in charting.
Do not modify output because charting code is relay on it.
"""
import freeOrionAIInterface as fo
from PlanetUtilsAI import get_capital
from ResearchAI import get_research_index
from character.character_strings_module import get_trait_name_aggression

def charting_text():
    import FreeOrionAI as foAI  # avoid circular imports
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    planet_id = get_capital()
    planet = None
    if planet_id is not None:
        planet = universe.getPlanet(planet_id)
    aggression_name_local = get_trait_name_aggression(foAI.foAIstate.character)

    print ("Generating Orders")
    print ("EmpireID: {empire.empireID}"
           " Name: {empire.name}_{empire.empireID}_pid:{p_id}_{p_name}RIdx_{res_idx}_{aggression}"
           " Turn: {turn}").format(empire=empire, p_id=fo.playerID(), p_name=fo.playerName(),
                                   res_idx=get_research_index(), turn=fo.currentTurn(),
                                   aggression=aggression_name_local.capitalize())
    print "EmpireColors: {0.colour.r} {0.colour.g} {0.colour.b} {0.colour.a}".format(empire)
    if planet:
        print "CapitalID: " + str(planet_id) + " Name: " + planet.name + " Species: " + planet.speciesName
    else:
        print "CapitalID: None Currently Name: None Species: None "
