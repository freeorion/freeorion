import freeOrionAIInterface as fo # pylint: disable=import-error
import FreeOrionAI as foAI
import TechsListsAI
from EnumsAI import AIPriorityType, getAIPriorityResearchTypes
import AIstate
import traceback
import sys
import ColonisationAI
import random

inProgressTechs={}

def generateResearchOrders():
    "generate research orders"

    universe=fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    print "Research Queue Management:"
    tRP = empire.resourceProduction(fo.resourceType.research)
    print "\nTotal Current Research Points: %.2f\n"%tRP
    print "Techs researched and available for use:"
    completedTechs = sorted(list(getCompletedTechs()))
    tlist = completedTechs+3*[" "]
    tlines = zip( tlist[0::3],  tlist[1::3],  tlist[2::3])
    for tline in tlines:
        print "%25s  %25s  %25s"%tline
    print""

    researchQueueList = getResearchQueueTechs()
    if tRP >= 20  and foAI.foAIstate.aggression > fo.aggression.cautious:
        if (empire.getTechStatus("LRN_PSIONICS") != fo.techStatus.complete)  and ( "LRN_PSIONICS" not in researchQueueList[:5]  )  :
            for specName in ColonisationAI.empireSpecies:
                thisSpec=fo.getSpecies(specName)
                if thisSpec:
                    if "TELEPATHIC" in list(thisSpec.tags):
                        res=fo.issueEnqueueTechOrder("LRN_DISTRIB_THOUGHT", 0)
                        res=fo.issueEnqueueTechOrder("LRN_PSIONICS", 0)
                        break

    if len(foAI.foAIstate.colonisablePlanetIDs)==0:
        bestColonySiteScore = 0
    else:
        bestColonySiteScore= foAI.foAIstate.colonisablePlanetIDs[0][1]
    if len(foAI.foAIstate.colonisableOutpostIDs)==0:
        bestOutpostSiteScore = 0
    else:
        bestOutpostSiteScore= foAI.foAIstate.colonisableOutpostIDs[0][1]
    needImprovedScouting = ( bestColonySiteScore <150  or   bestOutpostSiteScore < 200 )

    if needImprovedScouting:
        if (empire.getTechStatus("CON_ORBITAL_CON") != fo.techStatus.complete):
            if  ( "CON_ORBITAL_CON" not in researchQueueList[:2]  ) and  ((empire.getTechStatus("PRO_FUSION_GEN") == fo.techStatus.complete) or ( "PRO_FUSION_GEN"  in researchQueueList[:1]  )):
                res=fo.issueEnqueueTechOrder("CON_ORBITAL_CON", 1)
                print "Empire has poor colony/outpost prospects,  so attempted to fast-track %s,  got result %d"%("CON_ORBITAL_CON", res)
        else:
            pass
            #could add more supply tech
        if (empire.getTechStatus("SPY_DETECT_2") != fo.techStatus.complete):
            if  ( "SPY_DETECT_2" not in researchQueueList[:3]  ) and  (empire.getTechStatus("PRO_FUSION_GEN") == fo.techStatus.complete) :
                if  ( "CON_ORBITAL_CON" not in researchQueueList[:2]  ):
                    res=fo.issueEnqueueTechOrder("SPY_DETECT_2", 1)
                else:
                    CO_idx = researchQueueList.index( "CON_ORBITAL_CON")
                    res=fo.issueEnqueueTechOrder("SPY_DETECT_2", CO_idx+1)
                print "Empire has poor colony/outpost prospects,  so attempted to fast-track %s,  got result %d"%("CON_ORBITAL_CON", res)

    gotGGG = empire.getTechStatus("PRO_ORBITAL_GEN") == fo.techStatus.complete
    gotSymBio = empire.getTechStatus("GRO_SYMBIOTIC_BIO") == fo.techStatus.complete
    gotXenoGen = empire.getTechStatus("GRO_XENO_GENETICS") == fo.techStatus.complete
    #assess if our empire has any non-lousy colonizers, & boost gro_xeno_gen if we don't
    if gotGGG and gotSymBio and (not gotXenoGen) and foAI.foAIstate.aggression >= fo.aggression.cautious:
        mostAdequate=0
        for specName in ColonisationAI.empireColonizers:
            environs={}
            thisSpec = fo.getSpecies(specName)
            if not thisSpec: continue
            for ptype in [fo.planetType.swamp,  fo.planetType.radiated,  fo.planetType.toxic,  fo.planetType.inferno,  fo.planetType.barren,  fo.planetType.tundra,  fo.planetType.desert,  fo.planetType.terran,  fo.planetType.ocean,  fo.planetType.asteroids]:
                environ=thisSpec.getPlanetEnvironment(ptype)
                environs.setdefault(environ, []).append(ptype)
            mostAdequate = max(mostAdequate,  len(environs.get( fo.planetEnvironment.adequate, [])))
            if mostAdequate==0:
                researchQueue = empire.researchQueue
                researchQueueList = getResearchQueueTechs()
                for xgTech in [ "GRO_XENO_GENETICS", "GRO_GENETIC_ENG" ]:
                    if   xgTech not in researchQueueList[:2]  and  empire.getTechStatus(xgTech) != fo.techStatus.complete:
                        res=fo.issueEnqueueTechOrder(xgTech, 0)
                        print "Empire has poor colonizers,  so attempted to fast-track %s,  got result %d"%(xgTech, res)

    researchQueue = empire.researchQueue
    researchQueueList = getResearchQueueTechs()
    inProgressTechs.clear()
    if  researchQueueList:
        print "Techs currently at head of Research Queue:"
        for element in list(researchQueue)[:10]:
            if element.allocation > 0.0:
                inProgressTechs[element.tech]=True
            thisTech=fo.getTech(element.tech)
            if not thisTech:
                print "Error: can't retrieve tech ",  element.tech
                continue
            missingPrereqs = [preReq for preReq in thisTech.recursivePrerequisites(empireID) if preReq not in completedTechs]
            #unlockedItems = [(uli.name,  uli.type) for uli in thisTech.unlockedItems]
            unlockedItems = [uli.name for uli in thisTech.unlockedItems]
            if not missingPrereqs:
                print "    %25s  allocated %6.2f RP -- unlockable items: %s "%(element.tech,  element.allocation,  unlockedItems)
            else:
                print "    %25s  allocated %6.2f RP   --  missing preReqs: %s   -- unlockable items: %s "%(element.tech,  element.allocation,  missingPrereqs,  unlockedItems)
        print ""
    if (fo.currentTurn()==1) or ((fo.currentTurn()<5) and (len(researchQueueList)==0) ):
        if foAI.foAIstate.aggression <=fo.aggression.typical:
            newtech = TechsListsAI.primary_meta_techs(index = empireID % 2)
        else:
            newtech = TechsListsAI.primary_meta_techs(index = empireID % 3)
        #pLTsToEnqueue = (set(newtech)-(set(completedTechs)|set(researchQueueList)))
        pLTsToEnqueue = newtech[:]
        techBase = set(completedTechs+researchQueueList)
        techsToAdd=[]
        for tech in pLTsToEnqueue:
            if (tech not in  techBase):
                thisTech=fo.getTech(tech)
                if thisTech is None:
                    print "Error: desired tech '%s' appears to not exist"%tech
                    continue
                missingPrereqs = [preReq for preReq in thisTech.recursivePrerequisites(empireID) if preReq not in techBase]
                techsToAdd.extend( missingPrereqs+[tech] )
                techBase.update(  missingPrereqs+[tech]  )
        cumCost=0
        print "  Enqueued Tech:  %20s \t\t %8s \t %s"%("Name",  "Cost",  "CumulativeCost")
        for name in techsToAdd:
            try:
                enqueueRes = fo.issueEnqueueTechOrder(name, -1)
                if enqueueRes == 1:
                    thisTech=fo.getTech(name)
                    thisCost=0
                    if thisTech:
                        thisCost = thisTech.researchCost(empireID)
                        cumCost += thisCost
                    print "    Enqueued Tech: %20s \t\t %8.0f \t %8.0f" % ( name,  thisCost,  cumCost)
                else:
                    print "    Error: failed attempt to enqueued Tech: " + name
            except:
                print "    Error: failed attempt to enqueued Tech: " + name
                print "    Error: exception triggered and caught:  ",  traceback.format_exc()
        if foAI.foAIstate.aggression <= fo.aggression.cautious:
            researchQueueList = getResearchQueueTechs()
            defTechs = TechsListsAI.defense_techs_1()
            for defTech in defTechs:
                if   defTech not in researchQueueList[:5]  and  empire.getTechStatus(defTech) != fo.techStatus.complete:
                    res=fo.issueEnqueueTechOrder(defTech, min(3,  len(researchQueueList)))
                    print "Empire is very defensive,  so attempted to fast-track %s,  got result %d"%(defTech, res)
        if False and foAI.foAIstate.aggression >= fo.aggression.aggressive: #with current stats of Conc Camps, disabling this fast-track
            researchQueueList = getResearchQueueTechs()
            if "CON_CONC_CAMP" in researchQueueList:
                insertIdx = min(40,  researchQueueList.index("CON_CONC_CAMP"))
            else:
                insertIdx=max(0,  min(40, len(researchQueueList)-10))
            if "SHP_DEFLECTOR_SHIELD" in researchQueueList:
                insertIdx = min(insertIdx,  researchQueueList.index("SHP_ASTEROID_HULLS"))
            for ccTech in [  "CON_ARCH_PSYCH",  "CON_CONC_CAMP"]:
                if   ccTech not in researchQueueList[:insertIdx+1]  and  empire.getTechStatus(ccTech) != fo.techStatus.complete:
                    res=fo.issueEnqueueTechOrder(ccTech, insertIdx)
                    print "Empire is very aggressive,  so attempted to fast-track %s,  got result %d"%(ccTech, res)

        if (random.random() <= 0.5) and ( "SHP_WEAPON_2_1" in researchQueueList) : # somewhat prioritize a couple defensive techs
            idx = researchQueueList.index( "SHP_WEAPON_2_1")
            for def_tech in ["DEF_PLAN_BARRIER_SHLD_1", "DEF_DEFENSE_NET_2" ]:
                res=fo.issueEnqueueTechOrder(def_tech, idx)
                print "Empire feeling defensive today,  so attempted to fast-track %s,  got result %d"%(def_tech, res)

        print""

        generateDefaultResearchOrders()
        print "\n\nAll techs:"
        alltechs = fo.techs() # returns names of all techs
        for tname in alltechs:
            print tname
        print "\n-------------------------------\nAll unqueued techs:"
        #coveredTechs = newtech+completedTechs
        for tname in [tn for tn in alltechs if tn not in techBase]:
            print tname

    elif fo.currentTurn() >100:
        generateDefaultResearchOrders()

    researchQueueList = getResearchQueueTechs()
    if fo.currentTurn() >50 and len (AIstate.empireStars.get(fo.starType.blackHole,  []))!=0 and foAI.foAIstate.aggression > fo.aggression.cautious:
        for singTech in [  "CON_ARCH_PSYCH",  "CON_CONC_CAMP",  "LRN_GRAVITONICS" ,  "PRO_SINGULAR_GEN"]:
            if (empire.getTechStatus(singTech) != fo.techStatus.complete) and (  singTech  not in researchQueueList[:4])  :
                res=fo.issueEnqueueTechOrder(singTech,0)
                print "have a black hole star outpost/colony, so attempted to fast-track %s,  got result %d"%(singTech, res)
        researchQueueList = getResearchQueueTechs()

    if ColonisationAI.gotGG and empire.getTechStatus("PRO_ORBITAL_GEN") != fo.techStatus.complete  and "PRO_ORBITAL_GEN" not in researchQueueList[:4]:
        if "CON_ORBITAL_CON" in researchQueueList:
            insert_idx = 1+ researchQueueList.index("CON_ORBITAL_CON")
        else:
            insert_idx = 0
        res=fo.issueEnqueueTechOrder("PRO_ORBITAL_GEN",insert_idx)
        print "GasGiant: have colonized a gas giant, so attempted to fast-track %s, got result %d"%("PRO_ORBITAL_GEN", res)
        researchQueueList = getResearchQueueTechs()
            
    if ColonisationAI.gotAst and empire.getTechStatus("SHP_ASTEROID_HULLS") != fo.techStatus.complete  and "SHP_ASTEROID_HULLS" not in researchQueueList[:3]:
        if "CON_ORBITAL_CON" in researchQueueList:
            insert_idx = 1+ researchQueueList.index("CON_ORBITAL_CON")
        else:
            insert_idx = 0
        for ast_tech in ["SHP_ASTEROID_HULLS", "PRO_MICROGRAV_MAN"]:
            if (empire.getTechStatus(ast_tech) != fo.techStatus.complete) and (  ast_tech  not in researchQueueList[:insert_idx+2])  :
                res=fo.issueEnqueueTechOrder(ast_tech,insert_idx)
                print "Asteroids: have colonized an asteroid belt, so attempted to fast-track %s ,  got result %d"%(ast_tech, res)
        researchQueueList = getResearchQueueTechs()
            
    if ColonisationAI.gotRuins and empire.getTechStatus("LRN_XENOARCH") != fo.techStatus.complete  and "LRN_XENOARCH" not in researchQueueList[:4]:
        if "LRN_ARTIF_MINDS" in researchQueueList:
            insert_idx = 7+ researchQueueList.index("LRN_ARTIF_MINDS")
        elif "SPY_DETECT_2" in researchQueueList:
            insert_idx = max(0, researchQueueList.index("SPY_DETECT_2") -2 )
        else:
            insert_idx = 0
        for xenoTech in [  "LRN_XENOARCH",  "LRN_TRANSLING_THT",  "LRN_PHYS_BRAIN" ,  "LRN_ALGO_ELEGANCE"]:
            if (empire.getTechStatus(xenoTech) != fo.techStatus.complete) and (  xenoTech  not in researchQueueList[:4])  :
                res=fo.issueEnqueueTechOrder(xenoTech,insert_idx)
                print "ANCIENT_RUINS: have an ancient ruins, so attempted to fast-track %s  to enable LRN_XENOARCH,  got result %d"%(xenoTech, res)
        researchQueueList = getResearchQueueTechs()

    if  empire.getTechStatus("SHP_WEAPON_4_1" ) == fo.techStatus.complete:
        thisTech=fo.getTech("SHP_WEAPON_4_1")
        if thisTech:
            missingPrereqs = [preReq for preReq in thisTech.recursivePrerequisites(empireID) if preReq in researchQueueList]
            if  len(missingPrereqs) > 2 :
                for preReq in sorted(missingPrereqs,  reverse=True)[2:]: #leave plasma 4 and 3
                    if preReq not in researchQueueList:
                        break
                    res = fo.issueDequeueTechOrder(preReq)
                researchQueueList = getResearchQueueTechs()
                if "SHP_WEAPON_4_2" in researchQueueList: #(should be)
                    idx = researchQueueList.index("SHP_WEAPON_4_2")
                    res=fo.issueEnqueueTechOrder("SHP_WEAPON_4_2",  max(0,  idx-15) )

def generateDefaultResearchOrders():
    "generate default research orders"

    empire = fo.getEmpire()
    totalRP = empire.resourceProduction(fo.resourceType.research)

    # get all usable researchable techs not already completed or queued for research
    completedTechs = getCompletedTechs()
    possibleProjects = getPossibleProjects()
    researchQueue = empire.researchQueue
    researchQueueList = getResearchQueueTechs ()
    possibleResearchProjects = (set(possibleProjects)-(set(completedTechs)|set(researchQueueList)))

    print "Techs in possibleResearchProjects list after enqueues to Research Queue:"
    for techname in possibleResearchProjects:
        print "    " + techname
    print ""

    # store projects mapped to their costs, so they can be sorted by that cost
    projectsDict = dict()
    for name in possibleResearchProjects:
        projectsDict[name] = fo.getTech(name).researchCost(empire.empireID)

    # iterate through techs in order of cost
    print "enqueuing techs.  already spent RP: " + str(spentRP()) + "  total RP: " + str(totalRP)
    for name, cost in sorted(projectsDict.items(), key=lambda(k, v):(v, k)):
        # abort if no RP left
        if spentRP() >= totalRP:
            break

        # add tech to queue
        fo.issueEnqueueTechOrder(name, -1)
        print "    enqueued tech " + name + "  :  cost: " + str(cost) + "RP"
    print ""

def getPossibleProjects():
    "get possible projects"

    preliminaryProjects = []
    technames = fo.techs() # returns names of all techs
    empire = fo.getEmpire()
    for techname in technames:
        if empire.getTechStatus(techname) == fo.techStatus.researchable:
            preliminaryProjects.append(techname)

    unusableTechs = TechsListsAI.unusable_techs()
    possibleProjects = (set(preliminaryProjects)-set(unusableTechs))

    return possibleProjects

def spentRP():
    "calculate RPs spent this turn so far"

    queue = fo.getEmpire().researchQueue
    return queue.totalSpent

def getCompletedTechs():
    "get completed and available for use techs"

    completedTechs = []
    technames = fo.techs() # returns names of all techs
    empire = fo.getEmpire()
    for techname in technames:
        if empire.getTechStatus(techname) == fo.techStatus.complete:
            completedTechs.append(techname)

    return completedTechs

def getResearchQueueTechs():
    "get list of techs in research queue"

    empire = fo.getEmpire()
    researchQueue = empire.researchQueue
    researchQueueList = []
    for element in researchQueue:
        researchQueueList.append(element.tech)

    return researchQueueList
