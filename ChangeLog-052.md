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


* also search detagged article text for pedia search string


* grooming, comments


* use range_find_if instead of filter with break


* -rangify
-init locals

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

* [pedia] review changes 01 (geoffthemedio)


* [pedia] Add descriptions for bookkeeping specials LOWER_STEALTH_COUNT_SPECIAL and BASE_STEALTH_SPECIAL


* [pedia] add FLEET_UNSTEALTHINESS game concept, better stealth effect descriptions

* renaming, restructuring, rewording
* lists of main effects to stealth for planets and ships
* fix missing label

Future stuff (probably make issues out of these):
* add generated list of effects which affect planetary stealth (technologies, policies, ...)
* probably using a tag; maybe generating text via content analysis
* style: do render only the first occurence of METER_STEALTH as prominent link
* style: do not render METER_STEALTH as prominent link when showing the METER_STEALTH entry

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

* [focs][py] Add SpecialCapacity valueref (double valueref s1:name i1:object)


* [focs][py] Extend parser support for boolean valuerefs


* Update docker build ubuntu-24.10 -> ubuntu-25.04


* bump gcc/g++ versions to reported latest on each Ubuntu version


* -do Ubuntu builds on 22.04 and 24.04 for gcc and clang
-pick clang version with ternary in CC and CXX specifiers for Ubuntu

* -OSX15 instead of OSX13 builds
-do weekly build on OSX14

* -fix ships being moved to a new system but not having a new fleet generated for them due to then already being at the target object location
-add ships before naming new fleet
-move get_fleet and get_planet to anonymous namespace

* OSX workaround


* remove Boost system dependency. apparently only exists for backwards compatibility in recent Boost versions.


* quiet warnings


* [focs][py] std::string support for arg1 operators


* [focs][py] Add NoOpValue e.g. NoOpValue(float, 1.1)

currently supported
* NoOpValue(float, 1.1)
* NoOpValue(int, 1)
* NoOpValue(str, 'strii')

explicit typing is probably a bad idea (besides float/int), but consistent with other operators
we probably get some troubles as soon enums etc get calculated

* [focs][py] NoOpCondition and NoOpEffect


* expose Condition::NoOp as NoOpLog


* Postpone Automatic History Analyzer and make it more expensive

https://freeorion.org/forum/viewtopic.php?p=121862&

Behold the brainlessness of building the AHA!
The automatic history analyzer is very powerful we want to delay it
hoping some empires will choose to invest into something else before

- Adding LRN_ALGO_ELEGANCE (18RP) as prerequisite to LRN_PHYS_BRAIN.
- Increase RP cost of LRN_PHYS_BRAIN (maybe 20 RP, +10PP, +100%).

LRN_PHYS_BRAIN also unlocks translinguistics and liberty
in order not to delay liberty so much:
- move LIBERTY policy to LRN_ALGO_ELEGANCE

AI: research and policy choices should be looked at
en: the liberty and algorithmic elegance fluff might need enhancement

This delays building the AHA about 3-4 turns if fully committed to it.
QA: playtested researching & building the AHA
Probably needs more delay, but no harm in playtesting it in mainline,

* [focs][pyi] Restructure _effects.pyi for easier scripting


* [focs][py][lint] mypy plus ruff format


* [focs][py] rename species/species_macros/stealth -> .../stealth_trait

eases mypy. is a bit less ambiguous

* [focs][py] Adjust fleet unstealthiness, to properly uncouple next and previous system into starlane directions


* [focs][py] Add debug_starlane_travel effectsgroup to custom sitreps


* [focs] Remove unused NamedReal FLEET_UNSTEALTH_SHIPS_SCALING


* [focs][py] Fix Target.NextSystemID typing


* [en] Adjust stringtables of fleet unstealthiness


* [valrefs] LOG_UNKNOWN_VARIABLE_PROPERTY_TRACE in case of using fleet property on non-fleet object


* [focs][pyi] Add Target.Fleet


* [focs][py] Add condition All


* Make outpost parts cheaper -20%/-10PP

https://freeorion.org/forum/viewtopic.php?p=121850

Make establishing outposts cheaper and keep the colonization costs the same.

minor objections/look out for:
* might become unbalanced if content gives resources from outposts
* increasing colonization cost also increases recolonization cost

AI - nothing to adapt
en - stringtable description still fits

* [focs] Make ruff format step happy


* [AI] ShipDesign considers organic growth after expected combat (50 percent discounted)


* [AI] Make ruff happy


* [AI] AIDependencies get named values fixup for all organic hull types


* [AI] fix get_named_* in AIDependencies (Workaround for dependency loop)


* [py] fix error docstring/messages


* [AI][focs][strings] Refactor SH_RAVENOUS to use named values


* [focs] Fixup organic to wobblies values


* [AI][focs][strings] Refactor SH_SYMBIOTIC to use named values


* [AI][focs][strings] Refactor SH_PROTOPLASMIC and SH_SENTIENT to use named values


* [AI][focs][strings] Refactor SH_ORGANIC to use named values


* [AI][focs][strings] Refactor SH_ENDOSYMBIOTIC to use named values


* [AI][focs][strings] Refactor SH_ENDOMORPHIC to use named values


* [AI] Update AIDependencies BIOADAPTIVE growth


* [focs] Refactor SH_BIOADAPTIVE named values to reusable macros


* [AI][focs][strings] Refactor SH_BIOADAPTIVE to named values


* [AI][focs][strings] Rebalance Organic Growth/Organic Hulls


* [focs] My ruff format step happy


* [focs] Linear fleet unstealthiness (no AI)

note it seems there is no AI support yet for fleet unstealthiness at all

* fix modifying empire designs while iterating through them


* -store value as value
-indentation

* grooming


* remove redundant override


* grooming


* newline grooming


* store return by value as local value


* -set app size after signal
-move mapwnd and intro screen resize handing after setting app size

* add stream operator<< for X and Y


* specify type in Add


* only call GetOptionsDB() once


* -specify type in Add
-grooming

* only call GetOptionsDB() once


* -rework OptionsDB::Add to template parameter is used to ensure convertibility to specified type rather than just determine parameter type
-pass Option parameters by value

* constexpr ToInt


* static_assert -> requires


* use enum to indicate whether to simulate queue


* -use enum for whether ShipDesign is a monster
-named constant for stringtable flag

* whitespace tweak


* -use enums for Option Storable / Flag / Recognized flags
-grooming

* use range_min_element or range_min_element


* -grooming
-rangify

* -fix invokable on const reference get(pred) range use
-store lambda in local
-noexcept

* safety check


* safety checks


* safety checks


* add OptionsDB::OptionExistsAndHasTypedValue


* -grooming
-query that option exists before getting

* -constexpr
-noexcept
-static_assert tests

* -constexpr
-noexcept
-static_assert tests

* unnecessary copy


* grooming


* grooming


* factor out getting IDs of objects to set visibility of


* use range_contains


* use range_filter


* use range_to_vec


* -use vector copy construction instead of iterator pair constructor
-safety checks
-noexcept

* -use findIDs
-rangify

* -rangify
-use range_to
-noexcept

* -rangify
-use range_to_vec

* -use range_to_vec or range_to
-use range_find_if
-avoid redundant return copy
-use separated lambda
-noexcept

* use range_to_vec or range_to


* trivial whitespace grooming


* use range_to or range_to_vec


* use range_to_vec


* use range_to_vec


* -use range_to_vec
-extract lambda function

* -use range_to_vec or range_to
-use insert_or_assign

* -use range_any_of
-noexcept
-use range_to_vec or range_to

* -use range_to
-used deduced pair constructor
-use range_any_of

* use range_any_of


* -use range_to or range_to_vec
-noexcept

* use range_to


* use range_to_vec


* use DefaultLogPath()


* [[maybe_unused]]


* quiet unused function warning


* quiet unused variable warning


* add and use range_count


* replace deprecated Py_GetProgramFullPath() with import("sys").attr("executable")


* Move stringtable init earlier, before it is needed for initializing Font charsets


* fix bounds check producing junk adoption turns


* avoid redundant call


* rangify


* compile error workaround


* semi-rangify


* rangify


* rangify


* compile error workarounds


* -rangify
-forward_as_tuple
-structured binding

* store mutexes by value


* rangify


* constexpr


* -rangify
-constexpr

* use Networking::is_X


* -make sever GetApp return a reference
-avoid global gamestate getters
-don't get server within ServerApp code
-rename GGHumanClientApp::GetClientUI to ...::GetUI
-separate human client logging init and other init from construction
-grooming
-structured binding

* -immediately exit NewSinglePlayerGame if new game dialog is cancelled
-make some infrequent logger calls Debug instead of Trace level

* rangify


* replace SpeciesManager::PlayableSpecies() and NativeSpecies() with call site filters


* fix copy paste error


* Cherry-pick changes from release 0.5.1.1


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

TODOs
* check heavy bombers (aka has_bombers)
* check carrier designer

pyhon 3.9 compatible (if elif else  instead match case)

* trivial whitespace grooming


* reduce log spam


* parsing support for LatestTurnPolicyAdopted


* -track and expose latest turns policies are/were adopted
-list total and last turns of adoption in empire pedia articles

* noexcept


* rangify


* grooming


* use std::addressof


* -inlining
-grooming
-use std::addressof

* use vector and then sort instead of multimap


* -factor out and add safety checks to comparator
-rangify

* use std::addressof


* -use std::addressof
-use array instead of c-array for buffer

* -use std::addressof
-compiler error workaround
-rangify

* use std::addressof


* grooming


* -rangify counting forces and make a it free function
-use std::addressof

* -use std::addressof
-use .data() instead of &.front()

* -use std::addressof
-use .data() instead of &.front()

* use std::addressof


* rangify


* -use std::addressof
-grooming, noexcept

* more compiler error workaround attempts


* use std::to_address or std::addressof or workaround for compiler error


* use std::addressof or std::to_address or .get()


* rangify


* tweak commented-out code


* & -> std::addressof


* grooming


* rangify


* rangification


* rangify


* rangify


* -rangify
-grooming

* used enums instead of bools for flags


* -rangify
-grooming
-make lambda named local

* rangify


* -use range_all_of, range_any_of, range_transform, range_contains, range_filter
-name some lambdas
-grooming, noexcept

* use range_contains and range_any_of


* use range_find_if or range_transform


* -use range_find_if, range_filter
-operator[] -> emplace or insert_or_assign
-avoid string copies by using string_view for temp storage
-const

* -use range_find_if, range_values, range_filter
-constexpr, noexcept
-sort -> stable_sort
-grooming

* -use range_find_if, range_find, range_contains
-noexcept

* use range_find_if


* noexcept


* use range_find_if


* use range_find_if or range_find


* grooming


* use range_find_if


* use contains or range_contains


* refactor SpeciesManager to return ranges or containers instead of having begin/end functions directly in class


* add range_drop and range_empty


* missing #include


* Fix compilation after dismiss stale building cherry-picked


* Allow to see and dismiss stale ghost buildings


* [CI] Fix ruff warnings


* Run tests on all Python versions


* Rewrite savegame coder, to provide better testing


* Update tests for the Stub generator to support python 3.11


* Raise minumum python version to 3.10


* Drop modules isolation


* Provide file name for Python FOCS parser


* simplify erasing


* use insert_or_assign


* rangify


* use range_filter and range_contains


* simplify and rangify


* -use range_filter and range_transform
-grooming

* -use range_find_if
-use range_filter
-use insert_or_assign

* -use range_find or range_contains
-use piecewise_construct
-use range_keys

* use range_find or range_contains


* use range_find or range_contains


* -use std algorithms or constexpr fallbacks
-grooming

* -rework PolicyManager to store name-Policy pairs in a vector
-access that with a getter instead of top-level begin/end

* none_of -> !ranges_contains or range_none_of


* -use range_any_of
-++ to += for float

* -use range_find_if
-use emplace or insert_or_assign
-rangify
-use Networking::is_invalid
-grooming
-use range_any_of

* use range_all_of


* -replace for loop with range_filter
-grooming

* -replace for loop with range_any_of
-grooming

* noexcept


* -replace begin, end, established_begin, established_end with range getters
-make GetPlayer return a pointer instead of iterator
-rangification
-rename IsEstablished to IsEstablishedNamedValidClient

* -rework boost-based range_keys and range_values to work better for non map-like containers
-pass stuff to range adaptors with perfect forwarding
-fix forwarding

* add ranges_none_of


* add range_distance


* Convert second statistic file to test module reloading


* Tune parser module names


* Move parser modules to focs sub-folder


* Define interface of parser module


* Load parser module by hand with additional data


* Introduce module with content type registration function


* [CI] Use Python 3.10 for linting (#5237)

* Add tests for different Python versions
* Use Python 3.10 for linting
* Fix AI and universe generator lints

* CI: Disable Debian oldstable until it support Python 3.10+


* [CI] Add tests for different Python versions (#5234)


* Update Docker CI Fedora to 35


* Use Boost.Process V1 for MacOS


* Update to SDK v16 with python 3.10.16


* Don't use barrier for background parser completion


* Regenerate fixed colonies buildings


* buildings: colonies: fix exticts species colonies

The format conversion for colony building generation altered the logic
for colonization. This makes it impossible to colonize planets with
extinct species like "The Banforo".

For the colonization to be possible it should EITHER be required the
planet to be colonized has a supply-connection to a planet which:

1. Has already colony of the species
	1a) Which has sufficient population
	2b) Is happy enough

OR

2. Has the extict species special + required tech and a Xenoresurrection
   lab.

The conversion altered this condition to require both, which is not
possible.

Fix the extinct species resurrection by requiring only one of the above.

Signed-off-by: Matti Vaittinen <mazziesaccount@gmail.com>
Fixes: 58efaab1f ("[FOCS] Convert colonies buildings (#5185)")

* Remove warnings by adding  SR_GRAV_PULSE to upgrade dicts (#5231)

Remove warnings by adding  SR_GRAV_PULSE to WEAPON_UPGRADE_DICT and WEAPON_ROF_UPGRADE_DICT

Fixes #5227

* Fix Boost.Process V1 usage for Boost 1.88


* internal compiler error workarounds


* grooming


* incremental rangification


* use lambda to condense cases


* grooming


* grooming


* -structured binding
-operator[] -> insert

* -use Networking client type checkers
-rangify
-grooming

* use Networking client type checker


* use Networking client type checkers


* -rangify
-use Networking client type checkers
-grooming

* -rangify
-use Networking client type checkers
-default variable values

* -structured binding
-use Networking client type checkers

* make helper lambda to deduplicate nibble to char conversion


* -use Networking::is_ai and similar functions
-rangify
-structured binding
-grooming

* -use Networking::is_ai and similar instead of local versions or client type explicit checks
-operator[] -> insert_or_assign
-rangify

* use Networking::is_ai and replace if (false) continue with if (true) dostuff


* use Networking::is_ai and similar functions instead of directly comparing client types


* add is_ai, is_human, etc. helpers to Networking namespace


* rangify


* -rename ValFromTwoHexChars to HexCharsToUInt8
-add separate HexCharToUint8 for a single char
-add tests

* add / wrap range_contains


* structured binding


* use range_keys


* use range_values


* grooming


* -safety checks
-static tests

* -constexpr
-noexcept
-[[nodiscard]]

* use buffers for CircleArc rendering


* fix CircleArc tessellation to 36 slices and make verticies colors single fixed-size arrays


* -comment tweak
-use glColor instead of glColor4ub

* grooming


* -tweak casting
-use unsigned literals

* -put getting and initializing containers for vertices and colours into a function
-return vector instead of valarray

* -use function for angle correction
-find sin/cos immediately after

* -remove GroupBox files from CMakeLists.txt
-delete GroupBox files

* blank GroupBox related files and remove from MSVC project files


* render with buffers instead of glBegin / glEnd etc.


* render planets with buffers instead of old-style glVertex type calls


* restore and tweak planet shininess rendering


* add Clr::ToNormalizedRGBA


* -determine best environment for species in separate function
-structured binding

* calculate sphere coords at compile time


* const static init with lambdas


* -move sphere coord init out of RenderSphere function
-use static for storing light position
-pass texure ids rather than pointers to RenderSphere and RenderPlanet
-use Size() instead of Pt(Width(), Height())

* add Species::BestEnvironmentPlanetType and use instead of temporary map


* -grooming
-rangify

* -structured binding
-const

* -structured binding
-operator[] -> insert_or_assign

* static asserts and tests


* -use to_string instead of intermediate stringstream
-move from locals
-reserve

* rangify


* rangify


* disable warning


* fixing types


* -default values
-const noexcept
-explicit
-grooming

* internal compiler error workarounds (#5225)


* Cleanup version check for unsupported Python versions


* -default values
-const noexcept
-explicit

* remove unused template


* -check if program or shader has been flagged for deletion before deleting it (again)
-default member values

* pass by const reference


* -grooming
-const

* explicit


* whitespace grooming


* -grooming
-use AppSize

* -const
-use string_view not string&
-move empire ID getter into lambda / where used

* -safety checks
-reorder

* grooming


* quiet warnings


* -safety checks
-grooming
-use context, no global getter

* default member value


* pass along parameters that were dropped


* null pointer safety checks


* add const Empire getter to ClientApp


* grooming


* -make SDLGUI::FramebuffersAvailable static noexcept
-add SDLGUI::AppSize

* use range_values


* use local Client() getter, not global getter


* inline


* -add minimal-parameters GGHumanClientApp constructor overload
-move options queries into constructor for default values
-pass position params as X or Y

* grooming


* make GUI::GetFont and FreeFont static or const


* store UI by value


* -pass font with move
-noexcept
-safety checks

* handle empty texture


* -pass around context, objects, or client empire ID to avoid later global lookups
-replace boost::bind with lambdas and storing signal connections
-safety checks on result of GetApp
-grooming

* pass strings by value


* 🍌


* noexcept


* inlining


* inlining


* inlining


* -remove unused return value from StoreTexture
-use insert_or_assign

* -default constructor
-use simple inline getter instead of filling a new container

* CI: Update Ubuntu used for address sanitizer to 24.04


* Use ClientFSMEvents only in the GG client


* Fix parser type PlanetType -> PlanetSize

-fix parser type PlanetType -> PlanetSize
-add operator != for value_ref_wrapper<PlanetSize>
-re-expose value_ref_wrapper<PlanetType> operator!=

* Test optional result value before dereference


* Drop old FOCS parser for buildings types


* Update README with changes


* Reformat code with ruff


* Change black to ruff


* [CI] Bump static check tools


* Remove unused enqueue.macros


* Inline macros

With a single usage it's easier to manage it within the file

* Remove unneeded text file


* Use species parameter in Planet condition in FOCS Python parser


* Fix colonies buildings after conversion


* remove unused drawing functions


* quiet warnings


* mark override / quiet warning


* remove unused variable


* comment grooming


* pass by value / move


* pass by value


* pass by value / move


* quiet shadowing warning


* quiet warning about hidden virtual function in derived class


* quiet warning / maybe fix order of operations issue


* quiet warning


* inline constexpr lambdas in headers


* quiet warning


* quiet warnings


* disable uninitialized variable and comma subscript warnings in parsers


* -noexcept
-const
-null safety checks
-move
-auto

* quiet warning


* -use number of panels instead of querying system for number of planets
-grooming
-noexcept

* -pass ObjectMap to avoid global gamestate getters
-store connections in array
-put helper functions in separate namespace

* avoid global getters


* -avoid global getters by passing context or empire id or planet type/size
-grooming

* -pass in ObjectMap instead of using global getters
-pass in vector by value
-grooming
-try_emplace instead of find and operator[]
-exchange instead of double lookup
-use range_values

* -pass ClientApp or context/players/app empire id to avoid global gamestate getters
-store connections as members and use lambdas instead of boost::bind
-explicit constructor
-grooming
-noexcept

* auto


* pass context to GetAccountingInfo


* pass context to ObjectListWnd and ObjectListBox Refresh


* pass context to Update / Initialize


* pass context into UpdateEffectLabelsAndValues


* grooming


* make_tuple -> forward_as_tuple


* structured binding


* structured binding


* auto&& -> auto


* -auto&& to auto
-grooming

* -avoid global getters
-avoid duplication
-use exchange

* use context species


* -avoid global gamestate getters
-grooming

* -avoid global gamestate getters
-use unsigned counter

* -avoid global gamestate getters
-reserve
-use try_emplace instead of find and operator[]
-use std::exchange instead of repeated opeartor[]
-tpyos

* -avoid global gamestate getters
-avoid dynamic_pointer_cast

* -avoid global gamestate getters
-reserve
-use try_emplace instead of find and then operator[]
-use std::exchange instead repeated operator[]
-structured binding

* -pass Universe instead of full context
-safety checks

* -avoid global gamestate getters
-grooming

* [FOCS] Convert shipyards (#5204)


* CI: Restore Godot3 test for Fedora Rawhide Docker (#5202)


* Implement CanProduceShips condition in Python FOCS parser


* [FOCS] Convert shipyards part 1


* Bump ruff version


* refactor terraform building


* Rename building.py to avoid confusions


* [FOCS] Convert building


* inline Number constructor


* inline Turn constructor (with checksum calc) and operator==


* inline HasTag constructor (with checksum) and operator==


* inline Type constructor (with checksum calc) and operator==


* grooming


* -add function to compare pointers' pointed to values
-inline Number::operator==

* -quiet warning
-grooming

* quiet warning


* -quiet warning
-grooming

* don't cast unnecessarily


* quiet warnings


* fix possible reference to temporary


* -change RomanNumber parameter to uint16_t and cast if passed something else
-constrain attempted representations to representable range
-move #include into .cpp

* add range_count_if


* enable EnforceTypeConversionRules in MSVC project files


* determine Condition::Building checksum in constructor


* typedef -> using


* try to avoid repeatedly initializing Python


* remove extra "error:" from unit test logs


* quiet warning


* quiet warning


* quiet warning


* add test / quiet warning


* quiet warning


* removed unused constant / quiet warnings


* add tests / quiet warnings


* quiet warnings


* quiet warning


* grooming / possibly avoid reference to temporary


* quiet warning


* quiet warning


* -init all ValueRef checksums in constructors
-devirtualize ValueRefBase::GetCheckSum

* Introduce Const(type, value) to Python FOCS parser


* Adjust tech test - one was added


* Move SHP_TRANSSPACE out of robotic hull tech folder


* Move SH_TRANSPATIAL out of robotic hull line folder


* Make Transpatial Hull available early

* split the hull tech from the drive tech (480 PP -> 80PP + 400PP).
  the hull needs NANOTECH_PROD which is a useful tech on the way to super robo hulls
  the drive tech has exactly the same in-/direct prereqs like before
  cheapest hull in PP and RP which offers a core slot
  as closest alternative the composite line needs about 200PP but is more robust
* advanced engineering bay only necessary to build the drive
  ship yard + orbital drydock is enough for the hull
* make the hull a low fuel good scout (GOOD_HULL_DETECTION),
  base detection slightly better than protoplasmic hull
  so it will be nice hidden scout with the drive

* Prepare to split and fast-track the transspatial _HULL tech from the _DRIVE tech.

For that rename SHP_TRANSSPACE_DRIVE.focs.py to SHP_TRANSSPACE.focs.py which is intendend to contain both techs"
SHP_TRANSSPACE.focs.py is intendend to contain both the _HULL and _DRICE techs

* Unlock Graviton Pulsar ship part with Nanotech Production (an the way to transpatial hull)


* Graviton Pulsar weapon part available for 40PP in a core slot nearby


* Implement ValueRefPlanetType != ValueRefPlantetType in Python FOCS parser


* deduplicate glue code


* grooming


* Accept NumberOf(sortkey=str) in Python FOCS parser


* Implement int+ValueRefInt in Python FOCS parser


* Allow *NumberOf with default number argument in Python FOCS parser


* Fix Statistic(type, type, ...) and implement ValueRefInt*ValueRefInt in Python FOCS parser


* Implement Statistic(TypeReturn, TypeValue, StatisticType, value=ValueRef[TypeValue], condition=Condition) in Python FOCS parser


* Implement ValueRefPlanetEnvironment, ValueRefPlanetSize variable properties in Python FOCS parser


* Accept const PlanetType in PlanetTypeDifference in Python FOCS parser


* Implement ValueRefPlanetType variable properties in Python FOCS parser


* Use system graph if there is no empire view graph in PathFinder..LeastJumpsPath (i.e. for human and AI client) (#5179)


* -calculate checksums in constructors
-constexpr

* -inline Not constructor
-inline Not::operator==
-inline Not::GetCheckSum

* static_asserts for combinability / noexcept


* noexcept with parameter pack expansion


* -replace ValueRef::CalculateCheckSum with variadic CheckSums::GetCheckSum
-make a few Condition constructors explicit

* calculate PlanetType checksum in constructors


* calculate OrderedBombarded checksum in constructor


* -determine Stationary checksum in constructor
-make some other condition GetCheckSum functions constexpr
-add some static_assert for condition checksums and various ways to combine them

* make base class Condition::GetCheckSum constexpr


* -inline Or::GetCheckSum and use CalculateCheckSum
-tweak Or delegating constructors

* -add checksum cache member to Condition
-extend Condition constructors to take and store checksum value

* use delegating constructor in Condition::Or to track whether operands have been denested


* -combine StaticCast constructors
-determine StaticCast invariants in constructor

* comments


* comments


* noexcept


* determine ComplexVariable invariant in constructor


* determine Operation invariants with helper function


* -determine TotalFighterShots invariants and checksum in constructor
-constexpr tweaks

* -add RefsRTSLICE to determine root candidate, target, source, local candidate invariance and if a ValueRef is a constant expression
-use RefsRTSLICE for Statistic constructor

* move CondsRTSI into Condition.h


* make CondsRTSI variadic


* tuple -> pair


* [FOCS] Convert buildings


* [FOCS] Convert colonies buildings (#5185)


* Add option for alternative AI executable


* Add new badges for README.md


* Try to use address sanitizer in CI


* CI: Restore Manjaro Docker image


* Implement PlanetTypeDifference in Python FOCS parser


* Accept BuildingType(buildtime=float) and float+ValueRefDouble in Python FOCS parser


* Fix address issue in PathFinder


* Make Windows CI build fail on failed tests


* Revert "[FOCS] Convert building"

This reverts commit 492ae353b89f3c8dbb6da4e1647a761009666824.

* [FOCS] Convert building


* cmake: Remove old boost version handling for...

BOOST_OPTIONAL_CONFIG_USE_OLD_IMPLEMENTATION_OF_OPTIONAL

This is checking for boost version being > 1.60 and < 1.67,
whereas a few lines up in the file we have:

set(MINIMUM_BOOST_VERSION 1.73.0)

Signed-off-by: Vincent Legoll <vincent.legoll@gmail.com>

* Add option to enable/disable cppcheck


* CI: Use experimental g++ on Debian sid


* fix repeatedly creating strings and passing mismatched iterators


* fix repeatedly creating strings and passing mismatched iterators


* Fix address sanitizer error in pyhon FOCS parser

def_readonly accepts reference to value but literals alive only in
function they declared.

* Quickfix: Use generic system graph for finding immediate neighbors of a system

Test  https://github.com/freeorion/freeorion/pull/5163   (master should fail with the test but without the fix)
Issue https://github.com/freeorion/freeorion/issues/5158
QA Status: quickfix completely untested

the underlying issue why this surfaces is not fixed with this.

the underlying issue is that the empire graphs do get cleaned when the generic graph gets cleaned
and not rebuild before the AI uses the empire graphs

waiting for some comments on what the intended design is

* Add LIFECYCLE_MANIP_POPULATION_EFFECTS macros to Python FOCS parser


* Output warning if Pathfinder uses graph bigger than number of systems


* Implement empire statistics in Python FOCS parser and test it


* Add COLONIZATION_POLICY_MULTIPLIER macros to Python FOCS parser


* Implement SetEmpireCapital effect in Python FOCS parser


* Fix warning: operation on 'planet_id' may be undefined

universe/Conditions.cpp: In member function 'virtual std::string Condition::OnPlanet::Description(bool) const':
universe/Conditions.cpp:3485:76: warning: operation on 'planet_id' may be undefined [-Wsequence-point]
 3485 |     int planet_id = m_planet_id && m_planet_id->ConstantExpr() ? planet_id = m_planet_id->Eval() : INVALID_OBJECT_ID;
      |                                                                  ~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~

Signed-off-by: Vincent Legoll <vincent.legoll@gmail.com>

* Fix warning: comparison of integer expressions of different signedness

Empire/ResearchQueue.cpp: In member function 'void ResearchQueue::Update(float, const std::map<std::__cxx11::basic_string<char>, float>&, const std::vector<std::tuple<std::basic_string_view<char, std::char_traits<char> >, double, int> >&, const ScriptingContext&)':
Empire/ResearchQueue.cpp:333:35: warning: comparison of integer expressions of different signedness: 'const long unsigned int' and 'int' [-Wsign-compare]
  333 |                 if (this_tech_idx < next_res_tech_idx )
      |                     ~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~

Signed-off-by: Vincent Legoll <vincent.legoll@gmail.com>

* Fix warning: structured binding declaration set but not used

universe/Pathfinder.cpp: In member function 'void Pathfinder::PathfinderImpl::UpdateCommonFilteredSystemGraphs(const EmpireManager&, const ObjectMap&)':
universe/Pathfinder.cpp:1523:16: warning: structured binding declaration set but not used [-Wunused-but-set-variable]
 1523 |     const auto [eit, success] = m_graph_impl.empire_system_graph_views.emplace(
      |                ^~~~~~~~~~~~~~

Signed-off-by: Vincent Legoll <vincent.legoll@gmail.com>

* Fix warning: unused variable 'res'

GG/src/MultiEdit.cpp: In member function 'virtual void GG::MultiEdit::KeyPress(GG::Key, uint32_t, GG::Flags<GG::ModKey>)':
GG/src/MultiEdit.cpp:869:26: warning: unused variable 'res' [-Wunused-variable]
  869 |                 if (auto res = LineEndsWithEndlineCharacter(line, Text())) {
      |                          ^~~

Signed-off-by: Vincent Legoll <vincent.legoll@gmail.com>

* Fix warning: unused variable 'obj'

universe/Universe.cpp: In member function 'void Universe::ExecuteEffects(std::map<int, std::vector<std::pair<Effect::SourcedEffectsGroup, Effect::TargetsAndCause> > >&, ScriptingContext&, bool, bool, bool, bool, bool)':
universe/Universe.cpp:1906:25: warning: unused variable 'obj' [-Wunused-variable]
 1906 |         if (const auto* obj = m_objects.getRaw(obj_id))
      |                         ^~~

Signed-off-by: Vincent Legoll <vincent.legoll@gmail.com>

* Fix warning: unused variable 'bt'

UI/BuildingsPanel.cpp: In member function 'virtual void BuildingIndicator::RClick(GG::Pt, GG::Flags<GG::ModKey>)':
UI/BuildingsPanel.cpp:420:29: warning: unused variable 'bt' [-Wunused-variable]
  420 |     if (const BuildingType* bt = GetBuildingType(building_type)) {
      |                             ^~

Signed-off-by: Vincent Legoll <vincent.legoll@gmail.com>

* fix generated valueref descriptions like "Source's turns since annexation" that were using a format string with too few placeholders (#5157)


* [FOCS] Convert building


* CI: Check queued runs and tune message for weekly test builds


* Try to fix strings convertion on Windows


* Use Boost.Process V1 for Windows


* Implement ShortestPath(ValueRefInt, ValueRefInt) in Python FOCS parser


* [FOCS] Convert buildings to FOCS (#4867)

- CONC_CAMP_REMNANT
- EXPERIMENTOR_OUTPOST

* Fix default affiliation in GenerateSitRepMessage in Python FOCS parser


* Add tests for value references dumps


* Fix non-object reference Variable dump


* -make And and Or constructors variadic
-reimplement and inline Or::operator==

* move into class definition


* templatize Operation constructor for arbitrary numbers of operands


* need not be virtual


* -include property name and return type in ComplexVariable CheckSum
-add tests
-reorder ValueRefBase members
-mask checksum before storing

* use delegating constructor for ComplexVariable


* determine ComplexVariable checksum during construction


* determine Operation checksum during construction


* determine Statistic checksum during construction


* -determine Variable checksum during construction
-tweak noexcept specifications
-remove m_container_type check in Variable::operator== as it should be checked in ValueRefBase::operator==

* use parameter pack


* also determine Constant<std::string> checksum in constructor


* -add (24 bits) of checksum cache to ValueRefBase.
-add static_assert to verify that 24 bits is enough to contain the value of CHECKSUM_MODULUS
-add optional parameters to pass checksum down via ValueRef to ValueRef base
-tweak Constant copy constructor parameter name
-add separate basic cast for Constant constructor
-determine Constant checksum in constructor
-add comments for bit flags in ValueRefBase

* rework GetTypedObjects to avoid adding rather than filtering out duplicates


* Or initial candidates


* just use nullptr


* fallback constexpr sort for planet and ship meter type arrays


* And initial candidates


* add some GetDefaultInitialCandidateObjectTypes functions


* -comments
-trivial grooming

* add range_find


* quiet warning with signed vs unsigned compare


* -predetermine and store whether nested condition's contained condition matches only particular types
-specify some other condition matches types
-add a getter to Type condition for its matched types
-rework Contains and ContainedBy GetDefaultInitialCandidateObjects functions

* -constexpr ComtainedBy
-add NestedCondition base class for conditions that contain another condition

* trivial grooming


* add Capital::EvalAny taking IDs


* don't need lambda wrappers in static_assert


* grooming


* split up Match cases


* -add SortedNumberOf::EvalAny and All::EvalAny for ids
-use derived Condition Eval when available
-grooming
-missing #include
-compiler error workaround

* -make Condition::ObjectSet contain UniverseObjectCXBase*
-make non-mutable object pointers in ScriptingContext be UniverseObjectCXBase*
-store separate vectors of UniverseObjectCXBase* in ObjectMap so they can be gotten
-move four-parameter EvalImpl into Condition.h and make constexpr
-add a constexpr DoPartition helper for EvalImpl
-add (also constexpr) two-parameter EvalImpl
-add some static_assert tests of Contains condition simple match and using EvalImpl
-chang various functions/lambdas taking UniverseObject* to taking auto*
-fix Contains::Match using derived Condition pointers instead of base class pointers

* grooming


* -add function that sets visibility and filter condition at the same time to avoid double Refresh
-move filter condition check later when determining if object is visible

* -const
-noexcept

* constify invariance flags


* -refactor CondsRTSI and support multiple parameters
-move more Condition invariance determination into CondsRTSI in initializer lists

* const overload


* move extra values into list / loop


* structured binding


* add ranges_all_of


* add constexpr UniverseObject base class


* grooming


* fix PlanetFromObject wrong overload use


* Use Boost.Process V1 for linux


* Fix CI build on MSVS 14.43


* stringtables: Fix build constraint for Species InterDesign Academy (seven max)

Fixes #5120

Species InterDesign Academy building is limited to maximum 7 copies on 7 planets with unique species.
Not on only 6 planets

* pass trace_logging flag in PathFinder / GeneralizedLocation (#5137)


* CI: Drop Manjaro until images will be fixed


* Provide boost::asio::io_context to Process


* Initialize error handler within sub-interpreter context


* Accept ValueRefDouble in Turn condition in Python FOCS parser


* Save Windows Godot client logs to artifact


* Output user data directory in the Godot test


* Accept ValueRefInt as argument for Max-Min-OneOf on floats in Python FOCS parser


* Move BLD_BLACK_HOLE_POW_GEN_MIN_STABILITY valueref definition to named_values.focs.txt (#5138)

Co-authored-by: agrrr3 <agrrr3@users.noreply.github.com>

* null terminate concatenated strings


* early exit if dispatch was passed no effects groups


* check if trace logging enabled once when dispatching scope evaluations, and pass along


* check log threshold before calling TraceLogger()


* track and add getter for log thresholds


* [FOCS] Convert buildings (#5133)


* Implement ValueRefInt/int in Python FOCS parser


* Implement ValueRefInt+ValueRefDouble in Python FOCS parser


* [FOCS] Update type stubs to support building conversion


* Implement ValueRefInt<=int value ref in Python FOCS parser


* Rename Design contidion to HasDesign in Python FOCS parser


* Rename resources Industry and Inluence to resolve ambiguousness with meters


* Fix master DummyParser


* Don't use UserString in Dump


* CI: Restore building Windows test artifacts (#5122)


* build MSVC project file with x64 SDK


* remove Win32 configuration from MSVC project files


* Set exception on Pending / promise if parser fails


* Gather info if all files were successfully parsed


* use span instead of vector for TagVecs (#5119)


* Test species dump on Windows CI


* CI: Restore Fedora Rawhode and Debian sid in Docker (#5117)


* Add parser_context attribute to Python FOCS parser builtins (#4853)



Allows to use code like:

```python
import builtins

if not hasattr(builtins, "parser_context"):
```

to exclude unneeded code for parser.

* CI: Reorganize Docker godot settings


* CI: Fix getting version for post


* CI: Fix snap upload


* CI: Fix snap artifact download


* CI: Fix typo


* CI: Fix missing runs-on job's property


* Add weekly test build Snap publication


* Name weekly test build artifacts after SourceForge target


* Make empire statistic parser synchronized


* Add helper function to convert python object to value ref


* Test empire statistic parser on default files


* Implement MoveTowards(speed, x, y) in Python FOCS parser


* [tools] Update buiding parser to filter unrelated log lines (#5104)


* Update fr.txt for release 0.5.1

- On par with latest en.txt in release-v0.5.1 branch (Feb 9, 2025 - 3bc8ea5)

- Can be safely cherry-picked to release-v0.5.1 branch ( @Vezzra )

* Remove duplicated build dependency in CI/CD Dockerfiles

Signed-off-by: Vincent Legoll <vincent.legoll@gmail.com>

* CI: Disable debian sid due gcc 15.0.1 ICE


* remove unnecessary additional lib dependency


* remove specific VCToolsVersion specifications from more configurations


* -fix Version.cpp.in git index dependency for several project configs
-remove specific VCToolsVersion specifications

* attempting to make MSVC project files work with SDK after Boost 1.84


* Set Windows10 minimum supported


* Enable tests for Macos 14


* Fix non-Framework installation of SDL library


* Update SDK to Release 15


* Test empire statistic parser on test files


* Implement unary -ValueRefDouble in Python FOCS parser


* Update 0.5.1 release date in changelog and org.freeorion.FreeOrion.metainfo.xml


* Implement Design condition in Python FOCS parser

* Add AddStarlanes and RemoveStarlanes effects to Python FOCS parser

* Fix fighter damage with scaling info in pedia



* Bump streamlit from 1.30.0 to 1.37.0 in /default/python/charting (#5096)
* Add requirements for Boost.Container library
* Add requirements for Boost.Random and Boost.Graph libraries
* Fix Void missing boost.python library






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


#### Content and Balance

- Government
    - 
    
- Ships
    - 
    
- Planets / Buildings
    - 

- Pedia and Stringtables
    - 

- Other
    - 

#### AI

- Update AI for fighter damage pilot trait

#### Other Features

- 

#### Other Bug Fixes

- Fix possible invalid iterator dereference while parsing text
- Fix sorting of system names with non-Latin characters

- Crashes or Disconnects
    - Fix crash due to logging in Sound::Impl destructor


- Gameplay
    - Allow multiple objects to be recorded as destroying an object, rather than picking just one that damaged a destroyed object in a combat round

- Content
    - 


#### Technical / Internal

- Refactor and make functions and classes usable and testable at compile-time (constexpr) including:
    - conditions, valuerefs, checksums, font / text layout
- Improve data packing in ValueRef with bitfields for flags
- Add runtime safety checks throughout code
    - While moving cursor through Edit control text
- Add compile-time static_asserts to verify portability
- Use constructor parameter flag types instead of ambiguous overloads
- Refactor GG Font classes / code
- Fix double-forwarding of arguments in AI order-issuing wrapper function
- Replace uses of deprecated functions
- Avoid copying strings unnecessarily
- Fix compile errors with Boost >= 1.87
- Pass ScriptingContext through call chains instead of repeatedly getting / constructing them
- Use ScriptingContext gamestate getters rather than global getters
- Replace some boost::bind with lambdas
- Fix variable shadowing warnings
- Convert more loops and algorithms to C++ ranges
- Copy string_view into a (null terminated) string before passing to sscanf
- Don't mark some functions noexcept when they probably aren't
- Remove UniverseObjectVisitor classes and related functions
- Further tranisition to Python-based parsing of content scripts