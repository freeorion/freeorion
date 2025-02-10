Changelog
========

Notable changes to the FreeOrion project will be documented in this file.


[v0.5.1] - 2025-02-02
---------------

### Key Changes

- Species opinions of empires, depending on:
    - Empire policies
    - Species' planets being invaded
    - Being the capital species
    - Losing or destroying ships crewed by the species
    - Colonizing planets with the species
- Annexation of planets by empires allows non-combat conquest
    - Species opinions of empires affects annexation cost
- Blockading with stealthy ships reveals the minimum needed to do the blockade


### Detailed Changelog

#### Graphics / Interface

- Increased default and suggested "minimum" resolution to 1280x800
- Added a Revert Orders popup command to the Turn button
- Added an Annex planet button to the planet panels
- Added tooltips to Annex button to explain annexability and cost
- Fixed text cursor rendering issues
- Fixed double-rendering of last character of multiline text when the last character is selected
- Stopped showing scrap/unscrap popup menus for buildings not owned by the client
- Tweaked formatting of nested And conditions when generating building production location descriptions
- Render smaller fields on top of bigger ones on map
- Add ordering for game rules on galaxy setup window
- Fail to menu instead of aborting if attempting to load a missing save game file
- In list of named references, don't include the explanitory text at the top in the sorting of the entries
- Indicate paused techs on queue
- Improved reporting of detection of ships that launch fighters in combat logs
- Added sitreps for when a fleet's movement is blockaded
- Don't show scanlines on the map when there is no client empire
- In combat log, improve description of ships being detected when launching fighters
- Put default input focus in the password field of the password dialog
- Fixed some issues with text selection and copying text
- Fixed broken text formatting in chat window
- Improved reporting of initial forces in client log, in particular for stealthy neutral populated planets
- Properly labeled "Font Files" file dialog filter


#### Content and Balance

- Government
    - Tweaked ordering of Centralization, Checkpoints, and Indoctrination IP costs to not be affected by species modifiers
    - Marine Recruitment removes outpost troop penalty
    - Reduced Vassalization influence bonus
    - Adjusted various species' likes, dislikes, and free techs
    - Remove exclusion between Exploration and Colonization policies
    - Reworked Augmentation policy
    - Tweaked Isolation policy
    - Reworked Centralization policy
    - Bar Ancient Guardians from Vassalization effects
    - Increased speed requirement for Charge policy effects to 120 pp/turn
    - Species policy dislikes affect species' empire opinions
    - Added Insurgency policy that lowers enemy planet stability
    - Allowe Traffic Control and Lighthouses to work for peaceful and allied empires

- Ships
    - Made Snowflake detection range consistent with Radar Scouts
    - Made Psyonic Snowflake detection range consistent with Black Kraken/Bloating Juggernaut
    - Fix scaling for various weapon effects
    - Increase refuel rate of Ramscoop
    - Reskinned Acirema Guardship
    - Prevented Large Laser from targeting fighters
    - Gave Flak +1 shot with various weapon refinement techs

- Planets / Buildings
    - Resonant moon specials can appear on gas giants
    - Made Military Command cheaper
    - Require production locations to be visible to the producing empire and not destroyed
    - Prohibited annexation of and using recently conquered, annexed, or colonized planets
    - Make Ancient Guardians default focus Protection
    - Tweaked Kobuntura and Mu Ursh defense, detection, shield planet effects
    - Reduced cost of Energy Compressor to 150 PP
    - Tweak start location selection to have more planet type balance
    - Prevent Eccentric Orbit and Tidal Lock specials in starless systems
    - Make Philosopher special not modify planet type, but instead only appear on Radiated planets
    - Added a rule for Experimentor spawn base turn
    - Added Shipyard and Energy Compressor to Acirema planets
    - Don't deny Exobots focus due to the Racial Purity policy
    - Allowed directly terraforming outposts without using Exobots
    - Terror Suppression stability effects require aggressive fleet stance to work
    - Applied self-sustaining xenophobic malu sonly where self-sustaining bonus applies
    - Moved Bootstrapping growth boost after target/max effects
    - Made Bootstrapping never decrease industry, influence, stability, and research
    - Allowed Independence Decree to be built on populated planets
    - Allowed Environmentalism to affect all planet types
    - Tweaked Augmentation growth effects
    - Adjusting timing of policy effects to not be affected by species scaling and avoid quirks with focus changing
    - Made Marine Recruitment remove output troop reduction

- Pedia and Stringtables
    - Translation updates: French
    - Added details to various articles
    - Added some missing strings
    - Fixed some tpyos
    - Fixed accounting label for Fighters and Launch Bays 4 tech

- Other
    - Added Sitrep for neutral ship blocking colonization
    - Rebalanced Domesticated Monsters and Nest Eradication techs
    - Reworked how ships are revealed when blockading, so a few stealthy ships as is needed for the blockade are revealed
    - Added rules for what to do with planets, buildings, and ships when an empire concedes
    - Added rule for hiding other empires' adopted and available policies
    - Allow range 0 detection at the same location
    - Prevent monster spawning if a nest eradicator present


#### AI

- Correct some detection values
- Fixed serialization bugs
- Fixed some AI crash bugs
- Adjusted / fixed rating of Algorithmic Research policy
- Reworked focus switching logic
- Build prerequisite ship facilities when needed


#### Other Features

- Implemented species opinions of empires and related content
- Added game rules to support Tower Defense game mode
- Aggressive ships aren't revealed by blockading, but will reveal themselves if a combat occurs by attacking if able to do so
- Visibility by neutrals/non-empires is tracked in detail with mechanics similar to empire visiblity, rather than being always visible


#### Other Bug Fixes

- Fixed various issues with non-Latin Unicode paths on Windows

- Crashes or Disconnects
    - Fixed crash when opening sidepanel in moderator mode
    - Fixed crash when cancelling orders due to looping over orders while rescinding them

- Gameplay
    - Fix capital influence malus on first turn
    - Separate reporting of original and annexing empire in annexation sitrep
    - Fixed monster detection determiniation in combat

- Content
    - Fixed inconsistent documentation for Environmentalism and Border Checkpoints
    - Reorder policy effect priorities to avoid quirks when changing focus
    - Corrected Heavy Asteroid Hull description
    - Corrected Logistics Facilitator Hull description
    - Fixed Megalith effects preventing eachother from working
    - Correcting engine part references to speed effects
    - Fixed an initial turn influence malus at capital
    - Fixed interaction of Engineering policy with rule for ship part based upkeep
    - Fixed allie fleet repair supply connectivity logic
    

#### Technical / Internal

- Python parsing supports more conditions and effects
- Added Python typing
- Converted some content scripts from FOCS to Python
- Improved support for newer Boost releases
- Replaced lexical_casts with custom CharsToUInt8 function
- Various internal refactorings
- Avoid global gamestate lookups by passing in gamestate to various functions
- Increased minimum Boost version to 1.73
- Increased minimum Python version to 3.9
- Increased minimum MacOS version to 10.15
- Increased minimum Fedora version to 33
- Increased minimum CMake version to 3.21 on Windows
- Removed MSVC 2019 project files
- Required C++20 in non-GiGi code
- Use C++20 features in various places
- Improved Wayland support on Fedora and Gnome
- Replaced CanAddStarlaneTo condition with separate conditions HasStarlane, StarlaneToWouldCrossExistingStarlane, StarlaneToWouldBeAngularlyCloseToExistingStarlane, and StarlaneToWouldBeCloseToObject
- Added a SetFocus effect
- Added some constexpr / compile time testing support
- Include species info in content checksum
- Implemented CanSee affiliation for scripting
- Reworked how Font tag matching processing works
- Added a <reset> font tag to cancel all open tags
- Reworked text formatting to better distinguish between underlying data characters, UTF-8 code points, and rendered glyph indices


[v0.5.0.1] - 2024-03-04
---------------

### Bugfixes / Improvements

- Fixed issue with Boost for Boost version 1.82 and greater
- Fixed ship design validation
- Fixed species description in encyclopedia
- Fixed division by zero error in AI scripts
- Fixed issue with rendering caret at start of line in multi line edit boxes
- Update logistics facilitator description to fix number of external slots
- Fixed Megalith increase infrastructure effect
- Ignore irrelevant SDL error message
- Build prerequisite ship facilities when needed
- Only show scrap building popup if a scrap order could be issued for that building by the client empire
- Apply self-sustaining xenophobic malus only where self-sustaining bonus applies
- Fix division by zero when AI has researched everything


[v0.5] - 2023-03-21
---------------

### Key Changes

- Added a government policies system
    - Policies have various positive or negative effects on an empire, some of which were moved from techs to policies
    - Policies represent various social, economic, and military ideas and ideals
    - Empires have limited slots in which policies can be adopted
    - Slots are unlocked by techs, buildings, other policies, ships, species or other content
- Gave species likes and dislikes
    - Buildings, policies, or specials nearby or in an empire affect their planet stability
- Made planet stability have more consequences
    - Various planet effects depend on stability levels, including resource output growth rates and stable values
    - Low planet stability causes rebel troops to appear
    - Rebel troops can cause a planet to leave an empire
- Added influence resource
    - Spent to adopt policies
    - Consumed by colonies and outposts and various content
    - Generated by various content and associated planet focus setting
    - Going into debt will reduce planet stability
- Extensive content reworking and balance adjustments for buildings, techs, policies, species, ship hulls and parts
- Combats default to 4 bouts, with ship structure and damage adjusted accordingly
- Added additional fleet aggression settings for finer control of blockading, hiding, and combat initiation


### Detailed Changelog

#### Graphics / Interface

- Fixed UI button mouse rollover sound not playing
- Added another star forming nebula appearance
- Set minimum window sizes on design screen
- Improved pedia search performance and noted time to search at end of results
- Show + or - instead of brackets around second number (change) in StatisticIcon
- Don't show planet type wheel for asteroids and gas giant pedia pages
- Fixed extraneous precision in graph axis labels
- Allow galaxy setup panel of multiplayer lobby to scale with window size
- Fixed message color in chat history
- Made tab bar button size flexible with font size
- Disabled queue popup commands while order issuing is disabled
- Fixed hang when the production list is refreshed while a popup menu is open
- Added display of config and log directory on options directories tab
- Added a list of issued orders on the player's empire's pedia page
- Don't hide meter icons when the output is negative
- Fixed issue with multiplayer empire lists with eliminated empires when loading games
- Fixed fleets window scroll resetting when changing fleet aggression
- Added Unicode glyphs U+22C4 "Diamond Operator" and U+22C5 "Dot Operator" to Roboto regular font to use as a bullet point markers
- Added options to control for tooltip colours
- Made big fleet icons bigger
- Added an option to lock map zoom and panning
- Show script-derived floating point numbers in UI with only 3 digits
- Split Attack column on Objects List into Total Structure Damage per Battle and Number of Destroyed Fighters per Battle
- Replaced field mouseover image
- Excluded stale objects from map window determination of detector ranges
- Added (optional) rendering of future turn's predicted detection range circles for the selected fleet
- Switched to updating all object meters when showing system in the sidepanel, not just system contents, due to different results
- Started rendering fleets that are (somehow?) not in systems and not moving
- Added some hopefully-helpful explanation text to the output of "freeorion -h" on the command line
- Disallowed newlines in pedia search edit to prevent weirdness when pasting multi-line text
- Added a link to the configuration file directory in the Configuration pedia article
- Accounted for SI postfixes in text representations of numbers when sorting object list columns
- Added distance to the selected system or fleet to objects list columns
- Restricted selectable files to those that are valid and executable when setting server binary executable file on Windows
- Split ship damage and fighter destruction in ship tooltips
- Fixed neutral / unowned forces being labelled "Unknown" in combat initial stealth event descriptions
- Added clickable links in field type articles to fields in universe of those types
- Reworked automatic fleet aggression selection when splitting fleets
- Fixed some numbers being rendered as text with 1 digit of precision instead of 1 fractional digit of precision
- Fixed arrow textures having cut off glow at their edges
- Added number of worlds settled to the census report tooltip
- Removed the "No Star" label from the displayed names of starless system
- Displayed real system name on map and sidepanel for empty systems
- Replaced Colony Base Hull graphic
- Added lists of species to species traits pedia pages
- Comma separated list of planets on the homeworlds pedia page


#### Content and Balance

- Government
    - Added numerous policies that empires can unlock and adopt

- Ships
    - Ship structure and damage adjusted for 4 bout combats
    - Added the Flux Lance weapon that is "short range", attacking starting from bout 3 (or 2 if the Charge policy is adopted)
    - Added the Graviton Pulsar weapon that has 12 attacks that do 1 damage and targets anything, useful against fighters
    - Added the Space Flux Composite hull
    - Added the Small Robotic Hull
    - Split spatial flux hulls into their own hull line (were in robotic)
    - Made owning a ship with a flux bubble hull decrease colony production time    
    - Rebalanced fighter techs and pilot trait effects
    - Applied ship weapon damage rule scaling rule to species and monster weapon bonuses and 
    - Applied ship structure scaling to tree regeneration
    - Made blockade checks test that a fleet is set to Obstructive not Aggressive
    - Reworked Organic hull to be cheaper with more fuel and less slots
    - Added a stealth penalty to ships with large numbers of ships in the same system owned by the same empire
    - Added some ship stealth bonuses and penalties dependent on star type
    - Made Laenfa fuel regrow and white or blue stars
    - Made Sly refuel and repair at planets with own species, without a supply network
    - Reworked various monster ship designs
    - Added or reworked several monster weapons to do bout-dependent damage
    - Redesigned various monster ship designs and gave some extra shots or damage to monster hulls for some weapons the designs use
    - Various cost rebalancings
    - Removed prerequisite dependence between fighter and other weapon techs
    - Required a part to do more than 0 damage to destroy a figher
    - Made Chaos Wave bombardment remove outposts
    - Made snowflake bodies have good instead of excellent vision
    - Reduced living hull detection boost from 50 to 40
    - Gave Zero Point Fuel Generator a standard fuel part capacity of 20 and related default effect

- Planets
    - Added some luxury resource specials that some species like and thus get stability boosts from
    - Reduced spawn rates of some specials
    - Made various research and industry bonuses depend on stability
    - Added species traits for varying levels of baseline stability, shield strength, influence
    - Add precognitive detection to Fulver, giving basic visibility to starlane connected planets
    - Asteroid processor and Asteroid Reformation processor now remove themselves if their planet is not an asteroid
    - Made planet max shields not depend on recent combats in system
    - Adjusted troop levels after invasion 
    - Reworked Stellar Tomography to give bonuses based on the number of research-focused planets in a system
    - Reworked Distributed Thought Computing to give bonuses based on distance to the furthest owned ship    
    - Made just-colonized planets start with 6 troops
    - Disabled garrison tech troop regen on just-conquered planets
    - Rebalanced troop bonuses from garrison techs
    - Set infrastructure to 1 on just-conquered planets
    - Made self-sustaining metabolism only give population boosts on good planets
    - Rebalanced Trith species: medium starting planet, bad supply, bad industry, great influence, bad stability, make some planet types average environment
    - Reworked Replicon species
    - Gave player planets at least 6 troops in beginner mode
    - Gave Sly and Laenfa distinct non-distance dependent stability effects
    - Added the Imperial Regional Administration building that acts like a capital for colony supply distance cost calculations
    - Made distance and supply connection to the imperial capital or regional admin affect planet stability, dependent on adopted policies
    - Restricted the imperial palace stability effect to work only if the palace is on an imperial capital planet
    - Removed Galactic Infrastructure tech's supply-range boost effect
    - Made Gravitic Architecture only boost supply range of Logistics focused planets
    - Removed InterSpecies academy location requirement dependence on default focus
    - Made some species effects not fire if a planet is unowned
    - Reduced Concentration Camp industry boost
    - Added an Abandon Outpost building, which removes an empire's outpost from the planet
    - Added a Colony Independence Decree building, which removes an empire's ownership from the planet but leaves the population as an independent colony
    - Added the Nearly-Universal Translator building, which boosts influence focus influence resource output and grants a policy slot
    - Renamed Evacuation System building to Evacuation
    - Made the influence focus bonus population-dependent
    - Disabled generation of Head on a Spike from Cultural Archives
    - Added more penalties to resource meters when focus switching to mitigate focus-toggling strategies
    - Rebalanced Force Energy Structures to give fixed 3/turn faster growth or decay of meters
    - Made protection focus increase troop regeneration rate (if not attacked on previous turn)
    - Made species stability bonus for liking focus not require empire control of planet    
    - Removed Terraforming and Terraforming Reversion buildings
    - Added Popular Terraforming building that goes all the way to the preferred type for the planet's species in one step with suitable cost and time
    - Added Targeted Terraforming, separate for each target planet type, that doesn't depend on the planet's species to control the target
    - Made Trans-Organic Sentience tech boost the growth/decay rate of influence meters    
    - Reworked the population growth formula so bigger planets don't fill up faster than smaller planets
    - Added guard monsters to resource specials
    - Added an effect to Outposts so their influence moves towards target influence, which otherwise wouldn't happen.
    - Blocked enqueuing more than one of any colony building on a planet
    - Reduced scanning facility range boost from 75 to 50
    - Added some planet stealth bonuses and penalties dependent on star type
    - Reduced probabity for natives spawning on asteroids and gas giants during universe generation
    - Allowed natives species to spawn on adequate or poor planets if they have no good planet types during universe creation
    - Adjusted order of effects for species-trait scaling of resource output
    - Added guards to Biege Goo and Fiftyseven native planets
    - Added occasional defences to native planets
    - Stopped Lembalalam from being both moderate and high tech natives
    - Added some restrictions to what species research traits can have High Tech Natives specials
    - Made some monster nests randomly be stealthy from turn to turn
    - Randomly replaced 20% of ancient guardians creation with applying a stealthed planet special instead
    - Made ancient guardians less common
    - Made changes in max population affect growth more promptly
    - Set influence output to zero when evacuating a planet
    - Tweaked minimum distance to spawn monsters away from empire capitals
    - Made kraken under the ice specials harder to detect
    - Made the Lighthouse effect more gradual and less effective for planets/buildings
    - Tweaked priority for garrison troop tech effects
    - Changed Cynos liked focus to Research, which they can actually use
    - Rebalanced xenophobic self-sustaining population penalties

- Fields
    - Made star forming nebulae smaller, reduced speed and detection, increased stealth, limited number generated
    - Added Void rift
    - Added Nanite Swarm
    - Added Meteor Blizzard
    - Adjusted Molecular Cloud speed and size
    - Adjusted Ion Storm speed
    - Adjusted field spawning to be closer to the galaxy centre during universe generation
    - Made Nebulae not reduce ship speed below 5, so they don't get stuck in place
    - Added Field Repellor building
    - Made Nymnmn species attract some fields

- Pedia and Stringtables
    - Replaced various hard-coded numbers with content script-derived calculated values
    - Fixed placement limit documentation for InterSpecies academy
    - Documented Planetary Starlane Drive
    - Documented supply network propagation mechanics
    - Additional entries for planetary focus game concept
    - Updated or expanded articles for Spatial Distortion, Industry, Logistics, Planet Drive, Psychic Domination, Organic Hull
    - Added an "All Species" list in the species article
    - Improved imperial stockpile distribution documentation
    - Added lists of native, playable, and all species
    - Removed out-of-universe UI-related motivation for the planet drive beacon
    - Expanded the quick start guide
    - Elaborated on Sly description
    - Fixed robotic shields values
    - Added information about modifiers to stealth and detection
    - Updated translations: French, German, Spanish
    - Cleaned up some unused strings
    - Avoid referring to the player in the second person ("you", "your")
    - Added additional custom accounting labels for various effects
    - Renamed '%SPECIES% Colony' to 'Colony of %SPECIES%'
    - Added some missing options description strings for colours
    - Added a link in the 'new dyson forest growing' sitrep to the Dyson Forest pedia page 

- Other
    - Many building, species, and tech balance adjustments along with new government systems and influence-constrained growth
    - Adjusted info shared between allied and non-allied empires
    - Reworked how allied and not-mutual-allied supply network sharing and merging handles various situations
    - Nerfed Telepathy bonus to Psionics cost
    - Removed the Galactic Infrastructure tech
    - Added more and rebalanced universe creation frequency options 
    - Fixed issues with Starlane Bore and Starlane Nexus not correctly avoiding existing starlanes when generating new ones
    - Removed Cyborgs tech
    - Removed redundant prerequisite of N-Dimensional Subspace tech
    - Added the Asteroid coating special that appears on asteroids and gives owned asteroid ships a structure boost
    - Reworked detection levels to increment in steps of 25 instead of 25 or 50
    - Added native species: Celestephyte, Thenian, Nightsiders, Sleepers, Khakturian
    - Added a system special of derelict small troop ships crewed by Khakturian, similar to other derelict specials
    - Made Laenfa sneaky, giving them a halved research cost for planet stealth


#### AI

- AI adopts policies
- AI Manages influence and stability
- AI builds Regional Administrations and Nearly Universal Translators
- Improved invasion handling
- AI no longer uses protection focus for military purposes
- Many refactorings and adaptations for changed rules and content
- Corrected colony pod turns to build for when Lifecycle Manipulation is available
- Fixed issue with weapons that have 0 shots
- Made AI tech enqueue order more consistent between different machines
- Stopped AI from attempting to colonize unexplored systems (which may contain unobserved monsters)
- Added an option allowing human players in multiplayer games to take over playing AI empires
- Made AI aware of self-sustaining species population mechanics
- Made AI aware of distance restrictions of guard monsters
- Made AI not attempt to invade allied planets
- Rewrote Gas Giant Generator logic
- Fixed issue with slots during ship design
- Adjusted supply calculation for independant species


#### Other Features

- Added an option to save game after player issued orders
- Renamed Trade resource to Influence
- Treat empires without population and ships as eliminated
- Expanded fleet aggression settings to: initiate combat, obstruct passage (but don't initiate combat), defensive hide, and passive hide (don't fire even if attacked in combat)
- Introduced server-side auto turn advance option
- Added a game rule to make all objects visible
- Added a game rule that makes never-detected planets not (detectable regardless of stealth and detection strength) just by having an object in system
- Added a game rule that makes all systems at least basically visible to all empires
- Added a game rule that makes basic visibility systems reveal their name and star type
- Added a game rule that adjusts the tech victory difficulty
- Added a game rule that sets the minimum monster starting distance
- Added a game rule for whether extrasolar ships give detection, defaulting to off
- Added a game rule to enable repeating species during universe generation
- Added a game rule to adjust planet baseline stability
- Made ground combat allow allies to share forces
- Made planet focus reset when conquered by neutrals or rebels
- Made meter estimates count ground combat
- Ignored freshly established colonies for evacuation building effects
- Made maturing monsters keep their names
- Added some sanitization of user-input text to reject some formatting characters
- Made combat object damaged sitreps require partial (direct via detection) visibility of an object to receive
- Added sitreps when a colonization or outposting attempt fails
- Made the server disconnect AIs if a player joins who previously controlled the AI's empire
- Made the server kill the process of AI players of eliminated empires
- Showed number of fighters shot instead of number direct weapon hits in ship design pedia articles
- Reworked damage estimation and split damage to ships separately from numbers of fighters shot in ship design pedia articles
- Reworked calculation of empire total military strength
- Enabled multi-line debug python commands in chat by replacing ; with newlines before
- Added a server option to allow any player to be an observer
- Restricted some characters that can be used in player-specified names in orders
- Allow alliance victory with all empires at peace when diplomacy is disabled
- Disallowed giving away an empire's capital


#### Other Bug Fixes

- Crashes or Disconnects
    - Fixed server freeze when disconnecting a player
    - Fixed potential denial of service issue on server
    - Fixed crash when sorting UI lists
    - Fixed a crash if a fleet ended up with no previous or next system
    - Fixed crash during turn processing due to a failed check for whether a player connection is local
    - Fixed a crash with big combat logs
    - Fixed connect to server timout after 2 seconds

- Gameplay
    - Fixed bombardment not working if ships only have bombardment weapons
    - Fixed various issues with automatic turn advancing
    - Made simultaneous colonization fail instead of picking one empire based on ID
    - Fixed issue where two empires gifting eachother everything in a system would have order-dependence problems
    - Fixed issues with gifting, colonizing, invading, and scrapping at the same time
    - Disallowed mixing arrival starlanes in a new fleet or in a ship-fleet transfer
    - Made non-empire forces work like an empire for visibility within combat
    - Fixed population reduction when using planet Starlane Drives
    - Fixed issues with merging fleets arriving from different starlanes or not having arrived from anywhere
    - Fixed issue where AI clients weren't informed of modified game rules

- Content
    - Fixed Gaia special planet type selection
    - Fixed Artificial Black Hole generating multiple sitreps
    - Fixed Distortion Focus planet proximity check
    - Removed some redundant tech prerequisites
    - Fixed multiple sitreps appearing if multiple nova bombs are simultaneously detonated
    - Removed pointless damage boost for piloting for Flak
    - Fixed issue where kraken nests wouldn't hide under clouds if created after the initial turn
    - Fixed artificial moon building not removing itself after adding the resonant moon special
    - Fixed issue with loading stringtables that have many successive lines with just # comments on them
    - Added extra newlines to the end of autogenerated pedia pages to avoid issues with clickable links
    - Fixed a potential bug with the Enqueued condition that, if scripted with suitably varying parameters, could zero instead of one enqueued item if no lower limit was specified

- Python
    - Fixed Python logging memory leak
    - Fixed issue with Python text encoding on Windows systems


#### Technical / Internal

- Dependencies and Versions
    - Raised C++ language version to C++17
    - Moved to compiling with Visual Studio 2019 or 2022 on Windows, removed 2017 support
    - Python 3.7+ is now supported and required. 
    - Bumped minimum versions of various dependencies and versions included in the FreeOrion SDK
    - Improved support for later versions of Boost libraries
    - Fixed issues compiling with GCC13

- Added named ValueRefs that can be defined in scripts and referenced elsewhere in scripts or in stringtable entries
- Added a NoOp condition for debugging
- Salt combat PRNG seeding with the galaxy seed
- Partially implemented an alternative client in Godot with Android support
- Partially migrated scripted content from custom FreeOrion Content Script to Python scripts
- Fixed a probably long-standing (8 years?) bug with ContainedBy::Match that returned the opposite of what it should have, but which probably didn't affect any previously-scripted content
- Added a server flag that merges quickstart and load game options
- Compressed data in some larger message types when transmitting from server to clients over a network
- When loading single player games, assigned AIs to empires based on names in save
- Stopped client-side auto-turn-advance on errors


[v0.4.10.2] - 2021-08-01
-----------------------

#### Bug Fixes / Improvements

- Fixed issues with some unusual boost installations
- Added MSVC 2019 support
- Fixed compile issues with GCC 11 and MSVC 2019 toolset
- Fixed several compile/build issues
- Fixed issue with gifting where it was order-dependent for swapping all objects between two empires at a location


[v0.4.10.1] - 2020-09-25
-----------------------

#### Bug Fixes / Improvements

- Improved info displayed for average/total fighter damage
- Fixed Experimentors sitrep about spotted monsters
- Fixed Bio-Adaptive hull regeneration description
- Fix: apply damage scaling rule to species bonus for main line weapons and fighters
- Fix: don't make planetary max shield meter effects depend on combat conditions in system
- AI: determine colony ship build time and build cost from available parts, prevent miscalculating build time when calculating colonization priority
- Prevent crash caused by fleets getting into a glitched state when starlanes get removed
- Fixed issue where, when loading a multiplayer game, the empire drop down list in the lobby would choose the wrong empire
- Fixed Arc Disruptor icon quality
- Fixed build error when trying to build without the local git repository folder present in the source root directory
- Fixed build errors with "headless" build
- Fixed build errors with certain boost versions


[v0.4.10] - 2020-07-10
---------------

### Key Changes

- Python 3.5+ is now supported and required. Python 2 dependence is removed
- The Arc Disruptor weapon part and techs have been added, which are useful with bad pilots, against large numbers of cheap targets, and against ships without shields
- The possible targets for each weapon type has been limited, making weapon types useful in different situations
- Fighters have been rebalanced and the Heavy Bomber fighter type was added
- Added happiness requirements for normal planet resource meter growth
- Added team-based player homeworld placement for multiplayer games
- Rebalanced some bonuses from techs and specials to make high population less important and mitigate runaway empire growth "snowballing"


### Detailed Changelog

#### Graphics / Interface

- Added combat forces accordion panels to show more details
- Zooming to an object as a chat command can look up an object by ID (as well as name)
- Added arrived-from-system and ship production cost to objects list columns
- Added a mouseover indicator for fields that indicates their size
- Improved pedia search for entered text, which should now find more results, including articles generated at runtime
- Made dark red and dark blue empire colours a bit lighter, to be easier to see on black backgrounds
- Tweaked ship design screen layout
- Ship design screen description reworked to be toggled active and multiline
- Improved objects list refresh responsiveness
- Made objects list column default wider
- Added outer border highlighting of GUI window with input focus
- Added widget for bold font texture file in options window
- Moved font options to separate tab
- Added vertical scale labels to empire statistic graphs
- Added a right-click popup menu to empire statistic graphs, with several display options
- Made the window resize tab bigger
- Set a minimum size for some windows to prevent them from being made too small to be usable
- Reworked pedia window layout, putting search bar and navigation buttons at top, similar to a web browser standard layout
- Fixed display of circle arrow icons on production screen
- Made options.xml contain only modified options by default, but added a button to output the full options database to the options window
- Reworked splash / menu screen layout
- Added time to produce at the currently selected location with available PP to production item tooltips
- Added an option to reset empire readiness when joining a game, so orders can be revised even after they were submitted
- Replace total cost and minimum production time with turns to produce, if fully funded, at current production location to the producible items list
- Added indication in tooltips when production cost or minimum time are location-dependent and no location is selected
- Added option to disable disclosure of galaxy seed
- Prohibit typing some characters into galaxy seed text box, to prevent pedia layout script injection
- The server stop auto-advancing turns after a player has won the game
- Relabelled Resign button to Exit to Menu
- Removed unused / redundant keymaps configuration file
- Repeatedly clicking to close the game window will override the default autosave and exit faster

#### Content and Balance

- New Stuff
    - Added Arc Disruptor ship weapon part
    - Added sitreps for gifting to allied empires

- Reworked what weapons can target fighters, ships, and planets
    - Mass drivers, Lasers, Plasmas, Death Rays, and Spinal antimatter cannon attack ships and planets
    - Interceptors attack enemy fighters only
    - Strike Fighters (formerly called just "fighters") attack ships and fighters
    - Bombers attack ships only
    - Heavy bombers are re-enabled and attack ships and planets

- Rebalanced fighters
    - Interceptors launch 50% faster than other fighters
    - Fighter refinement techs increase interceptor capacity and launch rate, not damage
    - Techs increase strike fighter damage by +2, bomber damage by +3, and heavy bomber damage by +6
    - Adjusted costs and build times
    - Species piloting traits give -1 to +3 for all fighters except interceptors

- Ships
    - Ship production cost increases ("upkeep") also count ships on the production queue that have received some production points
    - Reduced Shield part costs
    - Colony Base hulls are more expensive, have 3 internal slots, but may not mount engines or fuel parts    
    - Scaled up ship structure to avoid fractional numbers
    - Made Space Flux Bubble hull be considered a robotic hull
    - Experimentor outpost spawns two instead of three Black Krakens, and starts 50 turns later
    - Nova Bomb prerequisites set to Zero Point Generation and Temporal Mechanics

- Planets
    - Tweaked ordering of population effects for Lembala'Lam species
    - Kobuntura prefer industry focus
    - Normal industry and research growth only if planet happiness is at least five
    - Force energy structure industry and research growth only if happiness is at least five
    - Planets can be attacked in combat with nonzero infrastructure, in addition to nonzero defense or shields, so that they will be regularly attacked every turn in combat and thus not regenerate meters every other turn
    - New colonies start with 1, 2, or 4 happiness depending on environment level
    - Outposts have default 1 infratructure so that they can be attacked every turn
    - Reduced some population-based bonuses to resource output to mitigate snowballing empire development
    - Rebalanced some flat bonuses from techs and Gas Giant Generators
    - Rebalance empire-wide pop-based specials and increase their protection
    - Added one refinement level to Nascent Artificial Intelligence and Adaptive Automation techs
    - Rebalanced Exobots to be adequate on all planet types and very bad at research
    - Made Singularity Generation tech require Temporal Mechanics instead of Gravitonics
    - Made Artificial Black Hole faster but slightly more expensive to produce
    - Converting an asteroid field into an artificial planet removes an Asteroid Processor

- Empires
    - Added separate empire affiliation for peace, distinct from allied
    - Enabled sharing of special visibility between allied empires
    - Added team-based player homeworld placement for multiplayer games
    - Empires are considered defeated if they have no population and no ships, rather than no population and no planets

- Pedia
    - Added stealth and slots to hull descriptions
    - Added articles about planetary focus game concept

- Stringtables
    - Added a few names to lists used for ship names
    - Translation updates: Spanish, French
    - Cleaned up stringtables, removing duplicates from english in other languages
    - Fuel efficiency effects are now described like "-40%" instead of like "60%"
    - Fixed some typos
    - Corrected colony base hull description about slots
    - Fixed descriptions of concentration camp effects

#### Other Bug Fixes

- Crashes or Disconnects
    - Fixed potential crashes and logged errors when executing fleet move orders
    - Fixed crash when passing empty string to set option on command line
    - Added some safety checks to network message handling to prevent some malformed / malicious messages from crashing the server
    - Made dedicated hostless servers not shut down on errors when idling
    - Fixed production and research queue popups causing the client to hang if used after a turn cycle occurs while they are open

- Gameplay
    - Fixed issue where supply propagation occasionally didn't work
    - Fixed some production queue order issues when saving and reloading a game, by using unique internal IDs for all production queue items. There may still be some issues with this, however.
    - Fixed calculation of some broken empire statistics
    - Fixed a bug where ships could be ordered to invade but then not do so
    - Fixed bug where a fleet would not block enemy supply propagation when it should have
    - Accounted for production epsilon to avoid rounding errors in producible items list time estimates
    - Fixed fighter hangar capacity being increased if any ship was owned by an empire with the appropriate tech, rather than just the ship's owner having it
    - Fixed issue with ship designs being added with conflicting IDs when loading a game
    - Fixed ships not getting the Interstellar Logistics speed bonus on the turn they were produced
    - Fixed potentially receiving multiple sitreps from a single Artificial Black Hole creation event
    - Fixed a scripting error within the Distortion Focus effect
    - Fixed issue where bombardment orders wouldn't work if a ship had no other weapons
    - Fixed issue with Sly refueling at gas giants or elsewhere
    - Fixed Gaia special to prevent it from cycling through planet environment types when occupied by Sly
    - Fixed bug in determining best environment type of a planet

- Fixed some broken pedia links in stringtable entries
- Fixed the sitrep "Last" button when the most-recent turn doesn't have any visible sitreps
- Fixed parsed content checksum for ShipDesigns, which was looking up the design name in the stringtable
- Fixed issue with options being reset / lost between executions within an app bundle
- Fixed chat messages marked with the colour of the empire with the same ID as a player's ID, rather than the colour of the empire the player controls
- Excluded galaxy generation info about spawn rate and limits of specials from checksums


#### Technical / Internal

- Dependencies and Versions
    - Updated to support and require Python 3.5+
    - Raised C++ language version to C++14
    - Raised minimum required version to build with CMake to 3.5
    - Moved to compiling with Visual Studio 2017 on Windows
    - Removed GLU library dependency
    - Merged SDLGUI library into FreeOrion client

- Scripting
    - Optimized some condition internal evaluation and scripted implementation
    - Optimized evaluation of effectsgroup scopes
    - Optimized some effect execution
    - Reworked hull and fuel regeneration part effects

- Python AI
    - Scripted AIs to scrap Gateways to the Void that they capture
    - Reprioritized fuel tech research
    - Reworked logic for responding to alliance requests
    - Exposed option to append to a fleet route in the API
    - Exposed productionLocation functions on ship parts and hulls to check if they can be produced at a location
    - Wrapped calls to Python functions with error handlers that log exceptions
    - Cached some results from calls to the API to improve performance
    - Various other internal performance improvements
    - Exposed empire stat records
    - Adjusted Colonization AI to devalue the Gaia special as Sly derive benefit from it

- FreeOrion Content Script Interface (FOCS)
    - Added UsedInDesignID ValueRef to access the ID of the ship design in which a part or hull is used
    - Added arrived-from system property value ref for fleets
    - Added ShipDesignCost ComplexValueRef
    - Made some ValueRefs return default values rather than throwing errors when invalid properties are evaluated
    - Added missing empire ID handling in several conditions
    - Replaced OwnerTradeStockpile with EmpireStockpile (that refers to industry stockpiled)
    - Added several falgs to hulls to prevent automatic generation of effects from the a hull's stats: NoDefaultSpeedEffect, NoDefaultFuelEffect, NoDefaultStealthEffect, and NoDefaultStructureEffect
    - Non-monster hulls now use NoDefaultFuelEffect to provide fuel efficiency
    - Added FieldType bound variable ValueRef to access what type a field is
    - Added CombatBout ValueRef to access the current round of combat
    - Added ValueRef access to the last turn a planet was colonized
    - Added ContainerID property
    - Added OnPlanet condition, which is faster than the equivalent composition of other conditions

- Enabled (non-portable) binary serialization for dispatching combat logs when client and server versions are consistent
- Provided FreeDesktop.org AppStream metainfo file
- Fixed a compile error in GCC 5.4.0


[v0.4.9] - 2020-02-02
---------------------

### Key Changes

- Multiplayer enhancements
    - Hostless servers can be started without connected players and left running for players to join to play their turns asynchronously
    - Empires in a game can be restricted to certain players based on username-password authentication
    - Known / previously joined servers are saved and appear in the servers list of the connection window
    - Chat is shared between the lobby and in-game
    - Chat history persists and is sent to players that connect to a server
    - The chat / message window flashes when a message is received
    - The lobby shows galaxy setup data for a loaded save or the game being played on the server
    - The lobby shows empires without assigned players as separate rows when loading a save or for the game being played on the server
    - Servers can have fixed game rule settings
    - Added an optional server turn timer

- Combat targeting
    - Flak targets only fighters
    - Interceptors preferentially target bombers, then any fighters, then ships
    - Bombers preferentially target ships, then fighters
    - Species can also affect targeting

- Ship hulls have fuel efficiency, which scales the additional fuel from fuel parts and other fuel-adding content

### Detailed Changelog

#### Graphics / Interface

- Chat box text commands:
    - /pm to send a private message to another player. Private messages from other players are flagged with (Whispers)
    - /help to give a list of commands

- Multiplayer
    - Lobby automatically adds AIs and assigns them to AI empires when loading a game
    - Eliminated empires can't have players assigned in lobby
    - Added player left game and player entered chat notifications
    - Observers and moderators are informed when joining about empires' readiness
    - Added icon to empires list to indicate an incoming diplomatic message
    - New icons for empire diplomatic status (War, Peace, Alliance)
    - Prevented selection of eliminated empires in lobby
    - Lobby galaxy setup settings use common client defaults (shared with single player)

- Galaxy Map
    - Added option to scale size of background starfield stars
    - New graphics for ion storm, star forming nebulae, accretion disk, and molecular cloud
    - Added tooltips to the turn button to explain turn processing event order

- Fleets
    - Disabled issuing fleet move orders after orders have been sent to the server in multiplayer games (unless orders are revised)
    - Made fleets of damaged ships split from an existing fleet have the same aggression as the existing fleet instead of always being passive
    - Added right-click popup menus for Fleets window meter icons
    - Fleets window or sidepanel move to the render order top when a fleet or system is shown, even if the shown fleet or system didn't change

- Pedia
    - Underlined modified rules for the current game on the pedia page listing game rules
    - Reorganized contents of species pedia pages
    - Hull descriptions list the tags applied to a hull

- Objects List
    - Added columns for environment rating for species on planets
    - Added total weapon damage column

- Galaxy setup
    - Made pressing enter not act as clicking OK in Galaxy Setup to avoid conflict with Alt-Enter to switch to/from fullscreen
    - Split multiplayer-related game rules into a separate tab

- Misc
    - Added popup commands to add ships or buildings to the top of the production queue
    - Enabled more flexible renaming of designs in Design window
    - Added new art for many species
    - Reduced minimum FPS limit to 0.1 when the game window is unfocused
    - Added a confirmation popup to close the program when OS window is closed, including indication if multiplayer game orders haven't been sent
    - Indicated species with which a planet would be colonized and for which habitability and population info in being displayed on sidepanel
    - Rearranged the display order of sitreps
    - Added gaseous metabolism to the census window
    - Various GUI layout tweaks
    - Added warning when stockpile contributions would put the stockpile above ten turns of maximum-rate extraction


#### Content / Gameplay

- Translation updates: French, German, Russian
- Various pedia article additions, updates, and corrections
- Added sitreps for weapon upgrades and detection upgrades
- Display "Unknown Design" rather than an error when lookup up an unknown design
- Players are informed of diplomatic status changes between empires
- Added a server option to enable/disable informing players about empire statistics
- Advanced shipyards can be enqueued if their prerequisites are enqueued, even if the prerequisites aren't yet produced
- Added the Fulver species
- Added game rules:
    - Allow or restrict diplomacy between players
    - Aggressive fleets are always visible at the start of combat
    - Enable a part-based upkeep calculation rather than ship-based
    - Empires are only told that another empire has researched a tech when both empires have researched it - currently may break client-side meter estimate calculations


#### Balance

- Increased colony base hull cost and added internal slots to mitigate "comsat" defense strategies
- Made colony base hull cost scale like other hulls with fleet upkeep and ship hull cost multiplier rule
- Adjusted hull fuel levels and numbers of slots for various hulls after adding hull fuel efficiency
- Increased spacing between systems and starlanes generated by the Starlane Nexus
- Conceding from a game destroys all an empire's buildings 
- Made Scylior BAD_SUPPLY and BAD_FUEL
- Made Chato bad pilots (BAD_WEAPONS) and bad population
- Planets with dying population no longer have their production and research zeroed, avoiding an issue where temporary disconnection from a growth special would cause sudden loss of resource output
- Added some additional tie-breaking checks when deciding what empire can supply in a system
- Stockpile focus is allowed on all colonies, but growth focus gives no stockpiling bonus
- Stockpile focus on a homeworld gives an extra bonus
- Combat
    - Launching fighters uncloaks ships
    - Firing weapons uncloaks ships to all empires in combat, not just the owner of the target being fired upon
    - Planets have an effective minimum infrastructure of 1 to trigger combat each turn to ensure there are no turns when invading isn't allowed due to cycling of meter growth and combat occurring
    - Interceptors launch at double the rate of other fighters


#### AI

- Improved AI judgement about fleet strengths and ship capacities with fighters
- Improved AI estimation of a standard enemy if no intel is available
- Improved AI assessment of enemy planet threat with respect to shield regeneration
- Made AI aware of fuel tech effects on fuel parts
- Raised AI minimum fuel requirements for warship designs to 2
- Fixed issues with AI merging and splitting fleets
- Added a fleet mission to protect a region
- Fixes issues with ships and fleets that are idle or after finishing missions
- Fixed AI divide by zero error when there was enemy supply in a system where the AI has a planet
- Fixed AI getting confused about whether a produced enqueue order was successful
- Made AI aware of stealth effects of Spatial Flux Bubble Hull
- Made AI more aggressive and less inclined to used doomstacks that it never moves
- Fixed AI picking ships that didn't have enough fuel to do a mission
- Reduced AI invasion overkill / waste
- Made AI consider lost fleets when determining unknown threats
- Made AI scrap Gateway to the Void building on planets it captures, if the planet has one. The AI can't handle that building and it's presence severely disrupted the AI in such cases.


#### Bugs

- Fixed potential crash in password entry box
- Fixed log level filters briefly not applying while being set
- Fixed laggy UI updates when resizing the sitreps window
- Fixed potential crash when setting fleet routes
- Fixed issue where generated filename timestamps could include invalid filename characters on systems with some languages
- Fixed issue where being in an alliance wasn't considered as being at peace for some conditions where it should
- Fixed issues with gifting objects between empires, particularly when allied
- Reordered timing of newly researched tech effects being active so that sitreps about tech research are shown on the same turn that techs take effect
- Fixed minor rendering quirks
- Fixed issue where fleets moving through a system but getting blockaded would lose track of their desired exit starlane
- Fixed text autocomplete in chat by pressing (default) TAB key
- Fixed issue where attempting to drag a window causing resizing instead
- Fixed crash when closing the application window while in a game with the Fleets window open
- Fixed issue where revising orders on a multiplayer server would result in previously-issued orders being lost
- Fixed issue where arriving enemy fleet sitreps were inconsistent if an effect modified the fleet's destination on the same turn as it was expected to arrive
- Fixed issue where arriving enemy fleet sitreps weren't being shown due to limits on how far the fleet could be from the system
- Fixed Interspecies Academy modifying current instead of target research meter
- Fixed some potential internal crashes
- Fixed issue where paired/active meters were reset to the value of the corresponding target/max meter
- Fixed issue where empire colours in a saved game could conflict with player colours in the multiplayer lobby
- Fixed server crash when receiving turn orders from incompatible client
- Fixed issue where shield regeneration could apply twice
- Fixed issue where monster shields wouldn't regenerate
- Fixed crash when editing long strings in a narrow window
- Fixed issue where certain ship part meter effects could not work
- Fixed issue where scrap orders couldn't be issued
- Made unowned fleets move before empire fleets, which avoids some issues with blockades
- Fixed issue with blockade determination so that blockaded planets are still connected to themselves
- Fixed issue with fleets that had never moved being unable to enforce blockades
- Fixed issue where loaded games weren't setting the server rules to those in the save
- Resolved some issues with premade ship designs being added (or not added) at start of game
- Fixed potential issues with invalid UTF8 strings when doing multi-line text layout
- Fixed issue with incorrect refueling calculations when determining move paths for fleets
- Fixed issue with combat log scrollbars
- Fixed error when observer or moderator clients joined a test server
- Fixed undefined behaviour when reporting a FramebufferFailedException when initializing OpenGL
- Fixed issue where, when gifting a planet, the recipient empire did not get production items that are currently produced at that planet transferred to its production queue
- Fixed issue where, after playing a game with modified rules, the default setting for a new game and the rule values in the intro screen pedia would be those of the previously-played game, rather than the defined rule default values
- Fixed issue where, when a certain system (system 0) was destroyed (e.g. by a Black Hole Collapser), an empire aware of this destruction would forget all starlane connections and loose all supply propagation between it's systems


### Technical / Internal

- Added basic support for a UDP message interface to query the server containing simple FOCS expressions
- Improved compatibility with various Boost library versions
- Use asynchronous messaging for all communication between client and server processes, not just from clients to server
- Added safeguards on incoming message size before allocating memory to store it, to prevent denial of service attacks
- Various extensions / additions to the Python APIs
- Adjusted what ships are considered armed: direct weapons with > 0 damage, or fighters present that can be launched and that have > 0 damage, and not directly dependent on the ship design, which doesn't account for some weapon-modifying effects
- Ship designs must have fighter bays and hangars to be considered as having fighters, rather than needing just one of those part types
- Capped the per-message number of combat log entries sent to clients to avoid sending an overly large single message
- Reordered some save game data to avoid breaking a save file (as much) if the UI data is corrupted
- Fixed issue where obsolete ship designs were being stored and serialized in C++11 containers that weren't handled correctly by the serialization library
- Added an option to set a game unique ID number, which is useful for multiplayer servers
- Python errors are output to logging instead of stderr
- Hostless servers autosave when shutting down
- Added an OrderedAlternativesOf condition that will use the first condition in its list of subconditions that matches something
- Added SVG rendering in pedia, and allowed SVG images to be included in articles
- Reworked timing of clients sending save state info to avoid potential save corruption
- A warning popup should be shown if the client's OpenGL version is < 2.0
- Added mechanism for chat messages to have automatic string formatting and stringtable substitutions when displayed
- Added an option to enable/disable network binary serialization
- Screensaver is no longer enabled/disabled in response to window minimize, maximize, or restore events
- Prevent AIs from replaying turns after loading a game, unless specified in configuration


[v0.4.8] - 2018-08-23
---------------------

### Key Changes

- Imperial stockpile
    - Unallocated production is stored, and can be used on colonies even if they are are disconnected from the empire supply network
    - Withdrawal limits are empire wide, based on planet stockpile meters and indicated on map and production screen
    - Techs, species, and planet focus settings affect stockpiling withdrawal limits
    - Stockpiling-focused species added: the Sly

- Game rules options which can be modified or enabled to affect balance, content, and game mechanics

- Unallocated research points are automatically allocated to the cheapest available tech

- UI Improvements
    - Fleet icon representation on the map
    - Blockade indicators
    - Ship designs, hulls, and parts can be edited, saved, obsoleted, and reordered

- Multiplayer improvements
    - Server run in hostless mode, with server-determined rules
    - Password-based player authentication
    - Observers and moderators can join ongoing games
    - Option to concede a game, subject to conditions
    - Allied victory rules

- Extensive AI improvements


### Detailed Changelog

#### Graphics / Interface

- Galaxy Map
    - Improved handling of overlapping fleets and fleet buttons.
    - Added right-click popup command to dismiss sensor ghosts.
    - Added option to ignore hostile ships while auto-exploring.
    - Added ship design breakdown info to fleets indicator at top of screen.
    - Made pressing escape close open windows in last-opened first-closed order.
    - Added system shield, defense, troops, and supply summary indicators to top of system sidepanel.
    - Let fleets window track moving fleets together.
    - Added an indicator that a fleet is blockaded.

- Research
    - Items can be deleted from the research queue with ctrl + left click.
    - Added right click popup commands to research list view rows.
    - Added or adjusted tech list view colour highlighting, rounded borders, icon sizes.
    - Added research list tooltips.
    - List view scrollbar position should retain its position better.

- Production
    - Fixed erroneous early projected completion times for production queue items with multiple repetitions.
    - Added commands to split or duplicate production items on the queue.
    - If the selected system is a production item's rally location, this is indicated on the item in the queue.

- Design screen
    - Ship designs can be reordered, which will affect their ordering on the production screen.
    - Removed dialog for saving ship designs; a right click popup command is used instead.
    - Added right-click popup commands to obsolete parts, hulls, or ship designs, or to delete designs.
    - Added buttons to toggle part / hull obsolete filters.
    - Indicated part / design availability and obsolescence with desaturated borders in lists.
    - Added prompt to toggle availability filters when the hulls list is empty.
    - Added prompt to saved designs to suggest adding some.
    - Fixed bug where replacing a design twice started added designs.
    - Allowed monster designs to be viewed, edited, and saved.
    - When dragging a part over a slot, a part already in the slot should be hidden unless the part in the slot is being dragged.
    - Ctrl-Dragged parts will remove or replace all parts of the same type.
    - Removed restriction on setting blank design descriptions.

- Multiplayer
    - Multiplayer lobby remembers previously-used settings when returning to the lobby after a game.
    - Added option to limit manipulation of settings in multiplayer lobby to the host.
    - Added server player name / passwords list, which can require players to password authenticate to join a server.
        - Players may be assigned roles, which control what they can do in the lobby.
    - Added optional timestamps to chat in multiplayer lobby and in-game chat.
    - Ready button in multiplayer lobby will lock if not enough players.
    - Added choice of client type in the network connection window.
    - Disambiguated "Player" connection type from "Human", which is also an in-game species.
    - Allowed joining an game being played as a moderator or observer.
    - Split Resign button into a Concede button and a Resign button.
       - Conceding from a multiplayer game, when conditions allow it, eliminates the empires' assets from the universe.
    - Hid load button in multiplayer games.

- Misc
    - Improved objects list filter dialog layout.
    - Added granular log detail-level options for specific game systems and clients / sever
    - Made some droplists not respond to mousewheel events when not open (dropped), so that they won't be unintentionally manipulated.
    - Added options for whether to auto-add default or saved designs to the player's empire.
    - Game will auto-save when quitting or resigning. This can be aborted during the save by the user.
    - Added options to enable autosaves at start and end of turns.
    - Improved thoroughness of effect accounting information.
    - Added continue button (and command line switch) to load the most recent saved game.
    - Continue and load buttons only appear if suitable save files are detected.
    - Added button on options screen to save a persistent config file, which will remain after version updates, and will override any subsequent option adjustments when restarting program.
    - Don't render mouse cursor if game window doesn't have OS focus.
    - The pedia and sitreps can now include links to files in the OS file manager or websites.
    - Replaced "planet bombarded" sitreps with "planet attacked" to disambiguation between bombardment and a planet being in a combat.
    - Made chat history longer.
    - Prevented changing the resource directory during a running game.
    - Made resource directory changes when not running a game cause the content to be re-parsed.
    - Various additional optimizations to GUI layout code, particularly when resizing windows.
    - Attempt to determine the system language when initializing stringtable option for the first time.
    - Added random species option to species droplists when setting up games.
    - Clicking the random seed button on the galaxy setup screen now sets the seed to "Random" which will be replaced by the server with a randomly-generated seed.
    - Various values in GUI should now round to nearest rather than next lower value for their displayed precision.
    - Added lists of available parts, hulls, and buildings to empire pedia articles.
    - Sorted researched techs list on empire pedia article by researched turn.
    - Made several GUI widgets react immediately to being pressed, instead of waiting until the mouse is released to complete the "click".


#### Content / Gameplay

- New pedia articles, article categorization, extended articles, corrections
- Translation updates: French, Russian
- Added dimensional matrix engine part.
- Made additional species playable.
- Added Replicon species.
- Tweaked various default ship designs.
- Replaced multiple fuel parts with upgrades to a single part.
- Added Nest Eradicator building.
- Added support for pedia articles for planets having different landscape images per-planet.
- Added population statistic.
- Added a sitrep when a ship naturally goes from 0.x to 1.x fuel.
- Added the Flux Bubble hull.
- Make the default diplomatic status be war rather than peace.
- Game rules:
    - Enable random reseeding
    - Stockpile import limits
    - Ship, buildings, techs are 1 PP/RP and 1 turn to produce / research
    - Number of rounds of combat
    - Enable Experimentors
    - Enable super test takeover building
    - Enable conceding and max allowed colonies when conceding from a multiplayer game
    - Restrictions on allied victories
    - Scale ship speed, ship structure, building cost, tech cost, hull cost, part cost
    - Planet size balance adjustments
    - How much to over-allocate on production queue to prevent rounding-related extra turns to complete items


#### Balance

- Super testers now have perfect stealth, which allows testing without AI interference.
- Restricted engine, detector, stealth, and shield parts to one per ship.
- Adjusted various species' balance.
- Made scrying sphere set visibility to the greater of current and partial visibility.
- Made telepathic detection set visibility to the greater of current and basic visibility.
- Reworked resupply / upgrading of ship parts, based on the last turn a ship was resupplied and the turn a tech was researched.
    - Upgrades now don't happen during the pre-combat movement phase of turns.
- Reduced cost of Transcendent Design tech.
- Required gifted fleets to be stationary.
- Gas Giant Generator now gives a smaller bonus if the planet is populated.
- Reduced Space Elevator bonus for Gas Giants by 1.
- Increased Colony Base hull cost to 3.
- Newly created ships and monsters now cannot block supply (until they have survived a turn) or enemy fleet movement.
- Do not remove production/research on dying planets while there is still remaining population.


#### AI

- Made AI consider cancelling colony buildings if a better species becomes available.
- Improved scout dispatch, to nearby instead of nearest to capital.
- Improved fleet management during invasions.
- Improved estimation of troop requirements for invasions.
- Made AI consider planet defenses more significant when bolstered by fleets.
- Added limit to troops in AI ship designs, to avoid waste.
- Let AI grow larger (by adjusting limit on number of colonies it will attempt to create).
- Made AI avoid blockaded starlanes when moving unarmed fleets.
- Adjusted AI tech priorities.
- Made AI consider refueling when calculating fleet routes.
- Improved AI interaction with stealthed planets.


#### Bugs

- Ancient guardians self-destruction only triggers on planets, rather than all possible objects with species.
- Reduced incidence of the mouse cursor or starlanes disappearing.
- Fixed issue where allied visibility of ship didn't include knowledge of the ship design.
- Fixed issue preventing multiplayer games from starting with moderators or observers.
- Made phototropic effects evaluate after target population setting effects, on which they depend.
- Fixed issue with negative target population being made closer to zero rather than more negative by effects intended to reduce target population.
- Fixed various potential crashes while exiting the client.
- Fixed various GUI memory leaks.
- Fixed crash if program was closed while a popup window was open on the map.
- Fixed crash if right-clicking slots while dragging a ship part on the design screen.
- Fixed issue with visibility-related supply obstructions lasting a turn longer than they should.
- Fixed issues with manual saves while an autosave was ongoing.
- Fixed issue where autocycling turns would cause an save-in-progress popup to never go away.
- Fixed issue with pedia links not highlighting when moused over.
- Fixed issue when an AI lost all its planets but still had a military fleet, its fleets could not be reassigned.
- Fixed issue with AI supply calculations.
- Fixed possible client hang when requesting previews of save files.
- Fixed issue with filenames with multi-byte characters being shown corrupted on Windows.
- Fixed various other AI bugs and incorrect or outdated calculations.
- Fixed memory leaks in content script parsers.
- Fixed issues with fleet move paths when fleet ownership is changed.
- Fixed issue with AI not setting research focus on planets without industry focus available.
- Fixed various issues with the movement blockading mechanic of fleets, space monsters, etc.
    - Fixed issue when a blockaded fleet became not blockaded and its route became invalid.
    - Fixed issue where fleets which had never been moved since their creation were unable to enforce blockades.
- Fixed potential crash when updating meters on many objects.
- Fixed erroneous / unnecessary cases of "Unknown" contributions to meter values in accounting tooltips.
- Fixed issue where troop strength shown before a ground combat was inconsistent with what was actually used.
- Fixed issue where ship designs could only be redesigned on the turn they were created.
- Fixed issue where a planet couldn't be colonized on the first turn due to a Tidal Lock special.
- Fixed issues with AIs replaying their turns when loading a game.
- Fixed combat log layout / scrollbar issues.
- Fixed issues with compressed XML saves containing invalid XML.
- Use correct auto aggression icon for new fleet aggression button tooltip.
- Fixed missing combat sensor ghosts.
- Fixed issues with visibility of stale objects.


#### Technical / Internal

- OSX version is now built as 64 bit binary.
- Improved unit testing infrastructure.
- Removed synchronous messaging; all client-server messaging now uses asynchronous message-response. This should resolve some client hang and lag issues.
- Changed logging format to show Year-Month-Day only on the initial line.
- If parsing a ship design fails, just fail that design, not the whole directory.
- Added UUIDs to ship designs.
- Added FOCS conditions: binary and ternary comparison operators between arbitrary values, which match all objects if true, or none otherwise.
- Added FOCS value refs: binary comparison operators between arbitrary values, which return the equivalent of 1 if true, or 0 otherwise.
- Added FOCS value refs: ternary and quaternary expressions, like binary comparison, but with a value to return if true, or values for both true or false.
- Various FOCS values exposed: SpecialCapacity, SpecialAddedOnTurn, EmpireObjectVisibility, TurnTechResearched, LastTurnConquered, LastTurnAttackedByShip
- Exposed FOCS value: HabitableSize (which replaces SizeAsDouble or SizeAsInt)
- Parallelized and deferred content parsing, reducing program startup lag time.
- Improved error logging when parsing an invalid ship design.
- Added checksums to parsed content, so that client and server content can be checked for consistency.
- Universe object ID generation no longer requires clients to request a new ID from the server for each new object.
- Don't autosave immediately after loading a game.
- Don't autosave at end of host's turn in multiplayer, as other players may not have finished their turns yet.
- Rework Python logging to use standard interface, and to log all errors, not just those manually logged.
- For Hulls, parts, and buildings, non-trivial cost and production time can now be evaluated in more cases when a source or target object aren't or can't be specified, such as in the pedia rather than on the production screen where the empire and production location are known.
- Improved robustness of networking when errors happen, hopefully preventing some hung AI processes.
- Added CMake option to build server only (without requiring client-specific dependencies).
- Reworked mechanics of effect-set visibility.
- Reorganized various content scripts into separate files instead of having many definitions in one file.
- Prevented server from closing when an unestablished client drops its connection.
- Made human client check for an existing server before launching another.
- Made AI client respond to command line arguments.
- Made server have a dedicated saves folder, outside of which clients cannot query for directory contents.
- Standardized names of options on command line.
- Added categorized listing of options on command line with --help option, instead of a single list of all.
- Implemented custom AI state string encoder, which should be safer to load.
- Improved OpenBSD support.
- Various meter modifications have been moved out of the engine and into scripted effects.
- Added FOCS access to immediate meter values, in addition to "initial" values after the last meter update.
- Converted contents of global_settings.txt into game rules.
- Added an ID to each game, which can be viewed in the pedia.


[v0.4.7.1] - 2017-09-03
-----------------------

#### Bug Fixes

- Production queue: reverted behavior when increasing batch size of a build item back to a proportional reduction of the progress already made, instead of resetting progress to 0.
- Fixed bug in production queue projections.
- Several fixes to AI calculations regarding population/colonization.
- Fixed potential issues for phototrophic species.
- Fixed issues with visibility after combat resolution.
- Fixed bug which caused starlanes not to be rendered on certain systems/setups.


[v0.4.7] - 2017-04-24
---------------------

### Key Changes

- Fighters
    - Launch during combat and attack on subsequent rounds
    - Ignore shields when attacking
    - Cannot attack planets and are not attacked by planets
    - Destroyed by any attack by other fighters or ships
    - Flak weapon shoots multiple times per round and can destroy fighters, but does minimal damage to ships

- Alliance Supply
    - Empires may propose and agree to an alliance with each other
    - Allied empires connect their supply networks through each others' supply connections
    - Allied empires resupply ships within each others' supplied systems
    - Allied empires share visibility information

- Fractional Production Progress
    - Production item progress is now stored as a fraction of completion, rather than amount of PP accumulated.
    - Changes in the production cost now can't instantly complete something that has had its production cost reduced.

- Beginner Bonus
    - When the max AI aggression is set to Beginner Mode, the human player gets bonuses to ship shields, planet troop garrisons, and resource output.

- AI Improvements
- Many new pedia articles, categorization, and improved search
- Production items on the queue may be paused and resumed
- Ancient Guardians may appear as a defensive ground force on planets with specials, instead of Sentries that attack any ship in the system


### Detailed Changelog

#### Graphics / GUI

- General
    - Reworked window layout updates and rendering timing to improve GUI responsiveness.
    - Made some options window GUI widgets resize with window.
    - Enable screensaver when minimizing the game and disable when maximizing.
    - Made some tooltips use the set browse time option value, rather than a default other time
    - Made tooltip rendering more consistent in style.
    - Added turn sound option.
    - Made droplists close when Esc is pressed and respond better to mouse actions.
    - Added right-click context menus for various icons in the GUI, such as meters in the planet panels.
    - Made mousewheel not scroll drop lists when the pointer is outside the list.
    - Made droplists scroll to keep the selected row visible when navigating with the keyboard.
    - Made numpad keys work with droplist navigation.
    - Made current item visible when drop down list opens.
    - Fixed formatting and layout in the save file dialog.
    - Changed default extended tooltip delay to 3.5 s.
    - Added a tooltip to fleet summary icon at top of screen.
    - Added option to show IDs after object names in GUI.
    - Made mousewheel manipulate credits scrolling.
    - Fixed droplists falling outside the application window.
    - Made some scrollable GUI widgets respond to keypresses to scroll their contents.
    - Made ListBox header show for empty list boxes.
    - Paused music when the application loses focus.
    - Closed credits if the app is resized.
    - Update FPS immediately when the option changes.
    - Stopped save file dialog from reloading saves after window focus changes.
    - Added dialog to ask user about waiting for savegame to complete before quitting.

- Pedia
    - Added access to pedia from intro screen.
    - Included more pedia articles in search results
    - Displayed identical parts as one entry in ship browse windows.
    - Made Enter/Return KeyPress events on EncyclopediaDetailPanel set the focus to the search edit.
    - Adjusted planet suitability report for tech-gated species.

- Map Window
    - Changed Fleets window quick close option from right click to left click
    - Made ordering of systems in cycling commands alphabetical.
    - Changed colours used to render some scanlines (indicating a not-visible object).
    - Changed display of map scale distance to integers.
    - Added an inner system circle on the galaxy map, which is drawn for systems with colonies.

- Sidepanel
    - Added automatic selection of ships for bombarding.
    - Added ability to rename systems in SidePanel. System sorting considers the system's id to disambiguate if necessary.
    - Displayed source planet in growth special accounting labels.
    - Added indication of defending troops to invade button.
    - In sidepanel, displayed tooltip not only for the selected focus, but also for available focus settings in the focus dropdown list.

- Fleet Window
    - Added greater variety and more flexible indicator icons to fleet and ship panels.
    - Labeled fleets window as "Near System X" for fleets in transit.
    - Showed empire name in fleets window for fleets in transit.
    - Added right-click menu command to split from a fleet ships that have less than full fighter complement.

- SitRep Window / Combat Log
    - Allowed SitRep right-click menu from majority of sitrep area.
    - Added Copy to sitrep right-click context menu.
    - Added link to help article on sitrep entry right click menu.
    - Added right click context to hide/show types of SitReps.
    - Changed combat log to use empire color to designate destroyed objects.
    - Added SitRep navigation for meter type pedia links.
    - Fixed combat log window layout issues.

- Objects Window
    - Added numerous new column options in the Objects window, and reorganized the options.
    - Reapportioned horizontal space in filter dialog parameter dropdown to improve legibility.

- Design Window
    - Made ctrl + left click in the design window obsolete designs.
    - Fixed hull list display issue.

- Research Window
    - Added right-click pause and resume commands to research queue in GUI.
    - Added pause and resume research functions to Empire, and getters to check if a tech / queue id is paused to research queue.
    - Made tech list columns sortable.
    - Display preferences for complete, researchable, unresearchable and partially unlocked techs will now persist between games

- Production Window
    - Fixed double click on production queue item so that it completes the map window system selection.
    - Fixed planet selection when the production window is open so that the production window info panel gets updated when another planet is selected.

- Game setup / Multiplayer Lobby
    - Reworked layout in multiplayer lobby window.
    - Multiplayer game starts when all human players are ready.
    - Used human player's name as default empire's name in multiplayer.
    - Added option for random species selection during game setup.
    - Prevented multiple players from having the same name in a multiplayer game by adding numeric postfixes.
    - Added new galaxy type images.


#### Content

- Translation updates: French
- Various AI tweaks: colonization, fighters, fleet movement
- Various stringtable tweaks, corrections, and updates.
- New encyclopedia articles about gameplay mechanics, the interface.
- Many encyclopedia articles have been categorized into lists of similar articles with a category summary article.
- New or updated icons / graphics for various content.
- Added Kraken in the Ice special.
- Added the Automated History Analyser building.
- Added Ancient Guardians as alternative to Sentries for Specials.
- Added the native species Lembala'Lam.
- Added mention of star type restriction to energy shipyard descriptions.


#### Balance

- Added stealth effects to Spatial Flux hull and increased build cost.
- Reset to 0 a planet's detection upon conquering.
- Adjusted supply propagation by adding tie-breaking conditions related to (unobstructed) distance to the nearest supply source.
- Removed build cost increase mechanic from Stargate.
- Massively increased cost of the Transformer building.
- Reduced other buildings' production times to equal the Transformer's production time.
- Prevented Ancient Guardians on native homeworlds.
- Restricted growth focus to be available on homeworlds of species that are on the planet, not any species' homeworld.
- Removed frontloading of production (due to changing to fractional production progress mechanic).
- Salted the PRNG seed with galaxy setup seed to hopefully avoid often getting the same random effect results due to using just the turn number as the seed.
- Added option to control whether to re-seed the PRNG repeatedly on the server.
- Tweaked Concentration Camps to set Happiness to zero after all other effects.
- AI invasion priority tweaks.
- Made AI handle species with fixed max population when assigning Colonization values
- Reduced initial number of scouts produced by AIs.
- Enabled AI use of XenoResurrection Lab and resurrected species.
- Adjusted AI to further prioritize automation, exobots, and some growth techs.
- In sparse galaxies, moderated the reduction in AI colonization activity that occurs after spotting enemies.
- Adjusted AI value for asteroids during first 40 turns.
- Made neutronium and asteroid armor parts available only to supply connected shipyards.
- Stopped monsters maturing during combat.
- Made Cultural Archives required for research bonus of Auto History Analyser.
- Added allied diplomatic status between empires. Allies share visibility and supply networks.
- Tweaked AI ship designer logic.
- Adjusted build costs and stats of the Solar hull, raised build costs of Titanic and Scattered Asteroid hulls.
- Lowered Protoplasmic and Symbiotic hull base stealth.
- Have the AI combat planning deem planetary defenses to be more significant when they are bolstered by fleet forces.


#### Bugs

- Fixed Windows installer continuing install before uninstall completes.
- Fixed issues with non-ascii characters in Windows usernames / filenames.
- Made the map distance scale circle track the selected system when the production window is open.
- Fixed some meter change estimates not considering some contributions, such as ship fuel regeneration.
- Fixed clipping of long hull names in design window.
- Fixed issues with the fleets window and the fleets in it moving between turns while the window stays open.
- Fixed some effects which trigger on negative planet target population.
- Fixed direction agreement of mousewheel scrolling in drop down list when dropped or not dropped.
- Fixed issue with production window not tracking non-production window selected system changes.
- Fixed drop down list not tracking main application window resizes, which made them seemingly unclickable.
- Fixed production window queue and list state preservation when updated.
- Fixed issue where a drag-drop on a list could start autoscrolling and never stop if the drop happened while the autoscroll was in progress.
- Fixed issue where rapidly pressing left and right keys while in a listbox could cause a crash.
- Use drydock from highest happiness system planet.
- Made planet stealth techs effect only owned planet not all planets in system.
- Fixed issue with Head on a spike special
- Hopefully prevented some potential crashes when generating random numbers.
- Fixed Hyperspatial Dam supply glitch.
- Fixed issues with unpopulated planets' (outposts') research and industry bonuses.
- Fixed issue where production and research times and costs were incorrect.
- Fixed crash due to multi-meter status bar attempting to render a zero valued meter.
- Fixed monsters blockading and being blockaded not working correctly.
- Fixed some issues when exiting game or closing the client window, such as dangling AI client or server processes which preventing subsequent games from starting properly.
- Fixed players that canceled out of multiplayer games not actually disconnecting from the server.
- Improved handling of AIs taking over empires for human players in loaded multiplayer games.
- Fixed issue with AIs not storing their aggression correctly in save games.
- Fixed AI trying to enqueue unresearchable techs.
- Fix for client crash / access violation upon receiving fatal error message from server.
- Fixed bug which prevented construction of colony buildings for tech gated species when a player doesn't have the required tech, but owns a colony of the species.
- Fixed unowned mines not destroying ships after reducing their structure to 0.
- Fixed numbers in planet suitability report being wrong on the turn right after a planet has been depopulated.
- Fixed issue with the Starlane Nexus building where lanes created together can be closer in angle than allowed, causing problems with later lane creations.
- Fixed bug causing AIs to be crippled by eventually switching an excessively high amount of colonies to research focus.
- Fixed issue in network code which could cause AI client processes to hang.
- Fixed issues with AI troop ship design and production.


#### Technical / Internal

- C++11 is now supported and required to compile.
- Implemented continuous integration / automated builds
- Fixed inconsistent computation of build number.
- Optimized Font layout code.
- Dropped support for MSVC compilers before 2015 version.
- Added separate description strings for binary and xml save files. Previously, the XML text was appearing in readable form in the binary save files.
- Hopefully made binary saving an automatic fallback for when xml serialization fails.
- Implemented sup and sub text formatting tags.
- Optimizations of effect evaluation.
- Optimized universe generation, substantially reducing time for bigger galaxies and many empires setups.
- Made universe generation more robust to platform / operating system differences.
- Added ipv6 support.
- Various network connection monitoring and shutdown tweaks.
- Increased client connection timeout and delays, which should help connectivity on some OSX machines.
- Allow build of program with unknown git commit.


[v0.4.6] - 2016-09-16
---------------------

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


[v0.4.5] - 2015-09-08
---------------------

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


[v0.4.4] - 2014-09-07
---------------------

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


[v0.4.3] - 2013-10-26
---------------------

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


[v0.4.2] - 2013-02-20
---------------------

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
  focused colonies in system with an asteroid belt outposts. Additional asteroid
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


[v0.4.1] - 2012-08-03
---------------------

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


[v0.4] - 2012-02-05
-------------------

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


[v0.3.17] - 2011-09-23
----------------------

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


[v0.3.16] - 2011-06-15
----------------------

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


[v0.3.15] - 2010-08-04
----------------------

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


[v0.3.14] - 2010-04-30
----------------------

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


[v0.3.13] - 2009-05-24
----------------------

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


[v0.3.11] - 2009-01-09
----------------------

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


[v0.3.10] -  2008-01-25
-----------------------

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


[v0.3.9] - ???
--------------

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


[v0.3.8] - ???
--------------

If you're confused about version numbers, pretend the last few "RC" releases
didn't have the RC.  Release naming should be more consistent in future.

This is mostly a bugfix release;  the crash when an AI empire was defeated issue
in particular should be fixed.  Additionally, the capacity to do Python AI
scripting has been significantly increased, should anyone else be interested in
working on that.


[v0.3.1-RC7] - ???
------------------

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


[v0.3.1-RC6] - ???
------------------

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


[v0.3.1-RC5] - ???
------------------

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
