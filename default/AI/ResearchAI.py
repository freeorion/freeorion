import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import TechsListsAI
from EnumsAI import AIPriorityType, getAIPriorityResearchTypes
import AIstate

def generateResearchOrders():
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
        projectsDict[name] = fo.getTech(name).researchCost

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
