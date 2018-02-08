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

We use [flake8-putty](https://pypi.python.org/pypi/flake8-putty)
which include [flake8](https://pypi.python.org/pypi/flake8)
which include [pycodestyle](https://pypi.python.org/pypi/pycodestyle) and other tools.

`flake8-putty` is `flake8` plugin that allows one to disable rule for certain file, see `putty-ignore` section in `tox.ini`

Plan is to fix all easy things (code formatting) and ignore hard things (`E722 do not use bare except`).
Final stage is to setup automatic check to avoid adding new cases.

## TODO section

- remove all folders from excludes
- don't ignore line length check (E501)

## Install dependencies

```sh
pip install flake8-putty
```

## Run checks
This script should be run from directory where `tox.ini` located

```sh
cd default/python
flake8
```
