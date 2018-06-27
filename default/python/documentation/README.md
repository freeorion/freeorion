This page will be devoted to matters relating to
python development in general for the FreeOrion project.
The next topics will be covered in separate documents:
- [boost::python interface](boost.md)
- [AI Python API](http://www.freeorion.org/index.php/AI_Python_API)
- [AI_Development](http://www.freeorion.org/index.php/Python_Development)
- [universe_generation](http://www.freeorion.org/index.php/Universe_Creation)
- [turn_events](turn_events.md)

A good understanding of the [Free_Orion_Content_Script_(FOCS)](http://www.freeorion.org/index.php/Free_Orion_Content_Script_(FOCS))
will also likely be very helpful since the use of python in FreeOrion
closely relates to scripted content.


## Python version
Supported Python version is 2.7

## Code style
C++ API use camelCase style
Python code should be written using general Python style
according to [PEP8](https://www.python.org/dev/peps/pep-0008/)
also useful to read [google recommendations](https://google-styleguide.googlecode.com/svn/trunk/pyguide.html)
We perform automatic Python style checks with flake8
`TODO find how to add the link to the description, it is one level up from this point`


## Extend C+ API via Python
In some cases, it is more easy and effective to extend
the interface of c++ objects from Python.
For example, add string representation of the object.

This code located in file `default\AI\freeorion_debug\extend_free_orion_AI_interface.py`

Add `__repr__` and `__str__` methods to objects as soon as you need it.

## AI state in save file
The AIState is stored as JSON string in save game files.
You can check information about this format in `AI/savegame_codec/__init__.py`

## Preserving backwards compatibility for save games
AI state have version attribute, which describes current version.

Adding or removing AIState attributes can
break save compatibility. If you're not entirely sure
how to handle it one option is to not fully remove them,
leave them with comment to remove, to make breaks less frequent.

When adding attributes or changing their names,
compatibility can be broken because some of the new code will try
to use attributes that the saved object won't have.

You should increment version and update `AI.AIstate.convert_to_version`
method to convert from older versions.

Keep in mind, this method is just used for converting version;
default values still need to be provided in `__init__` to
handle new games.


## Debugging
### Internal Debug Mode
To be accessible this must be enabled by an AI config option:

```#!ini
allow_debug_chat=1
```

For convenience, there is a premade config file that can be used
invoking freeorion from the command line with the following option:

```bash
 --ai-config .\default\python\AI\ai_debug_config.ini
```

Then, to access chat commands you need to choose send all
(default behavior) or select first AI
- Send `help` to chat window.

You will got instruction how to work with it.
`start <id>` will start debug mode with selected id `stop` will finish it.

To run start you need that this AI was selected or don't select any AI.

Most used variables already imported to scope with short name

- `e`: `empire`
- `u`: `universe`
- `ai`: `aiState`

Chat window does not support multiline input. Use semicolon as line separator.

In examples below use `$` is user name.
This chat works like python shell. You can assign variables,
print result and do almost all you want.

```
$ x = 1
$ x
1
$ print x + 3
4
$ e.playerName
'AI_3'
```

See more possibilities: [Tips and tricks](#tips-and-tricks)

### External Debug Mode
For Windows remove Python27.* from installed game folder to use system python(don't forget to install it)

#### PyDev
[Pydev manual](http://pydev.org/manual_adv_remote_debugger.html)

[Open console](http://stackoverflow.com/questions/25018869/pydev-how-to-invoke-debugging-specific-command-from-console-with-breakpoints/25065948#25065948)
If you want to use the interactive console
in the context of a breakpoint, a different approach would be
selecting a stack frame (in the debug view) > right-clicking it >
pydev > Debug Console
(or you can also in the debug view create a new console view
connected to the debug session for the same effect).

#### winpdb
[winpdb](http://winpdb.org/)

## Tips and tricks
### Reload module
- enter debug mode
    ```
    $ start 2
    ```

- import module to current scope
    ```
    $ import ProductionAI
    ```

- change module - reload module
   ```
   $ reload(ProductionAI)
   ```

- end turn and enjoy result

Note: Reload module that store some state will ruin the game (FreeOrionAI)

### Execute file
```
$ execfile('file_path')
```

path can be absolute or relative to current working dir

```
$ import os
$ print os.getcwd()
```

Python editor

It is your choice how to edit python code. Here are some suggestions:

- [PyCharm community edition](https://www.jetbrains.com/pycharm/download/)
- Eclipse based.

A good IDE will help you to make fewer mistakes,
and keyboard shortcuts and other IDE features can greatly speedup
your development. If you are unfamiliar with using an IDE then
two key features to be sure to learn are how to
quickly navigate to an item's declaration
(preferably with a shortcut key),
and how to use its code completion feature.

## Deploying code
Best way to deploy your code to game is to specify
`resource-dir` and `stringtable-filename` in persistent_config file.

- navigate to folder with [Config.xml](http://www.freeorion.org/index.php/Config.xml)
- create `persistent_config.xml`
- add next text and replace `repository_path` to your path

```#!xml
<?xml version="1.0"?>
<XMLDoc>
<resource-dir>repository_path/freeorion/default</resource-dir>
<stringtable-filename>repository_pathfreeorion/default/stringtables/en.txt</stringtable-filename>
</XMLDoc>
```

## FAQ
### This page can be better / has typo / ...
Welcome to forum, lets do it better.

### Which python version used?
Windows: python shipped with game(2.7.3) Other platforms use system python.

### Where does print output go?
stdout and stderr redirected to logs in the game settings folder.

### How do I test new code?
start/load a game (changing code during gameplay will not
affect a current game; the code is already in memory).
