Building FreeOrion
==================

These notes are applicable for building the master development Git branch
of FreeOrion. For building numbered FreeOrion releases, consult the
corresponding release branch, eg. [FreeOrion v0.5 BUILD.md]

Hardware and OS Requirements
----------------------------

FreeOrion should compile on Windows 8.1 (or later), macOS 11 (or later) and
Linux operating systems. Other operating systems have reported to work, but
support is not actively maintained by the FreeOrion developers. FreeOrion is
developed for x86 compatible processor architectures; other architectures
haven't been tested.

The FreeOrion source code makes heavy use of templates and requires much memory
to compile; 16 GiB RAM or more is recommended. Expect up to 45 minutes on a Intel
Core i5 system, or about 5 min on a 12-core AMD Ryzen 9 3.8 GHz system.


Required Software Dependencies
------------------------------

FreeOrion depends on the following software to build:

  * [Visual Studio] - 2022 for Windows Desktop ; Windows only
  * [Xcode] - 13.2 or later ; MacOS only
  * [CMake] - 3.8 or 3.16 (Windows) or later
  * A C++20 compliant compiler - Other Operating Systems
    * [GNU GCC] - 11 or later
    * [Clang] - 10 or later
  * [Python] - 3.9 or later
  * [Git]

FreeOrion depends on the following libraries or APIs to run:

  * OpenGL - 2.1 or later ; usually provided by the graphic card driver or
    Operating System
  * OpenAL - It's recommended to use the [OpenAL Soft] implementation
  * [Boost] - 1.73 or later
  * [zlib]
  * [libpython] - 3.9 or later
  * [FreeType2]
  * [libpng]
  * [libogg]
  * [libvorbis]
  * [SDL2]


Obtaining FreeOrion Source Code and Software Dependencies
---------------------------------------------------------

For Windows, MacOS, and Android, a [Software Development Kit] is provided as download to
assist with compiling FreeOrion from source. The SDK contains the preconfigured and
-compiled build and runtime dependencies for Visual Studio on Windows, the
MacOS 10.12 SDK with Xcode 10.1 or later on macOS and Android NDK r24.

For Linux or other Operating Systems, the build and runtime dependencies should
be installed by the preferred way for the respective OS (e.g. via Package
manager or compiling from source).

Step by step procedure for the development master-branch version:

 * On Windows:
   * Download the [FreeOrionSDK v14] from the FreeOrionSDK respository releases.
 * On MacOS:
   * The [FreeOrionSDK v14] is downloaded automatically when CMake creates the
     build environment.
 * For Android:
   * Download the [FreeOrionSDK v14] of appropriate architecture and Python standard library
     from the FreeOrionSDK respository releases.
 * Linux and other Operating Systems:
   * Install build and runtime dependencies by the preferred way for the
     respective OS.
 * Create a project directory `freeorion-project`.
 * On Windows:
   * Unzip the SDK archive contents into the project directory.
   * Execute the `bootstrap.bat` within the project directory. This will clone
     the FreeOrion repository and place the dependencies at the correct place.
   * If you want to create an out-of-source build using CMake, you should run 
     `git clone git@github.com:freeorion/freeorion.git FreeOrion` in the 
     `freeorion-project` directory, instead of running `bootstrap.bat`.
 * For Android:
   * Unzip the SDK archive contents into the project directory.
 * On MaxOS, Linux and other Operating Systems:
   * Navigate into the project directory.
   * Clone the project via Git:
     ```
     git clone git@github.com:freeorion/freeorion.git
     ```

This will leave you with the latest development branch `master` and the
FreeOrion source code in:

 * `freeorion-project/FreeOrion/` on Windows.
 * `freeorion-project/freeorion/` on MacOS, Linux and other Operating
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


Compiling FreeOrion
-------------------

FreeOrion uses CMake as build system on most platforms. On Windows a manually
maintained Visual Studio Project is used for building.


### Windows (Visual Studio)

To build FreeOrion open the `FreeOrion.sln` project solution within
`_source directory_\msvc2022` with Visual Studio.  Make sure that the
platform configuration (Win32 or x64) matches the version of the
FreeOrion SDK that you downloaded. Compile the whole project by
selecting the `Build` -> `Build Solution` menu entry.

After the build finished successfully the binaries can be found within
the `freeorion-project/FreeOrion` directory.

### Windows (CMake)

After preparing the SDK as above, open a Visual Studio x86 Native Tools command prompt (from the start menu Visual Studio folder), and change to the _source_directory_.

Create a `build` directory inside the _source_directory_ and change into
this directory. It will contain all compile FreeOrion build artifacs:

```
mkdir build
cd build
```

Execute cmake to generate a Visual Studio solution, eg:

```
cmake .. -G "Visual Studio 17 2022" -T v143 -A x64 -DBUILD_TESTING=On -DCMAKE_C_FLAGS=/MP -DCMAKE_CXX_FLAGS=/MP
```

Then run the build:

```
cmake --build . --config "Release" -- /maxcpucount
```

Alternatively, you can build FreeOrion by opening the cmake-generated
`FreeOrion.sln` project solution with Visual Studio.  Now compile the
whole project by selecting the `Build` -> `Build Solution` menu entry.

After the build finished successfully, the executable binaries can be
found within the `freeorion-project/Freeorion/build/Release` directory.
That directory will also contain a symbolic link to the default
resources directory, `freeorion-project/Freeorion/default`, allowing
the FreeOrion client to be run from that location.


### Mac OS X

Create a `build` directory aside the _source_directory_ and change into
this directory. It will contain all compile FreeOrion build artifacs.

Execute `cmake` to generate a Xcode project file:

```bash
cmake -G Xcode ../freeorion
```

After successfully creating the Project file, open `FreeOrion.xcodeproj`
with Xcode. Now compile the whole project by selecting the `ALL_BUILD`
scheme and pressing 'Command' + 'B'.

After the build finished successfully the binaries can be found within
the `freeorion-project/build/Release` directory.


### Linux and other Operating Systems

Create a `build` directory aside the _source_directory_ and change into
this directory. It will contain all compile FreeOrion build artifacs.

Execute `cmake` to generate Makefiles:

```bash
cmake ../freeorion
```

After successfully creating the Makefiles build the whole project by
calling:

```bash
make
```

In case you want to utilize multiple CPU cores by running parallel
compile jobs check out the [make jobs](`--jobs`) parameter of
`make`.

After the build finished successfully the binaries can be found within
the `freeorion-project/build` directory.

### Android

Create a `build` directory aside the _source_directory_ and change into
this directory. It will contain all compile FreeOrion build artifacs.

```bash
cmake -DANDROID_ABI=<android arch> -DANDROID_PLATFORM=24 -DANDROID_NDK=<android ndk path> -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=<android>/build/cmake/android.toolchain.cmake -DCMAKE_CXX_FLAGS=-std=c++14 -DANDROID_ALLOW_UNDEFINED_SYMBOLS=Off -DBUILD_SERVER=OFF -DBUILD_AI=OFF -DBUILD_CLIENT_GG=OFF -DBoost_INCLUDE_DIR=<sdk>/include/ -DBoost_USE_STATIC_LIBS=On -DBoost_LIBRARY_DIR=<sdk>/lib/ -DBUILD_CLIENT_GODOT=On -DICUI18N_LIBRARY=<sdk>/lib/libicui18n.a -DICUUC_LIBRARY=<sdk>/lib/libicuuc.a -DICUDATA_LIBRARY=<sdk>/lib/libicudata.a -DICONV_LIBRARY=<sdk>/lib/libiconv.so -DPYTHON_LIBRARY=<sdk>/lib/libpython3.9.a -DPYTHON_INCLUDE_DIR=<sdk>/include/python3.9/ ../freeorion
```

After successfully creating the Makefiles build the whole project by
calling:

```bash
cmake --build . -- -j2 VERBOSE=1
```

After the build finished successfully the godot libraries can be found within
the `freeorion-project/build` directory and copied to `godot/bin/`.

Call `strip` on those libraries to clean debug symbols.

Copy Python standard library to `godot/default/python/lib/python39.zip`

Export [Godot-Export-Android] with additional `default/*` resources.

To get logs run:

```bash
adb exec-out run-as org.godotengine.freeoriongodotclient cat files/freeorion-godot.log
```

[Visual Studio]: https://visualstudio.microsoft.com/vs/
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
[OpenAL Soft]: https://openal-soft.org/
[libvorbis]: https://xiph.org/downloads/
[SDL2]: https://www.libsdl.org/download-2.0.php
[Software Development Kit]: https://github.com/freeorion/freeorion-sdk
[FreeOrionSDK v14]: https://github.com/freeorion/freeorion-sdk/releases/tag/v14
[FreeOrion Releases]: https://github.com/freeorion/freeorion/releases
[make jobs]: https://www.gnu.org/software/make/manual/html_node/Parallel.html
[Python-For-Android]: https://github.com/python-cmake-buildsystem/python-cmake-buildsystem/pull/262
[Boost-For-Android]: https://github.com/moritz-wundke/Boost-for-Android
[FreeOrion v0.5 BUILD.md]: https://github.com/freeorion/freeorion/blob/release-v0.5/BUILD.md
