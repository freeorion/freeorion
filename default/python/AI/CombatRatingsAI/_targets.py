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
