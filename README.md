# FreeOrion

FreeOrion is a free, Open Source, turn-based space empire and galactic conquest
computer game.

FreeOrion is inspired by the tradition of the Master of Orion games, but does
not try to be a clone or remake of that series or any other game.  It builds
on the classic *4X* (eXplore, eXpand, eXploit and eXterminate) model.

By adding scripting capabilities to the game engine the FreeOrion project aims
to give the community an easy way to customize the game mechanics and
presentation to create a living, breathing universe in a *grand campaign* model.


## Requirements

FreeOrion requires an *OpenGL 2.1* capable graphic card and a display with a
minimum resolution of at least *800x600*.

FreeOrion requires at least *Windows XP* with *Service Pack 2* or later,
*Mac OSX 10.9* or later or any reasonably recent Linux distribution on x86
compatible processors.  Other operating systems and architectures have reported
to be working by users, but are not actively supported by the FreeOrion project.


## Download

[FreeOrion Stable Releases] are the recommended way to play FreeOrion.  Stable
Releases can be obtained as native installer binaries for Windows and Mac OSX
or as source releases for Linux and other UNIX-ish platforms from GitHub.

Also some Linux distributions like Fedora, Debian and Arch do provide packages
of FreeOrion.

[FreeOrion Weekly Releases] are in-development releases intended for enthusiasts
and testers, who want to track or contribute to the development.  Weekly
Releases can be obtained as native installer binaries for Windows and Mac OSX
from Sourceforge.


## Install

For Windows execute the native installer binary and follow the on-screen
instructions of the installer to install FreeOrion.

For Mac OSX, open the downloaded DMG file and copy the contained FreeOrion
application to your system Applications folder by Drag and Drop.

For Linux or other from-source installations in general please refer to the
[Build Instructions](BUILD.md).

Various Linux distributions provide the stable release of FreeOrion in
a prebuilt way.  Usually you can install those packages by either using
a graphical package manager and searching for *FreeOrion* or by installing the
packages via the command line.

  * [Debian package] stable release: `# apt-get install freeorion`
  * [Ubuntu package] stable release: `# apt-get install freeorion`
  * [Fedora package] stable release: `# dnf install freeorion`
  * [Void package] stable release: `# xbps-install freeorion`
  * [ArchLinux package] stable release

## Directories
* install directory  
The location of this README.md file, called `<install_dir>` below.
* config - game settings  
Called `<config_dir>` below.  
    * linux  
`$XDG_CONFIG_HOME/freeorion` which defaults to `~/.config/freeorion`  
    * OSX  
`$HOME/Library/Application Support/FreeOrion/` which defaults to `~/Library/Application Support/FreeOrion/`  
    * Windows  
`$APPDATA\FreeOrion`  
* data - local user data, saved games, log files  
Called `<data_dir>` below.  
    * linux  
`$XDG_DATA_HOME/freeorion` which defaults to `~/.config/freeorion`  
    * OSX  
`$HOME/Library/Application Support/FreeOrion/` which defaults to `~/Library/Application Support/FreeOrion/`  
    * Windows  
`$APPDATA\FreeOrion`  
* resource directory - audio, visual and textual UI content, python scripts  
Called `<resource_dir>` below.  
`<install_dir>/default/`  
`<resource_dir>` can be redirected in `<config_dir>/config.xml` or `<config_dir>/persistent_config.xml`  
* stringtables - translation indices for various languages  
    `<resource_dir>/stringtables/`  
* scripting - FreeOrion Content Scripts (FOCS) describing game content (tech, species etc.)  
    `<resource_dir>/scripting/`  
* AI - AI for computer controlled empires  
    `<resource_dir>/python/AI`  


## Contact and Getting Help

Visit the [FreeOrion Homepage] to learn more about the project.  Also you can
get in touch with the FreeOrion developers and join the community in the
[FreeOrion Forum].


## Contribute

The FreeOrion project encourages anybody to contribute to FreeOrion. For more
details please see the [Contribution Guidelines](CONTRIBUTING.md).


## License

The FreeOrion *source code* is licensed under the terms of [GPL v2],
*game assets* are licensed under the terms of [CC-BY-SA-3.0] and *game content
scripts* are licensed under the terms of both [GPL v2] and [CC-BY-SA-3.0].
For more details please see the [License File](default/COPYING).

Additional to the immediate project sources, the FreeOrion source tree bundles
some third party projects or assets which may be also licensed under different
terms than the FreeOrion project.  For more details please consult the
accompanying license file.

  * *GiGi* library located within the `GG/` directory.
  * *Roboto* font located within the `default/data/fonts/` directory.
  * *DejaVuSans* located within the `default/data/fonts/` directory.


[FreeOrion Homepage]: http://www.freeorion.org/
[FreeOrion Forum]: http://www.freeorion.org/forum/
[FreeOrion Stable Releases]: https://github.com/freeorion/freeorion/releases
[FreeOrion Weekly Releases]: https://sourceforge.net/projects/freeorion/files/FreeOrion/Test/
[FreeOrion Development]: https://github.com/freeorion/freeorion
[Debian Package]: https://packages.debian.org/source/sid/freeorion
[Ubuntu Package]: https://launchpad.net/ubuntu/+source/freeorion
[Fedora Package]: https://admin.fedoraproject.org/pkgdb/package/rpms/freeorion/
[Void package]: https://github.com/voidlinux/void-packages/tree/master/srcpkgs/freeorion
[ArchLinux Package]: https://aur.archlinux.org/packages/freeorion/
[GPL v2]: https://www.gnu.org/licenses/gpl-2.0.txt
[CC-BY-SA-3.0]: https://creativecommons.org/licenses/by-sa/3.0/legalcode
