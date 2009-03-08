from EnumsAI import AITargetType, AIFleetOrderType
import AITarget
import AIFleetOrder

def getAIFleetOrdersFromSystemAITargets(fleetID, aiTargets):
    result = []
    # TODO: use Graph Theory to construct move orders
    # TODO: take fuel into account
    fleetAITarget = AITarget.AITarget(AITargetType.TARGET_FLEET, fleetID) 
    for aiTarget in aiTargets:
        aiFleetOrder = AIFleetOrder.AIFleetOrder(AIFleetOrderType.ORDER_MOVE, fleetAITarget, aiTarget)
        result.append(aiFleetOrder)
        
    return result
