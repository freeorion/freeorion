# String tables

## Table of content
- [Glossary](#glossary)
- [Tool](#tool)

## Glossary

- `string table` a file with translation
- `reference table` en.txt string table.
  English is the development language, so any changes related to text should be in that file.
  Also, this table is used as a fallback for other translations, if a key is missing.
  This file is mostly maintained by people who change game code,
  so the list of entries in this file should be the most up-to-date.
- `entry` one piece of translation, it consists of
  - `key` a sting that represents entry. It could be referenced from code: C++, Python, FOCS.
     Also, new key could be added just for substitution purposes to the string table.
    This string must be unique.
  - `value` actual translation that will be shown to the user.

### Key
Only uppercase Latin alphabet (A-Z) and underscore (_) are allowed.
Renaming a key should be done with caution.
Many of the keys in the stringtable are used in the game source code explicitly,
or in content scripts.
Some keys are also generated programatically, such as by adding a prefix to other text.
C++ uses UserString(...) or UserStringNop(...) to look up strings or to mark stringtable keys that are used in code.
Python uses `fo.userString` and `fo.userStringList` for look up.

### Value syntax
#### Simple (single line)
A single line of text, should not be started and ended by any whitespace character.

```
KEY
value

>>>
value
<<<
```

#### Full (single or multiline)
Start and end with triple single quotes.
You can put anything except triple single quotes inside.

```
KEY_WITH_LEADING_SPACE
''' value'''

>>>
 value
<<<

KEY_WITH_MULTILINE
'''Hello
world'''

>>>
Hello
world
<<<

```

#### Tags
You can customize text with special markup tags. Not a full list of tags:

- `[[KEY]]` will be replaced with the text in the entry with key `KEY`.
  Substitution is recursive, if text has another substitution it will be resolved too.
  If key was not found in this table, it will be looked in the reference table. 

```
KEY_WORLD
world

KEY_HELLO
Hello [[KEY_WORLD]]!

>>>
Hello world
<<<
```

This feature should be used to keep the translation in a consistent state.
You define term once and use it everywhere.
For various kinds of game content, like techs, buildings, or species, etc., there are links

- `link` inserts a link to a wiki page about some scripted content or a ship design in the game universe.
Supported types
  - encyclopedia
  - buildingtype
  - fieldtype
  - metertype
  - predefinedshipdesign
  - shiphull
  - shippart
  - special
  - species
  - tech
  - policy
```
[[buildingtype BLD_SHIPYARD_ORBITAL_DRYDOCK]]
```

- `value` inserts value from content scripts into the text,
  eg. `[[value FLD_NANITE_SWARM_STRUCTURE_FLAT]]` will look up the value from
  the named valueref scripted as
  `NamedReal name = "FLD_NANITE_SWARM_STRUCTURE_FLAT" value = 5 * [[SHIP_STRUCTURE_FACTOR]])`
  and be replaced by whatever `5 * [[SHIP_STRUCTURE_FACTOR]]` equals,
  which might be just a constant in some cases,
  but in this case will depend on the value of a game rule.

### Comments

Comments can be added to stringtables by starting a line with `#`.
Comment lines are ignored when processing the stringtable. 
Comments are often written immediatley before a key line, 
and may contain information about parameters that will be substituted by 
the game engine into that string at runtime.

```text
# %1% amount of research points required to research the technology.
# %2% number of turns required to research the technology.
TECH_TOTAL_COST_ALT_STR
%1% RP / %2% Turns
```

Groups of related stringtable entries are often preceeded 
by a section marker comment, which is formatted with two initial blank lines,
and then three lines starting with `##`, with the middle ## line having a section title, eg.

```text


##
## TechTreeWnd
##

TECH_DISPLAY
Display
```

## Tool
The tool to format and validate and translate string tables: [st-tool](/check/README.md).
