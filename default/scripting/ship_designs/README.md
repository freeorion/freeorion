## Required design names
The game currently requires all of the following design names:

All entries contained within the default/scripting/ship_designs/required directory

Any entries mentioned within default/scripting/starting_unlocks/fleets.inf


## Instructions for editing ship or monster designs in game in the Design window.
1. Move or backup the `shipdesigns` directory in the freeorion data directory.  See the installation directory README.md##Directories for the data directory location.
2. Replace the `shipdesigns` directory with the `default/scripting/ship_designs` or `default/scripting/monsters` directory.
3. Start a new game.
4. In the Design window, the saved ship designs will be the default ship designs or monsters.  Edit the ships.
    * Add new saved designs.
    * Drag and drop to change ordering.
    * Use a stringtable index for **both** the ship name **and** description to save the stringtable index in the file and use the translated text for display.  To use a stringtable index, type its tag name in the ship name or description field.  Add/remove a part to cause the name and description in the pedia window to update from the stringtable. 
    * Note: monster hulls are not in the empty hulls list, so a saved design can not be started from a monster hull.  However, monster hull based designs can be edited once copied into the saved design directory.
5. Exit the game.
6. Replace the `default/scripting/ship_designs` or `default/scripting/monsters` directory with the `shipdesigns` directory from the data directory.
7. Start a new game to verify that the default ship designs or monsters have updated.
