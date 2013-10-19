import freeOrionAIInterface as fo # pylint: disable=import-error

import AIFleetOrder
import AITarget
import AIstate
import FleetUtilsAI
import FreeOrionAI as foAI
import MoveUtilsAI
import ProductionAI
import AIAbstractMission
import EnumsAI
import MilitaryAI


AIFleetMissionTypeNames = EnumsAI.AIFleetMissionType()
AIShipRoleTypeNames = EnumsAI.AIShipRoleType()

def dictFromMap(thismap):
    return dict(  [  (el.key(),  el.data() ) for el in thismap ] )

class AIFleetMission(AIAbstractMission.AIAbstractMission):
    '''
    Stores information about AI mission. Every mission has fleetID and AI targets depending upon AI fleet mission type.
    '''

    def __init__(self, fleetID):
        "constructor"

        AIAbstractMission.AIAbstractMission.__init__(self, EnumsAI.AIMissionType.FLEET_MISSION, EnumsAI.AITargetType.TARGET_FLEET, fleetID)
        self.__aiFleetOrders = []

    def __str__(self):
        "returns describing string"

        missionStrings=[]
        for aiFleetMissionType in self.getAIMissionTypes():
            universe = fo.getUniverse()
            fleetID = self.target_id
            fleet = universe.getFleet(fleetID)
            targetsString = "fleet %4d (%14s) [ %10s mission ] : %3d ships , total Rating:%7d "%(fleetID,  (fleet and fleet.name) or "Fleet Invalid",
                                                                                                 AIFleetMissionTypeNames.name(aiFleetMissionType) ,  (fleet and len(fleet.shipIDs)) or 0,  foAI.foAIstate.getRating(fleetID).get('overall', 0))
            targets = self.getAITargets(aiFleetMissionType)
            for target in targets:
                targetsString = targetsString + str(target)
            missionStrings.append( targetsString  )
        return "\n".join(missionStrings)

    def __getRequiredToVisitSystemAITargets(self):
        "returns all system AITargets required to visit in this object"

        result = []
        for aiFleetMissionType in self.getAIMissionTypes():
            aiTargets = self.getAITargets(aiFleetMissionType)
            for aiTarget in aiTargets:
                result.extend(aiTarget.get_required_system_ai_targets())
        return result

    def getVisitingSystemAITargets(self):
        "returns all system AITargets which will be visited"

        result = []
        for aiFleetOrder in self.getAIFleetOrders():
            if aiFleetOrder.getAIFleetOrderType() == EnumsAI.AIFleetOrderType.ORDER_MOVE:
                result.append(aiFleetOrder.getTargetAITarget())
        return result

    def getAIFleetOrders(self):
        return self.__aiFleetOrders

    def appendAIFleetOrder(self, aiFleetOrder):
        self.__aiFleetOrders.append(aiFleetOrder)

    def hasAIFleetOrder(self, aiFleetOrder):
        aiFleetOrders = self.getAIFleetOrders()
        return aiFleetOrders.__contains__(aiFleetOrder)

    def removeAIFleetOrder(self, aiFleetOrder):
        result = []
        for fleetOrder in self.__aiFleetOrders:
            if fleetOrder.__cmp__(aiFleetOrder) != 0:
                result.append(fleetOrder)
        self.__aiFleetOrders = result

        del aiFleetOrder

    def clearAIFleetOrders(self):
        self.__aiFleetOrders = []

    def __getAIFleetOrderFromAITarget(self, aiFleetMissionType, aiTarget):
        fleetAITarget = AITarget.AITarget(EnumsAI.AITargetType.TARGET_FLEET, self.target_id)
        orderType = EnumsAI.getFleetOrderTypeForMission(aiFleetMissionType,  option=None)
        result = AIFleetOrder.AIFleetOrder(orderType, fleetAITarget, aiTarget)
        return result

    def checkMergers(self,  fleetID=None,  context=""):
        if fleetID==None:
            fleetID = self.target_id
        #mainFleetMission=foAI.foAIstate.getAIFleetMission(fleetID)
        mainMissionTypeList = self.getAIMissionTypes()  #normally, currently, should only be one
        if len( mainMissionTypeList ) != 1:
            return #if this fleet has multiple mission types, will not subsume other fleets
        fleetRoleB =  foAI.foAIstate.getFleetRole(fleetID)
        mainMissionType = mainMissionTypeList[0]
        if mainMissionType not in [  EnumsAI.AIFleetMissionType.FLEET_MISSION_ATTACK,
                                                                        EnumsAI.AIFleetMissionType.FLEET_MISSION_DEFEND,
                                                                        EnumsAI.AIFleetMissionType.FLEET_MISSION_LAST_STAND ,
                                                                        EnumsAI.AIFleetMissionType.FLEET_MISSION_MILITARY,
                                                                        EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION,
                                                                        EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION,
                                                                        EnumsAI.AIFleetMissionType.FLEET_MISSION_SECURE,
                                                                        EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE,
                                                                    ]:
            return
        universe = fo.getUniverse()
        empireID = fo.empireID()
        fleetB = universe.getFleet(fleetID)
        systemID = fleetB.systemID
        if systemID == -1:
            return # can't merge fleets in middle of starlane
        sysStatus = foAI.foAIstate.systemStatus[systemID]
        destroyedList = list( universe.destroyedObjectIDs(empireID) )
        otherFleetsHere= [fid for fid in sysStatus.get('myFleetsAccessible', []) if ( (fid != fleetID) and (fid not in destroyedList) ) ]
        if otherFleetsHere==[]:
            return #nothing of record to merge with
        mainMissionTargets = self.getAITargets(mainMissionType)
        if mainMissionTargets == []:
            pass
            #return  #let's let invasion fleets with no target get merged
            mMT0=None
            mMT0ID = None
        else:
            mMT0=mainMissionTargets[0]
            mMT0ID = mMT0.target_id
        if len(mainMissionTargets)>1:
            pass
            print "\tConsidering merging fleets into  fleet  %d, but it has multiple targets: %s"%(fleetID,  str(mainMissionTargets))
        sys1=universe.getSystem(systemID)
        sysName = (sys1 and sys1.name) or "unknown"
        compatibileRolesMap={ EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE:         [EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE],
                                                                EnumsAI.AIFleetMissionType.FLEET_MISSION_MILITARY:                               [EnumsAI.AIFleetMissionType.FLEET_MISSION_MILITARY],
                                                                EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION:        [EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION],
                                                                EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION:                              [EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION],
                                                                }
        for fid in otherFleetsHere:
            fleetRoleA = foAI.foAIstate.getFleetRole(fid)
            if fleetRoleA not in  compatibileRolesMap[fleetRoleB] : #TODO: if fleetRoles such as LongRange start being used, adjust this
                continue # will only considering subsuming fleets that have a compatible role
            fleet2 = universe.getFleet(fid)
            if not (fleet2 and (fleet2.systemID == systemID)):
                continue
            if not (fleet2.ownedBy(foAI.foAIstate.empireID) and ( (fleet2.speed > 0) or (fleetB.speed == 0)  )):
                continue
            f2Mission=foAI.foAIstate.getAIFleetMission(fid)
            doMerge=False
            needLeft=0
            if  ( fleetRoleA== EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE ) or  (fleetRoleB== EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE ):
                if fleetRoleA==fleetRoleB:
                    doMerge=True
            elif  ( fleetRoleA== EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION ) or  (fleetRoleB== EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION ):
                if fleetRoleA==fleetRoleB:
                    doMerge=False#TODO: could  allow merger if both orb invaders and both same target
            elif not f2Mission and (fleetB.speed > 0) and (fleet2.speed > 0):
                doMerge=True
            else:
                f2MType = (f2Mission.getAIMissionTypes()+[-1])[0]
                f2Targets = f2Mission.getAITargets(f2MType)
                if len(f2Targets)>1:
                    pass
                elif len(f2Targets)==0 and ( (fleetB.speed > 0) or (fleet2.speed == 0)  ):
                    #print "\t\t\t ** Considering merging  fleetA (id: %4d)  into fleetB (id %d  ) and former has no targets, will take it.  FleetA mission was %s   "%(fid, fleetID,   f2Mission)
                    doMerge=True
                else:
                    targetB = f2Targets[0].target_id
                    if targetB == mMT0ID:
                        print "Military fleet %d has same target as %s fleet %d and will (at least temporarily) be merged into the latter"%(fid, AIFleetMissionTypeNames.name( fleetRoleB)  ,  fleetID)
                        doMerge=True #TODO: should probably ensure that fleetA  has aggression on now
                    elif (fleetB.speed > 0):
                        neighbors = foAI.foAIstate.systemStatus.get(systemID,  {}).get('neighbors', [])
                        if (targetB==systemID) and mMT0ID in neighbors: #consider 'borrowing' for work in neighbor system
                            if f2MType  in [  EnumsAI.AIFleetMissionType.FLEET_MISSION_ATTACK,
                                                                EnumsAI.AIFleetMissionType.FLEET_MISSION_DEFEND,
                                                                EnumsAI.AIFleetMissionType.FLEET_MISSION_MILITARY,
                                                                EnumsAI.AIFleetMissionType.FLEET_MISSION_SECURE,
                                                                ]:
                                #continue
                                if f2MType  in [  EnumsAI.AIFleetMissionType.FLEET_MISSION_DEFEND,
                                                                    EnumsAI.AIFleetMissionType.FLEET_MISSION_SECURE, #actually, currently this is probably the onle one of all four that should really be possibile in this situation
                                                                    ]:
                                    needLeft = 1.5*sum( [  sysStat.get('fleetThreat', 0) for sysStat in
                                                                                                          [foAI.foAIstate.systemStatus.get(neighbor, {}) for neighbor in
                                                                                                          [ nid for nid in foAI.foAIstate.systemStatus.get(systemID, {}).get('neighbors', []) if nid != mMT0ID    ]   ]  ] )
                                    fBRating = foAI.foAIstate.getRating(fid)
                                    if (needLeft < fBRating.get('overall', 0)) and fBRating.get('nships', 0)>1 :
                                        doMerge=True
            if doMerge:
                FleetUtilsAI.mergeFleetAintoB(fid,  fleetID,  needLeft,  context="Order %s  of mission %s"%(context,  str(self)))
        return

    def isValidFleetMissionAITarget(self, aiFleetMissionType, aiTarget):
        if not aiTarget.valid:
            return False
        if aiFleetMissionType == EnumsAI.AIFleetMissionType.FLEET_MISSION_EXPLORATION:
            if aiTarget.target_type == EnumsAI.AITargetType.TARGET_SYSTEM:
                empire = fo.getEmpire()
                if not empire.hasExploredSystem(aiTarget.target_id):
                    return True
        elif aiFleetMissionType  in [EnumsAI.AIFleetMissionType.FLEET_MISSION_OUTPOST,  EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST]:
            universe = fo.getUniverse()
            fleet = universe.getFleet(self.target_id)
            if not fleet.hasOutpostShips:
                return False
            if aiTarget.target_type == EnumsAI.AITargetType.TARGET_PLANET:
                planet = universe.getPlanet(aiTarget.target_id)
                if planet.unowned:
                    return True
        elif aiFleetMissionType in [ EnumsAI.AIFleetMissionType.FLEET_MISSION_COLONISATION,   EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_COLONISATION]:
            universe = fo.getUniverse()
            fleet = universe.getFleet(self.target_id)
            if not fleet.hasColonyShips:
                return False
            if aiTarget.target_type == EnumsAI.AITargetType.TARGET_PLANET:
                planet = universe.getPlanet(aiTarget.target_id)
                planetPopulation = planet.currentMeterValue(fo.meterType.population)
                if planet.unowned or (planet.owner==fleet.owner and planetPopulation == 0):
                    return True
        elif aiFleetMissionType in [ EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION,   EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION]:
            universe = fo.getUniverse()
            fleet = universe.getFleet(self.target_id)
            if not fleet.hasTroopShips:
                return False
            if aiTarget.target_type == EnumsAI.AITargetType.TARGET_PLANET:
                planet = universe.getPlanet(aiTarget.target_id)
                if not planet.unowned or planet.owner!=fleet.owner:
                    return True
        elif aiFleetMissionType in [ EnumsAI.AIFleetMissionType.FLEET_MISSION_MILITARY,  EnumsAI.AIFleetMissionType.FLEET_MISSION_SECURE,  EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE]:
            universe = fo.getUniverse()
            fleet = universe.getFleet(self.target_id)
            #if not fleet.hasArmedShips:
            #    return False
            if aiTarget.target_type == EnumsAI.AITargetType.TARGET_SYSTEM:
                return True
        # TODO: implement other mission types

        return False

    def cleanInvalidAITargets(self):
        "clean invalid AITargets"

        allAIFleetMissionTypes = self.getAIMissionTypes()
        for aiFleetMissionType in allAIFleetMissionTypes:
            allAITargets = self.getAITargets(aiFleetMissionType)
            for aiTarget in allAITargets:
                if not self.isValidFleetMissionAITarget(aiFleetMissionType, aiTarget):
                    self.removeAITarget(aiFleetMissionType, aiTarget)

    def issueAIFleetOrders(self):
        "issues AIFleetOrders which can be issued in system and moves to next one if is possible"

        # TODO: priority
        ordersCompleted = True
        print "Checking orders for fleet %d   (on turn %d)"%(self.target_id,  fo.currentTurn())
        #print "\t\t\t Full Orders are:"
        #for aiFleetOrder2 in self.getAIFleetOrders():
        #    print "\t\t\t\t %s"%aiFleetOrder2
        for aiFleetOrder in self.getAIFleetOrders():
            #print "   %s"%(aiFleetOrder)
            clearAll=False
            if aiFleetOrder.getAIFleetOrderType() in [EnumsAI.AIFleetOrderType.ORDER_COLONISE,  EnumsAI.AIFleetOrderType.ORDER_OUTPOST]:#TODO: invasion?
                universe=fo.getUniverse()
                planet = universe.getPlanet(aiFleetOrder.getTargetAITarget().target_id)
                if  not planet:
                    clearAll =True
                elif aiFleetOrder.getAIFleetOrderType() == EnumsAI.AIFleetOrderType.ORDER_COLONISE :
                    if  ( (planet.currentMeterValue(fo.meterType.population) >0) or  not ( planet.unowned  or planet.ownedBy(fo.empireID()) ) ) :
                        clearAll =True
                elif not planet.unowned:
                    clearAll =True
                if clearAll:
                    print "   %s"%(aiFleetOrder)
                    print "Fleet %d had a target planet that is no longer valid for this mission; aborting."%(self.target_id)
                    self.clearAIFleetOrders()
                    self.clearAITargets(([-1]+ self.getAIMissionTypes()[:1])[-1])
                    FleetUtilsAI.splitFleet(self.target_id)
                    return
            self.checkMergers(context=str(aiFleetOrder))
            if aiFleetOrder.canIssueOrder(verbose=True):
                #print "    " + str(aiFleetOrder) currently already printed in canIssueOrder()
                if aiFleetOrder.getAIFleetOrderType() == EnumsAI.AIFleetOrderType.ORDER_MOVE and ordersCompleted: #only move if all other orders completed
                    aiFleetOrder.issueOrder()
                elif aiFleetOrder.getAIFleetOrderType() not in [ EnumsAI.AIFleetOrderType.ORDER_MOVE,  EnumsAI.AIFleetOrderType.ORDER_DEFEND]:
                    aiFleetOrder.issueOrder()
                if not aiFleetOrder.isExecutionCompleted():
                    ordersCompleted = False
            else: #check that we're not held up by a Big Monster
                if aiFleetOrder.getAIFleetOrderType() == EnumsAI.AIFleetOrderType.ORDER_MOVE:
                    thisSysID = aiFleetOrder.getTargetAITarget().target_id
                    thisStatus = foAI.foAIstate.systemStatus.setdefault(thisSysID, {})
                    if ( thisStatus.get('monsterThreat', 0) >  fo.currentTurn() * ProductionAI.curBestMilShipRating()/4.0 )   :
                        if ( ( (self.getAIMissionTypes() + [-1] )[0] not in [  EnumsAI.AIFleetMissionType.FLEET_MISSION_ATTACK,
                                                                                        EnumsAI.AIFleetMissionType.FLEET_MISSION_MILITARY,
                                                                                        EnumsAI.AIFleetMissionType.FLEET_MISSION_HIT_AND_RUN,
                                                                                        EnumsAI.AIFleetMissionType.FLEET_MISSION_SECURE,
                                                                                    ])  or
                             (  aiFleetOrder !=   self.getAIFleetOrders()[-1]  )   # if this move order is not this mil fleet's final destination, and blocked by Big Monster, release and hope for more effective reassignment
                             ):
                            print "Aborting mission due to being blocked by Big Monster at system %d ,  threat %d"%(thisSysID,  foAI.foAIstate.systemStatus[thisSysID]['monsterThreat'])
                            print "Full set of orders were:"
                            for aiFleetOrder2 in self.getAIFleetOrders():
                                print "\t\t %s"%aiFleetOrder2
                            self.clearAIFleetOrders()
                            self.clearAITargets(([-1]+ self.getAIMissionTypes()[:1])[-1])
                            return
            # moving to another system stops issuing all orders in system where fleet is
            # move order is also the last order in system
            if aiFleetOrder.getAIFleetOrderType() == EnumsAI.AIFleetOrderType.ORDER_MOVE:
                fleet = fo.getUniverse().getFleet( self.target_id)
                if fleet.systemID != aiFleetOrder.getTargetAITarget().target_id:
                    break
        else: #went through entire order list
            if ordersCompleted:
                orders=self.getAIFleetOrders()
                lastOrder= orders and orders[-1]
                universe=fo.getUniverse()
                if orders and lastOrder.getAIFleetOrderType() == EnumsAI.AIFleetOrderType.ORDER_COLONISE:
                    planet = universe.getPlanet(lastOrder.getTargetAITarget().target_id)
                    system = universe.getSystem(planet.systemID)
                    sysPartialVisTurn = dictFromMap(universe.getVisibilityTurnsMap(planet.systemID,  fo.empireID())).get(fo.visibility.partial, -9999)
                    planetPartialVisTurn = dictFromMap(universe.getVisibilityTurnsMap(planet.id,  fo.empireID())).get(fo.visibility.partial, -9999)
                    pop=planet.currentMeterValue(fo.meterType.population)
                    if (planetPartialVisTurn == sysPartialVisTurn) and pop==0:
                        print "Potential Error: Fleet %d has tentatively completed its colonize mission but will wait to confirm population."%(self.target_id)
                        print "    Order details are %s"%lastOrder
                        print "    Order is valid: %s ; is Executed : %s  ; is execution completed: %s "%(lastOrder.isValid(),  lastOrder.isExecuted(),  lastOrder.isExecutionCompleted())
                        if not lastOrder.isValid():
                            sourceT = lastOrder.getSourceAITarget()
                            targT = lastOrder.getTargetAITarget()
                            print "        source target validity: %s   ; target target validity: %s "%(sourceT.valid, targT.valid)
                            if EnumsAI.AITargetType.TARGET_SHIP == sourceT.target_type:
                                shipID = sourceT.target_id
                                ship = universe.getShip(shipID)
                                if not ship:
                                    print "Ship id %d not a valid ship id"%(shipID)
                                print "        source target Ship (%d), species %s,   can%s colonize"%(   shipID,  ship.speciesName,    ["not", ""][ship.canColonize])
                        return  # colonize order must not have completed yet
                clearAll=True
                last_sys_target = -1
                if orders and lastOrder.getAIFleetOrderType() == EnumsAI.AIFleetOrderType.ORDER_MILITARY:
                    last_sys_target = lastOrder.getTargetAITarget().target_id
                    # if (AIFleetMissionType.FLEET_MISSION_SECURE in self.getAIMissionTypes())  or   # not doing this until decide a way to release from a SECURE mission
                    secure_targets = set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs + AIstate.invasionTargetedSystemIDs + AIstate.blockadeTargetedSystemIDs)
                    if   (last_sys_target in secure_targets) : #consider a secure mission
                        secureType="Unidentified"
                        if   (last_sys_target in AIstate.colonyTargetedSystemIDs):
                            secureType  = "Colony"
                        elif last_sys_target in AIstate.outpostTargetedSystemIDs :
                            secureType  = "Outpost"
                        elif last_sys_target in AIstate.invasionTargetedSystemIDs :
                            secureType  = "Invasion"
                        elif last_sys_target in AIstate.blockadeTargetedSystemIDs :
                            secureType  = "Blockade"
                        print "Fleet %d has completed initial stage of its mission to secure system %d (targeted for %s), may release a portion of ships"%(self.target_id, last_sys_target,  secureType)
                        clearAll=False
                fleetID=self.target_id
                fleet=universe.getFleet(fleetID)
                if fleet.systemID != -1:
                    loc = fleet.systemID
                else:
                    loc=fleet.nextSystemID
                if clearAll:
                    print "Fleet %d has completed its mission; clearing all orders and targets." % self.target_id
                    print "Full set of orders were:"
                    for aiFleetOrder2 in self.getAIFleetOrders():
                        print "\t\t %s"%aiFleetOrder2
                    self.clearAIFleetOrders()
                    self.clearAITargets(([-1]+ self.getAIMissionTypes()[:1])[-1])
                    if foAI.foAIstate.getFleetRole(fleetID) in [    EnumsAI.AIFleetMissionType.FLEET_MISSION_MILITARY,
                                                                                                                        EnumsAI.AIFleetMissionType.FLEET_MISSION_ATTACK,
                                                                                                                        EnumsAI.AIFleetMissionType.FLEET_MISSION_DEFEND,
                                                                                                                        EnumsAI.AIFleetMissionType.FLEET_MISSION_HIT_AND_RUN,
                                                                                                                        EnumsAI.AIFleetMissionType.FLEET_MISSION_SECURE       ]:
                        allocations = MilitaryAI.getMilitaryFleets(milFleetIDs=[fleetID],  tryReset=False,  thisround="Fleet %d Reassignment"%fleetID)
                        if allocations:
                            MilitaryAI.assignMilitaryFleetsToSystems(useFleetIDList=[fleetID],  allocations=allocations)
                else:
                    #TODO: evaluate releasing a smaller portion or none of the ships
                    sysStatus = foAI.foAIstate.systemStatus.setdefault(last_sys_target, {})
                    newFleets=[]
                    threat_present = (sysStatus.get('totalThreat',  0) != 0) or (sysStatus.get('neighborThreat',  0) != 0)
                    target_system = universe.getSystem(last_sys_target)
                    if (not threat_present) and target_system:
                        for pid in target_system.planetIDs:
                            planet = universe.getPlanet(pid)
                            if planet  and (planet.owner != fo.empireID()) and (planet.currentMeterValue(fo.meterType.maxDefense) > 0):
                                threat_present = True
                                break
                    if not threat_present:
                        print "No current threat in target system; releasing a portion of ships."
                        newFleets=FleetUtilsAI.splitFleet(self.target_id) #at least first stage of current task is done; release extra ships for potential other deployments
                    else:
                        print "Threat remains in target system; NOT releasing any ships."
                    newMilFleets = []
                    for fleetID in newFleets:
                        if foAI.foAIstate.getFleetRole(fleetID) in [    EnumsAI.AIFleetMissionType.FLEET_MISSION_MILITARY,
                                                                                                                            EnumsAI.AIFleetMissionType.FLEET_MISSION_ATTACK,
                                                                                                                            EnumsAI.AIFleetMissionType.FLEET_MISSION_DEFEND,
                                                                                                                            EnumsAI.AIFleetMissionType.FLEET_MISSION_HIT_AND_RUN,
                                                                                                                            EnumsAI.AIFleetMissionType.FLEET_MISSION_SECURE       ]:
                            newMilFleets.append(fleetID)
                    allocations=[]
                    if newMilFleets:
                        allocations = MilitaryAI.getMilitaryFleets(milFleetIDs=newMilFleets,  tryReset=False,  thisround="Fleet Reassignment %s"%newMilFleets)
                    if allocations:
                        MilitaryAI.assignMilitaryFleetsToSystems(useFleetIDList=newMilFleets,  allocations=allocations)

    def generateAIFleetOrders(self):
        "generates AIFleetOrders from fleets targets to accomplish"

        universe = fo.getUniverse()
        fleetID = self.target_id
        fleet = universe.getFleet(fleetID)
        if (not fleet) or fleet.empty or (fleetID in universe.destroyedObjectIDs(fo.empireID())): #fleet was probably merged into another or was destroyed
            foAI.foAIstate.deleteFleetInfo(fleetID)
            return

        # TODO: priority
        self.clearAIFleetOrders()
        ntargets=0
        for aiFleetMissionType in self.getAIMissionTypes():
            ntargets += len( self.getAITargets(aiFleetMissionType) )
        if ntargets ==0:
            return #no targets

        # for some targets fleet has to visit systems and therefore fleet visit them
        systemAITargets = self.__getRequiredToVisitSystemAITargets()
        aiFleetOrdersToVisitSystems = MoveUtilsAI.getAIFleetOrdersFromSystemAITargets(self.getAITarget(), systemAITargets)
        #TODO: if fleet doesn't have enough fuel to get to final target, consider resetting Mission
        #print "----------------------------------------"
        #print "*+*+ fleet %d :  has fleet action system targets:  %s"%(fleetID,  [str(obj) for obj in systemAITargets])
        #print "----------"
        #print "*+*+ fleet %d:  has movement  orders:  %s"%(fleetID,  [str(obj) for obj in aiFleetOrdersToVisitSystems])

        for aiFleetOrder in aiFleetOrdersToVisitSystems:
            self.appendAIFleetOrder(aiFleetOrder)

        # if fleet is in some system = fleet.systemID >=0, then also generate system AIFleetOrders
        systemID = fleet.systemID
        if systemID >= 0:
            # system in where fleet is
            systemAITarget = AITarget.AITarget(EnumsAI.AITargetType.TARGET_SYSTEM, systemID)
            # if mission aiTarget has required system where fleet is, then generate aiFleetOrder from this aiTarget
            aiMissionTypes = self.getAIMissionTypes()
            # for all targets in all mission types get required systems to visit
            for aiFleetMissionType in aiMissionTypes:
                aiTargets = self.getAITargets(aiFleetMissionType)
                for aiTarget in aiTargets:
                    if systemAITarget in aiTarget.get_required_system_ai_targets():
                        # from target required to visit get fleet orders to accomplish target
                        aiFleetOrder = self.__getAIFleetOrderFromAITarget(aiFleetMissionType, aiTarget)
                        self.appendAIFleetOrder(aiFleetOrder)


        # if fleet doesn't have any mission, then resupply if is current location not in supplyable system
        empire = fo.getEmpire()
        fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
        if (not self.hasAnyAIMissionTypes()) and not(self.getLocationAITarget().target_id in fleetSupplyableSystemIDs):
            resupplyAIFleetOrder = MoveUtilsAI.getResupplyAIFleetOrder(self.getAITarget(), self.getLocationAITarget())
            if resupplyAIFleetOrder.isValid():
                self.appendAIFleetOrder(resupplyAIFleetOrder)

    def getLocationAITarget(self):
        "system AITarget where fleet is or will be"
        # TODO add parameter turn
        universe = fo.getUniverse()
        fleet = universe.getFleet(self.target_id)
        systemID = fleet.systemID
        if systemID >= 0:
            return AITarget.AITarget(EnumsAI.AITargetType.TARGET_SYSTEM, systemID)
        else:
            return AITarget.AITarget(EnumsAI.AITargetType.TARGET_SYSTEM, fleet.nextSystemID)#TODO: huh?

def getFleetIDsFromAIFleetMissions(aiFleetMissions):
    result = []
    for aiFleetMission in aiFleetMissions:
        result.append(aiFleetMission.getMissionAITargetID())

    return result
