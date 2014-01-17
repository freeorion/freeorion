import freeOrionAIInterface as fo # pylint: disable=import-error
import FreeOrionAI as foAI
import AIstate
from EnumsAI import AIPriorityType, getAIPriorityResourceTypes, AIFocusType
import PlanetUtilsAI
from random import shuffle,  random
from time import time
import ColonisationAI
import AIDependencies

AIPriorityTypeNames=AIPriorityType()

resourceTimerFile=None
doResourceTiming=True
__timerEntries1=["TopResources",  "SetCapital",  "SetPlanets",  "SetAsteroids",  "SetGiants",  "PrintResources" ]
__timerEntries2=["getPlanets",  "Filter",  "Priority",  "Shuffle",  "Targets",  "Loop" ]
timer_entries = __timerEntries2
__timerFileFmt = "%8d"+ (len(timer_entries)*"\t %8d")

oldTargets={}
newTargets={}
currentFocus = {}
currentOutput={}
planetMap = {}
IFocus = AIFocusType.FOCUS_INDUSTRY
RFocus = AIFocusType.FOCUS_RESEARCH
MFocus = AIFocusType.FOCUS_MINING
GFocus = AIFocusType.FOCUS_GROWTH

useGrowth = True
limitAssessments = True

lastFociCheck=[0]

def getResourceTargetTotals(empirePlanetIDs):#+
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    #empirePlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    capitalID = PlanetUtilsAI.getCapital()
    oldTargets.clear()
    oldTargets.update(newTargets)
    newTargets.clear()
    currentFocus.clear()
    currentOutput.clear()

    targetPP = sum( map( lambda x: x.currentMeterValue(fo.meterType.targetIndustry),  planetMap.values()) )
    targetRP = sum( map( lambda x: x.currentMeterValue(fo.meterType.targetResearch),  planetMap.values()) )

    newFocus= IFocus
    for pid in empirePlanetIDs:
        planet=planetMap[pid]
        #canFocus= planetMap[pid].currentMeterValue(fo.meterType.targetPopulation) >0
        currentFocus[pid] = planet.focus
        #if  currentFocus[pid] == MFocus:
        #    mtarget=planetMap[pid].currentMeterValue(fo.meterType.targetIndustry)
        currentOutput.setdefault(pid,  {} )[ IFocus] =  planet.currentMeterValue(fo.meterType.industry)
        currentOutput[pid][ RFocus] =  planet.currentMeterValue(fo.meterType.research)
        if IFocus in planet.availableFoci and planet.focus !=IFocus:
            fo.issueChangeFocusOrder(pid, IFocus) #may not be able to take, but try
    universe.updateMeterEstimates(empirePlanetIDs)
    for pid in empirePlanetIDs:
        planet=planetMap[pid]
        itarget=planet.currentMeterValue(fo.meterType.targetIndustry)
        rtarget=planet.currentMeterValue(fo.meterType.targetResearch)
        if planet.focus == IFocus:
            newTargets.setdefault(pid,  {})[IFocus] = ( itarget,  rtarget )
            newTargets.setdefault(pid,  {})[GFocus] = [0,  rtarget]
        else:
            newTargets.setdefault(pid,  {})[IFocus] = ( 0, 0)
            newTargets.setdefault(pid,  {})[GFocus] = [0, 0]
        #if  currentFocus[pid] == MFocus:
        #    newTargets[pid][MFocus] = ( mtarget,  rtarget )
        if RFocus in planet.availableFoci and planet.focus!=RFocus:
            fo.issueChangeFocusOrder(pid, RFocus) #may not be able to take, but try
    universe.updateMeterEstimates(empirePlanetIDs)
    for pid in empirePlanetIDs:
        planet=planetMap[pid]
        canFocus= planet.currentMeterValue(fo.meterType.targetPopulation) >0
        itarget=planet.currentMeterValue(fo.meterType.targetIndustry)
        rtarget=planet.currentMeterValue(fo.meterType.targetResearch)
        if planet.focus == RFocus:
            newTargets.setdefault(pid,  {})[RFocus] = ( itarget,  rtarget )
            newTargets[pid][GFocus][0] = itarget 
        else:
            newTargets.setdefault(pid,  {})[RFocus] = ( 0,  0 )
            newTargets[pid][GFocus][0] = 0 
        if canFocus and currentFocus[pid]  != planet.focus:
            fo.issueChangeFocusOrder(pid, currentFocus[pid]) #put it back to what it was
    universe.updateMeterEstimates(empirePlanetIDs)
    return targetPP,  targetRP

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

def setPlanetResourceFoci(): #+
    "set resource focus of planets "
    newFoci = {}

    print "\n============================"
    print "Collecting info to assess Planet Focus Changes\n"
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    currentTurn = fo.currentTurn()
    freq = min(3,  ( max(5,  currentTurn-80)   )/4.0)**(1.0/3)
    if  limitAssessments and ( abs(currentTurn - lastFociCheck[0] ) <1.5*freq)   and ( random() < 1.0/freq ) :
        timer = 6*[time()]
    else:
        lastFociCheck[0]=currentTurn
        timer= [ time() ] # getPlanets
        empirePlanetIDs = list( PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID) )
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
        planets = map(universe.getPlanet,  empirePlanetIDs)
        planetMap.clear()
        planetMap.update( zip( empirePlanetIDs,  planets))
        if useGrowth:
            for metab,  metabIncPop in ColonisationAI.empireMetabolisms.items():
                for special in [aspec for aspec in AIDependencies.metabolimBoostMap.get(metab,  []) if aspec in ColonisationAI.availableGrowthSpecials]:
                    rankedPlanets=[]
                    for pid in ColonisationAI.availableGrowthSpecials[special]:
                        planet = planetMap[pid]
                        cur_focus = planet.focus
                        pop = planet.currentMeterValue(fo.meterType.population)
                        if (pop > metabIncPop - 2*planet.size) or (GFocus not in planet.availableFoci): #not enough benefit to lose local production, or can't put growth focus here
                            continue
                        for special2 in [ "COMPUTRONIUM_SPECIAL"  ]:
                            if special2 in planet.specials:
                                break
                        else: #didn't have any specials that would override interest in growth special
                            print "Considering  Growth Focus for %s (%d) with special %s; planet has pop %.1f and %s metabolism incremental pop is %.1f"%(
                                                                                                                                                          planet.name,  pid,  special,  pop,  metab,  metabIncPop)
                            if cur_focus == GFocus:
                                pop -=4 #discourage changing current focus to minimize focus-changing penalties
                            rankedPlanets.append(  (pop,  pid,  cur_focus) )
                    if rankedPlanets == []:
                        continue
                    rankedPlanets.sort()
                    print "Considering  Growth Focus choice for  special %s; possible planet pop,  id pairs are %s"%(metab,  rankedPlanets)
                    for spSize,  spPID,  cur_focus in rankedPlanets: #index 0 should be able to set focus, but just in case...
                        result = 1
                        if cur_focus != GFocus:
                            result = fo.issueChangeFocusOrder(spPID, GFocus)
                        if result == 1:
                            newFoci[spPID] = GFocus
                            if spPID in empirePlanetIDs:
                                del empirePlanetIDs[   empirePlanetIDs.index( spPID ) ]
                            print "%s focus of planet %s (%d) at Growth Focus"%( ["set",  "left" ][  cur_focus == GFocus ] ,  planetMap[spPID].name,  spPID)
                            break
                        else:
                            print "failed setting focus of planet %s (%d) at Growth Focus; focus left at %s"%(  planetMap[spPID].name,  spPID,  planetMap[spPID].focus)
        for pid in empirePlanetIDs:
            planet = planetMap[pid]
            if "COMPUTRONIUM_SPECIAL" in planet.specials:#TODO: ensure only one (extremely rarely needed)
                curFocus = planet.focus
                if RFocus not in planet.availableFoci:
                    continue
                newFoci[pid] = RFocus
                result=0
                if curFocus != RFocus:
                    result = fo.issueChangeFocusOrder(pid, RFocus)
                    if result == 1:
                        universe.updateMeterEstimates(empirePlanetIDs)
                if curFocus == RFocus  or result==1:
                    print "%s focus of planet %s (%d) (with Computronium Moon) at Research Focus"%( ["set",  "left" ][  curFocus == RFocus ] ,  planetMap[pid].name,  pid)
                    if pid in empirePlanetIDs:
                        del empirePlanetIDs[   empirePlanetIDs.index( pid ) ]
            elif  ( ([bld.buildingTypeName for bld in map( universe.getObject,  planet.buildingIDs) if bld.buildingTypeName in
                                                ["BLD_CONC_CAMP", "BLD_CONC_CAMP_REMNANT"]] != []  ) or
                             (  [  ccspec  for ccspec in planet.specials if ccspec in [ "CONC_CAMP_MASTER_SPECIAL",  "CONC_CAMP_SLAVE_SPECIAL"  ]  ]  != [] )):
                if IFocus not in planet.availableFoci:
                    continue
                curFocus = planet.focus
                newFoci[pid] = IFocus
                result=0
                if curFocus != IFocus:
                    result = fo.issueChangeFocusOrder(pid, IFocus)
                    if result == 1:
                        print ("Tried setting %s for Concentration Camp  planet %s (%d) with species %s and current focus %s, got result %d and focus %s"%
                               ( newFoci[pid],  planet.name,  pid,  planet.speciesName, curFocus,  result,  planetMap[pid].focus ))
                        universe.updateMeterEstimates(empirePlanetIDs)
                    if (result != 1) or planetMap[pid].focus != IFocus:
                        newplanet=universe.getPlanet(pid)
                        print ("Error: Failed setting %s for Concentration Camp  planet %s (%d) with species %s and current focus %s, but new planet copy shows %s"%
                               ( newFoci[pid],  planetMap[pid].name,  pid,  planetMap[pid].speciesName, planetMap[pid].focus,  newplanet.focus ))
                if curFocus == IFocus  or result==1:
                    print "%s focus of planet %s (%d) (with Concentration Camps/Remnants) at Industry Focus"%( ["set",  "left" ][  curFocus == IFocus ] ,  planetMap[pid].name,  pid)
                    if pid in empirePlanetIDs:
                        del empirePlanetIDs[   empirePlanetIDs.index( pid ) ]

        #pp, rp = getResourceTargetTotals(empirePlanetIDs,  planetMap)
        pp, rp = getResourceTargetTotals(planetMap.keys())
        print "\n-----------------------------------------"
        print "Making Planet Focus Change Determinations\n"

        ratios = []
        #for each planet, calculate RP:PP value ratio at which industry/Mining focus and research focus would have the same total value, & sort by that
        # include a bias to slightly discourage changing foci
        curTargetPP = 0.001
        curTargetRP = 0.001
        timer.append( time() ) #loop
        has_force = empire.getTechStatus("CON_FRC_ENRG_STRC") == fo.techStatus.complete
        preset_ids = set(planetMap.keys()) - set(empirePlanetIDs)
        ctPP0,  ctRP0 = 0, 0
        for pid in preset_ids:
            nPP,  nRP  = newTargets.get(pid, {}).get( planetMap[pid].focus,  [0, 0] )
            curTargetPP += nPP
            curTargetRP +=  nRP
            iPP,  iRP  = newTargets.get(pid, {}).get( IFocus,  [0, 0] )
            ctPP0 += iPP
            ctRP0 += iRP
        
        id_set = set(empirePlanetIDs)
        for adj_round in [1, 2, 3, 4]:
            maxi_ratio = ctRP0 / max ( 0.01,  ctPP0 ) #should only change between rounds 1 and 2
            for pid in list(id_set):
                if round == 1: #tally max Industry
                    iPP,  iRP  = newTargets.get(pid, {}).get( IFocus,  [0, 0] )
                    ctPP0 += iPP
                    ctRP0 += iRP
                    continue
                II, IR = newTargets[pid][IFocus]
                RI, RR = newTargets[pid][RFocus]
                CI, CR = currentOutput[pid][ IFocus],  currentOutput[pid][ RFocus]
                research_penalty = (currentFocus[pid] != RFocus)
                #calculate factor F at  which     II + F * IR  ==  RI + F * RR   =====>  F = ( II-RI ) / (RR-IR)
                thisFactor = ( II-RI ) / max( 0.01,  RR-IR)  # don't let denominator be zero for planets where focus doesn't change RP
                planet = planetMap[pid]
                if round == 2: #take research at planets with very cheap research
                    if  (maxi_ratio < priorityRatio) and (curTargetRP < priorityRatio * ctPP0) and (thisFactor <=1.0):
                        curTargetPP += RI #
                        curTargetRP += RR
                        newFoci[pid] = RFocus
                        id_set.discard(pid)
                    continue
                if (round == 3): #take research at planets where can do reasonable balance
                    if (has_force) or (foAI.foAIstate.aggression < fo.aggression.aggressive) or (curTargetRP >= priorityRatio * ctPP0):
                        continue
                    pop = planet.currentMeterValue(fo.meterType.population)
                    t_pop = planet.currentMeterValue(fo.meterType.targetPopulation)
                    #if AI is aggressive+, and this planet in range where temporary Research focus can get an additional RP at cost of 1 PP, and still need some RP, then do it
                    if (pop < t_pop - 5):
                        continue
                    if  ( CI > II + 8) or (( (RR>II) or ((RR-CR)>=1+2*research_penalty)) and ((RR-IR)>=3) and ( (CR-IR) >= 0.7*((II-CI)(1+0.1*research_penalty) ) )):
                        curTargetPP += CI -1 - research_penalty#
                        curTargetRP +=  CR+1
                        newFoci[pid] = RFocus
                        id_set.discard(pid)
                    continue
                #round == 4 assume default IFocus
                curTargetPP += II  #icurTargets initially calculated by Industry focus, which will be our default focus
                curTargetRP += IR
                newFoci[pid] = IFocus
                ratios.append( (thisFactor,  pid ) )

        ratios.sort()
        printedHeader=False
        fociMap={IFocus:"Industry",  RFocus:"Research", MFocus:"Mining",  GFocus:"Growth"}
        gotAlgo = empire.getTechStatus("LRN_ALGO_ELEGANCE") == fo.techStatus.complete
        for ratio,  pid in ratios:
            do_research = False#(newFoci[pid]==RFocus)
            if (priorityRatio < ( curTargetRP/ (curTargetPP + 0.0001)))  and not do_research: #we have enough RP
                if ratio < 1.1  and foAI.foAIstate.aggression >fo.aggression.cautious :  #but wait, RP is still super cheap relative to PP, maybe will take more RP
                    if priorityRatio < 1.5* ( curTargetRP/ (curTargetPP + 0.0001)) : #yeah, really a glut of RP, stop taking RP
                        break
                else: #RP not super cheap & we have enough, stop taking it
                    break
            II, IR = newTargets[pid][IFocus]
            RI, RR = newTargets[pid][RFocus]
            #if currentFocus[pid] == MFocus:
            #    II = max( II,  newTargets[pid][MFocus][0] )
            if do_research or (gotAlgo and (
                   (ratio > 2.0 and curTargetPP < 15) or
                   (ratio > 2.5 and curTargetPP < 25  and II > 5) or
                   (ratio > 3.0 and curTargetPP < 40  and II > 5) or
                   (ratio > 4.0 and curTargetPP < 100  and II > 10)  or
                   (  (curTargetRP+RR-IR)/max(0.001,  curTargetPP - II+RI) > 2*  priorityRatio  ))):  # we already have algo elegance and more RP would be too expensive, or overkill
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
            #if planetMap[pid].currentMeterValue(fo.meterType.targetPopulation) >0: #only set to research if  pop won't die out
            newFoci[pid] = RFocus
            curTargetRP += (RR-IR)
            curTargetPP -= (II-RI)
        print "============================"
        print "Planet Focus Assignments to achieve target RP/PP ratio of %.2f from current ratio of %.2f  ( %.1f / %.1f )"%(priorityRatio,  rp/(pp+0.0001),  rp,  pp)
        print "Max Industry assignments would result in target RP/PP ratio of %.2f  ( %.1f / %.1f )"%( ctRP0/ (ctPP0 + 0.0001), ctRP0,  ctPP0 )
        print "-------------------------------------"
        print "%34s|%20s|%15s |%15s|%15s |%15s "%("                      Planet  ", " current RP/PP ", " current target RP/PP ", "current Focus ","  newFocus  ", " new target RP/PP ")
        totalChanged=0
        for id_set in empirePlanetIDs,  preset_ids:
            for pid in id_set:
                canFocus= planetMap[pid].currentMeterValue(fo.meterType.targetPopulation) >0
                oldFocus=currentFocus[pid]
                newFocus = newFoci.get(pid,  IFocus)
                cPP, cRP = currentOutput[pid][IFocus],  currentOutput[pid][RFocus]
                otPP, otRP= newTargets[pid].get(oldFocus,  (0, 0))
                ntPP, ntRP = otPP, otRP
                if newFocus != oldFocus and newFocus in planetMap[pid].availableFoci:  #planetMap[pid].focus
                    totalChanged+=1
                    if newFocus != planetMap[pid].focus:
                        result = fo.issueChangeFocusOrder(pid, newFocus)
                        if result == 1:
                            ntPP, ntRP= newTargets[pid].get(newFocus,  (0, 0))
                        else:
                            print "Trouble changing focus of planet %s (%d) to %s"%(planetMap[pid].name,  pid,  newFocus)
                print "pID (%3d)  %22s |  c:  %5.1f / %5.1f |   cT:  %5.1f / %5.1f  |  cF: %8s |  nF: %8s  | cT:  %5.1f / %5.1f "%(pid,  planetMap[pid].name, cRP, cPP,   otRP, otPP,  fociMap.get(oldFocus, 'unknown'),  fociMap[newFocus] , ntRP, ntPP )
            print "-------------------------------------"
        print "-------------------------------------\nFinal Ratio Target (turn %4d) RP/PP : %.2f  ( %.1f / %.1f )  after %d Focus changes"%( fo.currentTurn(), curTargetRP/ (curTargetPP + 0.0001), curTargetRP,  curTargetPP ,  totalChanged)

    aPP, aRP = empire.productionPoints,  empire.resourceProduction(fo.resourceType.research)
    print "Current Output (turn %4d) RP/PP : %.2f  ( %.1f / %.1f )"%(fo.currentTurn(),  aRP/ (aPP + 0.0001), aRP,  aPP ), "\n------------------------"

    timer.append( time() ) #end
    if doResourceTiming and timer_entries==__timerEntries2:
        times = [timer[i] - timer[i-1] for i in range(1,  len(timer) ) ]
        timeFmt = "%30s: %8d msec  "
        print "ResourcesAI Time Requirements:"
        for mod,  modTime in zip(timer_entries,  times):
            print timeFmt%((30*' '+mod)[-30:],  int(1000*modTime))
        if resourceTimerFile:
            print "len times: %d  ;  len entries: %d "%(len(times),  len(timer_entries))
            resourceTimerFile.write(  __timerFileFmt%tuple( [ fo.currentTurn() ]+map(lambda x: int(1000*x),  times )) +'\n')
            resourceTimerFile.flush()

def generateResourcesOrders(): #+
    "generate resources focus orders"

    timer= [ time() ]
    ## calculate top resource priority
    ##topResourcePriority()
    timer.append( time() )
    ## set resource foci of planets
    ##setCapitalIDResourceFocus()
    timer.append( time() )
    #------------------------------
    ##setGeneralPlanetResourceFocus()
    setPlanetResourceFoci()
    timer.append( time() )
    #-------------------------------
    ##setAsteroidsResourceFocus()
    timer.append( time() )
    ##setGasGiantsResourceFocus()
    timer.append( time() )

    printResourcesPriority()
    timer.append( time() )

    if doResourceTiming and timer_entries==__timerEntries1:
        times = [timer[i] - timer[i-1] for i in range(1,  len(timer) ) ]
        timeFmt = "%30s: %8d msec  "
        print "ResourcesAI Time Requirements:"
        for mod,  modTime in zip(timer_entries,  times):
            print timeFmt%((30*' '+mod)[-30:],  int(1000*modTime))
        if resourceTimerFile:
            resourceTimerFile.write(  __timerFileFmt%tuple( [ fo.currentTurn() ]+map(lambda x: int(1000*x),  times )) +'\n')
            resourceTimerFile.flush()
