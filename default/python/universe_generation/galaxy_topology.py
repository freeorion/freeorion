import freeorion as fo


def get_systems_within_jumps(origin_system, jumps):
    """
    Returns all systems within jumps jumps of system origin_system (including origin_system).
    If jumps is 0, return list that only contains system origin_system.
    If jumps is negative, return empty list.
    """
    # TODO use a priority queue or at least sets
    # TODO move to C++ and use boost::graph
    if jumps < 0:
        return []
    matching_systems = [origin_system]
    next_origin_systems = [origin_system]
    while jumps > 0:
        origin_systems = list(next_origin_systems)
        next_origin_systems = []
        for system in origin_systems:
            neighbor_systems = [s for s in fo.sys_get_starlanes(system) if s not in matching_systems]
            next_origin_systems.extend(neighbor_systems)
            matching_systems.extend(neighbor_systems)
        jumps -= 1
    return matching_systems
