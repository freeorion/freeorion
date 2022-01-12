# Tool for work with string tables

## Glossary

- `string table` a file with translation
- `reference table` en.txt string table.
  English is the development language, so any changes related to text should be in that file.
  Also, this table us used as fallback for other translations, if a needed key is missing.
  This file is mostly maintained by people who change game code,
  so the list of entries in this file should be the most actual.
- `entry` one piece of translation, it consists of
  - `key` a sting that represents entry in the codebase (C++, Python).
    This string must be unique.
  - `value` actual translation that will be shown to the user.

### Key
Only English chars in uppercase and dash are allowed.
Renaming a key should be done with caution.
Many of the keys in the stringtable are used in the game source code explicitly,
or in content scripts.
Some keys are also generated programatically, such as by adding a prefix to other text.
C++ use `UserString` and `UserStringNop` to retrieve keys,
Python `fo.userString`, and `fo.userStringList`.

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
You can put anything except triple singe quotes inside.

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
You can customize text with special markup tags. Not full list of tags:
- `reference` special syntax that allows to put value from another entry.
  You add a key in double square braces and user will see the value of that key.
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
For most common used things like tech, building, etc we have more advanced type of reference: link


- `link` special syntax that will be rendered as a link to a wiki page about some scripted content or a ship design in the game universe.
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
  - value


## Maintain string tables
Formatting and basic checks are maintained by tool: `st-tool.py`.
See `.check/st-tool.py format --help` and `.check/st-tool.py check --help`
This two commands are part of GitHub automatic checks.
