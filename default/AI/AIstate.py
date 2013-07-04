import copy

import freeOrionAIInterface as fo

import AIFleetMission
import EnumsAI
import ExplorationAI
import FleetUtilsAI
import ProductionAI
import ResourcesAI
from EnumsAI import AIFleetMissionType, AIExplorableSystemType, AITargetType
from MilitaryAI import MinThreat


##moving ALL or NEARLY ALL  'global' variables into AIState object rather than module
##in general, leaving items as a module attribute if they are recalculated each turn without reference to prior values
# global variables
#foodStockpileSize = 1     # food stored per population
minimalColoniseValue = 3  # minimal value for a planet to be colonised
#colonisablePlanetIDs = []  # moved into AIstate
colonyTargetedSystemIDs = []
#colonisableOutpostIDs = []  # moved into AIstate
outpostTargetedSystemIDs = []
opponentPlanetIDs = []
opponentSystemIDs=[]
invasionTargets=[]
invasionTargetedSystemIDs = []
blockadeTargetedSystemIDs=[]
militarySystemIDs = []
militaryTargetedSystemIDs = []
colonyFleetIDs = []
outpostFleetIDs = []
invasionFleetIDs = []
militaryFleetIDs = []
fleetsLostBySystem={}
fleetsLostByID={}
popCtrSystemIDs=[]
colonizedSystems={}
empireStars={}
popCtrIDs=[]
outpostIDs=[]
outpostSystemIDs=[]

def dictFromMap(map):
    return dict(  [  (el.key(),  el.data() ) for el in map ] )

# AIstate class
class AIstate(object):
    "stores AI game state"

    # def colonisablePlanets (should be set at start of turn)
    # getColonisablePlanets (deepcopy!)

    def __init__(self,  aggression=fo.aggression.typical):
        "constructor"
        # 'global' (?) variables
        #self.foodStockpileSize =  1    # food stored per population
        self.minimalColoniseValue = 3  # minimal value for a planet to be colonised
        self.colonisablePlanetIDs = []  
        self.colonisableOutpostIDs = []  # 
        self.__aiMissionsByFleetID = {}
        self.__shipRoleByDesignID = {}
        self.__fleetRoleByID = {}
        self.__designStats={}
        self.__priorityByType = {}

        #self.__explorableSystemByType = {}
        #for explorableSystemsType in EnumsAI.getAIExplorableSystemTypes():
        #    self.__explorableSystemByType[explorableSystemsType] = {}
            
        #initialize home system knowledge
        universe = fo.getUniverse()
        empire = fo.getEmpire()
        self.empireID = empire.empireID
        self.origHomeworldID = empire.capitalID
        homeworld = universe.getPlanet(self.origHomeworldID)
        self.origSpeciesName = homeworld.speciesName
        self.origHomeSystemID = homeworld.systemID
        self.visBorderSystemIDs = {self.origHomeSystemID:1}
        self.visInteriorSystemIDs= {}
        self.expBorderSystemIDs = {self.origHomeSystemID:1}
        self.expInteriorSystemIDs= {}
        self.exploredSystemIDs = {}
        self.unexploredSystemIDs = {self.origHomeSystemID:1}
        self.fleetStatus={} #keys: 'sysID', 'nships', 'rating'
        self.systemStatus={} #keys: 'fleetThreat'. 'planetThreat', 'monsterThreat' (specifically, immobile nonplanet threat), 'myfleets', 'neighbors', 'name', 'myDefenses', 'myFleetsAccessible'(not just next desitination)
        self.needsEmergencyExploration=[]
        self.newlySplitFleets={}
        self.aggression=aggression
        self.militaryRating=0
        self.shipCount = 4
        self.misc={}
        self.qualifyingColonyBaseTargets={}

    def __del__(self):
        "destructor"
        del self.__shipRoleByDesignID
        del self.__fleetRoleByID
        del self.__priorityByType
        del self.__aiMissionsByFleetID
        del self.colonisablePlanetIDs
        del self.colonisableOutpostIDs
        del self.needsEmergencyExploration
        del self.newlySplitFleets
        del self.misc
        del self.qualifyingColonyBaseTargets
        
    def clean(self):
        global invasionTargets
        "turn start AIstate cleanup"
        
        fleetsLostBySystem.clear()
        fleetsLostByID.clear()
        invasionTargets[:]=[]
        
        ExplorationAI.graphFlags.clear()
        if fo.currentTurn() < 50:
            print "-------------------------------------------------"
            print "Border Exploration Update"
            print "-------------------------------------------------"
        for sysID in list(self.visBorderSystemIDs):
            ExplorationAI.followVisSystemConnections(sysID,  self.origHomeSystemID)
        newlyExplored = ExplorationAI.updateExploredSystems()
        nametags=[]
        universe = fo.getUniverse()
        for sysID in newlyExplored:
            newsys = universe.getSystem(sysID)
            nametags.append(  "ID:%4d -- %20s"%(sysID, (newsys and newsys.name) or"name unknown"  )    )# an explored system *should* always be able to be gotten 
        if newlyExplored:
            print "-------------------------------------------------"
            print "newly explored systems: \n"+"\n".join(nametags)
            print "-------------------------------------------------"
        # cleanup fleet roles
        #self.updateFleetLocs()
        self.__cleanFleetRoles()
        self.__cleanAIFleetMissions(FleetUtilsAI.getEmpireFleetIDs())
        print "Fleets lost by system: %s"%fleetsLostBySystem
        self.updateSystemStatus()
        ExplorationAI.updateScoutFleets() #should do this after clearing dead  fleets, currently should be already done here
        
    def assessSelfRating(self):
        return 1
        
    def assessRating(self,  empireID):
        """Returns assessed Rating of specified empire"""
        return 1

    def updateFleetLocs(self):
        universe=fo.getUniverse()
        movedFleets=[]
        for fleetID in self.fleetStatus:
            oldLoc=self.fleetStatus[fleetID]['sysID']
            fleet = universe.getFleet(fleetID)
            if not fleet:
                print "can't retrieve fleet %4d to update loc"%fleetID
                continue #TODO: update elsewhere?
            newLoc = fleet.systemID
            if newLoc != oldLoc:
                movedFleets.append( (fleetID,  oldLoc,  newLoc) )
                self.fleetStatus[fleetID]['sysID']= newLoc
                self.fleetStatus[fleetID]['nships']= len(fleet.shipIDs)
        if movedFleets:
            #print "(moved_fleet,  oldSys,  newSys): %s"%movedFleets
            pass
        else:
            #print "no moved_fleets this turn"
            pass

    def deleteFleetInfo(self,  fleetID, sysID=-1):
        for systemID in [sid for sid in   [sysID,   self.fleetStatus.get(fleetID, {}).get('sysID', -1)] if sid != -1]:
            if fleetID in self.systemStatus.get(sysID,  {}).get('myfleets', []):
                del self.systemStatus[ sysID ]['myfleets'][ self.systemStatus[sysID]['myfleets'].index( fleetID) ]
        if fleetID in self.__aiMissionsByFleetID:
            del self.__aiMissionsByFleetID[fleetID]
        if fleetID in self.fleetStatus:
            del self.fleetStatus[fleetID]
        if fleetID in self.__fleetRoleByID:
            del self.__fleetRoleByID[fleetID]

    def reportSystemThreats(self, sysIDList=None):
        if fo.currentTurn() >= 50:
            return
        universe = fo.getUniverse()
        if sysIDList is None:
            sysIDList = sorted( universe.systemIDs )# will normally look at this, the list of all known systems
        #assess fleet and planet threats
        #print "----------------------------------------------"
        #print "System Threat Assessments"
        #for sysID in sysIDList:
            #sysStatus = self.systemStatus.get(sysID,  {})
            #system = universe.getSystem(sysID)
            #print "System %4d  ( %12s ) : %s"%(sysID,  (system and system.name) or "unknown",  sysStatus)

    def assessPlanetThreat(self,  pid,  sightingAge=0):
        sightingAge+=1#play it safe
        universe = fo.getUniverse()
        planet = universe.getPlanet(pid)
        if not planet:
            return {'overall': 0,  'attack':0 ,  'health':0 }
        cShields = planet.currentMeterValue(fo.meterType.shield)
        tShields = planet.currentMeterValue(fo.meterType.maxShield)
        cDefense = planet.currentMeterValue(fo.meterType.defense)
        tDefense = planet.currentMeterValue(fo.meterType.maxDefense)
        shields = min(tShields,  cShields+2*sightingAge)#TODO: base off regen tech
        defense = min(tDefense,  cDefense+2*sightingAge)#TODO: base off regen tech
        #cInfra = planet.currentMeterValue(fo.meterType.construction)
        #tInfra = planet.currentMeterValue(fo.meterType.targetConstruction)
        #infra = min(tInfra,  cInfra+sightingAge)
        infra=0 #doesn't really contribute to combat since damage to shields & 'defense' done first
        return  {'overall': defense*(defense + shields + infra ),  'attack':defense ,  'health':(defense + shields + infra ) }

    def updateSystemStatus(self,  sysIDList=None):
        print"-------\nUpdating System Threats\n---------"
        universe = fo.getUniverse()
        empire=fo.getEmpire()
        empireID = fo.empireID()
        destroyedObjIDs = universe.destroyedObjectIDs(empireID)
        if sysIDList is None:
            sysIDList = universe.systemIDs # will normally look at this, the list of all known systems
        
        #assess enemy fleets that may have been momentarily visible
        enemyFleetIDs = []
        enemiesBySystem = {}
        myFleetsBySystem={}
        fleetSpotPosition={}
        sawEnemiesAtSystem={}
        currentTurn = fo.currentTurn()
        for fleetID in universe.fleetIDs:
            #if ( fleetID in self.fleetStatus ): # only looking for enemies here
            #    continue
            fleet = universe.getFleet(fleetID)
            if (fleet == None): continue
            if   not(fleet.empty) : 
                thisSysID = (fleet.nextSystemID!= -1 and fleet.nextSystemID) or fleet.systemID
                if  fleet.ownedBy(empireID):
                    if   (fleetID not in destroyedObjIDs):
                        myFleetsBySystem.setdefault( thisSysID,  [] ).append( fleetID )
                        fleetSpotPosition.setdefault( fleet.systemID,  [] ).append( fleetID )
                else:
                    partialVisTurn = dictFromMap(universe.getVisibilityTurnsMap(fleetID,  empireID)).get(fo.visibility.partial, -9999)
                    if partialVisTurn >= currentTurn -1 : #only interested in immediately recent data
                        sawEnemiesAtSystem[fleet.systemID] = True
                        if   (fleetID not in destroyedObjIDs):
                            enemyFleetIDs.append( fleetID )
                            enemiesBySystem.setdefault( thisSysID,  [] ).append( fleetID )
            
        #assess fleet and planet threats & my local fleets
        for sysID in sysIDList:
            sysStatus = self.systemStatus.setdefault(sysID,  {})
            system = universe.getSystem(sysID)
            #update fleets
            sysStatus['myfleets']=myFleetsBySystem.get(sysID,  [])
            sysStatus['myFleetsAccessible']=fleetSpotPosition.get(sysID,  [])
            localEnemyFleetIDs=enemiesBySystem.get(sysID,  [])
            if system:
                sysStatus['name']=system.name
                for fid in system.fleetIDs:
                    if fid in destroyedObjIDs: #TODO: double check are these checks/deletes necessary?
                        self.deleteFleetInfo(fid)#this is safe even if fleet wasn't mine
                        continue
                    fleet = universe.getFleet(fid)
                    if not fleet or fleet.empty: 
                        self.deleteFleetInfo(fid)#this is safe even if fleet wasn't mine
                        continue
                        
            #update threats
            sysVisDict = dictFromMap(universe.getVisibilityTurnsMap(sysID,  fo.empireID()))
            partialVisTurn = sysVisDict.get(fo.visibility.partial, -9999)
            enemyRatings=[]
            enemyRating=0
            monsterRatings=[]
            lostFleetRating=0
            enemyRatings =  [self.rateFleet(fid) for fid in localEnemyFleetIDs  ] 
            enemyAttack = sum( [rating.get('attack', 0) for rating in enemyRatings])
            enemyHealth = sum( [rating.get('health', 0) for rating in enemyRatings])
            enemyRating =  enemyAttack * enemyHealth
            if fleetsLostBySystem.get(sysID,  []) != []:
                #print  "     Assessing threats on turn %d ; noting that fleets were just lost in system %d ,  enemy fleets were %s seen as of turn %d, of which %s survived"%(
                #                currentTurn,  sysID, ["not", ""][sawEnemiesAtSystem.get(sysID, False)],  partialVisTurn,   localEnemyFleetIDs)
                lostFleetAttack = sum( [rating.get('attack', 0) for rating in fleetsLostBySystem.get(sysID,  {}) ] ) 
                lostFleetHealth = sum( [rating.get('health', 0) for rating in fleetsLostBySystem.get(sysID,  {} ) ]  )
                lostFleetRating= lostFleetAttack * lostFleetHealth
            if (not system) or partialVisTurn==-9999:
                #print "Have never had partial vis for system %d ( %s ) -- basing threat assessment on old info and lost ships"%(sysID,  sysStatus.get('name',  "name unknown"))
                sysStatus['planetThreat'] = 0
                sysStatus['fleetThreat'] = int( max(enemyRating,  0.98*sysStatus.get('fleetThreat',  0) ,  1.1*lostFleetRating) )
                sysStatus['monsterThreat']=0
                sysStatus['mydefenses'] = {'overall':0,  'attack':0,  'health':0 }
                sysStatus['totalThreat'] = sysStatus['fleetThreat']
                continue
                
            #have either stale or current info
            pthreat = 0
            pattack = 0
            phealth = 0
            mypattack,  myphealth = 0,  0
            for pid in system.planetIDs:
                prating = self.assessPlanetThreat(pid,  sightingAge=currentTurn-partialVisTurn)
                planet = universe.getPlanet(pid)
                if not planet: continue
                if planet.owner == self.empireID : #TODO: check for diplomatic status
                    mypattack += prating['attack']
                    myphealth += prating['health']
                else:
                    pattack += prating['attack']
                    phealth += prating['health']
            sysStatus['planetThreat'] = pattack*phealth
            sysStatus['mydefenses'] = {'overall':mypattack*myphealth,  'attack':mypattack,  'health':myphealth }

            if max( sysStatus.get('totalThreat', 0), pattack*phealth ) >= 0.6* lostFleetRating: #previous threat assessment could account for losses, ignore the losses now
                lostFleetRating=0

            if  not   partialVisTurn == currentTurn:  #(universe.getVisibility(sysID,  self.empireID) >= fo.visibility.partial):
                #print "Stale visibility for system %d ( %s ) -- last seen %d, current Turn %d -- basing threat assessment on old info and lost ships"%(sysID,  sysStatus.get('name',  "name unknown"),  partialVisTurn,  currentTurn)
                sysStatus['fleetThreat'] = int( max(enemyRating,  0.98*sysStatus.get('fleetThreat',  0),  1.1*lostFleetRating) )
                sysStatus['totalThreat'] = (pattack + enemyAttack + sysStatus.get('monsterThreat', 0)**0.5) * (phealth + enemyHealth + sysStatus.get('monsterThreat', 0)**0.5)
            else: #system considered visible #TODO: reevaluate as visibility rules change
                enemyattack,  enemyhealth,  enemythreat=0, 0, 0
                monsterattack,  monsterhealth,  monsterthreat = 0, 0, 0
                for fleetID in localEnemyFleetIDs:
                    fleet = universe.getFleet(fleetID)
                    if ( fleet) and  (not fleet.ownedBy(self.empireID)):
                        rating=self.rateFleet(fleetID)
                        if fleet.speed==0:
                            monsterattack += rating['attack']
                            monsterhealth += rating['health']
                        else:
                            enemyattack += rating['attack']
                            enemyhealth += rating['health']
                sysStatus['fleetThreat'] = int( max( enemyattack*enemyhealth,  lostFleetRating))  #fleetThreat always includes monster threat, and may not have seen stealthed enemies
                sysStatus['monsterThreat']= monsterattack * monsterhealth
                sysStatus['totalThreat'] = int(max( lostFleetRating,   ( enemyattack+monsterattack+pattack ) * (enemyhealth+ monsterhealth+  phealth )   )   )

        #assess secondary threats (threats of surrounding systems)  and update my fleet rating 
        for sysID in sysIDList:
            sysStatus = self.systemStatus[sysID]
            myattack, myhealth=0, 0
            for fid in sysStatus['myfleets']:
                thisRating=self.getRating(fid)
                myattack += thisRating['attack']
                myhealth += thisRating['health']
            if sysID != -1:
                sysStatus['myFleetRating'] = myattack * myhealth
                
            system = universe.getSystem(sysID)
            neighborDict = dictFromMap( universe.getSystemNeighborsMap(sysID,  self.empireID) )
            neighbors = neighborDict.keys()
            sysStatus['neighbors'] = neighborDict
            if system:
                threat=0
                for neighborID in neighbors:
                    neighborStatus= self.systemStatus.get(neighborID,  {})
                    nfthreat = neighborStatus.get('fleetThreat', 0)
                    threat += nfthreat
                sysStatus['neighborThreat'] = int( threat + 0.5 )
            else:
                sysStatus['neighborThreat'] = 0.9*sysStatus.get('neighborThreat',  0) # if no current info, leave as previous with partial reduction, or 0 if no previous rating

    def afterTurnCleanup(self):
        "removes not required information to save from AI state after AI complete its turn"
        # some ships in fleet can be destroyed between turns and then fleet may have have different roles
        #self.__fleetRoleByID = {}
        pass

    def __hasAIFleetMission(self, fleetID):
        "returns True if fleetID has AIFleetMission"
        return self.__aiMissionsByFleetID.__contains__(fleetID)

    def getAIFleetMission(self, fleetID):
        "returns AIFleetMission with fleetID"
        if fleetID in self.__aiMissionsByFleetID:
            return self.__aiMissionsByFleetID[fleetID]
        else:
            return None

    def getAllAIFleetMissions(self):
        "returns all AIFleetMissions"

        return self.__aiMissionsByFleetID.values()

    def getFleetMissionsMap(self):
        return self.__aiMissionsByFleetID

    def getAIFleetMissionsWithAnyMissionTypes(self, fleetMissionTypes):
        "returns all AIFleetMissions which contains any of fleetMissionTypes"

        result = []

        aiFleetMissions = self.getAllAIFleetMissions()
        for aiFleetMission in aiFleetMissions:
            if aiFleetMission.hasAnyOfAIMissionTypes(fleetMissionTypes):
                result.append(aiFleetMission)
        return result

    def __addAIFleetMission(self, fleetID):
        "add new AIFleetMission with fleetID if it already not exists"

        if self.getAIFleetMission(fleetID) == None:
            aiFleetMission = AIFleetMission.AIFleetMission(fleetID)
            self.__aiMissionsByFleetID[fleetID] = aiFleetMission

    def __removeAIFleetMission(self, fleetID):
        "remove invalid AIFleetMission with fleetID if it exists"

        aiFleetMission = self.getAIFleetMission(fleetID)
        if aiFleetMission != None:
            self.__aiMissionsByFleetID[fleetID] = None
            del aiFleetMission
            del self.__aiMissionsByFleetID[fleetID]
            
    def ensureHaveFleetMissions(self, fleetIDs):
        for fleetID in fleetIDs:
            if self.getAIFleetMission(fleetID) == None:
                self.__addAIFleetMission(fleetID)

    def __cleanAIFleetMissions(self, fleetIDs):
        "cleanup of AIFleetMissions"

        for fleetID in fleetIDs:
            if self.getAIFleetMission(fleetID) == None:
                self.__addAIFleetMission(fleetID)

        aiFleetMissions = self.getAllAIFleetMissions()
        deletedFleetIDs = []
        for aiFleetMission in aiFleetMissions:
            if not(aiFleetMission.getAITargetID() in fleetIDs):
                deletedFleetIDs.append(aiFleetMission.getAITargetID())
        for deletedFleetID in deletedFleetIDs:
            self.__removeAIFleetMission(deletedFleetID)

        aiFleetMissions = self.getAllAIFleetMissions()
        for aiFleetMission in aiFleetMissions:
            aiFleetMission.cleanInvalidAITargets()

    def hasAITarget(self, aiFleetMissionType, aiTarget):
        aiFleetMissions = self.getAIFleetMissionsWithAnyMissionTypes([aiFleetMissionType])
        for mission in aiFleetMissions:
            if mission.hasTarget(aiFleetMissionType, aiTarget):
                return True
        return False

        aiFleetMissions = self.getAIFleetMissionsWithAnyMissionTypes([aiFleetMissionType])
        for mission in aiFleetMissions:
            if mission.hasTarget(aiFleetMissionType, aiTarget):
                return True
        return False

    def rateFleet(self, fleetID):
        universe=fo.getUniverse()
        fleet=universe.getFleet(fleetID)
        if fleet and (not fleet.aggressive) and fleet.ownedBy(self.empireID):
            pass
            #fleet.setAggressive(True)
        if not fleet:
            return {}
        rating=0
        attack=0
        health=0
        nships=0
        for shipID in fleet.shipIDs:
            #could approximate by design, but checking  meters has better current accuracy
            ship = universe.getShip(shipID)
            if not ship: 
                continue
            stats = self.getDesignIDStats(ship.designID)
            rating += stats['attack'] * ( stats['structure'] + stats['shields'] )
            attack +=  stats['attack']
            health += ( stats['structure'] + stats['shields'] )
            nships+=1
        return {'overall':attack*health,  'tally':rating, 'attack':attack, 'health':health, 'nships':nships}

    def getRating(self,  fleetID,  forceNew=False):
        if (fleetID in self.fleetStatus) and not forceNew:
            return self.fleetStatus[fleetID].get('rating', {} )
        else:
            fleet = fo.getUniverse().getFleet(fleetID)
            if not fleet: 
                return 0 #TODO: also ensure any info for that fleet is deleted
            status = {'rating':self.rateFleet(fleetID),  'sysID':fleet.systemID,  'nships':len(fleet.shipIDs)}
            self.fleetStatus[fleetID] = status
            return status['rating']

    def updateFleetRating(self, fleetID):
        return self.getRating(fleetID,  forceNew=True)

    def getDesignStats(self,  design):
        if design:
            return self.getDesignIDStats(design.id)
        else:
            return  {'attack':0, 'structure':0, 'shields':0}
    def getDesignIDStats(self,  designID):
        if designID is None:
            return  {'attack':0, 'structure':0, 'shields':0}
        elif designID in self.__designStats:
            return self.__designStats[designID]
        else:
            stats=FleetUtilsAI.assessDesignIDStats(designID)
            self.__designStats[designID] = stats
            return stats

    def getShipRole(self, shipDesignID):
        "returns ship role for given designID, assesses and adds as needed"

        if shipDesignID in self.__shipRoleByDesignID and self.__shipRoleByDesignID[shipDesignID] != EnumsAI.AIShipRoleType.SHIP_ROLE_INVALID: #if thought was invalid, recheck to be sure
            return self.__shipRoleByDesignID[shipDesignID]
        else:
            self.getDesignIDStats(shipDesignID) # just to update with infor for this new design
            role = FleetUtilsAI.assessShipDesignRole(fo.getShipDesign(shipDesignID))
            self.__shipRoleByDesignID[shipDesignID] = role
            return role

    def addShipRole(self, shipDesignID, shipRole):
        "adds a ship designID/role pair"

        if not (shipRole in EnumsAI.getAIShipRolesTypes()):
            print "Invalid shipRole: " + str(shipRole)
            return
        elif shipDesignID in self.__shipRoleByDesignID:
            return
        else:
            self.__shipRoleByDesignID[shipDesignID] = shipRole

    def removeShipRole(self, shipDesignID):
        "removes a ship designID/role pair"

        if shipDesignID in self.__shipRoleByDesignID:
            print "Removed role for ship design %d named: %s "%(shipDesignID,  fo.getShipDesign(shipDesignID).name)
            del self.__shipRoleByDesignID[shipDesignID]
            return
        else:
            print "No existing role found for ship design %d named: %s "%(shipDesignID,  fo.getShipDesign(shipDesignID).name)

    def getFleetRolesMap(self):
        return self.__fleetRoleByID

    def getFleetRole(self, fleetID,  forceNew=False):
        "returns fleet role by ID"

        if (not forceNew) and fleetID in self.__fleetRoleByID and self.__fleetRoleByID[fleetID]!=AIFleetMissionType.FLEET_MISSION_INVALID :
            return self.__fleetRoleByID[fleetID]
        else:
            role=FleetUtilsAI.assessFleetRole(fleetID)
            self.__fleetRoleByID[fleetID] = role
            makeAggressive=False
            if role in [AIFleetMissionType.FLEET_MISSION_COLONISATION,  AIFleetMissionType.FLEET_MISSION_OUTPOST,  
                                AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST]:
                pass
            if role in [AIFleetMissionType.FLEET_MISSION_EXPLORATION]:
                thisRating=self.getRating(fleetID)
                if float(thisRating.get('overall', 0))/thisRating.get('nships', 1) >= 0.5 * ProductionAI.curBestMilShipRating():
                    makeAggressive=True
            else:
                    makeAggressive=True
            fo.issueAggressionOrder(fleetID,  makeAggressive)
            return role

    def addFleetRole(self, fleetID, missionType):
        "adds a fleet ID/role pair"

        if not EnumsAI.checkValidity(missionType): 
            return
        if fleetID in self.__fleetRoleByID:
            #print "Fleet ID " + str(fleetID) + " already exists."
            return
        else:
            self.__fleetRoleByID[fleetID] = missionType

    def removeFleetRole(self, fleetID):
        "removes a fleet ID/role pair"

        if fleetID in self.__fleetRoleByID:
            #print "Removed role for fleet ID: " + str(fleetID)
            del self.__fleetRoleByID[fleetID]
            return

    def sessionStartCleanup(self):
        ResourcesAI.newTargets.clear()
        self.newlySplitFleets={}
        for fleetID in FleetUtilsAI.getEmpireFleetIDs():
            self.getFleetRole(fleetID) #
            self.getRating(fleetID) #
            self.ensureHaveFleetMissions([fleetID])
        self.__cleanFleetRoles(justResumed=True)
        fleetsLostBySystem.clear()
        fleetsLostByID.clear()
        popCtrSystemIDs[:] = []#resets without detroying existing references
        colonizedSystems.clear()
        empireStars.clear()
        popCtrIDs[:] = []
        outpostIDs[:] = []
        outpostSystemIDs[:] = []
        ResourcesAI.lastFociCheck[0]=0
        self.qualifyingColonyBaseTargets.clear()

    def __cleanFleetRoles(self,  justResumed=False):
        "removes fleetRoles if a fleet has been lost, and update fleet Ratings"
        for sysID in self.systemStatus:
            self.systemStatus[sysID]['myFleetRating']=0

        #deleteRoles = []
        universe = fo.getUniverse()
        okFleets=FleetUtilsAI.getEmpireFleetIDs()
        fleetList = sorted( list( self.__fleetRoleByID ))
        unaccountedFleets = set( okFleets ) - set( fleetList )
        shipCount=0
        print "----------------------------------------------------------------------------------"
        print "in CleanFleetRoles"
        print "fleetList : %s"%fleetList
        print "-----------"
        print "FleetUtils empire-owned fleetList : %s"%okFleets
        print "-----------"
        if unaccountedFleets:
            print "Fleets unaccounted for in Empire Records: ",  unaccountedFleets
            print "-----------"
        #print "statusList %s"%[self.fleetStatus[fid] for fid in sorted( self.fleetStatus.keys() ) ]
        print "-----------"
        minThreatRating = {'overall':MinThreat,  'attack':MinThreat**0.5,  'health':MinThreat**0.5}
        for fleetID in fleetList:
            status=self.fleetStatus.setdefault(fleetID,  {} )
            rating = status.get('rating', {'overall':0,  'attack':0,  'health':0} )
            newRating = self.rateFleet(fleetID)
            oldSysID = status.get('sysID',  -2)
            fleet = universe.getFleet(fleetID)
            #if fleet and not fleet.aggressive:
            #    fleet.setAggressive(True)
            if fleet:
                sysID = fleet.systemID
                if oldSysID in [-2,  -1]:
                    oldSysID = sysID
                status['nships']=len(fleet.shipIDs)
                shipCount += status['nships']
            else:
                sysID = oldSysID #can still retrieve a fleet object even if fleet was just destroyed, so shouldn't get here
            if (fleetID not in okFleets):# or fleet.empty:
                if not ( (self.__fleetRoleByID.get(fleetID,  -1) ==-1)  ):
                    if not justResumed:
                        if rating.get('overall', 0) > MinThreat:
                            fleetsLostBySystem.setdefault(oldSysID,  []).append( rating )
                        else:
                            fleetsLostBySystem.setdefault(oldSysID,  []).append( minThreatRating )
                        sys=universe.getSystem(sysID)
                        sysName=(sys and sys.name) or "unknown"
                        #print "Fleet %d  with role %s was used up, lost or destroyed at sys (%d) %s"%(fleetID,  self.__fleetRoleByID[fleetID],  sysID, sysName)  #perhaps diff message for successful colony fleets
                        
                if fleetID in self.__fleetRoleByID:
                    del self.__fleetRoleByID[fleetID]
                if fleetID in self.__aiMissionsByFleetID:
                    del self.__aiMissionsByFleetID[fleetID]
                if fleetID in self.fleetStatus:
                    del self.fleetStatus[fleetID]
                continue
            else:#fleet  in ok fleets
                sys1 =  universe.getSystem(sysID)
                if sysID==-1:
                    sys1Name = 'starlane'
                else:
                    sys1Name = (sys1 and sys1.name ) or "unknown"
                nextSysID = fleet.nextSystemID
                sys2 =  universe.getSystem(nextSysID)
                if nextSysID==-1:
                    sys2Name= 'starlane'
                else:
                    sys2Name = (sys2 and sys2.name ) or "unknown"
                #print "Fleet %d (%s)  oldRating: %6d   | newRating %6d  | at system %d (%s)  | next system %d (%s)"%(fleetID, fleet.name,  rating.get('overall', 0),  newRating.get('overall', 0), 
                #                                                                                                     fleet.systemID,  sys1Name,  fleet.nextSystemID,  sys2Name)
                status['rating'] = newRating
                if nextSysID !=-1:
                    status['sysID'] = nextSysID
                elif sysID !=-1:
                    status['sysID'] = sysID
                else:
                    mainFleetMission=self.getAIFleetMission(fleetID)
                    mainMissionType = (mainFleetMission.getAIMissionTypes() + [-1])[0]
                    if mainMissionType != -1:
                        mainMissionTargets = mainFleetMission.getAITargets(mainMissionType)
                        if mainMissionTargets:
                            mMT0=mainMissionTargets[0]
                            if mMT0.getAITargetType()==AITargetType.TARGET_SYSTEM:
                                status['sysID'] = mMT0.getTargetID() #hmm, but might still be a fair ways from here
        self.shipCount = shipCount
        print "------------------------"
        print "Empire Ship Count: ",  shipCount
        print "------------------------"
                    
    def getExplorableSystems(self, explorableSystemsType):
        "get all explorable systems determined by type "

        #return copy.deepcopy(self.__explorableSystemByType[explorableSystemsType])
        if explorableSystemsType == AIExplorableSystemType.EXPLORABLE_SYSTEM_EXPLORED:
            return list(self.exploredSystemIDs)
        elif explorableSystemsType == AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED:
            return list(self.unexploredSystemIDs)
        else:
            print "Error -- unexpected  explorableSystemsType (value %s ) submited to AIState.getExplorableSystems "
            return {}

    def setPriority(self, priorityType, value):
        "sets a priority of the specified type"

        self.__priorityByType[priorityType] = value

    def getPriority(self, priorityType):
        "returns the priority value of the specified type"

        if priorityType in self.__priorityByType:
            return copy.deepcopy(self.__priorityByType[priorityType])

        return 0

    def getAllPriorities(self):
        "returns a dictionary with all priority values"

        return copy.deepcopy(self.__priorityByType)

    def printPriorities(self):
        "prints all priorities"

        print "all priorities:"
        for priority in self.__priorityByType:
            print "    " + str(priority) + ": " + str(self.__priorityByType[priority])
        print ""
