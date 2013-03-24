import freeOrionAIInterface as fo
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

useGrowth = True

lastFociCheck=[0]

def getResourceTargetTotals(empirePlanetIDs,  planetMap):#+
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
        newTargets.setdefault(pid,  {})[IFocus] = ( itarget,  rtarget )
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
        newTargets.setdefault(pid,  {})[RFocus] = ( itarget,  rtarget )
        #if canFocus and currentFocus[pid]  != planet.focus:
        #    fo.issueChangeFocusOrder(pid, currentFocus[pid]) #put it back to what it was
    #universe.updateMeterEstimates(empirePlanetIDs)
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
    global __timerFile
    newFoci = {}

    print "\n============================"
    print "Collecting info to assess Planet Focus Changes\n"
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    currentTurn = fo.currentTurn()
    freq = min(3,  ( max(5,  currentTurn-120)   )/4.0)**(1.0/3) 
    if  ( abs(currentTurn - lastFociCheck[0] ) <1.5*freq)   and ( random() < 1.0/freq ) :
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
        planetMap = dict(  zip( empirePlanetIDs,  planets)) 
        if useGrowth:
            for metab,  metabIncPop in ColonisationAI.empireMetabolisms.items():
                for special in [aspec for aspec in AIDependencies.metabolimBoostMap.get(metab,  []) if aspec in ColonisationAI.availableGrowthSpecials]:
                    rankedPlanets=[]
                    for pid in ColonisationAI.availableGrowthSpecials[special]:
                        planet = planetMap[pid]
                        pop = planet.currentMeterValue(fo.meterType.population)
                        if (pop > metabIncPop - planet.size) or (GFocus not in planet.availableFoci): #not enough benefit to lose local production, or can't put growth focus here
                            continue
                        for special2 in [ "COMPUTRONIUM_SPECIAL"  ]:
                            if special2 in planet.specials:
                                break
                        else: #didn't have any specials that would override interest in growth special
                            print "Considering  Growth Focus for %s (%d) with special %s; planet has pop %.1f and %s metabolism incremental pop is %.1f"%(
                                                                                                                                                          planet.name,  pid,  special,  pop,  metab,  metabIncPop)
                            rankedPlanets.append(  (pop,  pid) )
                    if rankedPlanets == []:
                        continue
                    rankedPlanets.sort()
                    print "Considering  Growth Focus choice for  special %s; possible planet pop,  id pairs are %s"%(metab,  rankedPlanets)
                    for spSize,  spPID in rankedPlanets: #index 0 should be able to set focus, but just in case...
                        result = 1
                        curFocus = planet.focus
                        if curFocus != GFocus:
                            result = fo.issueChangeFocusOrder(spPID, GFocus)
                        if result == 1:
                            if spPID in empirePlanetIDs:
                                del empirePlanetIDs[   empirePlanetIDs.index( spPID ) ]
                            print "%s focus of planet %s (%d) at Growth Focus"%( ["set",  "left" ][  curFocus == GFocus ] ,  planetMap[spPID].name,  spPID) 
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
                    if pid in empirePlanetIDs:
                        del empirePlanetIDs[   empirePlanetIDs.index( pid ) ]
            elif  ( ("BLD_CONC_CAMP" in [bld.buildingTypeName for bld in map( universe.getObject,  planet.buildingIDs)] ) or
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
                    if pid in empirePlanetIDs:
                        del empirePlanetIDs[   empirePlanetIDs.index( pid ) ]
                            
        pp, rp = getResourceTargetTotals(empirePlanetIDs,  planetMap)
        print "\n-----------------------------------------"
        print "Making Planet Focus Change Determinations\n"
        
        ratios = []
        #for each planet, calculate RP:PP value ratio at which industry/Mining focus and research focus would have the same total value, & sort by that
        # include a bias to slightly discourage changing foci
        curTargetPP = 0.001
        curTargetRP = 0.001
        timer.append( time() ) #loop
        hasForce = empire.getTechStatus("CON_FRC_ENRG_STRC") == fo.techStatus.complete
        for pid in newTargets:
            if pid in newFoci:
                if planetMap[pid].focus == newFoci[pid]:
                    nPP,  nRP  = newTargets[pid].get( newFoci[pid],  [0, 0] )
                    curTargetPP += nPP
                    curTargetRP +=  nRP
                    continue
                else:
                    print "Error: new focus %s set early but not applied for planet %s (%d) with species %s"%( newFoci[pid],  planetMap[pid].name,  pid,  planetMap[pid].speciesName )
            II, IR = newTargets[pid][IFocus]
            RI, RR = newTargets[pid][RFocus]
            CI, CR = currentOutput[pid][ IFocus],  currentOutput[pid][ RFocus]
            #consider straddling balance range within which 1RP costs 1PP
            if True and (foAI.foAIstate.aggression >= fo.aggression.aggressive):
                if (CR<RR) and ( (CR-IR) >= (II-CI) ) and (priorityRatio > ( (curTargetRP+CR+1)/ max(0.001, curTargetPP +CI -1))):
                    curTargetPP += CI -1 #
                    curTargetRP +=  CR+1
                    newFoci[pid] = RFocus
                    continue
            curTargetPP += II  #icurTargets initially calculated by Industry focus, which will be our default focus
            curTargetRP += IR
            newFoci[pid] = IFocus
            #if foAI.foAIstate.aggression < fo.aggression.maniacal:
            #    if currentFocus[pid] in [ IFocus,  MFocus] :
            #        II += min( 2,  II /4.0 )
            #    elif currentFocus[pid] == RFocus:
            #        RR += min( 2,  RR/4.0 )
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
            #if currentFocus[pid] == MFocus:
            #    II = max( II,  newTargets[pid][MFocus][0] ) 
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
        for pid in newTargets:
            canFocus= planetMap[pid].currentMeterValue(fo.meterType.targetPopulation) >0
            oldFocus=currentFocus[pid]
            newFocus = newFoci[pid]
            cPP, cRP = currentOutput[pid][IFocus],  currentOutput[pid][RFocus]
            if newFocus!= planetMap[pid].focus and newFocus in planetMap[pid].availableFoci:
                totalChanged+=1
                result = fo.issueChangeFocusOrder(pid, newFocus)
                if result != 1:
                    print "Trouble changing focus of planet %s (%d) to %s"%(planetMap[pid].name,  pid,  newFocus)
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

def generateResourcesOrders(): #+
    global __timerFile
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

    if doResourceTiming and __timerEntries==__timerEntries1:
        times = [timer[i] - timer[i-1] for i in range(1,  len(timer) ) ]
        timeFmt = "%30s: %8d msec  "
        print "ResourcesAI Time Requirements:"
        for mod,  modTime in zip(__timerEntries,  times):
            print timeFmt%((30*' '+mod)[-30:],  int(1000*modTime))
        if resourceTimerFile:
            resourceTimerFile.write(  __timerFileFmt%tuple( [ fo.currentTurn() ]+map(lambda x: int(1000*x),  times )) +'\n')
            resourceTimerFile.flush()
