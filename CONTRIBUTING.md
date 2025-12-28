Contributing to FreeOrion
=========================

As an open source project, FreeOrion both relies on contributions from long time
developers and one time contributors.  This guide should help you to create
contributions that can be easily integrated into the FreeOrion project.


I found an error in the game or I want to file a bug report
-----------------------------------------------------------

So you stumbled on a reproducible error in FreeOrion?  You can report it in
[FreeOrion Issues].  Please check before reporting the error if
someone else has already posted it.  When creating a new issue, you will see a
template of required and suggested information.  Please fill in the template
with all the information that you can, as applicable and appropriate.

After reporting the error, a developer will review the report and may ask
you for further information and feedback, so please stay responsive.
Follow up may take some time, however, depending on when someone has time to
review the submission.


I want to contribute code
-------------------------

We gladly accept useful additions to the project code and scripts.  However, before
you start developing something, please inform the developers about your endeavours
to avoid duplicating work.  If you're trying to fix a reported bug, please state
that you're working on the bug.  If you want to implement a new feature, get in
touch with the developers via the [FreeOrion Forum] to check if the feature complies
with our idea of the game.

To start contributing, first clone [FreeOrion Git] repository by following the
[Build Instructions](BUILD.md).  After that, create a branch with a concise name;
if the branch implements an issue call it `fix-<Issue Number>`.  To publish your
contribution, create a [Pull Request] and wait for feedback from the developers.

By submitting a pull request, you agree to license its contents under the applicable
[FreeOrion license(s)].

To ensure your PR won't be rejected, please make sure that:

* The code changes match the [Code Standards].  For Python code we follow
  [PEP 8].
* The commits of the PR are atomic.  This means that one commit should only
  contain a single logical and indivisible change.  If you need to describe the
  change inside the commit with a list or any other enumeration, you're probably
  doing it wrong.  Avoid lumping together functional changes and code style
  changes; whenever possible, put style changes into a separate commit.
* The [Commit Messages] follows the rules for a great Git commit message.
* The PR **IS** concise on the topic it addresses.  If you want to address
  different topics, create different branches and PRs for them.  If the
  topics/PRs relate to each other, document this in the PR message.
* The PR **DOES NOT** contain any merge commits from any other branch.  If you
  want to update your branch to a more recent master, use the `git rebase`
  command.

To build FreeOrion please refer to the [Build Instructions](BUILD.md).


Installing pre-commit hooks (optional)
--------------------------------------
Pre-commit hook is a good way to get feedback fast.
Hooks implement a subset of checks that run on the CI, so it will jus save your time.

Hooks work on all operating systems.
You need to install them once, pre-commit will track config file and update itself in case of changes.

- install git
- install Python
- [Install prek](https://github.com/j178/prek?tab=readme-ov-file#installation)
   ```shell
    pip install prek
    ```
- install hooks from the project root
    ```sh
    prek install
    ```

Notes:
- Hooks might change your files, you will need to add (`git add`) changes to commit.
- First commit after installation/update of hooks may take some time for installing tools.
- You can skip checks by adding `-n, --no-verify` to `git commit`. This is useful when you commit partial done job.

When you commit, hooks are run only on the changed files, if you want to run an all files:
```shell
prek run --all-files
```


Further documentation
---------------------

There are further specialized documents available that should be considered
when working within the FreeOrion project.  Those are:

* [GitHub Issue/PR Labels] - How to properly label Issues/PRs on GitHub.
* [Release Issue template] - Template for release checklist Issues.


[FreeOrion Git]: https://github.com/freeorion/freeorion.git
[FreeOrion Issues]: https://github.com/freeorion/freeorion/issues
[FreeOrion Forum]: http://www.freeorion.org/forum/
[GitHub Issue/PR Labels]: .github/labels.md
[Release Issue template]: .github/RELEASE_TEMPLATE.md
[Code Standards]: http://www.freeorion.org/index.php/Code_Standards
[Pull Request]: https://help.github.com/articles/proposing-changes-to-your-work-with-pull-requests/
[Commit Messages]: http://chris.beams.io/posts/git-commit/
[PEP 8]: https://www.python.org/dev/peps/pep-0008/
[FreeOrion license(s)]: https://github.com/freeorion/freeorion/blob/master/default/COPYING
