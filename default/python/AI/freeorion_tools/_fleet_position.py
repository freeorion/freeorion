import freeOrionAIInterface as fo


def get_fleet_position(fleet_id: int) -> int:
    """Return system where fleet is located or moving to."""
    fleet = fo.getUniverse().getFleet(fleet_id)
    return fleet.systemID if fleet.systemID > 0 else fleet.nextSystemID
