# python directory

Python scripts, while modifiable, may require specific functions which return
an appropriate response to FreeOrion.
For example, turn_events/turn_events.py is a required file and should define
an `execute_turn_events()` function returning a boolean (successful completion).

## Contents

* `AI/` Python code which controls the computer players.  This is a
sub-module of the resource directory and can be changed with the --ai-path flag.
* `auth/` Python code which manages authentication information stored either in a file
or in a database.
* `chat/` Python code which manages chat history stored either in a file or in
a database.
* `common/` Common files for code utilized by both the AI and the server.
* `handlers/` debug and development features, see [handlers/README.md](handlers/README.md)
* `turn_events/` Python scripts that run at the beginning of every turn, and
can trigger events that would be impossible to do purely in FOCS.
* `universe_generation/` Python scripts that get run at the beginning of the
game and create the galaxy. You can customise the galaxy generation by
editing options.py and universe_tables.py, both of which have more information
in comments over there. The latter, specifically, controls which star types,
planet types, planet sizes, and also other content get placed.

# Generating skeletons
API to fetch data/read content is created in runtime using C++, 
in order to have autocomplete in IDE skeletons (*.pyi) were generated.

Skeletons can be generated in runtime environment only.  

It is highly recomended to configure game to use AI script from the repo
https://www.freeorion.org/index.php/Python_Development#Deploying_code

```sh
freeorion --ai-config freeorion\default\python\handlers\inspect_interface_config.ini 
```
`freeorion` is a path to the game binary (freeorion.exe on Windows).
`freeorion\default\python\handlers\inspect_interface_config.ini` path to file in this repository. Note that relative pathes may not work, better specify an absolute path.


Starting new game and wait until it exits to main menu with an error.

Generated files will be saved to AI folder:
- freeorion\default\python\freeorion.pyi
- freeorion\default\python\AI\freeOrionAIInterface.pyi
You can check full paths in AI_1.log and freeoriond.log after `Skeleton written to`.  

Copy them to repository with replace, commit.

# Code style check
Each PR will be checked automatically, but you still can run checks manually.
Fixing all issues are mandatory before merging PR.

We use [flake8](https://pypi.python.org/pypi/flake8) for code style checks.
Settings for flake8 located in the `tox.ini`.

We use [black](https://pypi.org/project/black/) for code formatting.
Settings for black are located in `pyproject.toml`.

## Install dependencies

```sh
pip install flake8==3.7.9
pip install black
```

## Format python files
Black can be run on the top level directory:

```sh
black .
```

## Run checks
Flake should be run from directory where `tox.ini` located

```sh
cd default/python
flake8
```

# Automate check with pre commit hooks
Git could run a script
([hook](https://git-scm.com/book/en/v2/Customizing-Git-Git-Hooks)) 
when you commit changes.  

This feature could be used as duplication of the CI actions.
The main difference are that hooks are optional 
(a developer made a decision if they should be run) 
and that hooks gives you feedback much faster 
because they are run on developer machine.

Hooks wok on all operating systems.
You need to install them once.
Hook will track config file and update himself in case of changes. 

## First time setup
- install git
- install Python
- [Install pre-commit](https://pre-commit.com/#install) 
    ```sh
    pip install pre-commit
    ```
- install hooks from the project root
    ```sh
    pre-commit install 
    ```

Notes:
- First commit after installation/update of hooks may take some time.
- You can skip checks by adding `-n, --no-verify` to `git commit`.
