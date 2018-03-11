# Issue and Pull Request labels

This document describes the intended usage of GitHub Issue and Pull Request
labels within the FreeOrion project.  In this document when referring to
__Issues__ GitHub Issues are meant and when referring to __PR__ GitHub
Pull Requests are meant.


## Purpose of labels

Labels allow attaching common search and filter terms to Issues/PRs.  This makes
it easier for developers, managers and reporters to track the purpose, scope and
state of Issues/PRs without explicitly reading through lengthy comments, code
reviews or commits.  Please follow this guide as closely as possible to avoid
duplicating work and wasting time of other developers and to enable better
documentation on decisions made.


## Label categories

Labels are grouped in categories.  Those categories group together a common
concept like the current processing state of an Issue/PR or the project
component that is affected by the Issue/PR occurs.  Categories share a prefix in
the form `category_name:label_name` and are grouped together by color.  As a
rule of thumb there is at least one label from every category that needs to be
applied to an issue but use common sense to check if the label is appropriate or
not for the specific Issue/PR.

Currently there are three categories:


#### Label category `category`

Labels within this category describe the intent of the Issue/PR.  Usually there
should be exactly one intent for creating an Issue/PR and it should not change
during the lifetime of an Issue/PR.

#### Label category `component`

The component lists the parts of the project this Issue/PR affects or is
affected by.  This should help developers to identify if they have required
knowledge or interests to handle the issue.  There should be at least one
component listed for an Issue.

#### Label category `status`

With the status category developers, managers or issue reporters can check if
an issue needs further interaction and who should carry out this interaction.
An Issue/PR usually has exactly one status label, or no status label if the
Issue/PR is open/unmerged and requires no special handling.  Status may change
over the lifetime of the Issue/PR.


## Labels

This section lists when an label should be applied and what additional actions
may be taken.


#### Label `category:bug`

The Issue/PR describes or solves a perceived malfunction within the game.

#### Label `category:feature`

The Issue/PR describes or implements a new game feature.  Features are parts of
the game which a player can interact with e.g.: A new species, a new gameplay
mechanic or a new command line switch for the server that changes gameplay
behavior.

#### Label `category:refactoring`

The Issue/PR describes or contains a new or improved implementation of an
existing feature or any other code restructuring that doesn't modify the current
game mechanics or representation.

#### Label `category:tweak`

The PR contains insignificant code changes that don't involve features,
refactoring or bug fixing but code style grooming or value tweaking.

#### Label `component:AI`

The Issue/PR deals with the Python AI decision making code or affects it.

#### Label `component:art music text assets`

The Issue/PR deals with creating or modifying game assets or verification if the
assets match the style goals of the game.

#### Label `component:build system`

The Issue/PR deals with CMake, Visual Studio, Xcode, the respective platform
SDKs or the general build process.

#### Label `component:content scripting`

The Issue/PR deals with the FOCS language, turn events, the universe generator
and probably game mechanics.

#### Label `component:deployment`

The Issue/PR deals with the deployment process or the software used to create
the installer or other distribution packages.

#### Label `component:game mechanic`

The Issue/PR deals with the currently used or planned game mechanics.  The
content of the PR may require extensive gameplay balance testing.

#### Label `component:infrastructure`

The Issue/PR deals with non-game related infrastructure that aids development.
For example this includes issues with the project homepage, used external
services like continuous integration, hosting of release binaries, tools for
checking code style, etc.

Infrastructure Issues/PRs should not entail/contain any changes to the actual
codebase unless such changes are essentially required by the Issue/PR and would
not make sense outside of its context. Any such non-codebase Infrastructure
Issues/PRs should not be assigned a milestone unless they have particular
relevance to the milestone, such as by establishing some type of
testing/verification of the codebase which is desired to be completed for the
milestone. 

#### Label `component:internal`

The Issue/PR deals with any project component that has no explicit `component`
label.

#### Label `component:network`

The Issue/PR deals with TCP/IP networking and establishing a test network
infrastructure.

#### Label `component:technical docs`

The Issue/PR deals with technical documentation like API docs, style guides,
workflow documentation or similar documentation where the intended target
audience are developers, contributors or managers.

#### Label `component:translation`

The Issue/PR deals with the game style, game terminology, stringtable syntax
and the translation process in general.  It maybe helpful to know the language
of the translation to proofread.

#### Label `component:UI`

The Issue/PR deals with the game interface, a consistent representation style
in-game, the expected user experience and graphical APIs used (GiGi and OpenGL).

#### Label `component:user manual`

The Issue/PR deals with end-user documentation where the target audience are
mainly players and server hosters.  When writing _in-game_ manuals this label
should be used in conjunction with `component:art music text assets`; when this
Issue addresses external documentation like the FO wiki add the
`component:infrastructure` label.

#### Label `status:cherry-pick for release`

The PR should be applied to the currently open release branch.  Use this label
sparsely, only for bug fixes, and document a minimal set of commits that should
be considered by the release manager for cherry-picking.  Cherry-picking
PRs onto release branches should fix bugs, not introduce new ones.

#### Label `status:duplicate`

The topic of the Issue/PR is or was already covered by another Issue/PR.  Add
the number of the covering Issue/PR in a comment and close this Issue/PR.  For
PRs document why the other PR was chosen over this PR.  For Issues copy over
new knowledge that is valuable to solve the covering Issue.

#### Label `status:superseded`

The PR discussion showed that the proposed implementation is insufficient in
some way or the PR in itself is malformed.  Usually not required as we prefer
rebasing within the same PR to update the implementation.  When applying this
label reference the superseding PR number in the superseded one, add a comment
why the superseded PR requires a follow up and close the PR.

#### Label `status:help wanted`

The Issue/PR is classified by one or more developers as a valid Issue or as
a generally acceptable implementation but there is currently no manpower
available to review the PR in detail or to fix the underlying bug in an Issue or
to tidy up the PR implementation.  Other developers or external contributors are
welcome to take over this Issue/PR.

#### Label `status:invalid`

The Issue is not classified as a formal valid Issue report or applicable PR
There can be many reasons for this:

* The Issue isn't describing a bug but rather intended behaviour.
* The Issue opposes the design goals of the game.
* The Issue contains SPAM.
* The description is too sparse to make sense out of the intent of the Issue
  and the Reporter is unresponsive or unwilling to give further details.
* The feature implemented wasn't discussed previously with the team.
* The PR is a merge salad à la Chef and the developer is unable or unwilling to
  fix this.
* The PR contains intentionally game-breaking commits or offending changes and
  the developer is unwilling to change those.

Issues labeled as `status:invalid` shouldn't be merged and should be closed.

#### Label `status:merged`

All relevant commits of this PR were merged into the master development branch.
This can either happen by merging, cherry-picking or rebasing and pushing onto
master.  Close the PR if GitHub didn't close it automatically.

#### Label `status:resolved`

The Issue was resolved satisfactorily, either by answering properly or fixing
the underlying bug.  Close the Issue.

#### Label `status:testing requested`

The Implementation is ready for testing but can't be tested sufficiently by the
developer and they request testing by other developers.  Use this in case of
platform specific code or bug fixing when the bug is not reproducible on the
developer's machine.

#### Label `status:won't fix`

The Issue describes a bug but won't be fixed by the developers because of the
implementation or maintainance effort required, or it being deemed impossible or
problematic to resolve.  For example, if a valid Issue for an old and
unsupported version is written it would be labeled as `status:won't fix`.

Add a comment explaining why the Issue won't be fixed and close the Issue.  This
is __NOT__ a way to brush away user concerns or bad user experience so rethink
if the Issue can be avoided by redesigning the application part in question in
another way that the Issue may suggest.

#### Label `status:workaround`

The Issue/PR provides a workaround for the underlying problem.  This can be
advised user behaviour to avoid the problem in case of an Issue.  For PRs this
may mean it contains an implementation that solves the Issue but it doesn't
solve in a proper way and need a proper revision or refactoring.  Apply this
label when solving the Issue has higher priority than providing a proper fix.
This of course means that it should be used sparsely.

#### Label `status:work in progress`

The PR contains some implementation but isn't ready for merging onto the main
development branch.

#### Label `status:works for me`

The Issue can't be reproduced on a developer machine or the Issue doesn't
contain sufficient information to reproduce it.  Ask for more information from
the Issue reporter and if they don't respond in an appropriate period of time
close the Issue.
