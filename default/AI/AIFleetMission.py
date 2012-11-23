from EnumsAI import AIFleetOrderType, AIFleetMissionType, AITargetType, AIMissionType,  getFleetOrderTypeForMission,  AIShipRoleType
import MoveUtilsAI
import AITarget
import AIFleetOrder
import FleetUtilsAI
import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import AIstate

from AIAbstractMission import AIAbstractMission

AIFleetMissionTypeNames = AIFleetMissionType()
AIShipRoleTypeNames = AIShipRoleType()

class AIFleetMission(AIAbstractMission):
    '''
    Stores information about AI mission. Every mission has fleetID and AI targets depending upon AI fleet mission type.
    '''

    def __init__(self, fleetID):
        "constructor"

        AIAbstractMission.__init__(self, AIMissionType.FLEET_MISSION, AITargetType.TARGET_FLEET, fleetID)
        self.__aiFleetOrders = []

    def __str__(self):
        "returns describing string"

        missionStrings=[]
        for aiFleetMissionType in self.getAIMissionTypes():
            universe = fo.getUniverse()
            fleetID = self.getAITargetID()
            fleet = universe.getFleet(fleetID)
            targetsString = "fleet %4d (%14s) [ %10s mission ] : %3d ships , total Rating:%7d "%(fleetID,  (fleet and fleet.name) or "Fleet Invalid",   AIFleetMissionTypeNames.name(aiFleetMissionType) ,  
                                                                                                 (fleet and len(fleet.shipIDs)) or 0,  foAI.foAIstate.getRating(fleetID))
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
                result.extend(aiTarget.getRequiredSystemAITargets())
        return result

    def getVisitingSystemAITargets(self):
        "returns all system AITargets which will be visited"

        result = []
        for aiFleetOrder in self.getAIFleetOrders():
            if aiFleetOrder.getAIFleetOrderType() == AIFleetOrderType.ORDER_MOVE:
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
        fleetAITarget = AITarget.AITarget(AITargetType.TARGET_FLEET, self.getAITargetID())
        orderType = getFleetOrderTypeForMission(aiFleetMissionType,  option=None)
        result = AIFleetOrder.AIFleetOrder(orderType, fleetAITarget, aiTarget)
        return result

    def checkMergers(self,  fleetID=None,  context=""):
        if fleetID==None:
                        fleetID = self.getAITargetID()
        #mainFleetMission=foAI.foAIstate.getAIFleetMission(fleetID)
        mainMissionTypeList = self.getAIMissionTypes()  #normally, currently, should only be one
        if len( mainMissionTypeList ) != 1:
            return #if this fleet has multiple mission types, will not subsume other fleets
        fleetRoleB =  foAI.foAIstate.getFleetRole(fleetID)
        mainMissionType = mainMissionTypeList[0]
        if mainMissionType not in [  AIFleetMissionType.FLEET_MISSION_ATTACK,
                                                                        AIFleetMissionType.FLEET_MISSION_DEFEND, 
                                                                        AIFleetMissionType.FLEET_MISSION_LAST_STAND ,  
                                                                        AIFleetMissionType.FLEET_MISSION_MILITARY, 
                                                                        AIFleetMissionType.FLEET_MISSION_SECURE, 
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
        otherFleetsHere= [fid for fid in sysStatus.get('myfleets', []) if ( (fid != fleetID) and (fid not in destroyedList) ) ]
        if otherFleetsHere==[]:
            return #nothing of record to merge with
        mainMissionTargets = self.getAITargets(mainMissionType)
        if mainMissionTargets == []:
            return
        if len(mainMissionTargets)>1: 
            pass
            print "\tConsidering merging fleets into  fleet  %d, but it has multiple targets: %s"%(fleetID,  str(mainMissionTargets))
        sys1=universe.getSystem(systemID)
        sysName = (sys1 and sys1.name) or "unknown"
        mMT0=mainMissionTargets[0]
        mMT0ID = mMT0.getTargetID()
        #targetID = self.getTargetAITarget().getTargetID()
        #print "\tMain-Mission type %s  -- primary target :   %s"%( AIFleetMissionTypeNames.name(mainMissionType),  mMT0)
        for fid in otherFleetsHere:
            fleetRoleA = foAI.foAIstate.getFleetRole(fid)
            if fleetRoleA != AIFleetMissionType.FLEET_MISSION_MILITARY: #TODO: if fleetRoles such as LongRange start being used, adjust this
                continue # will only considering subsuming fleets that have a military type role
            fleet2 = universe.getFleet(fid)
            if not (fleet2 and (fleet2.systemID == systemID)):
                #print "\t\t considering merging fleet (id %d )  into fleet (id %4d)  but the former no longer exists or not actually in system %4d %s although AI records thought it should be"%(fid, fleetID,   systemID,  sysName)
                continue
            if not fleet2.ownedBy(foAI.foAIstate.empireID):
                #print "\t\t considered merging fleet (id %d ) into fleet (id %d ),  but former not owned by this empire"%(fid,  fleetID)
                continue
            f2Mission=foAI.foAIstate.getAIFleetMission(fid)
            doMerge=False
            needLeft=0
            if not f2Mission:
                #continue #no, let's take this fleet over
                doMerge=True
            else:
                f2MType = (f2Mission.getAIMissionTypes()+[-1])[0]
                #print "\t\t fleet2 (id %d)  has  mission type %s "%(fid,   AIFleetMissionTypeNames.name(f2MType))
                f2Targets = f2Mission.getAITargets(f2MType)
                if len(f2Targets)>1: 
                    #print "\t\t\t considered merging fleet (id %d ) into  fleet (id %d  ) but former has multiple targets: %s"%(fid, fleetID,   str(f2Targets))
                    pass
                elif len(f2Targets)==0: 
                    #print "\t\t\t *****"
                    print "\t\t\t ** Considering merging  fleetA (id: %4d)  into fleetB (id %d  ) and former has no targets, will take it.  FleetA mission was %s   "%(fid, fleetID,   f2Mission)
                    #print "\t\t\t ** FleetA has ship ids %s"%list(fleet2.shipIDs)
                    #print "\t\t\t *****"
                    doMerge=True
                else:
                    targetB = f2Targets[0].getTargetID()
                    if targetB == mMT0ID:
                        print "Military fleet %d has same target as %s fleet %d and will (at least temporarily) be merged into the latter"%(fid, AIFleetMissionTypeNames.name( fleetRoleB)  ,  fleetID)
                        doMerge=True #TODO: should probably ensure that fleetA  has aggression on now
                    else:
                        neighbors = foAI.foAIstate.systemStatus.get(systemID,  {}).get('neighbors', [])
                        if (targetB==systemID) and mMT0ID in neighbors: #consider 'borrowing' for work in neighbor system
                            if f2MType  in [  AIFleetMissionType.FLEET_MISSION_ATTACK,
                                                                AIFleetMissionType.FLEET_MISSION_DEFEND, 
                                                                AIFleetMissionType.FLEET_MISSION_MILITARY, 
                                                                AIFleetMissionType.FLEET_MISSION_SECURE, 
                                                                ]:
                                #continue
                                if f2MType  in [  AIFleetMissionType.FLEET_MISSION_DEFEND,  
                                                                    AIFleetMissionType.FLEET_MISSION_SECURE, #actually, currently this is probably the onle one of all four that should really be possibile in this situation
                                                                    ]:
                                    needLeft = 1.5*sum( [  (sysStat.get('fleetThreat', 0)-max(0,  sysStat.get('monsterThreat', 0)-200) ) for sysStat in 
                                                                                                          [foAI.foAIstate.systemStatus.get(neighbor, {}) for neighbor in  
                                                                                                          [ nid for nid in foAI.foAIstate.systemStatus.get(systemID, {}).get('neighbors', []) if nid != mMT0ID    ]   ]  ] )
                                    fBRating = foAI.foAIstate.getRating(fid)
                                    if needLeft < fBRating:
                                        doMerge=True
            if doMerge:
                #print "preparing to merge fleet %d into fleet %d,  leaving at least %d rating behind"%(fid,  fleetID,  needLeft)
                FleetUtilsAI.mergeFleetAintoB(fid,  fleetID,  needLeft,  context="Order %s  of mission %s"%(context,  str(self)))
        return

    def isValidFleetMissionAITarget(self, aiFleetMissionType, aiTarget):
        if aiTarget.isValid() == False:
            return False
        if aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_EXPLORATION:
            if aiTarget.getAITargetType() == AITargetType.TARGET_SYSTEM:
                empire = fo.getEmpire()
                if not empire.hasExploredSystem(aiTarget.getTargetID()):
                    return True
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_OUTPOST:
            universe = fo.getUniverse()
            fleet = universe.getFleet(self.getAITargetID())
            if not fleet.hasColonyShips:
                return False
            if aiTarget.getAITargetType() == AITargetType.TARGET_PLANET:
                planet = universe.getPlanet(aiTarget.getTargetID())
                if planet.unowned:
                    return True
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_COLONISATION:
            universe = fo.getUniverse()
            fleet = universe.getFleet(self.getAITargetID())
            if not fleet.hasColonyShips:
                return False
            if aiTarget.getAITargetType() == AITargetType.TARGET_PLANET:
                planet = universe.getPlanet(aiTarget.getTargetID())
                if planet.unowned:
                    return True
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_INVASION:
            universe = fo.getUniverse()
            fleet = universe.getFleet(self.getAITargetID())
            if not fleet.hasTroopShips:
                return False
            if aiTarget.getAITargetType() == AITargetType.TARGET_PLANET:
                planet = universe.getPlanet(aiTarget.getTargetID())
                planetPopulation = planet.currentMeterValue(fo.meterType.population)
                if not planet.unowned or planetPopulation > 0:
                    return True
        elif aiFleetMissionType in [ AIFleetMissionType.FLEET_MISSION_MILITARY,  AIFleetMissionType.FLEET_MISSION_SECURE]:
            universe = fo.getUniverse()
            fleet = universe.getFleet(self.getAITargetID())
            if not fleet.hasArmedShips:
                return False
            if aiTarget.getAITargetType() == AITargetType.TARGET_SYSTEM:
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
        print "Checking orders for fleet %d"%(self.getAITargetID())
        for aiFleetOrder in self.getAIFleetOrders():
            print "   %s"%(aiFleetOrder)
            clearAll=False
            if aiFleetOrder.getAIFleetOrderType() in [AIFleetOrderType.ORDER_COLONISE,  AIFleetOrderType.ORDER_OUTPOST]:
                universe=fo.getUniverse()
                planet = universe.getPlanet(aiFleetOrder.getTargetAITarget().getTargetID())
                if  not planet:
                    clearAll =True
                elif aiFleetOrder.getAIFleetOrderType() == AIFleetOrderType.ORDER_COLONISE :
                    if  ( (planet.currentMeterValue(fo.meterType.population) >0) or  not ( planet.unowned  or planet.ownedBy(fo.empireID()) ) ) :
                        clearAll =True
                elif not planet.unowned:
                        clearAll =True
                if clearAll:
                    print "Fleet %d had a target planet that is no longer valid for this mission; aborting."%(self.getAITargetID() )
                    self.clearAIFleetOrders()
                    self.clearAITargets(([-1]+ self.getAIMissionTypes()[:1])[-1])
                    FleetUtilsAI.splitFleet(self.getAITargetID() )
                    return
            self.checkMergers(context=str(aiFleetOrder))
            if aiFleetOrder.canIssueOrder(verbose=True):
                #print "    " + str(aiFleetOrder) currently already printed in canIssueOrder()
                if aiFleetOrder.getAIFleetOrderType() == AIFleetOrderType.ORDER_MOVE and ordersCompleted: #only move if all other orders completed
                    aiFleetOrder.issueOrder()
                elif aiFleetOrder.getAIFleetOrderType() != AIFleetOrderType.ORDER_MOVE:
                    aiFleetOrder.issueOrder()
                if not aiFleetOrder.isExecutionCompleted():
                    ordersCompleted = False
            else: #check that we're not held up by a Big Monster
                if aiFleetOrder.getAIFleetOrderType() == AIFleetOrderType.ORDER_MOVE:
                    thisSysID = aiFleetOrder.getTargetAITarget().getTargetID()
                    if (foAI.foAIstate.systemStatus.setdefault(thisSysID, {}).setdefault('monsterThreat', 0) > 3000) or (fo.currentTurn() <20  and foAI.foAIstate.systemStatus[thisSysID]['monsterThreat'] > 300):  #move blocked by Big Monster
                        if ( ( (self.getAIMissionTypes() + [-1] )[0] not in [  AIFleetMissionType.FLEET_MISSION_ATTACK,   
                                                                                        AIFleetMissionType.FLEET_MISSION_MILITARY, 
                                                                                        AIFleetMissionType.FLEET_MISSION_HIT_AND_RUN, 
                                                                                        AIFleetMissionType.FLEET_MISSION_SECURE, 
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
            if aiFleetOrder.getAIFleetOrderType() == AIFleetOrderType.ORDER_MOVE:
                fleet = fo.getUniverse().getFleet( self.getAITargetID() )
                if fleet.systemID != aiFleetOrder.getTargetAITarget().getTargetID():
                    break
        else: #went through entire order list
            if ordersCompleted:
                orders=self.getAIFleetOrders()
                if orders and orders[-1].getAIFleetOrderType() == AIFleetOrderType.ORDER_COLONISE:
                    lastOrder=orders[-1]
                    universe=fo.getUniverse()
                    planet = universe.getPlanet(lastOrder.getTargetAITarget().getTargetID())
                    pop=planet.currentMeterValue(fo.meterType.population)
                    if pop==0:
                        print "Fleet %d has tentatively completed its colonize mission but will wait to confirm population."%(self.getAITargetID() )
                        print "    Order details are %s"%lastOrder
                        print "    Order is valid: %s ; is Executed : %s  ; is execution completed: %s "%(lastOrder.isValid(),  lastOrder.isExecuted(),  lastOrder.isExecutionCompleted())
                        if not lastOrder.isValid():
                            sourceT = lastOrder.getSourceAITarget()
                            targT = lastOrder.getTargetAITarget()
                            print "        source target validity: %s   ; target target validity: %s "%(sourceT.isValid() ,  targT.isValid())
                            if AITargetType.TARGET_SHIP == sourceT:
                                shipID = sourceT.getTargetID()
                                ship = universe.getShip(shipID)
                                if not ship:
                                    print "Ship id %d not a valid ship id"%(shipID)
                                print "        source target Ship (%d), species %s,   can%s colonize"%(   shipID,  ship.speciesName,    ["not", ""][ship.canColonize])
                        return  # colonize order must not have completed yet
                clearAll=True
                if orders and orders[-1].getAIFleetOrderType() == AIFleetOrderType.ORDER_MILITARY:
                    lastOrder=orders[-1]
                    if lastOrder.getTargetAITarget().getTargetID() in list(set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs + AIstate.invasionTargetedSystemIDs + AIstate.blockadeTargetedSystemIDs)): #consider a secure mission
                        print "Fleet %d has completed initial stage of its mission to secure system %d, releasing a portion of ships"%(self.getAITargetID() ,  lastOrder.getTargetAITarget().getTargetID())
                        clearAll=False
                if clearAll:
                    print "Fleet %d has completed its mission; clearing all orders and targets."%(self.getAITargetID() )
                    print "Full set of orders were:"
                    for aiFleetOrder2 in self.getAIFleetOrders():
                        print "\t\t %s"%aiFleetOrder2
                    self.clearAIFleetOrders()
                    self.clearAITargets(([-1]+ self.getAIMissionTypes()[:1])[-1])
                else:
                    FleetUtilsAI.splitFleet(self.getAITargetID() ) #at least first stage of current task is done; release extra ships for potential other deployments
    def generateAIFleetOrders(self):
        "generates AIFleetOrders from fleets targets to accomplish"

        universe = fo.getUniverse()
        fleetID = self.getAITargetID()
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
            systemAITarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, systemID)
            # if mission aiTarget has required system where fleet is, then generate aiFleetOrder from this aiTarget 
            aiMissionTypes = self.getAIMissionTypes()
            # for all targets in all mission types get required systems to visit 
            for aiFleetMissionType in aiMissionTypes:
                aiTargets = self.getAITargets(aiFleetMissionType)
                for aiTarget in aiTargets:
                    if systemAITarget in aiTarget.getRequiredSystemAITargets():
                        # from target required to visit get fleet orders to accomplish target
                        aiFleetOrder = self.__getAIFleetOrderFromAITarget(aiFleetMissionType, aiTarget)
                        self.appendAIFleetOrder(aiFleetOrder)


        # if fleet don't have any mission, then resupply if is current location not in supplyable system
        empire = fo.getEmpire()
        fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
        if (not self.hasAnyAIMissionTypes()) and not(self.getLocationAITarget().getTargetID() in fleetSupplyableSystemIDs):
            resupplyAIFleetOrder = MoveUtilsAI.getResupplyAIFleetOrder(self.getAITarget(), self.getLocationAITarget())
            if resupplyAIFleetOrder.isValid():
                self.appendAIFleetOrder(resupplyAIFleetOrder)

    def getLocationAITarget(self):
        "system AITarget where fleet is or will be"
        # TODO add parameter turn
        universe = fo.getUniverse()
        fleet = universe.getFleet(self.getAITargetID())
        systemID = fleet.systemID
        if systemID >= 0:
            return AITarget.AITarget(AITargetType.TARGET_SYSTEM, systemID)
        else:
            return AITarget.AITarget(AITargetType.TARGET_SYSTEM, fleet.nextSystemID)#TODO: huh?

def getFleetIDsFromAIFleetMissions(aiFleetMissions):
    result = []
    for aiFleetMission in aiFleetMissions:
        result.append(aiFleetMission.getMissionAITargetID())

    return result
