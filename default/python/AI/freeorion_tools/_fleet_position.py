import freeOrionAIInterface as fo

from AIDependencies import INVALID_ID
from common.fo_typing import FleetId, SystemId


def get_fleet_position(fleet_id: FleetId) -> SystemId:
    """Return system where fleet is located or moving to."""
    fleet = fo.getUniverse().getFleet(fleet_id)
    return fleet.systemID if fleet.systemID != INVALID_ID else fleet.nextSystemID
