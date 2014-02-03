import copy
import math

import freeOrionAIInterface as fo # pylint: disable=import-error

import AIFleetMission
import EnumsAI
import ExplorationAI
import FleetUtilsAI
import ProductionAI
import ResourcesAI
from EnumsAI import AIFleetMissionType, AIExplorableSystemType, AITargetType
from MilitaryAI import MinThreat
import PlanetUtilsAI


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
piloting_grades = {}

def dictFromMap(this_map):
    return dict(  [  (el.key(),  el.data() ) for el in this_map ] )

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
        self.designStats={}
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
        self.origSpeciesName = (homeworld and homeworld.speciesName) or ""
        if homeworld:
            self.origHomeSystemID = homeworld.systemID
        else:
            self.origHomeSystemID = -1
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
        self.qualifyingOutpostBaseTargets={}
        self.qualifyingTroopBaseTargets={}
        self.empire_standard_fighter = (4,  ((4, 1),), 0.0, 10.0)
        self.empire_standard_enemy = (4,  ((4, 1),), 0.0, 10.0)
        
    def __setstate__(self, state_dict):
        self.__dict__.update(state_dict)   # update attributes
        for dict_attrib in ['qualifyingColonyBaseTargets',  'qualifyingOutpostBaseTargets',  'qualifyingTroopBaseTargets']:
            if dict_attrib not in state_dict:
                self.__dict__[dict_attrib] = {}
        for std_attrib in ['empire_standard_fighter',  'empire_standard_enemy']:
            if std_attrib not in state_dict:
                self.__dict__[std_attrib] = (4,  ((4, 1),), 0.0, 10.0)


    def __del__(self): #TODO: confirm if anything about bosst interface really requires this
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
        del self.qualifyingOutpostBaseTargets
        del self.qualifyingTroopBaseTargets

    def clean(self):
        "turn start AIstate cleanup"

        fleetsLostBySystem.clear()
        fleetsLostByID.clear()
        invasionTargets[:]=[]
        exploration_center = PlanetUtilsAI.getCapitalSysID()
        if exploration_center == -1: #a bad state probably from an old savegame
            exploration_center = self.origHomeSystemID

        ExplorationAI.graphFlags.clear()
        if fo.currentTurn() < 50:
            print "-------------------------------------------------"
            print "Border Exploration Update (relative to %s"%(PlanetUtilsAI.sysNameIDs([exploration_center,  -1])[0])
            print "-------------------------------------------------"
        if self.visBorderSystemIDs.keys() == [-1]:
            self.visBorderSystemIDs.clear()
            self.visBorderSystemIDs[exploration_center] = 1
        for sysID in list(self.visBorderSystemIDs):
            if fo.currentTurn() < 50:
                print "Considering border system %s"%(PlanetUtilsAI.sysNameIDs([sysID,  -1])[0])
            ExplorationAI.followVisSystemConnections(sysID,  exploration_center)
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
        
    def fleet_summary_to_estats(self, summary):
        if not summary:
            return None
        #print "converting summary: ",  summary
        estats = []
        try:
            for count,  ship_sum in summary:
                estats.append( (count,  {'attacks':dict(ship_sum[1]), 'shields': ship_sum[2],  'structure':ship_sum[3]}  ) )
            return estats
        except:
            print "Error converting fleet summary ",  summary
            return None
            
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
        print "----------------------------------------------"
        print "System Threat Assessments"
        for sysID in sysIDList:
            sysStatus = self.systemStatus.get(sysID,  {})
            system = universe.getSystem(sysID)
            print "System %4d  ( %12s ) : %s"%(sysID,  (system and system.name) or "unknown",  sysStatus)

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
        cur_e_fighters = { (0,  ((0, 0),), 0.0, 5.0):[0] } # start with a dummy entry
        old_e_fighters = { (0,  ((0, 0),), 0.0, 5.0):[0] } # start with a dummy entry
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
                    dead_fleet = fleetID in destroyedObjIDs
                    if not fleet.ownedBy( -1 ):
                        e_rating = self.rateFleet(fleetID)
                        e_f_dict = [  cur_e_fighters,  old_e_fighters ][ dead_fleet ]
                        for count, sum_stats in e_rating['summary']:
                            if sum_stats[0] > 0:
                                e_f_dict.setdefault( sum_stats,  [0])[0] += count
                    partialVisTurn = dictFromMap(universe.getVisibilityTurnsMap(fleetID,  empireID)).get(fo.visibility.partial, -9999)
                    if partialVisTurn >= currentTurn -1 : #only interested in immediately recent data
                        if   (not dead_fleet):
                            sawEnemiesAtSystem[fleet.systemID] = True
                            enemyFleetIDs.append( fleetID )
                            enemiesBySystem.setdefault( thisSysID,  [] ).append( fleetID )
        e_f_dict = [  cur_e_fighters,  old_e_fighters ][ len(cur_e_fighters)==1 ]
        std_fighter = sorted( [ (v, k) for k, v in e_f_dict.items()] )[-1][1]
        self.empire_standard_enemy = std_fighter

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
            #enemyRatings =  [self.rateFleet(fid) for fid in localEnemyFleetIDs  ]
            enemyRatings = []
            for fid in localEnemyFleetIDs:
                oldstyle_rating = self.old_rate_fleet(fid)
                newstyle_rating = self.rateFleet(fid, self.fleet_summary_to_estats([(1,  self.empire_standard_fighter  )]))
                enemyRatings.append(newstyle_rating)
                if oldstyle_rating.get('overall', 0) != newstyle_rating.get('overall', 0):
                    loc = ""
                    fleetname = ""
                    fleet = universe.getFleet(fid)
                    if fleet:
                        fleetname = fleet.name
                        sysID = fleet.systemID
                        system = universe.getSystem(sysID)
                        if system:
                            loc = " at %s" % system.name
                    print "AiState.updateSystemStatus for fleet %s id (%d)%s got different newstyle rating (%s) and oldstyle rating (%s)" % (fleetname, fid, loc, newstyle_rating, oldstyle_rating)
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
                        rating=self.rateFleet(fleetID,  self.fleet_summary_to_estats([(1, self.empire_standard_fighter)]))
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
                thisRating=self.getRating(fid,  True, self.fleet_summary_to_estats([(1,  self.empire_standard_enemy)]))
                myattack += thisRating['attack']
                myhealth += thisRating['health']
            if sysID != -1:
                sysStatus['myFleetRating'] = myattack * myhealth

            system = universe.getSystem(sysID)
            neighborDict = dictFromMap( universe.getSystemNeighborsMap(sysID,  self.empireID) )
            neighbors = neighborDict.keys()
            sysStatus['neighbors'] = neighborDict
            max_n_threat = 0
            if system:
                threat=0
                for neighborID in neighbors:
                    neighborStatus= self.systemStatus.get(neighborID,  {})
                    nfthreat = neighborStatus.get('fleetThreat', 0)
                    if nfthreat > max_n_threat:
                        max_n_threat = nfthreat
                    threat += nfthreat
                sysStatus['neighborThreat'] = int( threat + 0.5 )
                sysStatus['max_neighbor_threat'] = int(max_n_threat)
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
            if not(aiFleetMission.target_id in fleetIDs):
                deletedFleetIDs.append(aiFleetMission.target_id)
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
        
    def rateFleet(self, fleetID,  enemy_stats=None):
        """ for enemy stats format see adjust_stats_vs_enemy() 
            summary is (attacks, attack, shields, max_structure) """
        if enemy_stats is None:
            enemy_stats=[ (1, {}) ]
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
        summary={}
        for shipID in fleet.shipIDs:
            #could approximate by design, but checking  meters has better current accuracy
            ship = universe.getShip(shipID)
            if not ship:
                continue
            stats = dict(self.get_weighted_design_stats(ship.designID,  ship.speciesName))
            max_struct = stats['structure']
            structure = ship.currentMeterValue(fo.meterType.structure)
            shields = ship.currentMeterValue(fo.meterType.shield)
            stats['structure'] = structure
            stats['shields'] = shields
            self.adjust_stats_vs_enemy(stats,  enemy_stats)
            rating += stats['attack'] * stats['structure']
            attack += stats['attack']
            health += stats['structure']
            ship_summary = ( stats['attack'], tuple( [tuple(item) for item in stats['attacks'].items() ] or [(0, 0)] ),  stats['shields'],  max_struct )
            summary.setdefault( ship_summary, [0])[0] += 1
            nships+=1
        fleet_summary = [ (v[0],  k) for k, v in summary.items()]
        return {'overall':attack*health,  'tally':rating, 'attack':attack, 'health':health, 'nships':nships,  'summary':fleet_summary}

    def old_rate_fleet(self, fleetID):
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

    def getRating(self,  fleetID,  forceNew=False,  enemy_stats = None):
        "returns a dict with various rating info"
        if (fleetID in self.fleetStatus) and not forceNew and enemy_stats is None:
            return self.fleetStatus[fleetID].get('rating', {} )
        else:
            fleet = fo.getUniverse().getFleet(fleetID)
            if not fleet:
                return {} #TODO: also ensure any info for that fleet is deleted
            status = {'rating':self.rateFleet(fleetID,  enemy_stats),  'sysID':fleet.systemID,  'nships':len(fleet.shipIDs)}
            self.fleetStatus[fleetID] = status
            #print "fleet ID %d rating: %s"%(fleetID,  status['rating'])
            return status['rating']

    def updateFleetRating(self, fleetID):
        return self.getRating(fleetID,  forceNew=True)

    def get_piloting_grades(self,  species_name):
        if species_name in piloting_grades:
            return piloting_grades[species_name]
        weapons_grade=""
        shields_grade=""
        if species_name == "":
            spec_tags = []
        else:
            species = fo.getSpecies(species_name)
            if species:
                spec_tags = species.tags
            else:
                print "Error: get_piloting_grades couldn't retrieve species '%s'" % species_name
                spec_tags = []
        for tag in spec_tags:
            if "AI_TAG" not in tag:
                continue
            tag_parts = tag.split('_')
            tag_type = tag_parts[3]
            if tag_type == 'WEAPONS':
                weapons_grade = tag_parts[2]
            elif tag_type == 'SHIELDS':
                shields_grade = tag_parts[2]
        piloting_grades[species_name] = (weapons_grade,  shields_grade)
        return (weapons_grade,  shields_grade)

    def weight_attacks(self,  attacks,  grade):
        "re-weights attacks based on species piloting grade"
        #TODO: make more accurate based off weapons lists
        weight = {'NO':-1,  'BAD': -0.25,  '':0.0,  'GOOD':0.25,  'GREAT':0.5,  'ULTIMATE':1.0}.get(grade,  0.0)
        newattacks = {}
        for attack, count in attacks.items():
            newattacks[ attack + round(attack * weight) ] = count
        return newattacks

    def weight_shields(self,  shields,  grade):
        "re-weights shields based on species defense bonus"
        offset = {'NO':0,  'BAD': 0,  '':0,  'GOOD':1.0,  'GREAT':0,  'ULTIMATE':0}.get(grade,  0)
        return shields + offset

    def get_weighted_design_stats(self,  design_id,  species_name=""):
        """rate a  design, including species pilot effects
            returns dict of attacks {dmg1:count1}, attack, shields, structure"""
        weapons_grade,  shields_grade = self.get_piloting_grades(species_name)
        design_stats = dict( self.getDesignIDStats(design_id) ) #new dict so we don't modify our original data
        myattacks = self.weight_attacks(design_stats.get('attacks', {}),  weapons_grade)
        design_stats['attacks'] = myattacks
        #mystructure = design_stats.get('structure', 1)
        myshields = self.weight_shields(design_stats.get('shields', 0),  shields_grade) #designs currently return zero shield value
        design_stats['attack'] = sum([a * b for a, b in myattacks.items()])
        design_stats['shields'] = myshields
        return design_stats

    def adjust_stats_vs_enemy(self,  ship_stats, enemy_stats=None ):
        """rate a ship w/r/t a particular enemy, adjusts ship_stats in place
            ship_stats: {'attacks':attacks, 'structure': str, 'shields': sh } 
            enemy stats: None or [ (num1, estats1), (num2, estats2), ]
            estats: {'attacks':attacks, 'shields': sh , structure:str} 
            attacks: {dmg1:count1, dmg2:count2 }
            """
        if enemy_stats is None:
            enemy_stats = [ (1, {}) ]
        #orig_stats = copy.deepcopy(ship_stats)
        myattacks = ship_stats.get('attacks', {})
        mystructure = ship_stats.get('structure', 1)
        myshields = ship_stats.get('shields', 0)
        total_enemy_weights = 0
        attack_tally=0
        structure_tally = 0
        for enemygroup in enemy_stats:
            count = enemygroup[0]
            estats = enemygroup[1]
            if estats == {}:
                attack_tally += count * sum([ a * b for a, b in myattacks.items()])
                attack_total = sum( [num * max(0,  a_key) for a_key,  num in myattacks.items()] )
                attack_net = max(sum( [num * max(0,  a_key - myshields) for a_key,  num in myattacks.items()] ),  0.1 * attack_total) #TODO: reconsider capping at 10-fold boost
                #attack_diff = attack_total - attack_net
                if attack_net > 0: #will be unless no attacks at all, due to above calc
                    shield_boost = mystructure * ((attack_total / attack_net)-1)
                else:
                    shield_boost = 0
                structure_tally += count * (mystructure + shield_boost) #uses num of my attacks for proxy calc of structure help from shield
                total_enemy_weights += count
                continue
            eshields = estats.get('shields',  0)
            tempattacktally=0
            tempstruc = max(1e-4, estats.get('structure', 1))
            thisweight = count * tempstruc
            total_enemy_weights += thisweight
            e_attacks = estats.get('attacks', {})
            #structure_tally += thisweight * max(mystructure,  min(e_attacks) - myshields ) #TODO: improve shielded attack calc
            attack_total = sum( [num * max(0,  a_key) for a_key,  num in e_attacks.items()] ) #doesnt adjust for shields
            attack_net = max(sum( [num * max(0,  a_key - myshields) for a_key,  num in e_attacks.items()] ),  0.1 * attack_total) #TODO: reconsider approach
            #attack_diff = attack_total - attack_net
            if attack_net > 0: #will be unless no attacks at all, due to above calc
                shield_boost = mystructure * ((attack_total / attack_net)-1)
            else:
                shield_boost = 0
            structure_tally += thisweight * (mystructure + shield_boost) 
            for attack, nattacks in myattacks.items():
                adjustedattack = max(0,  attack-eshields)
                thisattack = min(tempstruc,  nattacks*adjustedattack)
                tempattacktally += thisattack
                tempstruc -= thisattack
                if tempstruc <= 0:
                    break
            attack_tally += thisweight * tempattacktally
        weighted_attack = attack_tally / total_enemy_weights
        weighted_structure = structure_tally / total_enemy_weights
        ship_stats['attack'] = weighted_attack
        ship_stats['structure'] = weighted_structure
        ship_stats['weighted'] = True
        #if enemy_stats != [ (1, {}) ]:
        #    print "adjusting default stats %s subject to enemy stats %s yielding adjusted stats %s"%(
        #                                        self.adjust_stats_vs_enemy(orig_stats), enemy_stats,  ship_stats)
        #    pass
       
        
        return ship_stats

    def getDesignIDStats(self,  designID):
        if designID is None:
            return  {'attack':0, 'structure':0, 'shields':0,  'attacks':{}}
        elif designID in self.designStats:
            return self.designStats[designID]
        design = fo.getShipDesign(designID)
        detect_bonus = 0
        if design:
            attacks = {}
            for attack in list(design.directFireStats):
                attacks[attack] = attacks.get(attack,  0) + 1
            parts = design.parts
            shields = 0
            if "SH_BLACK" in parts:
                shields = 20
            elif "SH_MULTISPEC" in parts:
                shields = 15
            elif "SH_PLASMA" in parts:
                shields = 12
            elif "SH_DEFLECTOR" in parts:
                shields = 7
            elif "SH_DEFENSE_GRID" in parts:
                shields = 4
            if "DT_DETECTOR_4" in parts:
                detect_bonus = 4
            elif "DT_DETECTOR_3" in parts:
                detect_bonus = 3
            elif "DT_DETECTOR_2" in parts:
                detect_bonus = 2
            elif "DT_DETECTOR_1" in parts:
                detect_bonus = 1
            #stats = {'attack':design.attack, 'structure':(design.structure + detect_bonus), 'shields':shields,  'attacks':attacks}
            stats = {'attack':design.attack, 'structure':(design.structure                       ), 'shields':shields,  'attacks':attacks}
        else:
            stats = {'attack':0, 'structure':0, 'shields':0,  'attacks':{}}
        self.designStats[designID] = stats
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
        self.qualifyingOutpostBaseTargets.clear()
        self.qualifyingTroopBaseTargets.clear()
        #self.reset_invasions()
        
    def reset_invasions(self):
        "useful when testing changes to invasion planning"
        invasionAIFleetMissions = self.getAIFleetMissionsWithAnyMissionTypes([EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION])
        for mission in invasionAIFleetMissions:
            mission.clearAIFleetOrders()
            mission.clearAITargets(([-1]+ mission.getAIMissionTypes()[:1])[-1])

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
        fighters = { (0,  ((0, 0),),  0.0, 5.0):[0] } # start with a dummy entry
        for fleetID in fleetList:
            status=self.fleetStatus.setdefault(fleetID,  {} )
            rating = status.get('rating', {'overall':0,  'attack':0,  'health':0} )
            newRating = self.rateFleet(fleetID,  self.fleet_summary_to_estats([(1, self.empire_standard_enemy)]))
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
                print "Fleet %d (%s)  oldRating: %6d   | newRating %6d  | at system %d (%s)  | next system %d (%s)"%(fleetID, fleet.name,  rating.get('overall', 0),  newRating.get('overall', 0),
                                                                                                                     fleet.systemID,  sys1Name,  fleet.nextSystemID,  sys2Name)
                print "Fleet %d (%s)  summary: %s"%(fleetID, fleet.name,  rating.get('summary', None)  )
                status['rating'] = newRating
                for count, sum_stats in newRating['summary']:
                    if sum_stats[0] > 0:
                        fighters.setdefault( sum_stats,  [0])[0] += count
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
                            if mMT0.target_type == AITargetType.TARGET_SYSTEM:
                                status['sysID'] = mMT0.target_id #hmm, but might still be a fair ways from here
        self.shipCount = shipCount
        std_fighter = sorted( [ (v, k) for k, v in fighters.items()] )[-1][1]
        self.empire_standard_fighter = std_fighter
        print "------------------------"
        print "Empire Ship Count: ",  shipCount
        print "Empire standard fighter summary: ",  std_fighter
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
