* Fix Manjaro docker image


* prevent nebulae from increasing ship speed


* -replace boost::asio::deadline_timer with boost::asio::system_timer
-use std::chrono::system_clock when available for parsing time into a time_point
-otherwise convert posix_time to time_point via total seconds since epoch

* clarify unit of time


* trivial grooming


* Reflected last changes in en.txt

Translated star names and small bugfixes

* grooming


* try-catch around setting description rich text


* avoid repeated mutex locking by inlining


* -wrap with try-catch around GetTexture call in ImageBlockFactory::CreateFromTag
-avoid duplicate GetTextureManager calls

* Convert default/scripting/fields/FLD_MOLECULAR_CLOUD.focs


* Convert default/scripting/fields/FLD_METEOR_BLIZZARD.focs


* Implement int / ValueRefDouble operation in Pythn FOCS parser


* Add more clear description of Genome Bank (#5383)


* Convert default/scripting/fields/FLD_ION_STORM.focs (#5379)


* Prepare convertion for default/scripting/fields/FLD_ION_STORM.focs

Implement comparisons ValueRefInt >= ValueRefDouble, ValueRefInt <=
ValueRefDouble.
Implement boolean operation ValueRefDouble & Condition.
IsField name argument is optional.
Implement UniverseCentreX, UniverseCentreY, UniverseWidth, RandomNumber

* [focs] Update types

Introduce _Int and _Float
This types represent attribute types and return values.

The difference with int and float is that this Types return _Conditon when used in compare operations

* Use the right key. Good times, the lint found a bug


* make ruff on github happy


* Fix Arc Disruptor 3 tech should +2 damage boost (only +4 for concentrator)

The +3 extra bonus for Arc Disruptor 3 was not intended.

* Add Arc Concentrator part

A heavy cousin of the arc disruptor for the Core slot, better at piercing shields
Fewer shots, higher shot damage.

Forum discussion https://freeorion.org/forum/viewtopic.php?p=122384

* 80PP, close range weapon, 3 shots, 8 base damage; raw damage exactly like arc disruptor against enemy with zero shields
* no pilot boosts
* policy boosts are like Direct Weapons (i.e. less efficient than arc disruptor)
* upgrades with arc disruptor tech
* current upgrades  arc2: +4dam  arc3: +6dam  arc4: +1 shot (same per shot as arc disruptor; in total half as effective)
* intended upgrades arc2: +4dam  arc3: +4dam  arc4: +1 shot (same per shot as arc disruptor; in total half as effective)

* stringtables for part, techs, policies adjusted
* added basic AIDependencies

TODO:
* nerf arc3; only +2dam both for disruptor and concentrator
* AI does not really know how to handle tech upgrades to few-bout weapons like this one

* Introduce Python FOCS parser module with effects and MoveInOrbit effect


* [ci] update python tools (#5375)

* Bump linters
* Replace pre-commit with prek

* Convert default/scripting/fields/FLD_ACCRETION_DISC.focs (#5376)


* Convert test field from old FOCS to Python FOCS parser


* Add test for field parser in the real content


* Add test for field parser in the test content


* Change signature of fields parser


* Remove unneeded assignation


* Drop old FOCS encyclopedia parser


* Convert all the encyclopedia articles to Python FOCS


* [focs] Update typing and suppressions (#5369)

- Add more checks for linting tools
- Replace/remove suppression to match current tools

* Implement Python FOCS for encyclopedia parser


* Add test for encyclopedia articles in category count


* Drop old FOCS support for empire statistics


* Replace pyright with purefly (#5364)

* Replace pyright with pyrefly

* Replace mypy with ty (#5363)

Replace mypy with ty. This checker has a similar philosophy, but ty is much faster.

* logarithmic instead of linear pow loop


* grooming


* store single file-scope constant of backup font Vera TTF bytes


* -pass charset container rather than iterators
-inline GUI GetFont functions
-in GUI::GetFont that takes an existing Font pointer, use the passed in pts size rather than the Font's own
-make GetFontImpl take a data and data_size pointer instead of vector<uint8_t>

* -inline HasFont
-use container rather than iterator template type

* add GetFace overload taking just pointer to file data, not a specific container


* -inline Font constructors
-add static override TextExtent
-conditionally constexpr GetExtent implementation
-test chaining text preprocessing steps

* -make colour picker dialog bigger
-make default width and height accessible as static constexpr class values
-use array instead of vector for fixed size container of custom colours
-default initialize member

* -vector -> array
-constinit

* whitespace grooming


* -return/accept pointers to const Font
-make some Font face/glyph functions static

* -store pointer to const font in TextBoxBrowseInfoWnd
-default initialize flag

* -store pointer to const Font in TextControl
-inline constructors

* remove unused variable and lookup


* -safety check
-grooming

* Fix usage of annexationcondition annexationcost in _species_pyi (#5362)

That are focs.txt parse tokens, in focs.py we use annexation_condition and annexation_cost

Co-authored-by: agrrr3 <agrrr3@users.noreply.github.com>

* Fix COLONY_INDEPENDENCE_DECREE; all colonies but capital may declare independence

happened in the py transition; weird; should have been caught by content checksum tests

* Fix PLC_INDOCTRINATION. Use Turns instead of TurnsSinceLast valrefs as the latter are broken. Also support annexation.

* the code expected TurnsSinceLastConquered to return zero when the planet never was conquered - that was changed in April 2024
* also the TurnsSinceLast... give back the same value for was-never and was-on-setup.

* [tmp] wip


* Test encyclopedia articles parser on test content


* Make encyclopedia articles parser synchronous


* Convert default/scripting/empire_statistics/SHIP_COUNT.focs


* [focs] Convert MILITARY_STRENGTH_STAT (#5349)


* [focs] Clenup type annotations (#5352)

Add missed imports and update annotations

* Convert default/scripting/empire_statistics/STATISTICS_TEST.focs


* Convert default/scripting/empire_statistics/RP_OUTPUT.focs


* Convert default/scripting/empire_statistics/PP_STOCKPILE.focs


* Convert default/scripting/empire_statistics/PP_OUTPUT.focs


* Convert default/scripting/empire_statistics/PLANET_COUNT.focs


* [focs] Update type annotations (#5350)

- Change the return type of statistic functions from _condition to int
- Add missed return type for functions in the _value_refs.pyi
- Change the type of value in EmpireStatistic to int | float to reflect usages.

* Convert default/scripting/empire_statistics/IP_STOCKPILE.focs


* Convert default/scripting/empire_statistics/IP_OUTPUT.focs


* [balance] set default FIGHTER_DAMAGE_FACTOR to 4.0 instead of 6.0

See https://freeorion.org/forum/viewtopic.php?p=122277

* -add build on Windows Server 2025
-with alternate path for makensis

* msbuild@v2


* -bump compiler version strings that get put into Version.cpp
-bump python executable path for currently unused solution configs

* quiet [[nodiscard]] warning


* also add new files to MSVC2026 project files


* Fix MSVS 2022 project


* Convert default/scripting/empire_statistics/Empire.focs


* add x64 build output directory to .gitignore


* add MSVC 2026 project files


* add static_assert test of HandleTags


* -store/return Font::Glyph from DummyGlyphMap instead of an anonymous struct containing just an advance field
-test PreRenderImpl in a static assert of multiline text

* TextControl::GetLineData doesn't need to be virtual


* comment out unused variables


* -add separate StoreGlyphImpl test
-tweak StoreGlyph test

* -noexcept
-pass value / move
-constinit  shared_cache
-reserve space for text regex parser results

* -constexpr StoreGlyph and HandleTag(s)
-constexpr RenderCache stand-in

* trivial grooming


* merge #if defined blocks


* safety check / error output if more widths than code points in AddWhitespace


* remove show font textures button from OptionsWnd


* -store Font textures by name + pts in TextureManager
-limit Font texture width
-indexes to range for
-noexcept

* construct ImageBlock from Texture not path


* add name (not path) lookup for Texture


* get MultiEdit text format rather than default constructing one


* add TextTag::TypeString()


* merge GLBufferBase into GLClientAndServerBufferBase


* comment out unused variables


* fix compile errors with operator[] on parsers with two actions within the [] separated by ,


* -quiet unused variable and function warnings
-rewrite potentially endless decrementing loops with rbegin/rend
-move extra index increment to separate line for clarity
-rework loop index limits and incrementing to quiet warning

* additional static_assert test


* collect and pass TextTag not TextElement


* quiet implicit conversion warnings


* -use some static constexpr string_view instead of const or temporary string
-remove redundant explicit base class init

* add a no-tags CharData constructor and preferably use in loops, just setting tags once or after looping


* Arc disruptor buff - AD4 tech

Split AD3 tech
* getting the damage boost earlier
* at AD4 get another shot (25% total damage increase)

https://freeorion.org/forum/viewtopic.php?p=113703#p113703

checklist content changes
x stringtables additions
x stringtables refactorings
x AI basic integration - updated tech upgrade for ship measurements
o AI support - think no arc tech beyond level 1 gets researched?
x test checksums

* Without domestication let owned nests spawn wild monsters. Add BLD_NEST_RESERVE to reduce such spawns.

change categories
* py content
* additional content

* Convert default/scripting/empire_statistics/COLONIES_COUNT.focs


* fix missed boost::filesystem -> std::filesystem


* CI: Drop Godot 3 from Debian sid i386


* boost::filesystem -> std::filesystem


* Update GPG keys expiration date


* remove / inline SetJustification


* -pass and store Font point size as uint16_t
-don't store Texture point in Glyph

* remove unnecessary repeated RenderText calls for pre-highlight, highlight, and post-highlight text


* -fix tab text rendering quirks, particularly when selecting / highlighting
-consolidate determining spacing of glyphs in SetTextElementWidths

* -fix calculation of glyph width
-use it rather than regetting from SubTexture

* -constexpr implementation for operator~(Flags<FlagType> flags)
-conditionally constexpr for ValidateFormat
-delete operator, for flag types

* account for tabs in SetTextElementWidths


* remove TextAndElementsAssembler::Impl


* predetermine TextElement for FPSIndicator text rather than parsing in SetText


* -remove Font and TextControl ChangeTemplatedText
-use SetText instead

* -structured binding
-separately define lambda for sorting
-grooming

* add tests for AssembleLineData with whitespace


* RequirePreRender need not be virtual


* noexcept


* inlining


* noexcept


* inlining, noexcept


* grooming


* -add a custom RichText block containing a CUIMultiEdit with tags ignored
-override block factory for unformatted text in pedia
-use that for showing stringtable contents instead of the default TextBlock, so as to allow text selection

* grooming


* grooming


* don't pass unused format


* -use lambda
-extract params later

* -add PreformattedTextBlockFactory that creates RichText TextBlock with FORMAT_IGNORETAGS added
-add RichText::SetUnformattedText that creates such a TextBlock

* inline constructor


* avoid possible endless recursion when resizing or setting the max width of a RichText TextBlock


* noexcept


* explicit


* pass font pointer by value


* -noexcept
-remove unused function

* noexcept / fix compile error


* remove unnecessary defaulted destructors


* -remove unnecessary virtual destructor
-remove unused MapKeys function

* add StyleToTextFormat converter function


* rewrite AddWhitespace


* -check for simple case continue before complicated cases
-de-indent

* reserve


* -use std::next instead of +
-grooming

* remove unnecessary explicitly defaulted move constructor


* fix compile error with constexpr string_view construction


* fix init list order warning


* fix compile error


* handle <reset> tags in RichText TagParser


* inline RichTextTag


* move tag counting into RenderState


* use custom inline colour stack


* do HandleTag loop inside helper function


* use vector not stack for tracking RenderState colours


* remove operator== declaration


* forward without casting


* -move CharsToUInt8 into Clr
-add Clr::RGBAClr functions that create a Clr from string_view(s)

* replace Clr constructor taking a string_view with a static factory function HexClr to disambiguate the expected text format


* -remove Width() and cached_width from TextElement
-pass text and widths instead of a TextElenemt to AddTextWordbreak and AddTextNoWordbreak

* separate tag-related info out of TextElement


* only pass necessary info into HandleTag


* -Substring tests
-TextElement tests
-using to deduplicate in TestTextElems helper
-remove unnecessary explicit string_view constructor for Substring == ""

* -Substring::operator==(const char* rhs) const
-noexcept

* push_back desired value instead of 0 and then possibly modifying .back()


* add CharData constructor taking tags as mutable rvalue ref


* -constify parameters
-ensure call order correctness
-comments

* use reference to avoid repeated lookups


* [[nodiscard]] constexpr


* -remove unnecessary defaulted ~EmpireHasAdoptedPolicy
-inline EmpireHasAdoptedPolicy constructors

* use member variables for index storage


* don't use turn related constant in a non-turn-related place


* remove unnecessary low= and high=


* -use persistent storage for vertices in PlanetPanel
-use persistent storage array objects for colours

* -const
-remove redundant reserve that should be done in calling code
-grooming

* make linter happy


* -~Number(low=1 ...  to  Number(high=0 ...
-Not Number low = 2 ...  to Number high = 1 ...

* -remove redundant low=0 in Number conditions
-remove redundant low=1 in Enqueued conditions

* use !EvalAny if checking for 0 subcondition candidate matches in Number::Match, with no lower limit or a lower limit of 0


* more specific compiler version requirement


* grooming


* remove missed glEnd()


* use EvalAny if checking for at least 1 subcondition candidate match in Number::Match but no upper limit


* optional() -> none


* boost::optional -> std::optional


* boost::optional -> std::optional


* make some parameters non-optional since they are always provided


* -declare/define TextElement move/copy/operator= on GCC < 13 in order to not access mutable member in constexpr code to work around compile error
-simplify noexcept specifications / add some static_asserts to verify expectations

* -handle both boost version strings and numbers
-Use C++17 fro GCC with Boost < 1.80. There may be a GCC version > 13 that can compile Boost < 1.80 in C++20 but I don't have a way to test all relevant combinations since distros tend to update both at the same time.

* GCC version and Boost version dependent C++ standard for GG


* tweak return types and avoid issues comparing pair with different internal types


* only do test with indexing past end of string_view on MSVC


* tweak feature test macros and only do test of indexing past end of string on MSVC


* -move Substring::EMPTY_STRING declaration earlier in class definition
-tweak feature test macros and compiler version checks

* fix extra index increment in test


* compiler error workaround


* -don't use mutable member in constexpr evaluation of const function
-add defaulted TextElement copy constructor
-grooming

* add additional fedora version docker build, based on rawhide Dockerfile


* restore debian-oldstable docker build


* drop macos-14 tests


* do weekly builds on macos-15


* add macos-26 builds


* Use the fact that IsBuilding's name parameter is a list (#5327)

Currently, the fact that IsBuilding accepts a list is only used in one place out of over 1000, so I looked where else that might save a few cycles1.
Testing Done (using old saved games)

    Queueing the various Terraforming variants behaves as normal
    Gaia still excludes Terraforming
    Nexus/Bore exclusion works
    The various Artificial Planet versions still exclude each other

* As suggested


* Fix obvious copy mistakes in SHP_TRANSSPACE.focs.py

These two fields should be the same as in the erroneously named [SHP_SINGULATIRY_ENGINE_CORE](https://github.com/freeorion/freeorion/blob/master/default/scripting/techs/ship_parts/speed/SHP_SINGULATIRY_ENGINE_CORE.focs.py) - if not moved into its own file altogether.

* Allemand révisé


* Mention starlanes can be drilled from outposts in pedia


* [focy][py][nocode] fix docstring - for InGame matches non-negative object ids (#5322)

Co-authored-by: agrrr3 <agrrr3@users.noreply.github.com>

* Update NEUTRONIUM_FORGE.focs.py (#5302)

In the late game, you can queue an entire "production pipeline" in one go - except the Neutronium Forge, which has to wait for the first step to be completed. This "fixes" that slight perceived inconsistency. 



Not I didn't try to allow the queueing once the extractor is queued, that wouldn't be so trivially simple...

* [py] Remove unused method from print_utils (#5311)


* pre-commit migrate-config (#5314)


* Queueing several starlane nexii or mixing bores with nexus makes no sense (#5316)


* Pedia: BLD_FIELD_REPELLOR_DESC shows highlander effect (#5317)


* [focs][py] Fix bounds for InGame() as object IDs start at zero


* remove support for preformatted <pre>tags<i>ignored</pre> text, which was apparently unused and which complicated text parsing


* -const
-comment

* inlining


* inlining


* -use scoped_connection to manage connection lifetime
-replace boost::bind with lambda

* -use scoped_connection to manage connection lifetime
-replace boost::bind with lambda
-inlining
-noexcept
-safety checks
-grooming

* -use scoped_connection to manage connection lifetime
-replace boost::bind with lambda

* -use scoped_connection to manage connection lifetime
-replace boost::bind with lambda

* -use scoped_connection to manage connection lifetime
-replace boost::bind with lambda
-make cast explicit

* boost::bind -> lambdas


* boost::bind -> lambdas


* inline


* use separate tag-aware and tag-ignoring regexes and internal state tracking whether tags are being ignored in CompiledRegex


* use mutex / locksto guard tag handler access or modification of custom tags


* indenting


* -put locks around FT_Done_Face and FT_New_Face calls
-use getter instead of global wrapper around FT_Library

* -move static stuff to anonymous namespace
-make tags const and move to anonymous namespace

* remove empty search results before deciding whether to show a class of result based on whether there are any


* -sort and uniquify pedia search results
-tweak padding

* make CompileRegex regex_with_tags uses thread-safe with a mutex.






* [focs][pyi] Cast ID to int to outsmart pyright. Cc NPM delendam esse.


* [focs][py] Skip unstealthiness effect for out-of-game/ShipDesigner ships


* [focs][py] Add InGame() condition. Add FleedID parse support


* Fix truncated log "Special Placement Summary" (#5308)

* Fix truncated log "Special Placement Summary"
* Suggested truncation-proof print of specials table in case many more are added

* A quick spellchecking run (#5310)


* F-string conversion for all report_error calls (#5305)


* [py] Remove one dead code line in empires.py (#5306)


* Just some comment typos I noticed by chance (#5304)

Some of these were hinted at in #5262 - but this PR is comments only, though I globally searched for the same typo over all source types. This "Adtoption" line, however, will have to wait.


* [pedia] Add descriptions for bookkeeping specials LOWER_STEALTH_COUNT_SPECIAL and BASE_STEALTH_SPECIAL


* [pedia] add FLEET_UNSTEALTHINESS game concept, better stealth effect descriptions

* renaming, restructuring, rewording
* lists of main effects to stealth for planets and ships
* fix missing label



* Fix regression - count only ships in current system, not in all systems


* (Blindly) synchronize checksums with content


* Adapt checksum adding specials and changing tech


* Adapt checksum changing monster hull


* Fix EXP_OUTPOST, has GRAV_PULSE in 1st slot


* Fix all JUGGERNAUT, have GRAV_PULSE in 4th slot


* [focs][py][pyi] Refactor fleet unstealthiness using boolean valuerefs

* [focs][pyi] Stealth meters, Stealth condition, fix TargetIndustry sig

Add special LOWER_STEALTH_COUNT_SPECIAL
Add special BASE_STEALTH_SPECIAL




* -fix ships being moved to a new system but not having a new fleet generated for them due to then already being at the target object location
-add ships before naming new fleet
-move get_fleet and get_planet to anonymous namespace




* [focs][py] Add NoOpValue e.g. NoOpValue(float, 1.1)

currently supported
* NoOpValue(float, 1.1)
* NoOpValue(int, 1)
* NoOpValue(str, 'strii')


* [focs][py] NoOpCondition and NoOpEffect
* expose Condition::NoOp as NoOpLog



* [focs] Linear fleet unstealthiness (no AI)




* Let ShipDesignAI considers flak, estimate distractions for multi-target-class weapons
in case there is no shield:
* combat rating applies flak factor structure extension depending linear on the ratio of enemy damage of direct weapons and fighters
* this is the most complex/sophisticated part and it may also be the correct one for the general case - needs review

* AI design add multi target class heuristics depending on target type
** it considers flexibility of multi--target-class weapons as a good thing
** but tries to estimate the effect of being distracted for multi--target-class weapon types

* AI war design considers using weapons contributing to _flak_shots
** add flak factor combination (this extends the usage of structure in case of armed enemy fighters)
** combine flak and shield factor calculations
* tweak standard enemy for design; do expect the fighter inquisition
* AI uses flak_shots to increase structural integrity value in case of an enemy with fighters

* Refactor: apply fighter survival_rate at the end of the bout (easier to grok)
* default ship can damage planets

* Tweaking fighter generic_survival_rate (at 0.2 basically no fighters were produced, 0.6 delivers more sensible results)







[v0.5.2] - 2026-0X-0Y
---------------

### Key Changes

- 
    - 
- 
- 


### Detailed Changelog

#### Graphics / Interface

- Use IP address as name in server list if server name is empty
- Use 99999 instead of powers of two to represent a large / unknown meter value
- When searching pedia articles, search with and without internal formatting / link tags
- Restored planet shininess rendering
- Allow ghost buildings (previously but not now visible) to be dismissed
- List total and last turns of adoption in empire pedia articles

#### Content and Balance

- Government
    - 
    
- Ships
    - Added Graviton Pulsar core slot weapon
    - Made Transpatial Hull available early
    - Made Transpatial Hull a good low fuel hidden scout
    - Split Transpatial Hull and Drive techs
    - Rebalance Organic Growth/Organic Hulls
    - Make outpost parts cheaper -20%/-10PP


- Planets / Buildings
    - Postpone Automatic History Analyzer and make it more expensive


- Pedia and Stringtables
    - Translation updates: French

- Other
    - 

#### AI

- Update AI for fighter damage pilot trait

#### Other Features

- Add option for alternative AI executable

#### Other Bug Fixes

- Fix possible invalid iterator dereference while parsing text
- Fix sorting of system names with non-Latin characters
- Fix broken null pointer check generating OnPlanet condition Description

- Crashes or Disconnects
    - Fix crash due to logging in Sound::Impl destructor

- Gameplay
    - Allow multiple objects to be recorded as destroying an object, rather than picking just one that damaged a destroyed object in a combat round

- Content
    - Fix fighter damage with scaling info in pedia
    - Fix pedia InterDesign Academy unique species limit: 7 not 6
    - Fix generated valueref descriptions like "Source's turns since annexation" that had too few placeholders


#### Technical / Internal

- Refactor and make functions and classes usable and testable at compile-time (constexpr) including:
    - Conditions, ValueRefs, checksums, Font / text layout, UniverseObject
- Null terminate concatenated strings in Species construction
- Determine more content script invariance flags at construction instead of when evaluating
- Implement more Condition EvalAny overrides
- Determine Conditions' possible matched object types during initialization and propagate to enclosing conditions
- Improve data packing in ValueRef with bit fields for flags
- Add runtime safety checks throughout code
    - While moving cursor through Edit control text
- Add compile-time static_asserts to verify portability
- Use constructor parameter flag types instead of ambiguous overloads
- Refactor GG Font classes / code
- Fix double-forwarding of arguments in AI order-issuing wrapper function
- Replace uses of deprecated library functions
- Avoid copying strings unnecessarily
- Fix compile errors with Boost >= 1.87
- Pass ScriptingContext through call chains instead of repeatedly getting / constructing them
- Use ScriptingContext gamestate getters rather than global getters
- Replace some boost::bind with lambdas
- Fix variable shadowing warnings
- Convert more loops and algorithms to C++ ranges or Boost or custom fallback implementations
- Copy string_view into a (null terminated) string before passing to sscanf
- Don't mark some functions noexcept when they probably aren't
- Remove UniverseObjectVisitor classes and related functions
- Further transition to Python-based parsing of content scripts
- Require minimum Boost 1.73
- Require minimum Windows 10
- Require minimum Python 3.10
- Tweak / update MSVC project files
    - Add MSVC 2026 files
    - Remove Win32 configuration
- Add a function to query Logger thresholds
- Fix signed / unsigned comparison warnings
- Tweaked pathfinding system graph fallback to full universe graph if no empire-filtered graph exists
- Rework various content scripts to use named values in place of hardcoded constants
- Replace custom process code with Boost Process
- Replace all use of glBegin/glEnd/glVertex with buffer-based rendering
- Add tracking and scripting access latest turns policies are/were adopted