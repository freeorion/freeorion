# Build FreeOrion


## Hardware and OS Requirements

FreeOrion will compile on Windows 7 (or later), Mac OS X 10.10 (or later) and
Linux operating systems. Other operating systems have reported to work, but
support is not actively maintained by the FreeOrion developers. FreeOrion is
developed for x86 compatible processor architectures, other architectures
haven't been tested.

The FreeOrion source code makes heavy use of templates and requires much memory
to compile ; 6 GiB RAM or more is recommended. To build FreeOrion, expect up to
45 minutes on a Intel Core i5 system.


## Required Software Dependencies

FreeOrion depends on the following software to build:

  * [Visual Studio] - 2017 for Windows Desktop ; Windows only
  * [Xcode] - 8.3 or later ; Mac OS X only
  * [CMake] - 3.4 (Mac OS X) ; 3.1 or later (Other non-Windows)
  * A C++14 compliant compiler - Other Operating Systems
    * [GNU GCC] - 5.0 or later
    * [Clang] - 3.9 or later (4.0 or later on FreeBSD)
  * [Python] - 3.5.* or later
  * [Git]

FreeOrion depends on the following libraries or APIs to run:

  * OpenGL - 2.1 or later ; usually provided by the graphic card driver or
    Operating System
  * OpenAL - It's recommended to use the [OpenAL Soft] implementation
  * [Boost] - 1.58 or later
  * [zlib]
  * [libpython] - 3.5.* or later
  * [FreeType2]
  * [libpng]
  * [libogg]
  * [libvorbis]
  * [SDL2]


## Obtaining FreeOrion Source Code and Software Dependencies

For Windows and Mac OS X a [FreeOrion Software Development Kit] is provided as
download to compile FreeOrion from source. It contains the preconfigured and
-compiled build and runtime dependencies for the Visual Studio v140 toolchain on
Windows and Mac OS X 10.9 SDK with Xcode 6.4 or later on Mac OS X.

For Linux or other Operating Systems the build and runtime dependencies should
be installed by the preferred way for the respective OS (e.g. via Package
manager or compiling from source).

Step by step procedure:

 * On Windows:
   * Download the [FreeOrionSDK v11] from the FreeOrionSDK respository releases.
 * On Mac OS X:
   * The [FreeOrionSDK v11] is downloaded automatically when CMake creates the
     build environment.
 * Linux and other Operating Systems:
   * Install build and runtime dependencies by the preferred way for the
     respective OS.
 * Create a project directory `freeorion-project`.
 * On Windows:
   * Unzip the SDK archive contents into the project directory.
   * Execute the `bootstrap.bat` within the project directory. This will clone
     the FreeOrion repository and place the dependencies at the correct place.
   * If you want to create an out-of-source build using CMake, you should run 
     `git clone https://github.com/freeorion/freeorion.git FreeOrion` in the 
     `freeorion-project` directory, instead of running `bootstrap.bat`.
 * On Max OS X, Linux and other Operating Systems:
   * Navigate into the project directory.
   * Clone the project via Git:
     ```
     git clone https://github.com/freeorion/freeorion.git
     ```

This will leave you with the latest development branch `master` and the
FreeOrion source code in:

 * `freeorion-project/FreeOrion/` on Windows.
 * `freeorion-project/freeorion/` on Mac OS X, Linux and other Operating
   Systems.

This directory will be referred to as _source directory_ in the rest of the
document.

To build a specific release check out the desired version via `git checkout`:

```
git checkout vxxx
```

Where `vxxx` indicates the desired version.  A list of all available version
can be listet by invoking `git tag -l`, where releases are indicated by a
leading `v` followed by the release version number.


# Compiling FreeOrion

FreeOrion uses CMake as build system on most platforms. On Windows a manually
maintained Visual Studio Project is used for building.

Step by step procedure:

 * Enter the _source directory_.
 * On Windows:
   * Open `msvc2015\FreeOrion.sln` with Visual Studio.
   * Compile the whole project by selecting the `Build` -> `Build Solution`
     menu entry.

 * On Windows (cmake):
   * Change into the `Freeorion` directory.
   * Create a `build` directory, which will contain all compile FreeOrion
     build artifacs.
   * Change into the `build` directory on the command line.
   * Execute cmake to generate Makefiles:

     ```
     cmake .. -G "Visual Studio 15 2017"
     ```
   * Compile the whole project by calling `MSBuild.exe -p:Configuration=Release FreeOrion.sln`
     within the build directory. In case you want to utilize multiple CPU
     cores, you can add the `-m` option to the command.
   * Alternatively, you can compile the project by the Visual Studio GUI.


 * On Mac OS X:
   * Create a `build` directory, which will contain all compile FreeOrion
     build artifacs.
   * Change into the `build` directory on the command line.
   * Execute cmake to generate a Xcode project file:

     ```
     cmake -GXcode ..
     ```
   * Open `FreeOrion.xcodeproj` with Xcode.
   * Compile the whole project by selecting the `ALL_BUILD` scheme and
     pressing 'Command' + 'B'.

 * On Linux and other Operating Systems
   * Create a `build` directory, which will contain all compile FreeOrion
     build artifacs.
   * Change into the `build` directory on the command line.
   * Execute cmake to generate Makefiles:

     ```
     cmake ..
     ```
   * Compile the whole project by calling `make` within the build directory.
     In case you want to utilize multiple CPU cores by running parallel compile
     jobs check out the the [make jobs](`--jobs`) parameter of `make`.
   * Create a symbolic link to the data directory inside the build directoy
     by invoking:

     ```
     ln -s ../default .
     ```

This will leave you with a build of FreeOrion executables.

 * `freeorion-project/FreeOrion` on Windows if you compile it with the 
    standalone msvc project.
 * `freeorion-project/Freeorion/build/Release` on Windows if you compile it 
    with CMake. To run the executable without creating the symbolic link, you
    can first change the directory to `freeorion-project/Freeorion`, then run
    `./build/Release/FreeOrion.exe`.
 * `freeorion-project/build/Release` on Mac OS X.
 * `freeorion-project/freeorion/build` on Linux and other Operating Systems.


[Visual Studio]: https://www.visualstudio.com/de/vs/older-downloads/
[Xcode]: https://itunes.apple.com/de/app/xcode/id497799835?mt=12
[CMake]: https://cmake.org/download/
[GNU GCC]: https://gcc.gnu.org/releases.html
[Clang]: http://releases.llvm.org/download.html
[Python]: https://www.python.org/downloads/
[Git]: https://git-scm.com/downloads
[Boost]: http://www.boost.org/users/download/
[zlib]: https://zlib.net/
[libpython]: https://www.python.org/downloads/
[FreeType2]: https://www.freetype.org/download.html
[libpng]: http://www.libpng.org/pub/png/libpng.html
[libogg]: https://xiph.org/downloads/
[OpenAL Soft]: http://kcat.strangesoft.net/openal.html
[libvorbis]: https://xiph.org/downloads/
[SDL2]: https://www.libsdl.org/download-2.0.php
[FreeOrionSDK]: https://github.com/freeorion/freeorion-sdk
[FreeOrionSDK v11]: https://github.com/freeorion/freeorion-sdk/releases/tag/v11
[FreeOrion Releases]: https://github.com/freeorion/freeorion/releases
[make jobs]: https://www.gnu.org/software/make/manual/html_node/Parallel.html
