<!--
Please  use the issue title 'Release X.Y.Z preparation' and replace X, Y and Z
with the major, minor and patch release version number.

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

* [ ] Create a new GitHub Issue from `.github/RELEASE_TEMPLATE.md` and follow
      instructions. ;)
* [ ] Determine initial milestones and dates for release.
* [ ] Create release branch from development branch master by using:

```
git checkout -b release-vX.Y.Z master
```
* [ ] *on release branch* - Update version number in `CMakeLists.txt` to
      `vX.Y.Z`, remove trailing `+` if needed.
* [ ] *on release branch* - Remove `Supertesters` species.
* [ ] *on master branch* - Update version number in `CMakeLists.txt` to
      `vX.Y.Z+`.
* [ ] Push updated release and master branch to GitHub.
* [ ] Update `ChangeLog.md`.
* [ ] Build release candidate artifacts.
* [ ] Upload release candidate artifacts to SourceForge.
* [ ] Announce release candidate in `News` section of FreeOrion Wiki.
* [ ] Build release artifacts for all platforms.
* [ ] *on release branch* - Tag the release by using:
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


### Blocking issues

<!--
List the issue numbers that blocking the release
-->
