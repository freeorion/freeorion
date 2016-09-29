# Changelog

Notable changes to the FreeOrion project will be documented in this file.


## [v0.4.6] - 2016-09-16


### Key Changes

- Reworked supply mechanics (details below)
- Reworked weapon part refinements
- Reworked ship repair: Drydocks, Damage Control techs,
Robotic hulls self repair (details below)
- Many small additions/changes to GUI rendering, layout, functionality,
customization options, and displayed information that collectively
substantially improve usability, responsiveness, and system resource usage.
- Turn processing time improvements
- AI improvements
- New specials, hulls, ship parts, giving new strategic or fun gameplay options


### Detailed Changelog


#### Graphics / GUI

- New default font Roboto ver. 2.001101 (2014) (with a few extra
missing glyphs needed / used in the GUI)
- Fixed layout of production / research info panel
- Reworked rendering of various GUI windows to improve performance
- Improved functionality of pedia search box
 - Get a list of results instead of jumping to the first matching entry found
- Added mousewheel cycling through items in droplists
- Improved window layout in response to game window resizing
- Made queue and info windows resizable / movable
- Increased default tooltip delay
- Increased default sitrep icon size
- Added a tooltip for when the in-game save button is disabled.
- Added messages when game save is started.
- Improved blending between empires' visibility radii colours
- Added various hotkey commands, including mapping the map or tech view
- Suppressed rendering of ridiculously huge visibility circles, such
as from super testers
- Added menu button to open freeorion.org
- Tweaked appearance of colour picker indicator in HS square to have a
hole/outline where the lines meet
- Add the distance scale circle to the MapWnd right-click menu
- Added separate option for FPS limit when the game window doesn't
have OS input focus
- Added 3D starfield effect on map
- Add an entry on the right-click menu to add to top of build queue
- When left-clicking production queue items, open the system and
select the planet instead of just jumping to the system on the map.
- Made ctrl-left click remove an item from build queue.
- Made ctrl-double click add entries to the top of the build queue
- Pedia articles can now have images in them
- New icons for auto and manual turn cycling button
- Tweaked Sidepanel planet pole tilt angle
- Tweaked Sitrep layout
- Reworked rotating selection indicator rendering to be smoother
- Re-enabled sitreps for winning / losing the game
 - Add an icon to the PlayerListWnd when an empire wins/loses
- Made Pedia ship design detail page show troop stats reflecting
racial bonus/malus
- Clicking a sitrep planet handle will select the planet for production
- Tweaks to rendering techs on tech screen when zooming
- Positioned cursor at end of historical input from pressing up/down
in messages window
- Added current turn expected progress to research and production
progress meters
- Added specialized tooltip for resource icons
- Made encyclopedia detail panel of production and research screens closable
- Switched a few objects list columns from showing object ids to object names
- Added nearest system column and number of specials column to objects list
- Production and research queue items show max spendable PP or RP per turn
- Added ship rally point tracking / setting to production queue. New
fleets are grouped by rally point on the turn they are produced. Ship
production items can be right clicked to set their rally point to the
selected system.
- Added tooltips to production selector items explaining failed
production location conditions
- Fixed issues navigating the directory structure in the save/load
file dialog on Windows when accented / non-Latin characters were
present in path names.
- Stripped selected Wnd text of tags before passing to clipboard
- Implemented selecting or moving cursor to next word edge when
holding CTRL when a text GUI widget has input focus
- Hid map screen pedia when opening the production screen
- Tweaked tech tree layout
- Added icons to tech tree panels showing unlocked or affected other content
- Added button to show partially unlocked techs on tech tree, which
are now shown by default, and are techs that have at least one prereq
researched.
- Allowed wide/tall windows to spam multiple monitors
- Added option to disable effect accounting in client, which could
speed up GUI responses to things that would normally cause an
accounting update, such as focus changes
- Made spin controls on Options screen wider
- Improved layout of various UI windows / popups for variable font sizes
- Added "Update Design" and "Add New Design" commands to Design Screen
- Added drop position indicator for completed designs list
- Improved format and detail of combat log, including calculations for
each attack and attackers being revealed by attacking
- Added right-click Cut-Copy-Paste popup menus to text entry boxes
- Made shift-delete cut and shift-insert paste in text entry boxes
- Expanded set of allowed hotkey combinations
- In SidePanel confine planet rename context menu item inside planet name
- Moved Infrastructure indicator to population panel.
- Moved supply meter from military to resources panel.
- Options screen widgets to set objects list column widths
- Tweaked how panels in fleets and planets lists resize with or
without a scrollbar present
- Added right-click popup menu command to copy text in most GUI text
- Fixed issues where characters font textures weren't loaded even when
using a suitable stringtable, making it impossible to use those
characters in game
- Added a right-click command to dismiss sensor ghosts on the fleets
window, which causes an empire to forget a fleet existed, removing it
from the map.
- Disabled sound system init when sound and music are disabled
- Fix crash when OpenAL device does not initialize
- Added message box at startup informing player in case of audio
system init failiure
- Added message box at startup informing player in case of
insufficient OpenGL version
- Made pressing escape close more GUI windows
- Added tech turn and RP requirements to tech panels on tech tree
- Modified autogenerated fleet names to better reflect fleet contents
- Increase fleet drop target width and add an icon to it
- Modified how client and server communicate in a single player game
to avoid firewall popups about network access
- Added scanlines over fleets when not currently visible
- Added nearly-accurate estimate of meter changes on the next turn to
meter tooltips
- Made MultiMeterStatusBar rescale to show largest value
- In DesignWnd add ability to drag parts away from design
- Fixed confusing behaviour of main menu when displaying credits
(main menu was dimmed, but still active, now requires pressing ESC or
mouse click anywhere to stop displaying credits)


#### Content

- Translation updates: French, Russian
- Removed / updated various missing, unused, or incorrect stringtable entries
- Various adjustments to AI priorities
- Made AI more cautious of hidden threats after losing fleets in a
system it can't detect into
- Updated pedia descriptions of various content and UI windows,
including adding more links
- Reworked intro sitrep to be less immersion-breaking
- Moved start-of-game unlocking sitreps to before-the-first turn to
avoid spamming the player
- Removed "Average X" descriptions of species, leaving these only for
the all-average humans
- Made collapsing nebulae create various coloured stars
- Added a "Rough Military Strength Estimate" empire statistic
- Added Spinal Antimatter Cannon
- Improved cross-platform consistency of random results in universe
generation and in-game effects
- Added Starlane Nexus and Starlane Bore buildings that can be used to
create starlanes
- Reworked Spatial Distortion Generator to push ships back along their
approach starlane
- Added Black Hole Collapser and Subspace Rift buildings
- Adjusted some star names
- Added sitreps to Planetary Starlane Drive effects
- Various new hull art and slot positions, tech icons, building icons
- Added the Accretion Disc special
- Fixed / updated bioweapon sitreps
- Added a random turn event that creates Space Krill in an empty system
- Added new galaxy shape "Disc"
- Added the feature to resurrect extinct species found in ancient ruins with
the Xenoresurrection Lab (which can only be built on a planet)
- Added the new special Temporal Anomaly
- Reworked weapon part refinements, so that instead of multiple parts,
techs increase the strength of parts (although getting the increase to
take effect requires being in supply range)
- Added Energy Frigate hull, adjusted some other hulls' balance accordingly
- Updated premade / default ship designs, made naming consistent
- New bombardment weapons (feature in-progress)
- Added EMP and electric overcharge ship parts
- Added Solar Concentrators which increase laser weapon strength for
Organic Line hulls depending on local star type
- Split up stealth part and planetary stealth techs
- Added more AI greeting messages, made random
- Added Scrying Sphere buildings, which can be produced and appear
around the map randomly during galaxy generation, and share vision
between planets on which they are located
- At start of game, the Honeycomb will now turn all planets without specials
or inhabitants within 5 jumps into Asteroids instead of destroying them —
without renaming them
- Removed the special Volcanic Activity
- Tweaked planet / asteroid naming
- All systems except deep space, black hole and neutron star systems now
contain at least one planet or asteroid belt
- Added "Telepathic Detection" species trait (currently only assigned to Trith)
which gives basic visibility of inhabitated planets nearby a colony of a
species that has that trait.
- Fixed "no repair" sitreps caused by drydocks on outposts


#### Balance

- Reworked supply propagation mechanics
 - In any system, at most one empire can supply ships and exchange
production between planets.
 - Empires' supply ranges push against eachother in neutral systems,
with higher range supply sources blocking lower range (where range
decays with distance from the source)
 - Armed ships prevent enemy supply from propagating in/out of a system
 - In the absence of armed ships, supply won't propagate into an enemy
system where a supply source planet is located
- Substantially reworked ship repair
 - Drydocks require a ship to be stationary and only repair a proportion
of ship structure of the bigger hulls
 - No damage control effects work on the turn a combat has taken place except
for the Logistic Facilitator flagship, which is substantially reduced
 - Balance pass on Robotic hull line and Damage Control techs repair rates
- Various building / tech cost adjustments
- Split Ground Troops species trait into Defensive Ground Troops and
Offensive Ground Troops.
- Made Neutronium Forge only be buildable at locations with a Basic
Shipyard (like all other shipyard upgrade buildings)
- Tweaked monster detection and stealth stats
- Artificial Moons can no longer be built when a Resonant Moon special
is present
- Reworked various modifiers to planet population
- Tweaked Honeycomb and World Tree special locations
- Re-Added the Tidally Locked Rotation special with a population
penalty and changed the penalty of the Eccentric Orbit special to a
Supply minus
- Space Monster weapon balance tweaks
 - Adjusted Experimentor monster weapons
- Planetary Starlane Drive modified so it can't be built on gas giants
- Death Spores and Bio-Terminators now only affect organic species
- Monsters now only bombard one planet at a time
- Reworked how derelict visibility effect works - grants visibility
directly instead of temporarily increasing ship detection range
- Added preferred focus for most species
- Made producing troop pods require a production location with troops
 - Species with no offensive Ground Troops will no longer be able to
build ships with Ground Troop pods
- Balanced research and production bonuses
- Reprioritised mine effects to always occur before repair effects
- Balanced BioAdaptive hull to work same as Nanorobotic
- Tweaked numbers for Logistics Facilitator
- Enable queue additions of energy shipyards in systems with pending
artificial blackhole
- Prevent Experimentor spawn location from sundering the galaxy
- Balance Hyperspatial Dam
- Made planets in systems which contain a Gateway to the Void building
not provide any supply


#### Bugs

- Prevented dragging sitrep entries
- Fixed depopulation of planets to work when planet has a negative population
- Fixed rendering of object window items selected with right click
- Fixed multiplayer galaxy setup droplists problematic scrollbars
- Fixed protection focus effect on max troops
- Fixed AI building neutronium extractors
- Fixed AI problem with ship design with id 0
- Fixed experimentor spawn location being blocked by natives
- Fixed a segfault when resigning a game with no empire
- Fixed Fortress special mines effects not properly working together
with System Defense Mines techs
- Prevented unowned planet mines from affecting unowned guard ships
- Fixed unowned fortress mines damaging unowned monsters and made
mines able to destroy monsters, not just damage them
- Fixed ion storm stealth effect to not affect fields (and thereby itself)
- Fixed issue with stealth effect of Transpatial Drive
- Reduced credits scroll flicker
- Fixed quirks with list scrollbars and row positioning
- Fixed crash when manipulating fleets in Fleets window
- Fixed issue where saves would record an extra nonexistant player,
causing problems when loading the save
- Fixed issue with dragging parts over another part in a design
- Prevent a bug with high CPU load and low FPS. A font is now cached
even if a requested glyph is not part of the font, but a 'replacement
character' is provided
- Fixed crash when setting hotkeys
- Fixed a bug where calling the suitability report from the production
window would toggle the production window pedia panel visibility. Now
shows suitability report in production window pedia.
- Fixed alphabetical sorting of object list columns when accented
letters or other non-latin characters are present
- Fixed a bug where pedia closed instead of showing an entry
- Fixed deleting production items by pressing delete
- Fixed crashes when clicking links in pedia
- Fixed calculation of FPS
- Fixed resource panel not showing expansion
- Fixed a bug where the AI would sometimes think it can't build parts
at a planet despite it actually being able to
- Fixed fleet destination text being "holding at ..." when moving
along a path that returns to the start location.
- Fixed xenophobic self-sustaining pop malus and xenophobic frenzy
happiness malus


#### Technical / Internal

- Made it possible to read binary or compressed XML saves regardless
of current save output format setting
- Made communication between clients / servers on different operating
systems more flexible / robust
- Reworked XML save format to have an uncompressed somewhat
readable-as-text header with basic info about the saved game
- Added --version parameter to freeorion{,d} executables which outputs
the version string to the console
- (*nix) Moved user config and data files to conform with
XDG Base Directory Specification
- Reorganized content definitions into multiple files


## [v0.4.5] - 2015-09-08


### Key Changes

- Replaced Ogre3D with SDL based windowing for human client.
  - Improved mouse rendering and responsiveness and keyboard behaviour,
    particularly when switching between windows.
- Interface and turn processing time improvements.
  - Tech tree rendering speed especially improved.
- Extensive AI updates.
- New pedia content including introductory articles on game concepts and the
  interface.
- Revised colonization mechanics to allow producing a building that will
  convert an outpost to a colony.
  - Species added must have a supply connection from a already-populated planet
    with sufficient happiness.
  - Production time depends on distance from the production location to the
    already-species-populated planet.
- New graphical combat summary.


### Detailed Changelog


#### Graphics / GUI

- Added a pin toggle to most UI windows, which fixes them in place.
- Added a Pedia text search box and adjusted Pedia layout.
- Adjusted default window positioning.
  - Made some window positions be saved between sessions.
- Various new/updated icons, including building and sitrep icons.
- Added an option to save games in an XML text format, which should be readable
  on any OS, although is a larger file than the binary serialization (default).
- Adjusted the size of the planet focus droplist.
- Changed layout of sitreps to be multi-line.
- Tweaked ship design description part list to show multiple copies of a part
  with a number, instead of repeating the part name.
- Changed allowed hotkey bindings, so most printable character hotkeys need to
  have a modifier key (CTRL, SHIFT, ALT) held, to prevent hotkeys from firing
  while typing text.
- Removed some unused sound options.
- Additional object list column types and filter conditions.
- Added popup menu command to save ship designs as a file, and list of saved
  designs that can be used when designing ships.
- Added monster designs tab to ship design screen.
- Tweaked layout in the generic file load/save dialog.
- Made mousewheel scrolling pan the tabs in a tabbed window widget.
- Fixed layout placement of buttons in object filter window.
- Made colonization suitability report always include Exobots.
- Added an option to toggle whether the pedia in the design screen will
  instantly update as a designs text is edited.
- Removed various unused properties from ship designs in the pedia
  (and internally).
- Added some helper sitreps informing of a planet having sufficient
  population / happiness to be used for recolonization.
  - These helpers are easily customizable by players.
- Changed default player empire colour.
- Added combat assessment to ship designs 'pedia pages.
- Added right-click commands to object list to set planet focus.
- Made the objectlist update when a planet changes focus.
- Made cycling through sitrep turns skip turns with no sitreps.
- Made display of moving fleet ETA controllable with a config option.
- Add "does not stack" information to building descriptions.
- Removed unimplemented statistics about empires from pedia.
- Added tooltips to auto/manual turn advance toggle.
- Made copy and paste functions in GUI work with OS clipboard.
- Added right-click command on icons to look up building types.
- Added an optional mapscale circle overlay to MapWnd, centered on the
  currently selected system.
- Added (yellow, 3-flanged) mouseover indicator for unexplored systems
- Made some of the darkest empire colors slightly less dark.
- Made pressing return to map hotkey close the fleetwnd, if open, and if not,
  the sidepanel, if open, if no other subscreens were open.
- Ensured that if a planet has nonzero max defenses (or shields or troops) the
  SidePanel PlanetPanel will display the military panel regardless of whether
  the planet is populated or owned.
- Troop ships and fleets will now show their total troops value
- Adjusted sitrep font size to be the greater of the client UI standard font
  size or 75% of the size for sitrep icons.
- Added several new hotkey commands.
- Made most interface windows be constrained to within their parent window, or
  the application window.
- Added support for an optional `persistent_config.xml` settings file which may
  contain a subset of settings from `config.xml`, which will override
  `config.xml`.
- Made double-clicking planets on the sidepanel open the production screen.
- Enabled "snoozing" (dismissing from display) of a sitrep by double-clicking
  or right-clicking.
- Removed max size from options window.
- Made the DesignWnd Redundancy check be applied to all part types, not just
  weapons.
- Enabled customization of the DesignWnd redundant part filtering criteria via
  `common_user_customizations.txt`.
- Removed (most of) the extra space in ListBoxes after the end of the content.
- Made SitRep panel remember which SitRep it was showing first, when it is
  resized.


#### Content

- Translation updates, most notably an (almost) complete French translation.
- Extensive pedia content reorganization and additions, including introductory
  articles.
- Transpatial drive part.
- Improved randomness of random empire species selection.
- Exobot colony origin building replaced by new-standard add-species-to-planet
  style building, but without a standard species requirement of having a source
  of that species supply connected to the production location.
- Made AIs occasionally respond to chat messages (not meaningfully).
- Added three new specials: World Tree, Honeycomb and Fortress.
- Added two variants of Artificial Planets: Artificial Paradise World and
  Artificial Factory World.
- Made high tech natives special produce one or more buildings on planets.
  - "Primitive" natives can't get this special.
- Added a higher-capacity advance ground troops pod.
- More empire statistic tracking.
- Added sitreps, including nebula collapse, dyson forest birth.
- Expanded Nebula collapse to create random combinations of asteroid fields and
  planets.
- Added colony buildings for natives that can colonize but can't build ships.
- Added the Maintenance Ship as an alternative to the Sentry monster type.
- New native species: Happybirthday.
- Added Species Homeworlds page to Pedia.
- Improved slot positions of ship hulls.
- Changes to fields: Ion Storms and Molecular Clouds now start very small, but
  grow large before shrinking again.


#### Balance

- Tweaked galaxy special distributions to lessen frequency of multiple specials
  on one object.
- Concentration camps require a population of at least 3 to be produced, and
  set planet happiness to 0.
- Hull production and research cost rebalancing.
- Adjusted artificial planet cost.
- Adjusted cost of tech victory.
- Reworked troop level calculations from various techs and buildings, including
  being more population-dependent.
- Adjusted timing and results of Experimentors doing things, tweaked their
  shields and defense.
- Made mines work on monsters.
- Made the requirement for attacking during combat be having partial visibility
  of the target, to be consistent with the criteria for starting a battle.
- Added requirements for a minimum number of planets and systems near home
  system locations.
- Reduced shield part strengths and costs.
- Tweaked monster maturation to depend on the game turn.
- Added shields to the Bloated Juggernaut and reduced its damage value.
- Adjusted the Krill Spawner part (from Ancient Ruins) so that when in an
  unarmed ship it provides a stealth bonus.
- Removed biospores from level 1 Kraken and Juggernauts. For level 3 of same,
  changed biospores to bioterminators.
- Phototrophic natives can now have bright or dim stars (not only orange/yellow
  ones).
- Changed time to build Phinnert colonies to 75%.
- Removed Large Planet trait from Scylior and Hhhoh (natives) and added it to
  the Etty (playable).
- Self Sustaining Xenophobes (ie Trith) should have more stable population
  problems as it's based on explored systems not visible systems.
- Adjusted monster nest special monster spawn rate to be sensitive to
  GalaxyMonsterFrequency; left the domesticated spawn rates alone.
- Ancient Ruins will have a location condition of at least 3 jumps from empire
  homes.
- Dyson Forests will have a location condition of at least 3 jumps from empire
  homes.
- Dyson Forest's regeneration effect will receive a no-combat-last-turn-here
  condition.
- Homeworlds and capitals get +5 happiness.
- Rebalanced planetary stealth and detection modifiers.
  - Standardised modifiers as multiples of 20.
  - Each stealth special can be penetrated by the same level detection.
  - All planets given base stealth 5.
- Made the megalith producible only on planets with an imperial palace, and
  provide a troop bonus to nearby systems.
- Added ProductionQueue support for frontloading and/or topping-up, controlled
  by `default/global_settings.txt`.
- Planetary Ecology will now grant a flat + 1 bonus to Good and Adequate that
  expires with Symbiotic Biology.
- Starting planets have max happiness.


#### Bugs

- Fixed multiplayer lobby player list scroll position resetting when receiving
  updates from the server.
- Fixed rare bug where Floaters could spawn Forests in almost every system.
- Removed design deletion right-click popup for moderators to avoid crash.
- Fixed issue where setting the default save filename in a FileDlg would be
  overwritten when the working directory was set.
- Fixed checks of slot type when automatically adding parts to a design.
- Fixed in-progress building icon tooltips on sidepanel.
- Tweaks to child/parent windows to prevent possible memory leaks.
- Fixed potential hang when single player galaxy setup was cancelled.
- Fixed rendering issue with detection rangle circles on map.
- Fixed bug where a ship that is destroyed during a combat round when it it
  attacked multiple would be counted multiple times in the ships-destroyed
  statistic tracking.
- Excluded empty (destroyed) fleets from empire pedia listing of owned fleets.
- Fixed issue where after pasting text into an edit control, the cursor would
  always be at the end of the text.
- Prevented natives from spawning in player home systems.
- Fixed glitch with blockades.
- Made rounding precision "epsilon" value of production and research larger
  (0.01), so being a tiny fraction of a point away from complete is considered
  being complete.
- Fixed bug where a planet could have its combat round squandered due to having
  another planet selected as target (invalid).
- Fixed a bug where a combat sitrep would not be sent to an empire whose outpost
  had been in combat, if the empire had no non-outpost combatants.
- Changed combat attacker order shuffle algorithm to standard Fisher–Yates
  shuffle to avoid a bias.
- Fixed issue where centred button text labels weren't repositioning properly
  when resizing.
- Fixed object list column sorting of numbers (as numbers instead of as text).
- Fixed a bug where outposts could be prematurely excused from combat, allowing
  them to wrongly get shield regen the next turn.
- Fixed multicolored system names on the MapWnd for systems with more than one
  empire owning planets.
- Fixed names in multiplayer lobby, which were showing the local player's name
  for all players.
- Fixed combat 'reveal' of stealthed attackers to be after all the attacks of a
  given round have been completed.
- Fixed bug with fleet movement path deterimation, where it treated a fleet as
  refueling when in supply even if it did not stop at a system.
- Fixed MapWnd cycle to next/prev fleet so that it skips over destroyed fleets.
- Fixed Pedia Species Homeworlds bug that had caused entries with more than 5
  occupied planets to be left out of the display.
- Fixed planet names on sidepanel losing formatting tags when the planet is
  renamed.
- Fixed multiline text display bug when rendering colored highlighted text.
- Fixed Exobots not having Bad Ground Troops as described.


## [v0.4.4] - 2014-09-07


### Key Changes

- Interface and turn processing time improvements.
- Extensive AI updates.
- Combat mechanics reworked.
- Balance changes including supply mechanics, damage control, ship hulls.
- New "core" slot type.
- Configurable Hotkeys.
- New save / load file dialog.
- Queued fleet move orders.
- Python scripted universe generation.


### Detailed Changelog


#### New Features / Improvements

- Implemented Python scripted universe generation.
  - Can be easily modified by editing script files.
  - Accessible through "Irregular 2" galaxy shape in galaxy setup.
- New "core" ship design slot type, added to some hulls and required for some
  powerful ship parts.
- Made all empires sitreps visible to moderators and observers.
- System and ship names moved into stringtable, so they can be translated with
  the rest of the in-game text.
- Added a bombardment mechanic, allowing ships to target and affect planets
  outside of battle.
- Added a UI for configurable hotkey command key bindings.
- Galaxy setup data is stored and can be accessed from the pedia during a game.
- Graphs display added that allows plotting game history of various statistics
  about empires.
- Made it possible to refresh the stringtable or change stringtables without
  restarting the program (although not all of the GUI will immediately refresh
  text when doing this).
- Gifting mechanic allows giving ships to other empires with a presence in the
  same system.
- Improved support for FreeBSD.
- Modified system naming to group adjacent systems and give them similar names,
  distinguished by Greek suffixes, to prevent running out of system names in
  large galaxies.
- Reordered turn processing to improve consistency between server results and
  client-side estimates of effects processing.
- Added tracking and pedia display of various statistics about empires, such as
  numbers of things destroyed, lost, invaded….
- Combat mechanics reworked:
  - 3 rounds, during which objects all fire simultaneously at random targets.
  - Initially undetected ships that fire become targetable on later combat
    rounds.
- Made low-aggression AIs even less dangerous to help new players.
- Added shift-right-clicking to queue fleet move orders.
- Set default multiplayer autosave option to true.


#### Content

- Various tech tree adjustments, including removing prerequisite techs and
  merging of similar techs.
- Translation updates.
- Added / updated / corrected descriptions of various parts, techs, hulls,
  buildings….
- Added more star names and ship names, and gave unowned monsters names.
- Added sitreps for unlocking buildings, hulls, ship parts, monster growth, ….
- Reformatted / standardized sitrep text, added information to sitreps.
- Replaced the "Weapons" species bonus with piloting skills, which modifies ship
  damage.
- Organic hulls "grow", increasing structure over time.
- Core slot type introduced, appearing on some hulls, and with some parts only
  mountable in core slots.
- Changed default galaxy options to 150 stars and 6 AIs.
- Added more predefined ship designs.
- Ancient Ruins special:
  - Modest decrease in spawn rate.
  - Preventing them from doubling up in a system.
  - Much rarer now if Monsters Frequency is set to "None" (in part due to lack
    of guard monsters in this case).
  - Explored Ancient Ruins are now marked as excavated.
  - Made less sparse on Low Planet Density setting.
- Added preferred focus to species, to which new colonies will default.
- Made all (including non-monster) unowned ship hulls move around on their own.
- Head on a Spike specials track the locations of captured empire capitals.
- Natives sometimes given advanced tech bonuses, making them more dangerous to
  attack.
- Reworking the Experimentors.
  - Starlanes removed at the beginning of the game will not be replaced until
    after turn 200.
  - Sitreps announce monster release events.
  - The stealth levels of Black Kraken and Bloated Juggernauts are reduced.
  - A Victory Sitrep is generated if the Experimentor Outpost building is
    captured.
  - Ensuring after capture / destruction, fleets are not trapped in a no-lane
    system.
- Removed some old and uninteresting specials and renamed some existing
  specials.
- Added Robotic Interface Shields, which get bonuses from being located near
  other ships with the same part.
- Added Krill Spawner part as a possible Ancient Ruin unlock.
- Added Ramscoop part.
- Added Transpatial Drive part.
- Removed Interstellar Lighthouse because it was incompatible with current
  visibility mechanics.
- Reworked Psychic Domination to require a focus setting to enable, and to
  require telepathic species.
- Added some restrictions to when buildings can be added to the production
  queue.
  - Can't build a second palace.
  - Prevent quirks where a second something could be added to the queue, but
    would then become unproducible due to there being too many of it enqueued.
- AIs announce their aggression at the start of a game.


#### Graphics / GUI

- New and updated icons for various ship parts, buildings, techs.
- Added right-click popup on players list to look up empires.
- Fleet management improvements.
  - New automatically created fleets are set to aggressive if they contain
    warships, or passive otherwise.
  - The new fleet aggression toggle can be set to automatically decide
    aggression for new fleets, as above.
  - Right-clicking fleets or ships gives management commands: merging,
    splitting, and splitting by design.
- Added changing expression planet happiness icon.
- Production queue made to highlight items in the currently selected system.
- Added production queue right-click menu commands.
- Improved rendering of visibility circles.
- Limited copy-cut-paste functionality in some GUI text controls.
- Indicated mountable slot types in part descriptions.
- Made some multiline text areas scroll multiple lines per single mousewheel
  tick.
- Fixed longstanding missing item drop sound issue on options screen.
- Made looking up techs, buildings, ship parts or hulls from the map screen just
  show the pedia entry, and not open the design, production, or tech screens.
- Increased thresholds for displaying wasted PP or RP notifications, to avoid
  them appearing due to rounding errors.
- Sorted system droplist by system name (except for Deep Space, which are
  scatted through the list).
- Numerous pedia text and GUI control placement layout adjustments.
- Tweaked cloud spawning effects.
- Design screen disables the confirm button to prevent creating duplicate
  designs.
  - Added tooltips for design confirm button to explain why it is sometimes
    disabled.
- Restored system name colouring by owners, including multiple colours when
  shared between empires.
- Added right-click command to rename ship designs.
- Fixed / implemented mouseover highlighting in popup menus.
- Made clicking on a non-command in a popup menu not cancel the popup.
- Fixed a few places where changing text or list contents would annoyingly reset
  list scroll positions.
- Hid fleet/ship names when panels are small to avoid overlapping with
  destination text.
- Added Linux install targets and icons.
- Added toggle buttons for info windows: graph, messages, empires.
- Modified rendering of GUI windows to show a pointed bottom-right corner when
  resizable, and a cut off corner when not.
- Added buttons to adjust objects list window visibility filters more quickly.
- Added tooltips to tech tree tech panels, production queue.
- Made production / research queue widths adjustable in options.
- Reworked fleet icons to be more complicated and informative.
- Added All / None sitrep filter toggles.
- Enabled sitrep filters for observers.
- Added turn auto-cycle button, which automatically ends turns for players
  (immediately) or moderators (when all players have finished their turn).
- Made clicking empire PP and RP total indicators open the production or
  research screens.
- Improved disabling of order-giving GUI controls when a player's order giving
  is disabled between turns.
- Redesign of save/load file dialog, now showing information about each game,
  and sorting by file modification date.
- Added automatic generation of repeat keypresses while holding down a key.
  - Rates and delays for keyboard and mouse repeat have options in the options
    screen.
- Made objects list columns configurable by right-click popup menu.
- Enabled (glitchy) sorting of objects by any column by left-clicking the column
  heading.
- Incomplete ship design name changes are now immediately reflected in the pedia
  description.
- Added Available, Unavailable, and Redundant parts filtering on the design
  screen.
- Added right-click popup to enqueue production items and techs on their
  respective screens.
- Hulls and designs can be dragged from the list to the main design panel to
  load them (and can also be double-clicked, as before).
- Made clicking the design background image show the hull in the pedia instead
  of the incomplete design.
- Added toggle to pin GUI windows in place.
- Removed X from top-right of windows that can't be closed.
- Changed the default design screen layout.


#### Balancing

- Extensive ship hull rebalancing.
- Made colonization require at least partial visibility of the target planet.
- Made stargate construction require at least 1 population, so the required
  focus can be set.
- Added penalties to planet resource meters when changing focus, to discourage
  doing so frequently.
- Tweaked various effects, to ensure intended-bonuses never actually amplify
  penalties, and penalties never actually give bonuses.
- Various supply mechanics changes:
  - Made supply meter grow 1 per turn up to max supply.
  - Changed Interstellar Logistics to give a fixed +3 to supply range.
  - Planet size affects supply: smaller gives more supply than large.
  - Space Elevator gives any planet equivalent supply to tiny planets.
  - Species can have good or bad supply traits.
- Increased native planet troop levels, making invasions more costly.
- Adjusted universe generation special distributions.
- Made meters reset to 0 if a planet is conquered, and disabled meter growth on
  the turn after capture.
- Changed Ion Storms to give fixed 40 point penalty to stealth and vision.
- Made derelict specials stealthy (40 stealth).
- Adjusted monster spawning rate dependence on galaxy setup setting.
- Damage control techs rebalanced.
- Made Concentration Camps require a population of 3 to be built.


#### Bugs

- Fixed production costs at start of queue processing, so that processing the
  queue doesn't affect the costs of other queue items during the same turn.
- Hopefully fixed issue causing dangling server processes from failed load game
  attempts, which would prevent starting a new game.
- Fixed issue with having multiple different production block sizes on the queue
  leading to incorrect production costs.
- Fixed Collective Thought Network bonus range.
- Fixed bug causing display errors in the production queue, including "never"
  for items that should complete in one turn.
- Fixed xenophobic self script triggering from outposts with negative
  population, or from use of exobots.
- Fixed homeworld growth focus overriding homeworld bonus for the same species
  elsewhere if it has more than one homeworld.
- Fixed crash and memory leaks when filtering the objects list.
- Gaia special will no longer trigger for Exobots on an Adequate planet.
- Fixed issue on OSX where selecting observer or moderator mode in multiplayer
  lobby could cause a crash.
- Fixed memory leak in tech tree display code.
- Fixed memory leaks in AI ship design code.
- Removed rename command from other empires' fleets' popup menus.
- Fixed issue where failed colonization attempts could leave ships in an
  inconsistent state.
- Fixed issue where hotkeys would be enabled while entering text, preventing
  certain letters from being typed.
- Fixed issue where auto-exploring fleets wouldn't update the GUI to show that
  they had been told to stop exploring.
- Hopefully fixed issue where the system instead of bundled Python framework was
  being used on OSX.
- Fixed issue with a ship moving from a system with the derelict special to
  another system with the derelict special not receiving both bonuses.
- Fixed issue with scrapping multiple ships causing a crash.
- Disabled game saving in GUI when doing so could cause problems.
- Stopped Asteroid Snails from moving.
- Fixed (broken) use of galaxy setup seed in multiplayer games.
- Fixed crash with observers enabling multiplayer autosave.
- Fixed glitchy window dragging.
- Fixed random seeding functions to properly seed more random number generation.
- Fixed some crashes when a model dialog was open (such as a droplist) when turn
  updates arrived and caused a GUI refresh.
- Fixed Cosmic Dragon adding Nova Bomb Activator specials to any / all planets
  when it was not in a system.
- Attempted to fix some quirky GUI state issues when alt-tabbing or clicking
  in/out of the game window.
- Stopped setting of galaxy map pedia article when opening the research or
  production screens.
- Adjusted population reduction formula for the Concentration Camps and
  Evacuation buildings, to correct odd results when target population is
  substantially below current population.
- Adjusted colonization process so colonizing an outpost doesn't cancel scrap
  orders on buildings on the planet.
- Fixed issue with multiplayer chat history display.
- Fixed issue with ships with self-repairing hulls starting with 0 structure
  when produced.
- Fixed rare situation where if a floater is the only monster outside a system,
  it could spawn forests at almost every system.
- Removed design window options for moderators in multiplayer to prevent
  crashes.


## [v0.4.3] - 2013-10-26


### Key Changes

- Shields now reduce damage, and all new shield & armor content.
- Many fixes and improvements to the AI.
- Detailed combat log display.
- Numerous GUI tweaks and improvements.
- Galaxy Generation is now based on a editable random seed.
- New clouds and nebulae on empty systems and moving around the galaxy.


### Detailed Changelog


#### New Features / Improvements

- Fundamental change to ship shield mechanics: ship shields reduce the amount of
  damage suffered from a hit by a fixed value (determined by the shield strength
  stat), instead of providing additional structure that regenerates between
  battles.
- Reworked supply block / fleet movement blockade mechanics:
  - Supply blocks and fleet movement blockades have been harmonized to occur in
    the same situations.
  - Non-stationary ships no longer create blockades / supply-blocks.
  - Armed aggressive fleets arriving in a system and then remaining stationary
    will maintain any supply flow for their empire, and block it against enemies
    not already having armed aggressive fleets present.
  - If armed aggressive fleets for two enemy empires arrive at a system at the
    same time, supply maintenance takes precedence over supply blocks -- they
    will each maintain supply for for their empire, and not block supply for the
    other, for so long as both sides remain and survive.
  - An empire 'secures' use of a starlane as an exit from a system simply by
    having a fleet enter that system along that starlane.  If no armed
    aggressive enemies were already present in the system, then passage through
    all exiting starlanes is secured for that empire so long as it maintains an
    armed aggressive fleet presence in that system.
  - Passage of an empire's fleets through 'unsecured' starlanes exiting a
    blockaded system is blocked until the blockade is completely broken, by
    defeating all armed aggressive enemy forces present (previously movement was
    only halted for one game turn); exit along a 'secured' starlane will be
    allowed after the fleet has halted for at least one turn in the system, even
    if the blockade is not broken.
- Raised max number of systems to 5000 in GUI.
  - Note FreeOrion is balanced for and performs best with a galaxy in the low
    100s of systems.  Higher numbers make a game increasingly unbalanced and
    make rendering performance and turn processing times worse.
- Many fixes and improvements to the AI.
- Added a configurable random seed value for galaxy generation.
- Made focus visible on detected planets a player doesn't control.
- Detailed combat log display.
- New Moderator player type in multiplayer mode:
  - Sees the full universe like an observer.
  - Controls the length of turns.
  - Can create and destroy objects, add or remove starlanes, and set ownership
    of objects.
  - Can be used with no other human players and several AIs to have a
    more-controlled AI test arena than using observer mode.
- Made fields visible if any part of them is within detection range, rather than
  their centre.


#### Content

- Major revision of shields/armor/weapons (due to the new shield mechanics):
  - Added new shield techs and parts.
  - Added new armor techs and parts.
  - Rearranged tech tree for armor and shield techs:
    - Changed starting techs: instead of basic shields players now get basic
      armor at the start of the game.
  - Completely reworked/rebalanced shield, armor and weapon parts stats, tech
    and build costs.
  - Adjusted space monsters accordingly.
  - Adjusted a few basic pre-made/starting ship designs, discarded the rest.
  - Adjusted affected species picks accordingly.
  - Reduced Acirema ship shield bonus to +1.
- Added molecular clouds that reduce ship shields, which spawn outside the
  galaxy and move in.
- Added nebulae that spawn on no-star systems.
- Added more species write-ups.
- Added terraforming reversion building.
- Translation updates: Finnish, French, German, Italian, Polish, Spanish,
  Russian.
- Tweaks/refinements for Evacuation building.
- Concentration Camps now cause a lingering negative effect on population, of
  indefinite duration even if the camp is removed.
- Fixed Solar Orbital Generator building description.
- Fixes to Psychogenic Domination and System Defense Mines techs.
- Corrected description of requirements for the Static Multicellular hull.
- Fixed Egassem species description.
- Tech tree reorganization and various adjustments to techs.
- To avoid confusion removed "can colonize" property from species that can't
  build ships.
- Fixed weapon species picks:
  - Grant not only a specific refinement level, but also all preceding techs at
    the start of the game.
  - Lowered weapon tech for ultimate weapons from Laser 4 to Laser 3, adjusted
    premade ship design "Mark IV" accordingly.


#### Graphics / GUI

- Replaced galaxy map toolbar button text with icons.
- Added detection range circles opacity control to galaxy map options.
- Added empire statistics to empires window.
- Sitrep window will not pop up if the design, research or production screens
  are open.
- Sitrep messages:
  - Added sitrep message when tame monsters mature.
  - Added sitrep message upon total population loss from Concentration Camp.
  - Added sitrep messages for mines.
  - Added sitrep messages for Psychogenic Domination.
  - Reordered sitrep order of presentation; is now customizable via content
    files.
- Augmented census popup with tag/characteristic info.
- Reorganization of species description text.
- Planet and field icons in objects list.
- On the ship design screen ship parts are now hidden for which improved/refined
  parts have become available that are superior in every aspect (so, Laser 2
  hides Laser 1 etc.).
- Added "X" close buttons to objects list and pedia windows.
- Added aggregate fleet stats icons at top of fleet window.
- Moved focus drop box to top level of planet panel, out of the resources
  subpanel.
- Overhauled research screen rendering:
  - Rewrote rendering of tech panels.
  - Adding tooltips to tech panels with details that are now not shown on the
    panel itself.
  - Highlighted enqueued techs on tree.
  - Tech panel mouseover and selection indicators.
  - Removed tech type buttons on tree filter.
  - Added prerequisites and unlocked techs to tech descriptions in pedia.
  - Removed tech navigator. Pedia entry for techs have equivalent links.
  - SHIFT + single click enqueues a tech.
  - CTRL + double click enqueues a tech to top of queue.
- Ship stats of a ship design displayed on the ship design screen now take into
  account effects groups acting on a ship.
- Readjusted fleet window summary bar to show current stat rather than projected
  next turn values.
- Modified pedia panel "up" button behavior to always be enabled and to always
  go directly to the index, regardless of what page the view is currently on.
- Replaced pedia panel button text with arrow icons.
- Tweaked multiplayer default empire names, species and colors selection.


#### Balancing

- Reduced required distance to moving ships for Collective Thought Network
  (from 500 to 200).
- Species balancing:
  - Having xenophobic species in an empire causes disruption (in form of
    maluses) for the colonies of the xenophobic species as well as for other
    colonies near them.
  - Xenophobic species get the Concentration Camp tech at the start of the game.
  - Reduced Etty detection range malus (from 20 to 9).
- Increased chance for no star systems.
- Made Gas Giant Generator and Interstellar Lighthouse building bonuses
  non-stacking.


#### Bugs

- Fixed spawning of Acirema guard ships so as to not be triggered by (player
  owned) Acirema ships.
- Fixed several issues with one click colonize/outpost buttons.
- Fixed issue with empty fleets appearing in-game after their ships have been
  destroyed.
- Various fixes/adjustments to Concentration Camps.
- Fixed issue that sometimes stars were stacked on top of each other during
  galaxy generation.
- Fixes for various issues with loading/saving games.
- Restricted speed bonuses granted by Infrastructure Ecology tech and
  Interstellar Lighthouse building to ignore immobile ships.
- Fixed stealth reducing effect of Interstellar Lighthouse building.
- Fixed display of system name for systems that have no star but do have
  planets.
- Fixed map hotkey issues.
- Fixed issue that production progress stops for ships if their design is
  deleted.
- Fixed non-working "Cancel Scrapping Ships" order for fleets.
- Fixed issue where planets that lost population and converted to outposts
  retained their focus setting.
- Fixed some crashes in research screen / tech tree.
- Fixed detection range circle overlap artifacts at high detection ranges.
- Fixed issues with fleet stat calculations.
- Fixed sometimes inaccurate population projections on colonize button and
  planet suitability report.
- Numerous other bug fixes.


## [v0.4.2] - 2013-02-20


### Key Changes

- Very much improved, non-cheating AI. Sometimes experienced 4X players loose.
- Many GUI enhancements and shortcuts.
- Galactopedia expanded with game mechanics articles and many cross-links.
- Batch production of ships now possible.
- Improved sitrep notifications
- Reworked stealth and detection
- Almost everything has been enhanced, reworked, and better balanced.


### Detailed Changelog


#### New Features / Improvements

- Very much improved AI:
  - Added an AI aggression setting during game setup.
- Keyboard presses should now account for the operating system language setting,
  and it should be possible to type non-Latin characters in-game as long as the
  appropriate character set is used in the selected stringtable file.
- Added an objects list window, with collapsable rows showing objects containing
  other objects:
  - Objects list has an incomplete filtering function to control what objects
    are shown.
  - Right clicking an object in the objects list give a command to data dump
    about that object to the messages window. (For debug purposes).
- Added ion storms moving around the map. Ships and planets in storms have
  reduced stealth and detection.
- Improved Sitrep:
  - Added type filters.
  - Old sitrep entries are kept on later turns, and may be reviewed.
  - Different icons for different event types.
- Added droplists to items on production queue to produce multiple items before
  being removed from the queue.
- All ships produced by an empire on a single turn are now put into a single
  fleet, rather than a separate fleet for each ship.
- Added a happiness meter to planets.
- Disabled production of duplicate buildings when one already has been produced.
- Various turn-processing and GUI speed improvements.
- Added a Protection Focus
- Reworked combat:
  - An attacking ship will fire each of its weapons separately at independently-
    selected targets.
  - A ship that is not visible on the map to an empire cannot be fired at by
    that empire's ships in battle until the ship fires on one of that empire's
    ships.
- Reworked autosaving
  - Autosaves are in a separate directory from regular saves
  - There is a customizable limit on the number of autosaves to keep


#### Balancing

- Unified resource and fleet supply into a single meter: the Supply meter.
  Infrastructure is not involved.
  - Outpost now provide 0 jump supply, and colonies 1 jump supply.  Techs and
    buildings can extend that.
- Removed infrastructure from having any effect on producing Industry or
  Research.
- Greatly reduced the number of Theory techs that have no direct effect.
- Adjusted price and research time of many techs.
- Recallibrated Stealth & Detection Strength numbers:
  - to max around 100.
  - to work with the new mechanics.
  - Reduced detection ranges for ships and planets by roughly half.
- Loosened requirements for Asteroid hulls/parts
  - Asteroid processors are no longer shipyards, but must be built on a belt for
    a shipyard anywhere in the system to build asteroid hulls.
  - Rock/Crystallized Armor Plating now requires an Asteroid Reformation
    Processor somewhere in the empire.
- Rebalancing of weapons, ship hulls, armour, and shield parts.
- Made Terraforming more expensive, and made it get more expensive for
  repeatedly terraforming the same planet.
- Made colony ships more expensive.
- Disable production of shipyards at outposts.
- Adjusted Microgravity Industry to "Increases Industry by +5 on all Industry-
  focused colonies in system with an asteroid belt outposts. Additional asteriod
  outposts do not provide additional benefit."
- Removed loophole that made Exobot Origin buildings cheaper than building.
  Exobot colony ships on Exobot worlds.
- Some rough species balancing:
  - Weakened Trith with Narrow EP.
  - Strengthened Egassem with Ultimate Industry.
  - Strengthened Laenfa with Average Industry.
  - Strengthened Gyisache with Great Research.
  - Species with a starting weapons bonus now also start with more ships.
- Major Redo of Ground Troops:
  - Protection focus now increases max troops to 200%.
  - Species Bonus/Malus is now multiplied.
  - Native planets and homeworlds now have a bonus to troops.
  - Outposts have weaker defenses.
  - Increased cost of Garrison Techs, and decreased capacity of Troop Pods.
- Increased penalties to population on poor and hostile planets.
- Planet meters, such as population, can now be negative, which can lead to
  faster loss of population on uninhabitable planets.
- Redstricted production of imperial palaces if an empire already has one that
  it produced.
- Some buildings deduct from their planet's infrastructure, reducing benefits
  from having high infrastructure on those planets.
- Added increasing cost to some ship hulls depending on the number of ships an
  empire owns.
- Incrased amount of information players are sent about other empires, including
  capitals and their supply network.


#### Content

- More Game Concepts 'pedia entries.
- Russian & French translation update.
- Added Dampening Cloud monster.
- Added Planetary Evacuation.
- New ultimate detection strenght tech "Omni-scanners".
- New detction strength booster special, Panopticon Complex.
- Added Philosopher Planet special.
- Added Furthest species.
- Added a ship speed boost effect to lighthouses.
- New engine ship parts.
- Added planetary shield and defense boosting techs.
- Reworked stargate mechanics.
- Made three of the indigenous species telepathic.


#### Graphics

- New Focus & Planet meter icons with matching chart colors.
- Sitrep icons.
- More unique icons, especially fo ship parts.
- Added scanlines to more places in GUI to indicate objects the player
  previously saw but can't see on the current turn.
- Added multi-turn expanding ring effect for supernovae.


#### GUI

- Added one-click colonize and invade buttons to sidepanel.  Selecting ships in
  the FleetWnd will override the default selection of ships to use for these
  actions.
- Added cumulative weapon, structure, and sheild, information to fleets so it is
  easy to gauge a fleet's chance of winning.
- Numpad keypresses now enter numbers when numlock is on, and act as arrow
  navigation keys in text edit widgets.
- Added option to swap left and right mouse buttons, for left-handed players.
- Added line to producible item tooltips to indicate that items must be
  unlocked.
- Added key remapping function for hotkeys, via the file `keymaps.txt`.
- Recoloured most planet meters.
- Added right-click popup menu commands to:
  - set map / misc options (enabled in options screen).
  - split damaged ships from a fleet.
  - split unfueled ships from a fleet.
  - scrap a whole fleet.
  - have a fleet automatically explore.
  - look up special icons in the 'pedia.
  - look up ship designs in the 'pedia.
  - generate planet-species suitability report.
- New Census tootlip that shows the species breakdown of the player's empire's
  population.
- Other empire resource icons have simple tooltips.
- Adjusted contrast of progress bars.
- Hid some details of projects on production queue when there is insufficient
  screen space.
- Added red/green colour coding of colony predicted target populations on
  colonize button.
- Added ! indicators on top of screen to indicate if PP or RP are going unspent.
  - These indicators may be clicked to open the appropriate production/research
    screen.
- Added thick starlane indicators in empire colour to show connected systems
  that can share production.
  - The colour of these indicators is altered conspicuously when the group of
    systems has unspent PP available.
- Alphabetized entries in most encyclopedia lists.
- Added fleet summary statistics (number of ships, weapon strength, etc.) to
  fleet data panels in fleets window.
- Added somewhat unsightly text indicator of star colour to system sidepanel.
- Made popup menus less transparent by default.
- Changed how names are shown for enemy ships and fleets.
- Added Can Produce Ships and Can Colonize conditions to popups for ship designs
  in the production screen, to better indicate why they can / can't be produced
  at a location.
- Added links to techs that can unlock buildings, ship hulls, and ship parts in
  their 'pedia entries.
- Added empire summary icons for empire detection and a count of ships owned.
  - Removed mineral and trade icons.
- Added a plain-text stealth indicator to the building tooltip.


#### Bugs

- Disabled building shipyards at outposts.
- Disabled the construction of any duplicate buildings at the same location.
  It is still possible to double-queue a building and get multiples.
- Fixed longstanding font corruption with non-Latin text.
- Partial fix for a crash with font text colours wrapping over line ending in
  the encyclopedia window.  May still result in glitchy text colouring, but
  hopefully won't crash.
- Fixed issue where some producible items would be replaced by empty space on
  their list.
- Fixed issue where in-game options taking text would be saved as "1" instad of
  blank text.
- Hopefully fixed crashes when manipulating fleets.
- Fixed miscalculation of number of production projects in progress.
- Fixed building indicator progress bar
- Prevented ships from engaging in combat with planets of which they have only
  basic visibility.


## [v0.4.1] - 2012-08-03


### New Features/Improvements

- Players can control zero population planets or 'outposts'.
- Reworked stealth and visibility system. Empires now have a detection rating,
  and ships/planets have a detection range. Within range, detection strength is
  determined by the empire's detection ability. Outside range, there is no
  detection.
  - Specials now have stealth ratings, and may not be visible to empires with
    insufficient detection ability.
- Removed minerals. Planets now just generate industry that is used for
  production.
- Added an aggression setting for fleets, which can be toggled between passive
  and agressive. Passive fleets try to hide and don't attack enemies. Aggressive
  fleets attack any enemy they can, and can block supply propagation.
- Added very basic diplomatic system. Empires start at peace, and can declare
  war, propose, or accept peace with other empires. Battles between empires
  happen only when at war. Diplomacy is controlled through right-click menu on
  the players list.
- Made unarmed ships unable to block supply propagation.
- New 'Game Concepts' section of the 'Pedia to help exlain the game.
- Macro support in script files.
- Experimental AI process priority management on Windows builds, which will
  hopefully improve GUI performance at the start of turns in games with multiple
  AI players.
- Many new sitrep messages.
- Many Tooltips are now more informative.


### Balancing

- Reworked how poplation growth works. Food and health no long exist. Instead,
  planets can be set to "growth" focus to boost population directly. There is no
  longer any food distribution or starvation.
- Rebalanced stealth & detection levels.
- Balanced bonuses to population to max at 100
- Flattened the distribution curve of star and planet types.
- Increased cost of troop pods to make conquest less casual.
- Made monsters nests last on average twice as long.
- Halved the generation of floaters from forests.
- Colony and Outpost bases (for in-system use only) can be built without
  shipyards.
- Greatly weakend the Sentry and Sentinel.
- Increased the effectiveness of shields and armor by ~2x.
- Removed internal slot from basic small hull, but increased fuel capacity.
- Switched the premade colony and outpost ships to a medium hull, but increased
  the cost of the colony and outpost parts to compensate.


### Content

- Fixed many spelling errors in various descriptions.
- Updated many inaccurate descriptions.
- Tweaked ShipDesign encyclopedia entries to show total cost instead of per-turn
  cost, to be consistent with buildings.
- Translation updates, particularly Russian and French.
- Species updates:
  - All species now have different aptitudes at resource production, ground
    combat & so on.  A few have special stealth and detection abilities.
    - Sorry, currently very un-balanced.
  - 5 classes of species: Organic, Lithic, Robotic, Phototrophic,
    and Self-Sustaining.  Population in each flourishes for different reasons.
    More differences planned.
  - All Homeworlds now provide a population bonus, and for some species, can be
    set to 'Growth' to provide population bonuses for colonies of the same
    species.
  - Added Phototrophic species pick, which increases population with brighter
    suns.
  - Added Exobots, a constructable robot species.
  - Changed roster of playable species.
  - Acirema native colonies now occasionally produce ships, and have better
    defenses.
  - Additional minor species.
- Monsters updates:
  - Greatly increased the strength of monsters.
  - Added Experimentor outpost which produces dangerous monsters later in the
    game.
    - These monsters may be overpowered for the time being.
    - Destroying/capturing this outpost is a victory condition.
  - New Asteroid Snail space monster.
  - Added Nova Bomb and biological weaponry.
- Special updates:
  - Adding 3 new growth Special Resources each for Organic, Lithic, & Robotic
    species.
  - Abandoned Colony special now works.
  - Added three Derelict system specials.
  - Added new stealthy planet specials, and stealth-making monsters.
  - Computronium Moons now have empire-wide benefit
- Tech updates:
  - Removed Farming and Mining oriented Techs.
  - Reduced the number of different types of shipyards while increasing the
  cost.


### Graphics

- All monster types now have unique icons.
- Added unique graphics for each hull.
- Unique graphics for more shipyards.
- Added overlay for Ancient Ruins.
- Added new fleet icon for outpost ships.
- Assigned hull thumbnails to show in the sidebar of the design screen.
- Improved the logic behind the choice of graphic to display for hulls and ship
  designs.
- Disabled 'shininess' in sidepanel planet rendering, which wasn't working
  properly.
- Hid map detection range circles for objects not visible to the player.
- Made all resource indicators just show the available / produced amount, rather
  than also having a change prediction for some of them.


### GUI

- Removed slot type and availability toggles on the design screen part palette.
  Now only available parts of all slot types are shown.
- Renamed the 'Construction' meter to 'Infrastructure'.
- Added some diplomatic status, and player type and status information icons to
  the players list.
- Generate fleet arrival sitreps for all visible fleets, not just the player's
  own fleets.
- Added basic combat damage sitreps.
- Made top-of-screen toolbar not draggable again.
- Rearranged military icons of planets to make troops more prominently
  displayed.
- Adjusted display of numbers like 240 so they are shown as "240" instead of
  "0.24 k" in some cases.


### Bugs

- Fixed bug where new fleets created by the player wouldn't appear in fleets
  window until the next turn.
- Fixed memory leak / massive FPS drop when the sidepanel had a rotating planet
  control open and the system was repeatedly modified.
- Fixed crash when adding new ship designs and ending the turn.
- Fixed randomization of starting species in multiplayer lobby.
- Fixed crash when enqueuing buildings with very low costs.
- Fixed ordering of planets on sidepanel, which was by internal id number
  instead of orbit number, which lead to numbered planets appearing out of
  order.
- Fixed crash generating cluster galaxies with few stars.
- Fixed some crashes with test / temporary code in 3D combat testing.
- Fixed issue where last tech on the queue wouldn't disappear when removed by
  the player.
- Fixed broken pre-made ship fleet window icons.
- Many description improvements, and some spelling corrections.


## [v0.4] - 2012-02-05


### New Features / Improvements

- Further reduced turn processing times.
- Added ship repair.
- Split LifeFrequency galaxy setup setting into separate Monster and Native
  Frequencies.
- Added lists of planets and fleets to Empire encyclopedia entries.
- Disabled order issuing on various screens between turns, which should avoid
  potential crashes.


### Balancing

- Simplified the tech tree.
- Adjusted stealth and detection values for monsters and early ship hulls.
  - This helps avoid sudden appearances of ships in player systems, battles, and
    resulting loss of ship messages without warning.
- Imperial palace now adds +10 to the capital planet troop meter.
- Switched research output to be population based.
- Added ship shield regeneration when they have not recently fought in battles.


### Content

- Added techs for increasing the size of troop garrisons.
- Added new 'Planetary Defense' tech category.
- Combined some theory techs with their application to simplify the tech tree.
- Made some techs, ship parts, ship hulls, and building types that had very
  large costs be officially not researchable or producible.
- New Sitrep messages:
  - when a planet is terraformed by the Terraforming building.
  - when a tame space monster is 'born'.
- AI script updates.
- New default pre-made shipdesigns for empires.


### Graphics

- Made companion stars always visibile in in side-panel.
- Added clearer fleet icons.
- Made AI empires choose more distinctive colors.
- Made structure meter icon distinct from health meter icon.
- Added icons for some ship parts.
- Added animated system selection indicators.
- Replaced ground invasion overlay graphic.


### GUI

- Sorted techs alphabetically in encyclopedia index.
- Added a separate message for starting up AIs so any problems with that won't
  appear to happen during universe generation.
- Hid unresearchable and unproducible items on the tech tree, production screen,
  and design screen.
- Made map zoom with the slider smoother.
- Removed separate fleet windows that opened when double-clicking a fleet in the
  main fleets window.
- Limited messages window history to 100 lines.
- Coloured fleet window icons with empires / monster colour.
- Tweaked tech dependency arc positions.
- Added tooltips to producible items in production screen list.
- Added highlighting to show which producible items can be produced on the
  selected planet.
- Made automatically adding parts to ship designs more flexible; parts will be
  moved around to accomodate the new one, if possible and necessary.
- Improved in-game chat autocomplete.


### Bugs

- Fixed problem with non-Latin font character rendering.
- Fixed tamed monsters wandering off on their own initiative.
- Fixed some crash bugs related to the Psychic Domination tech.
- Fixed bug where the tech tree view reset every time a tech was added or
  removed from the queue.
- Fixed / prevented crashes when font files are missing.
- Fixed misplaced text on tech tree tech panels.
- Fixed a crash occurring during tech tree layout.
- Fixed centring on techs on tech tree.
- Fixed issue that caused AIs to do nothing on OSX.
- Fixed crash or graphical glitch ocurring when transferring all ships out of a
  moving fleet.


## [v0.3.17] - 2011-09-23


### New Features / Improvements

- Added space monsters.
- Added neutral natives on planets.
- Added frequecy of monsters and natives can be adjusted during galaxy setup.
- Ground troop ship parts that allow a ship to invade neutral or enemy planets.
  Planets can be invaded only if their shield meters are zero.  The original and
  new owners get sitreps after invasions succeed.
- Very basic ground combat system for capturing planets - ship combat can no
  longer capture planets.
- Reduced turn processing time, particularly for large galaxies.
- New versions will ignore old `config.xml` files, preventing some crashes after
  updating.
- Added more lists of content, and more details about content, in the
  encyclopedia.
- Reduced file save size by using binary format (instead of semi-readable text).


### Balancing

- Generally a lot of little tweaks to make the beginning of the game more
  playable.
- Decreased cost of colony pods by 20%.
- Unexplored systems block resource sharing between planets, preventing players
  from knowing about a hostile presense in a system before exploring it.
- Ships no longer have functional detection when moving on starlanes.  Ships
  must be in a system to detect objects.
- Planets can be seen, although without details, by any object in their system,
  regardless of detection and stealth.  Detection strength above planet stealth
  gives details about the planet.
- Planets do not increase their shield, troops, or defense meters on a turn they
  are attacked.
- Fleets must end a turn's movement if they arrive at a system that contains
  hostile fleets and no friendly fleets.
- Increased base detection of all hulls to at least 5.
- Colonies target construction is increased to 20 so once the meter fills, they
  create 1 jump supply lines.


### Content

- Added a lot of minor "native" species, a few of which have extended
  descriptions.
- Experimental addition of various space monsters.
  - Added domesticateable space monster nests.
- Added Small & Medium basic hulls (smaller and cheaper than standard).
  - Improved, usually cheaper, pre-made ship designs.
- Added Computronium Moon & Damaged Computronium Moon specials.
- Added automatic terraforming to Gaian planets.
- Added guard ships sometimes to Gaian, Ancient Ruins, & Computronium specials.


### Graphics/GUI

- Added binary star variant star graphics.
- Added new building graphics.
- Systems with no stars but with populated planets now show their system name on
  the map.
- Systems with only non-player populated planets have a lighter name on map than
  non-populated systems.
- New sitrep messages.
- Slightly improved resolution settings apply button functionality on OSX.
- Made default sidepanel with slightly larger so that the 6th meter icon in
  planet panels will not be partly hidden.
- Made default message window and players list wider and less tall.
- Invade button on planet panels works similarly to the Colonize button; a troop
  ship must be selected in the same system.


### Bugs

- Fixed issue where empty fleets would remain when moving ships around.
- Fixed crash when using conditions to check the distance between two systems.
- Made sidepanel disable focus droplists when the planet is not owned by the
  client's player.
- Modified the Imperial Palace to only work on planets owned by the empire that
  produced it, preventing captured palaces from moving a player's capital.
- Fixed bug with detection system where ships in a system could detect
  arbitrarily stealthy objects in the same system regardless of their detection
  strength.
- Fixed issue where planets would not feed themselves, even if they produced
  adequate food, while being blockaded.
- Fixed issue where ordering a single ships to colonize multiple planets in the
  same system could leave nonfunctional cancel buttons on planet panels.
- Fixed bug where actual system name was being shown instead of "Deep Space" in
  the FleetWnd.


## [v0.3.16] - 2011-06-15


Changes since the last release include:

- Replaced the fullscreen turn progress window that covered the map with
  progress indicator text in the message window.  The map now stays visible
  between turns, but with order-issuing UI disabled until the next turn starts
  being playable.
- Chat messages are now enetered in the same message window with a more obvious
  interface for entering text and displaying old messages.
- Added an empire / player list window above the chat / message window. When any
  empire or player is selected, chat messages are sent only to that player.
- The server now sends updates several times between turns, including fleet
  movements before combats, and a full update (as before) at the start of the
  next turn.
- Added an encyclopedia to the in-game interface, showing information about game
  content like ship parts, techs, species, etc.
- Added the option to have observer players in multiplayer games. These players
  can see the whole universe, but don't have an empire to control and can't
  control when turns are ended.
- Added the option to have AI players in multiplayer games.
- With the observer option and AI players in the multiplayer lobby, it is now
  possible to create an AI-only game with only a human observer.  This works
  like an AI test arena, with the AIs playing against eachother and no human
  player.
- Added a "Drop" option in the multiplayer lobby, to allow removing human or AI
  players from games.
- Added "Deep Space" to possible types of systems in universe. These systems
  have no star, but may still have a planet.
- Made map screen zoom centre on the cursor instead of the centre of the screen.
- Improved rendering support for graphics cards that don't support rendering
  buffers and shaders.
- Made system sidepanel resizable and draggable.
- Made the UI generally respond better to FreeOrion window resizing.  The intro
  screen generally looks good, but production and research don't resize
  properly, yet.
- Content updates, including new species.
- Tweaked fleet window layout to remove the new fleet drop panel when the window
  is very small.
- Added optional autogenerated effects descriptions to various tooltips and
  content descriptions. These must be enabled in the options to be seen, since
  many people complained that the autogenerated text was hard to read.
- Translated stringtable updates.
- Updated AI scripts.
- Reworked autoresolved combat.
- Added an "Apply" button on the resolution screen.  This seems to work on
  Windows, but may not on other operating systems.
- Changed the tech tree layout code to a custom implementation.  There may be
  some quirks with this new system.
- Tweaked layout and rendering of meter bars.
- Fixed issues with food distribution to planets.
- Made rollover link text colour modifiable in options.
- Made SidePanel retain scroll position of planets list between opening and
  closing the panel or changing system, and when pressing the turn button.
- Fixed issue with SidePanel where scrolling up planet panels would cause the
  system droplist and forward/backward buttons to become unusable.
- Fixed sidepanel scrolling problem when expanding and collapsing planet panels.
- Tweaked how colonization works, so that ships ordered to colonize, and their
  fleets, don't disappear until the turn is ended.  Ships ordered to colonize
  are marked with an indicator.  As well, colonization now can't occur if there
  are enemy armed ships in the same system.
- Various bug fixes and new features within the game content and AI scripting
  systems.
- Made the client respond more promptly to server disconnections, so that
  players don't attempt to finish their turn before being made aware of the
  disconnection.


## [v0.3.15] - 2010-08-04


Changes since the last release include:

- Added scriptable species definitions in species.txt. Species have names,
  icons, focus settings, evironmental preferences, food consumption and effects
  associated with them.  Many data-table based meter modifications have been
  moved into species effects, including population and health dependence on
  planet environment level for a species.
- Removed secondary focus setting, and removed the balanced focus option.
  Planets now have only a single focus selection, all of which much be specific
  and (hopefully) more interesting to play with and easier to create content
  for.
- Made focus settings moddable as part of Species defintions.  Available foci on
  a planet can depend on arbitrary conditions, which may make for interesting
  gameplay changes with advancing tech or situational changes during a game.
  Each species can have a distinct set of available foci as well.
- Fixed issue with custom ship designs that would frequently crash the game when
  they were built.
- Modified behaviour of design screen when modifying a design:  the incomplete
  design's info is now shown in the encyclopedia panel whenever the design is
  modified or if the main panel is clicks.  Clicking a part or hull once shows
  info about that part or hull, as before.  The information about ship designs
  in the encyclopedia is also greatly expanded.
- Added right-click popup menu command on the list of completed designs on the
  design screen: "Delete Design".  This lets players remove designs from the
  list that appears on the production screen, and from the completed designs
  list itself on the design screen.
- Added species property to planets and to ships, which is used when colonizing
  to determine the species of the new colony.  Built colony ships get the
  species of the planet where they were built.
- Added an indication of ships species next to the design name of a ship on the
  fleets window, and planets species in their population tooltip.
- Modified colonization UI to require a *single* colony ship to be selected
  before showing the colonize button for colonizable planets.  The colonize
  button now shows the initial (determined by the colony ship capacity) and
  target planet population (determined by the species to be colonizing) on the
  colonize button.
- Added some Python library files in a zip in the main FreeOrion directory in
  the Win32 installer.  This should fix issues with nonfunctional AI when Python
  hasn't been installed on a user's system.
- Removed the homeworld special, and modified species to track all the planets
  which are their homeworld, which are set during universe generation.  Multiple
  planets can be a homeworld for a species, which is helpful as there are
  presently only two species available.
- Fixed (hopefully) some issues with directories-finding code on Linux.
- Modified apperance of meter bars on the sidepanel, to support showing current
  value, change in current value, and target/max value, even if the target is
  less than the current value.
- Added a food consumption meter type, which is shown on the planet panels on
  the sidepanel, next to health and population.
- Planet population icons use their species icon, rather than a generic
  population icon.
- Added a droplist to pick species during single or multiplayer game setup.
- Replaced contents of `empire_names.txt` with various generic empire names,
  rather than the species names that were there, since species are now a
  separate concept from empires.
- Modified how empires and players information are stored in and retreived from
  save files, to make it easier to get this information when loading games
  without needing to load the whole save file.
- Made loading games much less likely to crash the client or server.  Any errors
  that occur will abort the load and (hopefully) show an error message, and
  likely return the player to the intro screen, rather than crash.
- Fixed issue with saving or attempting to write to an unwritable log file,
  which could previously crash the client or server.
- Added some logging of the time it takes to execute various initialization
  tasks, to help future performance tuing.
- Added an "inherent" meter cause type, for a few default/basic meter levels
  that are set automatically to make the game mechanics, particularly for
  stealth, work better.
- Reworked Meter concept.  Now meters have just a single value, instead of a
  current a max as before. Some meters have associated meters that are used like
  a max or target value, but not all, which is useful because various uses of
  meters don't need a max or a target value.
- Added new meter types for max and target meters, as these are now separate
  meters internally.
- Modified the population growth mechanics to account for food allocation meter
  values and the possibility of larger population than target population.
- Removed upper bounds on meter values. This may cause some UI problems if
  values are larger than can be shown in the available space, but we'll deal
  with this when it becomes a problem.
- Modified tooltips to handle new paired (or unpaired) meters.
- Changed clickable links in sitrep entries and encyclopedia entries to be
  always distinctly coloured rather than underlined when moused over.
- Modified effect parsing for new meter system, and accordingly updated content
  files.
- Moved various data table-based meter modifications into effects.
- Modified parts palette on the design screen to not show buttons for classes of
  parts for which none exist.
- Changed resource output calculation to use meter values, rather than a more
  complicated multiplication and scaling between meter value and planet
  population.  The resource output can still depend on population by making the
  effect that sets the resource meter value depend on population.  This makes
  understanding output and where it comes from simpler.
- Tweaked colour of industry meters.
- Added Dump functions to various gamestate objects, which can be used for
  debugging or logging.
- Added numerous content scripting object properies and modified existing ones,
  including adding a string property type, and properties such as object name,
  species, building type name, planet focus, ship design id, fleet id of a ship,
  planet id of a building, system id for any object, final and next and previous
  system id for ships, and the number of ships in a fleet.
- Added a SetSpecies effect and a Species condition.
- Changed Homeworld condition to use new system of species having one or more
  homeworld objects, rather than working with a special.  Homeworld condition
  can match any homeworld, or can take a list of species names whose homeworlds
  should be matched.
- Modified FocusType condition to take a list of focus type names to match.
  Planets with those foci will be matched.
- Modified BuildingType condition to takte a list of building type names.
- Fixed some issues with CMake build system.
- Wrapped some code in try-catch blocks to prevent some crashes when parsing
  stringtable entries.
- Fixed issue where empires would lose visibility of starved planets, and never
  actually observe them being stared, so would perpetually think they were in
  the state they were immediatly before starving.
- Disabled the message box that popped up to inform players they had won, as
  this was annoying, particularly when playing a game with no AIs, in which case
  a player would always win on the first or second turn.
- Replaced ship health meters with structure and max structure, as health meters
  were changed to health and target health, and it doesn't make sense to have
  more structure (-al integrity) than the max, even though a planet could have
  (temporarily) more health than its long term stable health value.
- Renamed some tooltip resource "production" strings to "output", to avoid
  confusion with the separate "production" concept in game.
- Consolidated some stringtable entries related to meter tooltips, which now use
  standard meter name stringtable entries.
- Modified victory sitreps by adding a text parameter that can be filled in with
  an entry in the stringtable by the client.
- Added VarText support (in stringtable entries) for building types, specials,
  ship hulls and parts.
- Fixed some meter value discrepancies between server and client.
- Changed conditions and effects evaluation to use the initial value of a meter
  on a given turn, so that meter values can be repeatedly recalculated and not
  accumulate as a result.
- Removed several resource output object properties from content scripting,
  since these are now redundant with the resource meter values.
- Added new hull design backgrounds.
- Removed unused ship icons.
- Fixed some app bundle isues on MacOSX
- Removed "previous" value from meters... which contained the value of the meter
  on the previous turn, as this wasn't being used anywhere.
- Tweaked behaviour of code that sets default rendering settings dependent on
  graphics card GL version support, which is intended to reduce the number of
  crashes on older rendering hardware.
- Tweaked behaviour of stealth threshold slider on galaxy map.
- Fixed crash when clicking between FleetWnd and other windows and the map
  surface.
- Added initial code to 3D combat system to support cached rendering of objects.
- Improved simultaneous server/client combat timestep updating.
- Fixed issue with server logging that was shutting down before all logging was
  complete.  This should make server log files more useful after crashes.
- Fixed a minor graphical glitch on the encyclpedia panel, where a border line
  wouldn't be rendered.
- Fixed issue with autogenerated condition / effect descriptions that treated a
  "Value" in a SetMeter effect as a property of the source object, rather than
  the target object that is should be.
- Various stringtable changes / updates, etc.


## [v0.3.14] - 2010-04-30


Changes since the last release include:

- Progress on the 3D interactive combat system. This can be tested during
  regular games by running `freeorion --test-3d-combat` or directly by running
  `freeorion --tech-demo`.  This is not yet meant to be playable, however.
- Transitioned to CMake from SCons build system (Visual Studio and XCode
  projects are also still available).
- Tweaked planet population growth dependence on health.
- Misc. content / effect fixes.
- Various rendering tweaks and configurable options.
- Made the expected locations of binary and data directories more flexible.
- Fixed issue where running `freeorion -h` would still start the program after
  outputting help text for command-line options.
- Added indicator to system resource summary to indicate when there is no import
  or export of a resource * Added location indicator to production queue items.
- Various AI script improvements.
- Fixed bug where planets weren't properly autoselected on production screen,
  preventing production items from being enqueued in certain situations.
- Fixed issue where fleet buttons were rendered over top of sitrep or sidepanel.
- Moved "Moving To Unknown System" or "Holding at System" text to fleet panels,
  rather than a separate text box below the fleets list on the fleets window.
- Changed resource meter growth to +1/turn from a much more complicated formula.
- Fixed issues with meter growth prediction.
- Made ships regenerate 0.1 fuel/turn when not moving.
- Fixed issue where previous effectsgroups's target set were being used for
  subsequent effectsgroups, leading to problems with techs or buildings acting
  on the wrong objects.
- Fixed issues with Number, NumberOf, WithinDistance, and Affiliation
  conditions.
- Fixed issues with SetTechAvailability effect.
- Added minimum population requirements, below which population of planets is
  set to 0, to prevent very small numbers being tracked.
- Made buildings on a planet become owned by empires that colonize the planet.
- Added missing TradeStockpile property for effect definitions.
- Tweaked part tooltips on the design screen.
- Expanded part encyclopedia entry text.
- Added or fixed CreateBuilding and CreateShip effects and parsing.
- Allowed full effectsgroup definitions to be added to ship parts and hulls,
  rather than just stat-setting effects.
- Fixed issue with UI number rendering showing wrong numbers of digits.
- Made systems with resource sharing connections provide fleet supply.
- Completely reworked the visibility system to take into account stealth,
  detection and distance, and to have empire remember previously-seen but
  not-currently-visible objects and include these in displayed systems in GUI.
- Added fog-of-war indicator on systems and planets that aren't currently
  visible to player, but which the player previously had visibility of.
- Fixed issue with unreadable text on research queue.
- Added Stationary condition, that matches non-moving objects (for effects
  definitions).
- Added right-click popup command to split fleets.
- Added the ability to right-click command buildings or ships to be scrapped or
  cancel scrapping.  Ordered-scrapped ships aren't included in fleet fuel and
  speed calculations.  Ordered scrapped ships and buildings have a large red X
  over their icons.
- Added an optional coloured disc around objects to show their visibility
  radius, and an additional optional slider to set the "assumed stealth level"
  that is deducted from visibility ring radius, to show how far away objects of
  the indicated stealth could be seen by ships, planets or buildngs.
- Added a check for OpenGL version and automatic setting of some rendering
  options at first startup to hopefully prevent some crashes on systems without
  OpenGL 2.0 support.
- Made default mouse mode non-exclusive on Win32, to avoid issues with
  disappearing mouse pointer.
- Reworked planet panel rendering.
- Changed UI default colour scheme.
- Fixed issue with sidepanel scrollbar not showing up when needed.
- Stored empire name and server address be remembered from game to game.
- Disabled deleting of old autosaves when loading a save, to aide debugging.
- Added safety / sanity checks to order execution to prevent AIs from crashing
  the game.
- Removed MSVC 2005 project files.
- Made resource distribution and fleet supply use known systems, rather than
  only currently-visible systems.
- Fixed issues with fleet arrival sitreps on turns when those fleets were also
  destroyed.
- Fixed fleet window layout issues.
- Fixed issue with inconsistency between expanded or collapsed panels on
  sidepanel in or out of production mode.
- Fixed crash where dragging a fleet onto the ships list would crash.
- Various translation updates.
- Fixed some issues with old gamestate flashing on the screen briefly when
  loading another game, or with old galaxy map appearing over the intro screen
  after resigning or being disconnected.
- Fixed issue where meter accounting estimates were wrong when loading games.
- Hid autogenerated effect / condition descriptions by default, since many
  people were complaining about how hard they were to understand.  Showing
  nothing is probably better in most cases, but they can be turned back on if
  desired.
- Fixed starvation sitrep generation.
- Fixed code that determine the directory where the running binary is located on
  various operating systems.
- Reworked automatic combat resolution so that combats can occur over several
  turns, and ships and planets can be damaged (see ship health indicator and
  planet construction dropping).
- Added a planet captured sitrep message.
- Disabled system name multi-empire colouring due to issue it was causing when
  interacting with underlining or bolding of system names to indicate presence
  of a shipyard or homeworld.
- Added option to specify player's name in single player.
- Fixed missing close X on FleetWnd.
- Fixed issue with "byte order mark" in stringtable files.
- Improved credits scroll rendering.
- Added an optional zoom slider to the map.
- Added utility variable "Value" to set meter effects, to allow modifying the
  existing value that's being set without needing cumbersome variables like
  `Target.MaxFarming`.
- Separated full screen and windowed resolution options.
- Made clicking on a tech on the queue show that tech in the encyclopedia.
- Made players joining multiplayer lobbies for new games be assigned different
  colours from any other players in the lobby, and be assigned empire name equal
  to their player names.
- Added meters to ships to store / determine their part stat values (range, ROF,
  damage), and effects to modify these meters.
- Added backup stringtable lookup in the English table. This means that missing
  strings in translated stringtables will be shown as English text, rather than
  an error message in game.
- Tweaked layout of multiplayer lobby screen.
- Added `--load` and `--auto-advance-first-turn` console commands to load a
  single player save with the specified name and to autmatically advance the
  turn once, which are useful for debugging.
- Fixed issue with SitRep being stuck under other windows on the map screen.
- Fixed issue where button text on intro screen would overflow available space.
- Fixed up FreeOrion icon file to have multiple resolutions, and to be used for
  the Win32 window and executable file.
- Fixed some issues with Win32 installer and uninstaller.


## [v0.3.13] - 2009-05-24

Changes since the last release include:

- Various incremental improvements to the combat tech demo, some visible
  (asteroids) and some not.
- New fleet icons that indicate the type and size of a fleet and point in the
  direction of travel when not at a system, with various options to control how
  they are displayed.
- Different arrangement of fleet icons around systems.
- Scanline-like fog of war indicators over system icons, which can be disabled
  in options.
- Reorganized UI colour options to make more sense and remove redundancies.
- Added circles around systems that have known starlanes connected to them.
- Fixed bug with irregular galaxy star placement.
- Significant AI script improvements: AIs explore, research, build ships,
  colonize and are aware of fuel range limits.
- Added a map distance scale indicator, with option to turn off.
- Changed positioning of fleet icons moving between systems to be squished
  (slightly) into the space between system rings, so there is always a
  ship-icon-free space around the star / within the system ring.
- Added scripting support for defining various properties about ship parts and
  hulls in text files, including variou statistics about ship weapons.
- Added indicators on map to show which fleet icon(s) contain(s) the selected
  fleet(s) in the active fleets window.
- Added indicators along fleets' move lines to show the positions they will be
  on future turns, up to their final destination ETA.
- Changed fleet move lines to use scrolling dots rather than stippled lines.
- Made ships passing through a system that can supply fuel at any point during
  a turn get refueled, rather than the previous requirement of starting a turn
  in a system to get refueled, and using up 1 unit of fuel to depart a system
  that has fuel supply.
- Fixed some issues with meter estimates when loading saves.
- Fixed issues with various lists in game not automatically repopulating
  themselves when something about their contents changed.
- Shrunk the size of asteroid "planet" panels on sidepanel
- Made system and planet names have italics, underlines and bold to indicate
  homeworlds, shipyards and capitols, respectively.
- Fixed some effect description parsing issues.
- Made the Imperial Palace building set the empire's capitol.
- Made techs cost nontrivial RP and turns, since many people objected to the
  1 RP and 1 turn development costs that were present.
- Added switches in a few source files to make techs, buildings and ships quick
  and cheap to build or research, without needing to make changes to their
  definition text fields.
- Updates to Russian, German, Polish, French translations.
- New Finnish translation.
- Made the fleets window resizable and changed the information displayed on ship
  and fleet panels and in the fleet window itself.
- Added tooltips to meter statistic icons on fleet panels in fleets window
- Added parsing code and text files to define or list the techs, ship designs
  and fleets that empires are given at the start of a new game.
- Added a new Optical Scanner ship part
- Moved some config files around to a directory determined at run time, so that
  nothing is expected to be in the working directory, making things work better
  on Linux in particular.
- Made colony ship colonist capacity / new colony population depend on the
  capacity of the colonization ship parts in the ship design.
- Added backgrounds behind ship parts on design screen to indicate the types of
  slots those parts can be placed in, and added matching slot graphics to
  indicate what parts can be put in them.
- Made planet panel selection work anywhere on the planet panel when the
  production screen is open.
- Fixed planet autoselection when the production screen is open, hopefully
  eliminating confusing cases where no planet was selected and people couldn't
  figure out how to build something.
- Made the selected planet and other selectable planets much more obvious on the
  production screen, by making it brightly outlines, and making unselectable
  planets have a gray background.
- Added a new Orbital Drydock building that's needed to produce some ship hulls.
  The homeworld starts with one, but building more requires some research.
  Adding this building means that colony ships can be built at the homeworld at
  the start of the game, which was previously not possible, leading to confusion
  for new players.
- Improved network connectivity on Win32 systems.
- Updated fonts, allowing more non-latin characters such as ж
- Added Mac XCode project files to SVN.
- Fixed problem where the sidepanel wouldn't return to the state it was prior to
  ending a turn or opening and closing the research, production or design
  screens.
- Added a right-click popup menu to the options screen colour selectors, that
  lets one revert a colour to its default value.
- Increased the health on adequate and good planets, so that adequate planets
  actually grow before health research is done. This lets some AI colonies grow
  that were previously stuck at their initial population.
- Added some error output to console if config.xml writing fails, hopefully to
  help figure out some issues with Russian and Greek language Win32 systems that
  are getting "Invalid UTF-8" crashes.


## [v0.3.11] - 2009-01-09

- Fixed effect accounting, seen on tooltips for meter icons for planets on the
  sidepanel, so that newly-created buildings take effect immediately.
- Gave ships in fleet window fuel indicators.
- Fixed new colony starvation bug.
- Added tooltips to part icons on design screen.
- Various translation stringtables were updated.
- Added a MoveTo effect that can be used in techs or buildings to move ships,
  whole fleets, planets or buildings around the galaxy map.
- Star graphic updates.
- Fixed route-length calculations for moving fleets.
- Fixed bug where not enough systems were generated to fit the requested number
  of AI players. Now extra systems are generated when appropriate.
- New and modified tech icons.
- Fixed incorrectly drawn fleet move lines that occured in some situations.
- Added initial, limited, implementaiton of coloured starlanes along which
  empires can exchange resources. Lanes are coloured half-way to the next system
  if the starting system could propagate resources further, but is being blocked
  by a blockade or the next system being unexplored.
- Modified food and minerals distribution and stockpiling to be limited by which
  systems are connected and able to share resources.
- Added a few options on the UI tab for the galaxy map. Can now disable
  background stars and galaxy gas rendering, can disable optimized system
  rendering, and can turn turn on or off resource-sharing and fleet-supply lane
  colouring.  Can also adjust the size of lane colouring.
- Made it possible to refer to other stringtable entries in strings.
  `[[STRING_NAME]]` will be replaced with the string with the appropriate other
  string inside another stringtable entry.
- Highlighted the buttons at the top right of the map screen to indicate which
  of the various sub-screens are open (Research, SitRep, Menu, Design and
  Production).
- UTF-8 stringtable text endcoding was added, allowing non-latin text
  (eg. cyrillic) to be rendered.  Some glitches in this remain, however.
- Removed log4cpp dependency.
- Various other minor bug fixes and tweaks.


## [v0.3.10] -  2008-01-25


This is an unstable development update, with various new features, but also
various bugs.  Changes since the last release include:

- Toggles to turn off galaxy gas rendering and optimized star rendering that
  might cause problems for people with older video cards.
- Various art additions, including ship parts.
- Updated AI scripts.
- Removed the very verbose usage / command line output when outdated config.xml
  is present, and allowed the game to keep running in this case, just ignoring
  any invalid entries in the file.
- Various Design screen UI improvements.
- Improved default layout on the tech screen.
- Various tech / content additions. You now need a shipyard to build ships, and
  start with one in your home system.
- New panel of military-related indicators on the sidepanel, for things like
  shields and fleet supply range.
- Ship hulls and parts can now be given effects, and some have had some added to
  do things like give ships more fuel capacity.
- Tweaked how meter estimates are recalculated after focus changes, hopefully to
  improve some reported horrendously long delays after changing focus in
  long/big games.  Feedback on this would be appreciated.
- Various translation updates.
- A selector on the galaxy setup screen now allows one to select how many AIs
  are in new single player games.
- Added a preliminary list view to the tech screen.
- Ships now require and consume fuel while moving, and get it replenished when
  not moving in a fleet-supplied system, which are indicated by the scrolling
  dashed lines emanating from systems. Fleets now have a fuel indicator on their
  panels in the fleets window.  (Some of the aforementioned bugs are related to
  fleet movement as a consequence of this).
- Various other bug fixes (and additions).


## v0.3.9 - ???


Changes since the last release include:

- A mostly-statically-linked Linux binary is provided that should work on many
  distributions.
- Various new and updated art (tech icons, nebulae, map stars, etc.).
- Gassy substance on the map to give galaxies shape.
- Internal details of ship designs, hulls and parts and their interaction with
  other game features, including how they're defined in text files.
- Completely reworked design screen.  The current version is much closer to the
  planned design, though is still incomplete.  It can be used to design ships by
  drag-dropping parts after selecting hulls, and to review existing ship
  designs.
- New ship parts.
- Various Python AI interface improvements.
- Various minor UI improvements.
- Various bug fixes.
- OpenGL 1.5 is now semi-required on Linux.  The program might run without it,
  but this is unsupported.  Note that future versions will require GL 2.0 for
  Windows and Linux.


## [v0.3.8] - ???


If you're confused about version numbers, pretend the last few "RC" releases
didn't have the RC.  Release naming should be more consistent in future.

This is mostly a bugfix release;  the crash when an AI empire was defeated issue
in particular should be fixed.  Additionally, the capacity to do Python AI
scripting has been significantly increased, should anyone else be interested in
working on that.


## [v0.3.1-RC7] - ???


Some new stuff includes:

- Many new icons. Yay for art people.
- The infamous food bug is fixed! (and various other bugs).
- A new initial test ship design screen. You can pick a hull and add parts to
  it, and the ship will be buildable on the production screen.  Naming your
  design doesn't work properly due to map keyboard accelerators not being
  disabled on the design screen, though.
- An initial test fleet supply distribution route calculation and display
  system.  There are small checkered lines emanating from systems with planets
  you control over top of starlanes.  These can be expanded by researching the
  Fleet Logistics tech.  Enemy fleets block the propagation.
- Various UI improvements.


## [v0.3.1-RC6] - ???


The full list of changes is too lengthy to list completely, but some of the
notables include:

- FMOD is gone!  OpenAL is now used for sound, eliminating a non-free
  dependency.
- Several other dependency versions have changed (though this isn't visible to
  most players).
- AI runs on Python.  Anyone can edit the script in /default/AI/FreeOrionAI.py
  without needing to recompile FreeOrion.  If interest in this is expressed, I
  can write up documentation about the interface FO provides for AI, so contact
  me if you'd like that information.
- The production and research screens have been overhauled.  The UI is much more
  customizable now, with movable and resizable windows and more flexible
  displays of the tree view or the buildable items.  As well, queues have icons
  for their items on them.
- The sidepanel is significantly changed, with entirely reworked resource,
  population and buildings panels.
- Building panels on the sidepanel show in-progress and completed buildings.
- Specials on a planet are shown below the rotating planet render.
- There are tooltips on the sidepanel, in various stages of (in-)completeness,
  particularly on the resource panel, that provide lots of hopefully-useful
  information about what's going on to produce the numbers that are displayed.
  This should help make understanding how the game works much easier.
- Meter and resource production estimates on the sidepanel now account for
  effects much more accurately.  There are still some issues with this to be
  resolved, but results are shown graphically.
- Much art has been added, including new tech and special icons.
- Various bugs have been fixed…


## [v0.3.1-RC5] - ???


RC5 has been released.  It represents a lot of behind-the-scenes work, and some
more user-noticable stuff too.

**Note**: The save file format has completely changed, and so your old files
will not work.

- A few more techs, buildings, and graphics associated with techs and buildings
  have been added.
- It should be a lot faster to load and save games, and to receive new turns
  from the server. (This is the behind-the-scenes stuff.)
- There is now an indicator that shows up over a system when the cursor is over
  it, and another indicator over the currently-selected system, if there is one.
- The options window was completely rewritten. It should be more complete now,
  and should work a little better.
- There are now two modes of running FreeOrion available in the start menu --
  full-screen and windowed.
- Numerous bug fixes.
