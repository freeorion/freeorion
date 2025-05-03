from logging import error

from AIDependencies import CombatTarget

_issued_errors = set()


def get_allowed_targets(partname: str) -> int:
    """Return the allowed targets for a given hangar or short range weapon."""
    try:
        return CombatTarget.PART_ALLOWED_TARGETS[partname]
    except KeyError:
        if partname not in _issued_errors:
            error(
                "AI has no targeting information for weapon part %s. Will assume any target allowed."
                "Please update CombatTarget.PART_ALLOWED_TARGETS in AIDependencies.py "
            )
            _issued_errors.add(partname)
        return CombatTarget.ANY


def get_distractability_factor(allowed_targets: int, target_class: int) -> float:
    """
    Return a factor for the likeliness to be distracted by other targets.
    The expected number of targets is usually fighters > ships > planets, so
     e.g. planets are expected to not distract much from other targets
    """
    if target_class == CombatTarget.FIGHTER:
        if allowed_targets & CombatTarget.SHIP:
            return 0.95
    elif target_class == CombatTarget.PLANET:
        if allowed_targets & CombatTarget.SHIP:
            return 0.9
        if allowed_targets & CombatTarget.FIGHTER:
            return 0.7
    elif target_class == CombatTarget.SHIP:
        if allowed_targets & CombatTarget.FIGHTER:
            return 0.8
    return 1.0


def get_multi_target_split_damage_factor(allowed_targets: int, target_class: int) -> float:
    """
    Return a heuristic factor how much expected damage needs to be scaled down
    for a certain each class of targets in case there multiple valid target classes.
    If the military AI puts the ship to the correct use, the expected damage will be higher
    than a simple division by the number of classes.
    """
    if not (allowed_targets & target_class):
        error(f"bad call, not possible to target intended target ({(allowed_targets, target_class)}")
        return 0.0
    target_classes_cnt = 0
    target_classes_cnt += int(allowed_targets & CombatTarget.FIGHTER != 0)
    # target_classes_cnt += int (allowed_targets & CombatTarget.PLANET != 0) # damage is not considered in design value
    target_classes_cnt += int(allowed_targets & CombatTarget.SHIP != 0)
    if target_classes_cnt in [0, 1]:
        # no relevant distractions, single resulting damage type
        return 1.0
    elif target_classes_cnt == 2:
        # one type of distraction, two types of resulting damage
        factor = 0.7
    elif target_classes_cnt == 3:
        # two types of distraction, three types of resulting damage
        factor = 0.5
    else:
        error("bad target class count %i" % target_classes_cnt)
        return 0.0

    return factor * get_distractability_factor(allowed_targets, target_class)
