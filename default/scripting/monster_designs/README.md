# Instructions for editing ship or monster designs in game in the Design window.
1. Move or backup the `shipdesigns` directory in your freeorion data directory.
2. Copy the `default/scripting/ship_designs` or `default/scripting/monsters` directory into your
data directory and replace your `shipdesigns` directory.
3. Start a new game.
4. In the Design windo, your saved ship designs will be the default ship designs or monsters.  Edit the ships.
    i. Add new saved designs.
    ii. Drag and drop to change ordering.
    iii. If you use a stringtable index for **both** the ship name **and** description then it will save the stringtable index and use the translated text for display.  You can check this in the pedia window.  When you add/remove parts the name and description in the pedia window will be pulled from the stringtable. 
    iv. Note: You can not start a ship with a monster hull, but you can add/remove components to monster designs in the saved design directory.
5. Exit the game.
6. Copy the `shipdesigns` directory from your data directory back into the `default/scripting/ship_designs` or `default/scripting/monsters` directory.
