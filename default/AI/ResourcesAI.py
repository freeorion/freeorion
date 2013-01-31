import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import AIstate
from EnumsAI import AIPriorityType, getAIPriorityResourceTypes, AIFocusType
import PlanetUtilsAI
from random import shuffle,  random
from time import time

AIPriorityTypeNames=AIPriorityType()

resourceTimerFile=None
doResourceTiming=True
__timerEntries1=["TopResources",  "SetCapital",  "SetPlanets",  "SetAsteroids",  "SetGiants",  "PrintResources" ]
__timerEntries2=["getPlanets",  "Filter",  "Priority",  "Shuffle",  "Targets",  "Loop" ]
__timerEntries = __timerEntries2
__timerFileFmt = "%8d"+ (len(__timerEntries)*"\t %8d")

oldTargets={}
newTargets={}
currentFocus = {}
currentOutput={}
IFocus = AIFocusType.FOCUS_INDUSTRY
RFocus = AIFocusType.FOCUS_RESEARCH
MFocus = AIFocusType.FOCUS_MINING
GFocus = AIFocusType.FOCUS_GROWTH


def topResourcePriority():
    "calculate top resource priority"

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    empirePlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)

    resourcePriorities = {}
    for priorityType in getAIPriorityResourceTypes():
        resourcePriorities[priorityType] = foAI.foAIstate.getPriority(priorityType)

    sortedPriorities = resourcePriorities.items()
    sortedPriorities.sort(lambda x,y: cmp(x[1], y[1]), reverse=True)
    topPriority = -1
    for evaluationPair in sortedPriorities:
        if topPriority < 0:
            topPriority = evaluationPair[0]

    return topPriority

def getResourceTargetTotals():
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    empirePlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    capitalID = PlanetUtilsAI.getCapital()
    #    planet = universe.getPlanet(planetID)
    #   if planet.currentMeterValue(fo.meterType.population) >0:
    planets = map(universe.getPlanet,  empirePlanetIDs)
    planetMap = dict(  zip( empirePlanetIDs,  planets)) 
    oldTargets.clear()
    oldTargets.update(newTargets)
    newTargets.clear()
    currentFocus.clear()
    currentOutput.clear()
    
    targetPP = sum( map( lambda x: x.currentMeterValue(fo.meterType.targetIndustry),  planets) )
    targetRP = sum( map( lambda x: x.currentMeterValue(fo.meterType.targetResearch),  planets) )
    
    newFocus= IFocus
    for pid in empirePlanetIDs:
        currentFocus[pid] = planetMap[pid].focus
        if  currentFocus[pid] == MFocus:
            mtarget=planetMap[pid].currentMeterValue(fo.meterType.targetIndustry)
        currentOutput.setdefault(pid,  {} )[ IFocus] =  planetMap[pid].currentMeterValue(fo.meterType.industry)
        currentOutput[pid][ RFocus] =  planetMap[pid].currentMeterValue(fo.meterType.research)
        fo.issueChangeFocusOrder(pid, IFocus) #may not be able to take, but try
    universe.updateMeterEstimates(empirePlanetIDs)
    for pid in empirePlanetIDs:
        itarget=planetMap[pid].currentMeterValue(fo.meterType.targetIndustry)
        rtarget=planetMap[pid].currentMeterValue(fo.meterType.targetResearch)
        newTargets.setdefault(pid,  {}).setdefault(IFocus,  (0, 0))
        newTargets[pid][IFocus] = ( itarget,  rtarget )
        if  currentFocus[pid] == MFocus:
            newTargets[pid][MFocus] = ( mtarget,  rtarget )
        fo.issueChangeFocusOrder(pid, RFocus) #may not be able to take, but try
    universe.updateMeterEstimates(empirePlanetIDs)
    for pid in empirePlanetIDs:
        itarget=planetMap[pid].currentMeterValue(fo.meterType.targetIndustry)
        rtarget=planetMap[pid].currentMeterValue(fo.meterType.targetResearch)
        newTargets.setdefault(pid,  {}).setdefault(RFocus,  (0, 0))
        newTargets[pid][RFocus] = ( itarget,  rtarget )
        if currentFocus[pid]  != RFocus:
            fo.issueChangeFocusOrder(pid, currentFocus[pid]) #put it back to what it was
    universe.updateMeterEstimates(empirePlanetIDs)
    return targetPP,  targetRP
    
def setCapitalIDResourceFocus():
    "set resource focus of CapitalID planet"

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    empirePlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    capitalID = PlanetUtilsAI.getCapital()
    topPriority = topResourcePriority()

    if topPriority == AIPriorityType.PRIORITY_RESOURCE_GROWTH:
        newFocus = AIFocusType.FOCUS_GROWTH
        for planetID in empirePlanetIDs:
            planet = universe.getPlanet(planetID)
            focus = newFocus
            if planetID == capitalID and focus in planet.availableFoci:
                fo.issueChangeFocusOrder(planetID, focus)
    elif topPriority == AIPriorityType.PRIORITY_RESOURCE_PRODUCTION:
        newFocus = AIFocusType.FOCUS_INDUSTRY
        for planetID in empirePlanetIDs:
            planet = universe.getPlanet(planetID)
            focus = newFocus
            if planetID == capitalID and focus in planet.availableFoci:
                fo.issueChangeFocusOrder(planetID, focus)
    elif topPriority == AIPriorityType.PRIORITY_RESOURCE_RESEARCH:
        newFocus = AIFocusType.FOCUS_RESEARCH
        for planetID in empirePlanetIDs:
            planet = universe.getPlanet(planetID)
            focus = newFocus
            if planetID == capitalID and focus in planet.availableFoci:
                fo.issueChangeFocusOrder(planetID, focus)

def setGeneralPlanetResourceFocus():
    "set resource focus of planets except capitalID, asteroids, and gas giants"
    global __timerFile

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    timer= [ time() ] # getPlanets
    empirePlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    timer.append( time() ) #filter
    capitalIDL = [PlanetUtilsAI.getCapital()]
    asteroids = PlanetUtilsAI.getTypePlanetEmpireOwned(fo.planetType.asteroids)
    gasGiants = PlanetUtilsAI.getTypePlanetEmpireOwned(fo.planetType.gasGiant)
    generalPlanetIDs = list(set(empirePlanetIDs) - (set(capitalIDL)|set(asteroids)|set(gasGiants)))
    timer.append( time() ) #Priority
    #fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
    #fleetSupplyablePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(fleetSupplyableSystemIDs)
    ppPrio = foAI.foAIstate.getPriority(AIPriorityType.PRIORITY_RESOURCE_PRODUCTION)
    rpPrio = foAI.foAIstate.getPriority(AIPriorityType.PRIORITY_RESOURCE_RESEARCH)
    priorityRatio = float(ppPrio)/(rpPrio+0.0001)
    timer.append( time() ) # shuffle
    # not supporting Growth for general planets until also adding code to make sure would actually benefit
    shuffle(generalPlanetIDs)
    timer.append( time() ) # targets
    pp, rp = getResourceTargetTotals()
    targetRatio = pp/(rp+0.0001)
    nplanets = len(generalPlanetIDs)
    nChanges = int(  (  nplanets *  max( 1.0/(rp+0.001),  2.0/(pp+0.0001))  *  ( (pp - rp* priorityRatio)/(priorityRatio +2)))  +0.5 ) # weird formula I came up with to estimate desired number of changes
    print "current target totals -- pp: %.1f   rp: %.1f   ; ratio %.2f  ; desired ratio %.2f  will change up to %d  foci from total of %d planets"%(pp, rp, targetRatio,  priorityRatio,  nChanges,  nplanets)
    timer.append( time() ) #loop
    iChanges = 0
    for planetID in generalPlanetIDs:
        if iChanges >= nChanges: break
        planet = universe.getPlanet(planetID)
        oldFocus=planet.focus
        targetRatio = pp/(rp + 0.0001)
        ratioRatio = priorityRatio / ( targetRatio + 0.0001)
        if  (AIFocusType.FOCUS_MINING in planet.availableFoci): #and ((priorityRatio > 0.5 ) or ( ratioRatio > 0.9  ) ) :  #could be a more complex decision here, 
            if oldFocus != AIFocusType.FOCUS_MINING:
                iChanges +=1 #even if not necessarily  directed towards desired ratio
                fo.issueChangeFocusOrder(planetID, AIFocusType.FOCUS_MINING)
                print "Changing planet focus for ID %s : %s  from %s to %s "%(planetID,  planet.name,  oldFocus,   AIFocusType.FOCUS_MINING )
            continue
        newFocus=None
        if ( ratioRatio > 1.1 ): #needs industry
            if oldFocus == AIFocusType.FOCUS_INDUSTRY:
                continue
            else:
                newFocus = AIFocusType.FOCUS_INDUSTRY
        elif ( ratioRatio < 0.91 ):
            if oldFocus == AIFocusType.FOCUS_RESEARCH:
                continue
            else:
                newFocus = AIFocusType.FOCUS_RESEARCH
        if newFocus:
            #pp -= planet.currentMeterValue(fo.meterType.targetIndustry)
            #rp -= planet.currentMeterValue(fo.meterType.targetResearch)
            fo.issueChangeFocusOrder(planetID, newFocus)
            iChanges += 1
            #pp += planet.currentMeterValue(fo.meterType.targetIndustry)
            #rp += planet.currentMeterValue(fo.meterType.targetResearch)
            print "Changing planet focus for ID %s : %s  from %s to %s "%(planetID,  planet.name,  oldFocus,   newFocus )
    timer.append( time() ) #end
    if doResourceTiming and __timerEntries==__timerEntries2:
        times = [timer[i] - timer[i-1] for i in range(1,  len(timer) ) ]
        timeFmt = "%30s: %8d msec  "
        print "ResourcesAI Time Requirements:"
        for mod,  modTime in zip(__timerEntries,  times):
            print timeFmt%((30*' '+mod)[-30:],  int(1000*modTime))
        if resourceTimerFile:
            print "len times: %d  ;  len entries: %d "%(len(times),  len(__timerEntries))
            resourceTimerFile.write(  __timerFileFmt%tuple( [ fo.currentTurn() ]+map(lambda x: int(1000*x),  times )) +'\n')
            resourceTimerFile.flush()


def setAsteroidsResourceFocus():
    "change resource focus of asteroids from farming to mining"

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    empirePlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    newFocus = AIFocusType.FOCUS_INDUSTRY
    for planetID in empirePlanetIDs:
        planet = universe.getPlanet(planetID)
        focus = newFocus
        if planet.type == fo.planetType.asteroids and 'GRO_ORBIT_FARMING' in empire.availableTechs and focus in planet.availableFoci:
            fo.issueChangeFocusOrder(planetID, focus)

def setGasGiantsResourceFocus():
    "change resource focus of gas giants from farming to research"

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    empirePlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    newFocus = AIFocusType.FOCUS_RESEARCH
    for planetID in empirePlanetIDs:
        planet = universe.getPlanet(planetID)
        focus = newFocus
        if planet.type == fo.planetType.gasGiant and 'GRO_ORBIT_FARMING' in empire.availableTechs and focus in planet.availableFoci:
            fo.issueChangeFocusOrder(planetID, focus)

def printResourcesPriority():
    "calculate top resource priority"

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    empirePlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    print "Resource Management:"
    print ""
    print "Resource Priorities:"
    resourcePriorities = {}
    for priorityType in getAIPriorityResourceTypes():
        resourcePriorities[priorityType] = foAI.foAIstate.getPriority(priorityType)

    sortedPriorities = resourcePriorities.items()
    sortedPriorities.sort(lambda x,y: cmp(x[1], y[1]), reverse=True)
    topPriority = -1
    for evaluationPair in sortedPriorities:
        if topPriority < 0:
            topPriority = evaluationPair[0]
        print "    ResourcePriority |Score: %s  | %s "%(AIPriorityTypeNames.name(evaluationPair[0]),  evaluationPair[1])

    # what is the focus of available resource centers?
    print ""
    warnings={}
    print "Planet Resources Foci:"
    for planetID in empirePlanetIDs:
        planet = universe.getPlanet(planetID)
        planetPopulation = planet.currentMeterValue(fo.meterType.population)
        maxPop = planet.currentMeterValue(fo.meterType.targetPopulation)
        if maxPop < 1 and  planetPopulation >0:
            warnings[planet.name]=(planetPopulation, maxPop)
        statusStr =    "  ID: " + str(planetID) + " Name: % 18s  -- % 6s  % 8s "%(str(planet.name) ,  str(planet.size),  str(planet.type) ) 
        statusStr += " Focus: % 8s"%("_".join(str(planet.focus).split("_")[1:])[:8]) + " Species: " + str(planet.speciesName) + " Pop: %2d/%2d"%(planetPopulation,  maxPop)
        print statusStr
    print "\n\nEmpire Totals:\nPopulation: %5d \nProduction: %5d\nResearch: %5d\n"%(empire.population(),  empire.productionPoints,  empire.resourceProduction(fo.resourceType.research))
    if warnings != {}:
        for pname in warnings:
            mp, cp = warnings[pname]
            print "Population Warning! -- %s has unsustainable current pop %d  -- target  %d"%(pname, cp,  mp )
        print ""
    warnings.clear()


def generateResourcesOrders_Old():
    global __timerFile
    "generate resources focus orders"

    timer= [ time() ]
    # calculate top resource priority
    topResourcePriority()
    timer.append( time() )
    # set resource foci of planets
    setCapitalIDResourceFocus()
    timer.append( time() )
    setGeneralPlanetResourceFocus()
    timer.append( time() )
    setAsteroidsResourceFocus()
    timer.append( time() )
    setGasGiantsResourceFocus()
    timer.append( time() )

    printResourcesPriority()
    timer.append( time() )

    if doResourceTiming and __timerEntries==__timerEntries1:
        times = [timer[i] - timer[i-1] for i in range(1,  len(timer) ) ]
        timeFmt = "%30s: %8d msec  "
        print "ResourcesAI Time Requirements:"
        for mod,  modTime in zip(__timerEntries,  times):
            print timeFmt%((30*' '+mod)[-30:],  int(1000*modTime))
        if resourceTimerFile:
            resourceTimerFile.write(  __timerFileFmt%tuple( [ fo.currentTurn() ]+map(lambda x: int(1000*x),  times )) +'\n')
            resourceTimerFile.flush()


def tallyStream(curVal,  targetVal,  nTurns,  force=False):
    tally=0
    if curVal <= targetVal:
        delta = [1, 3][force]
        mycomp=min
    else:
        delta = [-1, -5][force]
        mycomp=max
    for turn in range(nTurns):
        tally += mycomp(targetVal,  curVal+turn*delta)
    return tally
    
def setPlanetResourceFoci():
    "set resource focus of planets except capitalID, asteroids, and gas giants"
    global __timerFile

    print "\n============================"
    print "Collecting info to assess Planet Focus Changes\n"
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    currentTurn = fo.currentTurn()
    freq = (1.0 + currentTurn/4.0)**(1.0/3)
    if random() > 1.0/freq:
        timer = 6*[time()]
    else:
        timer= [ time() ] # getPlanets
        empirePlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
        timer.append( time() ) #filter
        timer.append( time() ) #Priority
        #TODO: take into acct splintering of resource groups
        #fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
        #fleetSupplyablePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(fleetSupplyableSystemIDs)
        ppPrio = foAI.foAIstate.getPriority(AIPriorityType.PRIORITY_RESOURCE_PRODUCTION)
        rpPrio = foAI.foAIstate.getPriority(AIPriorityType.PRIORITY_RESOURCE_RESEARCH)
        priorityRatio = float(rpPrio)/(ppPrio+0.0001)
        timer.append( time() ) # shuffle
        # not supporting Growth for general planets until also adding code to make sure would actually benefit
        #shuffle(generalPlanetIDs)
        timer.append( time() ) # targets
        pp, rp = getResourceTargetTotals()
        planets = map(universe.getPlanet,  empirePlanetIDs)
        planetMap = dict(  zip( empirePlanetIDs,  planets)) 
        print "\n-----------------------------------------"
        print "Making Planet Focus Change Determinations\n"
        
        ratios = []
        #for each planet, calculate RP:PP value ratio at which industry/Mining focus and research focus would have the same total value, & sort by that
        # include a bias to slightly discourage changing foci
        curTargetPP = 0
        curTargetRP = 0
        newFoci = {}
        timer.append( time() ) #loop
        hasForce = empire.getTechStatus("CON_FRC_ENRG_STRC") == fo.techStatus.complete
        for pid in newTargets:
            II, IR = newTargets[pid][IFocus]
            RI, RR = newTargets[pid][RFocus]
            if currentFocus[pid] == MFocus:
                II = max( II,  newTargets[pid][MFocus][0] ) 
            curTargetPP += II  #icurTargets initially calculated by Industry focus, which will be our default focus
            curTargetRP += IR
            newFoci[pid] = IFocus
            if foAI.foAIstate.aggression < fo.aggression.maniacal:
                if currentFocus[pid] in [ IFocus,  MFocus] :
                    II += min( 2,  II /4.0 )
                elif currentFocus[pid] == RFocus:
                    RR += min( 2,  RR/4.0 )
                #calculate factor F at  which     II + F * RI  ==  RI + F * RR   =====>  F = ( II-RI ) / (RR-IR)
            thisFactor = ( II-RI ) / max( 0.01,  RR-IR)  # don't let denominator be zero for planets where focus doesn't change RP
            if foAI.foAIstate.aggression >fo.aggression.aggressive:
                if currentOutput[pid][ IFocus] > II +RI - RR:
                    thisFactor = min(thisFactor,  1.0 + thisFactor/10.0 ) 
            ratios.append( (thisFactor,  pid ) )
                
        ctPP0 = curTargetPP
        ctRP0 = curTargetRP 
        ratios.sort()
        printedHeader=False
        fociMap={IFocus:"Industry",  RFocus:"Research", MFocus:"Mining",  GFocus:"Growth"}
        gotAlgo = empire.getTechStatus("LRN_ALGO_ELEGANCE") == fo.techStatus.complete
        for ratio,  pid in ratios:
            if priorityRatio < ( curTargetRP/ (curTargetPP + 0.0001)) : #we have enough RP
                if ratio < 1.1  and foAI.foAIstate.aggression >fo.aggression.cautious :  #but wait, RP is still super cheap relative to PP, maybe will take more RP
                    if priorityRatio < 1.5* ( curTargetRP/ (curTargetPP + 0.0001)) : #yeah, really a glut of RP, stop taking RP
                        break
                else: #RP not super cheap & we have enough, stop taking it
                    break
            II, IR = newTargets[pid][IFocus]
            RI, RR = newTargets[pid][RFocus]
            if currentFocus[pid] == MFocus:
                II = max( II,  newTargets[pid][MFocus][0] ) 
            if gotAlgo and (
                   (ratio > 2.0 and curTargetPP < 15) or   
                   (ratio > 2.5 and curTargetPP < 25  and II > 5) or 
                   (ratio > 3.0 and curTargetPP < 40  and II > 5) or 
                   (ratio > 4.0 and curTargetPP < 100  and II > 10)  or
                   (  (curTargetRP+RR-IR)/max(0.001,  curTargetPP - II+RI) > 2*  priorityRatio  )):  # we already have algo elegance and more RP would be too expensive, or overkill
                        if not printedHeader:
                            printedHeader=True
                            print "Rejecting further Research Focus choices  as too expensive:"
                            print "%34s|%20s|%15s |%15s|%15s |%15s |%15s"%("                      Planet  ", " current RP/PP ", " current target RP/PP ", "current Focus ","  rejectedFocus  ", " rejected target RP/PP ",  "rejected RP-PP EQF")
                        oldFocus=currentFocus[pid]
                        cPP, cRP = currentOutput[pid][IFocus],  currentOutput[pid][RFocus]
                        otPP, otRP= newTargets[pid].get(oldFocus,  (0, 0))
                        ntPP, ntRP= newTargets[pid].get(RFocus,  (0, 0))
                        print "pID (%3d)  %22s |  c:  %5.1f / %5.1f |   cT:  %5.1f / %5.1f  |  cF: %8s |  nF: %8s  | cT:  %5.1f / %5.1f |         %.2f"%(pid,  planetMap[pid].name, cRP, cPP,   otRP, otPP,  fociMap.get(oldFocus, 'unknown'),  fociMap[RFocus] , ntRP, ntPP , ratio)
                        continue  # RP is getting too expensive, but might be willing to still allocate from a planet with less PP to lose
            if planetMap[pid].currentMeterValue(fo.meterType.targetPopulation) > 0:
                newFoci[pid] = RFocus
                curTargetRP += (RR-IR)
                curTargetPP -= (II-RI)
        print "============================"
        print "Planet Focus Assignments to achieve target RP/PP ratio of %.2f from current ratio of %.2f  ( %.1f / %.1f )"%(priorityRatio,  rp/(pp+0.0001),  rp,  pp)
        print "Max Industry assignments would result in target RP/PP ratio of %.2f  ( %.1f / %.1f )"%( ctRP0/ (ctPP0 + 0.0001), ctRP0,  ctPP0 )
        print "-------------------------------------"
        print "%34s|%20s|%15s |%15s|%15s |%15s "%("                      Planet  ", " current RP/PP ", " current target RP/PP ", "current Focus ","  newFocus  ", " new target RP/PP ")
        totalChanged=0
        for pid in newTargets:
            oldFocus=currentFocus[pid]
            changeFocus=False
            newFocus = newFoci[pid]
            cPP, cRP = currentOutput[pid][IFocus],  currentOutput[pid][RFocus]
            if newFocus==RFocus and oldFocus!=RFocus:
                changeFocus = True
            elif newFocus == IFocus:
                if  ( MFocus in planetMap[pid].availableFoci )  and ( newTargets[pid].setdefault(MFocus,  (25, IR))[0] > newTargets[pid][IFocus][0]):
                    newFocus=MFocus
                if newFocus !=oldFocus:
                    changeFocus = True
            else:
                pass #not supporting growth focus yet
            if changeFocus:
                totalChanged+=1
                fo.issueChangeFocusOrder(pid, newFocus)
            otPP, otRP= newTargets[pid].get(oldFocus,  (0, 0))
            ntPP, ntRP= newTargets[pid].get(newFocus,  (0, 0))
            print "pID (%3d)  %22s |  c:  %5.1f / %5.1f |   cT:  %5.1f / %5.1f  |  cF: %8s |  nF: %8s  | cT:  %5.1f / %5.1f "%(pid,  planetMap[pid].name, cRP, cPP,   otRP, otPP,  fociMap.get(oldFocus, 'unknown'),  fociMap[newFocus] , ntRP, ntPP )
        print "-------------------------------------\nFinal Ratio Target (turn %4d) RP/PP : %.2f  ( %.1f / %.1f )  after %d Focus changes"%( fo.currentTurn(), curTargetRP/ (curTargetPP + 0.0001), curTargetRP,  curTargetPP ,  totalChanged)
        
    aPP, aRP = empire.productionPoints,  empire.resourceProduction(fo.resourceType.research)
    print "Current Output (turn %4d) RP/PP : %.2f  ( %.1f / %.1f )"%(fo.currentTurn(),  aRP/ (aPP + 0.0001), aRP,  aPP ), "\n------------------------"
    
    timer.append( time() ) #end
    if doResourceTiming and __timerEntries==__timerEntries2:
        times = [timer[i] - timer[i-1] for i in range(1,  len(timer) ) ]
        timeFmt = "%30s: %8d msec  "
        print "ResourcesAI Time Requirements:"
        for mod,  modTime in zip(__timerEntries,  times):
            print timeFmt%((30*' '+mod)[-30:],  int(1000*modTime))
        if resourceTimerFile:
            print "len times: %d  ;  len entries: %d "%(len(times),  len(__timerEntries))
            resourceTimerFile.write(  __timerFileFmt%tuple( [ fo.currentTurn() ]+map(lambda x: int(1000*x),  times )) +'\n')
            resourceTimerFile.flush()

def generateResourcesOrders():
    global __timerFile
    "generate resources focus orders"

    timer= [ time() ]
    # calculate top resource priority
    #topResourcePriority()
    timer.append( time() )
    # set resource foci of planets
    #setCapitalIDResourceFocus()
    timer.append( time() )
    #setGeneralPlanetResourceFocus()
    setPlanetResourceFoci()
    timer.append( time() )
    #setAsteroidsResourceFocus()
    timer.append( time() )
    #setGasGiantsResourceFocus()
    timer.append( time() )

    printResourcesPriority()
    timer.append( time() )

    if doResourceTiming and __timerEntries==__timerEntries1:
        times = [timer[i] - timer[i-1] for i in range(1,  len(timer) ) ]
        timeFmt = "%30s: %8d msec  "
        print "ResourcesAI Time Requirements:"
        for mod,  modTime in zip(__timerEntries,  times):
            print timeFmt%((30*' '+mod)[-30:],  int(1000*modTime))
        if resourceTimerFile:
            resourceTimerFile.write(  __timerFileFmt%tuple( [ fo.currentTurn() ]+map(lambda x: int(1000*x),  times )) +'\n')
            resourceTimerFile.flush()
