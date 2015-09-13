# Modding Guide (Work in Progress)

## Contents of the default/ directory:

* buildings.txt, fields.txt, ship_hulls.txt, ship_parts.txt, specials.txt, species.txt, techs.txt: These define the basic content types referred to in their names. Note that species.txt often contains the effects of techs/buildings, for technical reasons. See [the FOCS specification](http://freeorion.org/index.php/Free_Orion_Content_Script_(FOCS)) for more information.
* stringtables/, encyclopedia.txt: The first contains files all text displayed in-game (both translations and the English!), including descriptions for content. If you add just about anything, you'll need to add the entries to stringtables/en.txt for its name and descriptions. The second file lets you add custom entries to the in-game pedia as documentation (with the text itself still in stringtables/language.txt).
* data/art/, data/sound/: Contain the audiovisual assets FreeOrion uses. New content can grab an icon from data/art/icons, or a graphic from another of the folder; there are many currently-unused files!
* AI/: Python code to control the computer players. The AI can automatically adapt to many content changes, but some it needs to have hard-coded to successfully exploit/avoid.
* python/turn_events/: Python scripts that run at the beginning of every turn, and can trigger events that would be impossible to do purely in FOCS.
* python/universe_generation/: Python scripts that get run at the beginning of the game and create the galaxy. You can customise the galaxy generation by editing options.py and universe_tables.py, both of which have more information in comments over there. The latter in specific controls which star types, planet types, planet sizes, and also other content get placed.
* col_bld_gen.py: This is a script to generate col_buildings.txt, which is included by buildings.txt and should not be edited directly, from the list of species that can build colonies.
* premade_ship_designs.txt: These define what built-in ship designs may exist, without you having to design them in-game. Technically this has no gameplay effect, since you can just delete the whole file and redesign them yourself, if you wish.
* preunlocked_items.txt, starting_buildings.txt, starting_fleets.txt: Define what all empires start out with. The former are the things you start off with researched/able to build, the latter physical things you get on/in the same system as your homeworld on turn 1.
* space_monsters.txt: This defines what hulls and parts the various space monsters have; the FOCS code that actually make them move and act are in ship_hulls.txt, where the hull definitions for the space monsters are.
* space_monster_spawn_fleets.txt: This defines where/how space monsters may show up at the beginning of the game.
* shared_macros.txt: Useful macros (text substitutions) used in various FOCS files. Editing an entry here may affect several files (which may be useful, or not what you intended).
* content_specific_parameters.txt: Contains lists read by FreeOrion controlling its behaviour, including which buildings italicise the system name, which species tags will be shown in the Census pop-up, and which specials count as growth specials.
* global_settings.txt: This should also contain data used to control game mechanics.
* customizations/custom_sitreps.txt: Edit this to generate sitreps as useful reminders. The file itself has more in-depth information.
* customizations/common_user_customizations.txt: Another file that control the interface. See the contents for more information.
* empire_statistics.txt, alignments.txt: These are various calculations made on a per-empire basis. They don't currently have any effect but in the future they may interact with species populations.
* keymaps.txt: Edit if you want to change how FreeOrion interprets a keypress. If you press a key corresponding to the first number, it will be interpreted as the second number; if they're between 8-127 these numbers are interpreted as ASCII codes.
* empire_colors.xml: Options for the color for empires; the formatting is a standard RGB scheme, shown by example in the file.
* shaders/: Stuff used by the rendering engine. Unlikely to be necessary (or useful) to change without code changes.
* DejaVuSans files: the fonts that FreeOrion uses; you can change the font either by replacing these files, or changing your config.xml.
* credits.xml: Acknowledgments of those who have made FreeOrion possible.
* COPYING: Open-source license for the default content
