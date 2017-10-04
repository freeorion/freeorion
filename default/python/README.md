# python directory

Python scripts, while modifiable, may require specific functions which return
an appropriate response to FreeOrion.
For example, turn_events/turn_events.py is a required file and should define
an execute_turn_events() function returning a boolean (successful completion).

## Contents

* AI/  -  Python code which controls the computer players.  This is a
sub-module of the resource directory and can be changed with the --ai-path flag.
* auth/  -  Python code which manages auth information stored either in a file
or in a database.
* common/  -  Common files for code utilized by both the AI and the server.
* handlers/  -  see handlers/README.md
* turn_events/  -  Python scripts that run at the beginning of every turn, and
can trigger events that would be impossible to do purely in FOCS.
* universe_generation/  -  Python scripts that get run at the beginning of the
game and create the galaxy. You can customise the galaxy generation by
editing options.py and universe_tables.py, both of which have more information
in comments over there. The latter, specifically, controls which star types,
planet types, planet sizes, and also other content get placed.


# Code style check

Code style checks are done with [pycodestyle](https://pypi.python.org/pypi/pycodestyle)

This part is in progress.

Plan is to fix all easy things (code formatting) and ignore hard things (`E722 do not use bare except`).
Final stage is to setup automatic check to avoid adding new cases.

## Why we ignore
- `E241 multiple spaces after ':'`  this is ok only for `AIDependencies.py` because it is used as config file
- `E501 line too long` it is ok to ignore it for `AIDependencies.py`

## List of check that should pass:
`pycodestyle python/AI/AIDependencies.py --ignore=E501,E241`
`pycodestyle python/AI/AIFleetMission.py --max-line-length=120`
`pycodestyle python/AI/AIstate.py --max-line-length=120`
`pycodestyle python/AI/ColonisationAI.py --max-line-length=120`
`pycodestyle python/AI/CombatRatingsAI.py --max-line-length=120`
`pycodestyle python/AI/DiplomaticCorp.py --max-line-length=120`
`pycodestyle python/AI/EnumsAI.py --max-line-length=120`
`pycodestyle python/AI/ExplorationAI.py --max-line-length=120`
`pycodestyle python/AI/fleet_orders.py --max-line-length=120`
`pycodestyle python/AI/FleetUtilsAI.py --max-line-length=120`
`pycodestyle python/AI/FreeOrionAI.py --max-line-length=120 --ignore=E402,E722`
`pycodestyle python/AI/InvasionAI.py --max-line-length=120`
