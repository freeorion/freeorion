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
* chat/  -  Python code which manages chat history stored either in a file or in
a database.
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

Each PR will be checked automatically, but you still can run checks manually.
Fixing all issues are mandatory before merging PR.

We use [flake8-putty](https://pypi.python.org/pypi/flake8-putty)
which include [flake8](https://pypi.python.org/pypi/flake8)
which include [pycodestyle](https://pypi.python.org/pypi/pycodestyle) and other tools.

`flake8-putty` is `flake8` plugin that allows one to disable one or more rules for a certain file,
see `putty-ignore` section in `tox.ini`. This allows ignoring certain
warnings only for specified files, like bare excepts, that are hard to fix,
or special files with tables.


## TODO section

See TODO section in tox.ini

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
