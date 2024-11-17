---
name: Publish release
about: "Team only: Create a tracking issue for a FreeOrion release."
title: Release X.Y.Z preparation
assignees: Vezzra
---
<!--
Please replace X, Y and Z with the major, minor and patch release version
number in the title above.

The X.Y.Z convention as placeholder for the actual version number is used
thoughout this document.
-->

This issue is used to organize the version X.Y.Z release of FreeOrion.


### Milestones

Important milestones for the release.  Changes will be announced by the release
manager.

<!--
Add all relevant milestones ordered by date or sequence.  Use the YYYY-mm-dd
format for the date.  When a milestone entry is postponed copy the whole entry
and add it below the original entry.  Use ~~ to strike out the date in the
original entry and add a **POSTPONED** in front of the original milestone.

The time references used are mere suggestions.  'SP' stands for starting point,
the first milestone of the release.  '+1W', '+2W' should be interpreted as
'one week after SP', 'two weeks after SP' and so on.
-->

| Date             | Milestone                         |
|:---------------- | --------------------------------- |
| <!-- SP  -->     | Create release branch from master |
| <!-- +1W -->     | Release Candidate 1               |
| ~~<!-- +2W -->~~ | **POSTPONED** Release Candidate 2 |
| <!-- +3W -->     | Release Candidate 2               |
| <!-- +3W -->     | Declaring Release                 |


### Checklist

This is a checklist for the release manager to ensure all steps are done before
doing the actual release.


#### Preparations

* [ ] Create a new GitHub Issue using `Publish release` template and follow
      instructions. :wink:
* [ ] Determine initial milestones and dates for release.
* [ ] Add the new release in `packaging/org.freeorion.FreeOrion.metainfo.xml`
* [ ] Update the screenshots in `packaging/org.freeorion.FreeOrion.metainfo.xml` if necessary
* [ ] Create release branch from development branch master by using:
```
git checkout -b release-vX.Y.Z master
```
* [ ] *On release branch HEAD* - Update version number in `CMakeLists.txt` and
      `cmake/make_versioncpp.py` to `vX.Y.Z`, remove trailing `+` if needed.
* [ ] *On release branch HEAD* - Disable `Super Testers` species by default.
* [ ] *On master branch HEAD* - Update version number in `CMakeLists.txt` and
      `cmake/make_versioncpp.py` to `vX.Y.Z+`.
* [ ] Push updated release and master branch to GitHub.
* [ ] Update `ChangeLog.md`.


#### Release candidate #

<!-- Copy this section if you plan to release multiple release candidates -->

* [ ] Integrate incoming bugfixes.
* [ ] Update `ChangeLog.md` again if needed.
* [ ] Update the release date in `packaging/org.freeorion.FreeOrion.metainfo.xml` if necessary
* [ ] Update the screenshots in `packaging/org.freeorion.FreeOrion.metainfo.xml` if necessary
* [ ] *On release branch HEAD* - Tag the release candidate by using:
```
git tag --annotate --message="X.Y.Z Release Candidate #"  vX.Y.Z-rc#
```
* [ ] Push release candidate tag to GitHub.
* [ ] Build release candidate artifacts.
* [ ] Upload release candidate artifacts to SourceForge.
* [ ] If this is the first release candidate, announce availability of
      release candidates in `News` section of FreeOrion Wiki.
* [ ] Announce release candidate on Twitter as @FreeOrion
* [ ] Test release candidate.


#### Declare release

* [ ] Determine if another release candidate is needed or if the latest release
      candidate is ready to be declared as the official stable release.
* [ ] *On release branch* - Tag the release from the latest release candidate
       by using:
```
git tag --annotate --message="X.Y.Z Stable Release"  vX.Y.Z
```
* [ ] Push release tag to GitHub.
* [ ] Write `Key Changes` description for GitHub releases page.
* [ ] Upload release artifacts to GitHub.
* [ ] Announce release in `News` section of FreeOrion Wiki.
* [ ] Update `Main` page of FreeOrion Wiki.
* [ ] Update `Download` page of FreeOrion Wiki.
* [ ] Announce release on Twitter as @FreeOrion
* [ ] Announce release on YouTube.


#### Cleanup

* [ ] Cherry-pick updates of `ChangeLog.md` to master.
* [ ] Prune release candidate tags.
* [ ] Prune release branch.


### Blocking Issues and PRs

<!--
Provide link to filtered list of issues which are blocking the release
-->
[Issues blocking vX.Y.Z](https://github.com/freeorion/freeorion/milestone/<release_milestone#>)
