## default/scripting directory

Contains directories needed for game content.

Each sub-directory is allowed multiple files/directories,
with the exception of *starting_unlocks/*

There are 2 types on files loaded, by extension: .focs.txt and .inf.
All other file extensions are safely ignored, to disable a file simply rename
the extension.

For more info on the FreeOrion Content Script language,
see http://www.freeorion.org/index.php/Effects

## .focs.txt files

May contain any number of entries and or macros.

If you need to reference a macro in a separate file, use an #include statement.

## .inf files

These files are specifically targetted and may not be broken up or renamed.
inf files will not normally be included.

## includes

Work as normal, however all of the script files in each section are loaded.
(e.g. all of the scripts in techs and its sub-directories are loaded as techs)

Include statements should not reference script files that contain a definition.

## Contents

**All directories must contain at least one *name*.focs.txt file, with at least
one associated definition (except for common and starting_unlocks).
Additional entries may be required from other content, such as the AI or other
definitions.**

* buildings/  -  All in-game building definitions.
* common/  -  A directory to group files commonly included, no files are
loaded from here unless included elsewhere.
* empire_statistics/  -  Calculations made on a per-empire basis.
* encyclopedia/  -  Entries for in-game pedia articles.
* fields/  -  Definitions of fields (e.g. Ion Storm)
* monster_designs/  -  Ship designs specifically for space monsters.
* ship_designs/  -  Pre-defined ship designs, this is **required for the AI**.
* ship_hulls/  -  All ship hull definitions.
* ship_parts/  -  All ship part definitions.
* specials/  -  All specials definitions.  Each individual object
(ship/planet/etc) may have specials attached.
* species/  -  All in-game species definitions.  **The *SP_HUMAN* definition
is required.**
* techs/  -  All in-game technology definitions and categories.
* keymaps.inf  -  TBD - Keymap definitions.  **This file and at least one entry
are required.**
* monster_fleets.inf  -  Fleet definitions for space monsters
(spawn rate/quantity/limitations).  **This file and at least one entry are
required.**
* starting_unlocks/  -  Contains lists of items that are unlocked for each
player at the start of the game.
* starting_unlocks/items.inf  -  Items each player starts the game with:
completed techs and available buildings, ship hulls, and ship parts.  **This
file and at least one entry are required.**
* starting_unlocks/buildings.inf  -  Buildings each player starts with
pre-built.  **This file and at least one entry are required.**
* starting_unlocks/fleets.inf  -  Fleets each player starts with.  **This file
and at least one entry are required.**


## filename guidelines

* .focs.txt and .inf extensions are lower case

The following are not strictly required, but are standard guidelines:

* Replace spaces with strings
* Single entry files are in all caps
    * SAMPLE_ENTRY.focs.txt
* Multiple entry files are in camel case, remove any spaces
    * SampleEntries.focs.txt
* Macro definitions are lower case with a .macros extension
    * sample_entry.macros
* Disabled files have the extension .disabled
    * SAMPLE_ENTRY.disabled
