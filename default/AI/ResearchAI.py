import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import TechsListsAI
from EnumsAI import AIPriorityType, getAIPriorityResearchTypes
import AIstate
import traceback
import sys

inProgressTechs={}

def generateResearchOrders():
    global inProgressTechs
    "generate research orders"
    empire = fo.getEmpire()
    empireID = empire.empireID
    print "Research Queue Management:"
    print "\nTotal Current Research Points: %.2f\n"%empire.resourceProduction(fo.resourceType.research)
    print "Techs researched and available for use:"
    completedTechs = sorted(list(getCompletedTechs()))
    tlist = completedTechs+3*[" "]
    tlines = zip( tlist[0::3],  tlist[1::3],  tlist[2::3])
    for tline in tlines:
        print "%25s  %25s  %25s"%tline
    print""
    researchQueue = empire.researchQueue
    researchQueueList = getResearchQueueTechs()
    inProgressTechs.clear()
    if  researchQueueList:
        print "Techs currently at head of Research Queue:"
        for element in list(researchQueue)[:10]:
            if element.allocation > 0.0:
                inProgressTechs[element.tech]=True
            thisTech=fo.getTech(element.tech)
            missingPrereqs = [preReq for preReq in thisTech.recursivePrerequisites(empireID) if preReq not in completedTechs]
            unlockedItems = [(uli.name,  uli.type) for uli in thisTech.unlockedItems]
            if not missingPrereqs:
                print "    %25s  allocated %6.2f RP -- unlockable items: %s "%(element.tech,  element.allocation,  unlockedItems)
            else:
                print "    %25s  allocated %6.2f RP   --  missing preReqs: %s   -- unlockable items: %s "%(element.tech,  element.allocation,  missingPrereqs,  unlockedItems)
        print ""
    if fo.currentTurn()<=1:
        newtech = TechsListsAI.primaryMetaTechsList()
        #pLTsToEnqueue = (set(newtech)-(set(completedTechs)|set(researchQueueList)))
        pLTsToEnqueue = newtech[:]
        techBase = set(completedTechs+researchQueueList)
        techsToAdd=[]
        for tech in pLTsToEnqueue:
            if (tech not in  techBase): 
                thisTech=fo.getTech(tech)
                if thisTech is None:
                    continue
                missingPrereqs = [preReq for preReq in thisTech.recursivePrerequisites(empireID) if preReq not in techBase] 
                techsToAdd.extend( missingPrereqs+[tech] )
                techBase.update(  missingPrereqs+[tech]  )
        for name in techsToAdd:
            try:
                enqueueRes = fo.issueEnqueueTechOrder(name, -1)
                if enqueueRes == 1:
                    print "    Enqueued Tech: " + name
                else:
                    print "    Error: failed attempt to enqueued Tech: " + name
            except:
                print "    Error: failed attempt to enqueued Tech: " + name
                print "    Error: exception triggered:  ",  traceback.format_exc()
        print""
        generateDefaultResearchOrders()
        print "\n\nAll techs:"
        alltechs = fo.techs() # returns names of all techs
        for tname in alltechs:
            print tname
        print "\n-------------------------------\nAll unqueued techs:"
        coveredTechs = newtech+completedTechs
        for tname in [tn for tn in alltechs if tn not in coveredTechs]:
            print tname

    elif fo.currentTurn() >50:
        generateDefaultResearchOrders()

def generateResearchOrders_old():
    "generate research orders"

    empire = fo.getEmpire()
    print "Research Queue Management:"
    print ""
    print "Techs researched and available for use:"
    completedTechs = getCompletedTechs()
    for techname in completedTechs:
        print "    " + techname
    print""

    print "Techs currently in Research Queue:"
    researchQueue = empire.researchQueue
    researchQueueList = getResearchQueueTechs()
    for element in researchQueue:
        print "    " + element.tech
    print ""

    # get the highest research priorities
    print "Research Queue Priorities:"
    researchPriorities = {}
    for priorityType in getAIPriorityResearchTypes():
        researchPriorities[priorityType] = foAI.foAIstate.getPriority(priorityType)

    sortedPriorities = researchPriorities.items()
    sortedPriorities.sort(lambda x,y: cmp(x[1], y[1]), reverse=True)
    topPriority = -1
    for evaluationPair in sortedPriorities:
        if topPriority < 0:
            topPriority = evaluationPair[0]
        print "    ID|Score: " + str(evaluationPair)

    print "  Top Research Queue Priority: " + str(topPriority)
    print ""

    if topPriority == AIPriorityType.PRIORITY_RESEARCH_LEARNING:
        primaryLearningTechs = TechsListsAI.primaryLearningTechsList()
        pLTsToEnqueue = (set(primaryLearningTechs)-(set(completedTechs)|set(researchQueueList)))
        if not pLTsToEnqueue:
            print "All primaryLearningTechs are enqueued or completed."
            print""
            generateDefaultResearchOrders()
        else:
            for name in pLTsToEnqueue:
                fo.issueEnqueueTechOrder(name, -1)
                print "    Enqueued Tech: " + name
            print""
            generateDefaultResearchOrders()
    elif topPriority == AIPriorityType.PRIORITY_RESEARCH_GROWTH:
        primaryGroTechs = TechsListsAI.primaryGroTechsList()
        pGTsToEnqueue = (set(primaryGroTechs)-(set(completedTechs)|set(researchQueueList)))
        if not pGTsToEnqueue:
            print "All primaryGrowthTechs are enqueued or completed."
            print""
            generateDefaultResearchOrders()
        else:
            for name in pGTsToEnqueue:
                fo.issueEnqueueTechOrder(name, -1)
                print "    Enqueued Tech: " + name
            print ""
            generateDefaultResearchOrders()
    elif topPriority == AIPriorityType.PRIORITY_RESEARCH_PRODUCTION:
        generateDefaultResearchOrders()
    elif topPriority == AIPriorityType.PRIORITY_RESEARCH_CONSTRUCTION:
        generateDefaultResearchOrders()
    elif topPriority == AIPriorityType.PRIORITY_RESEARCH_ECONOMICS:
        generateDefaultResearchOrders()
    elif topPriority == AIPriorityType.PRIORITY_RESEARCH_SHIPS:
        primaryShipsTechs = TechsListsAI.primaryShipsTechsList()
        pSTsToEnqueue = (set(primaryShipsTechs)-(set(completedTechs)|set(researchQueueList)))
        if not pSTsToEnqueue:
            print "All primaryShipsTechs are enqueued or completed."
            generateDefaultResearchOrders()
            print ""
        else:
            for name in pSTsToEnqueue:
                fo.issueEnqueueTechOrder(name, -1)
                print "    Enqueued Tech: " + name
            print ""
            generateDefaultResearchOrders()

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

    unusableTechs = TechsListsAI.unusableTechsList()
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
