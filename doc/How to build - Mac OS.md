
# Building PrusaSlicer on Mac OS

To build PrusaSlicer on Mac OS, you will need the following software:

- XCode
- git
- brew

XCode is available through Apple's App Store, the other three tools are available on
[brew](https://brew.sh/) (use `brew install cmake git gettext` to install them).

### Dependencies

PrusaSlicer comes with a set of CMake scripts to build its dependencies, it lives in the `deps` directory.
Open a terminal window and navigate to PrusaSlicer sources directory.
Use the following commands to build the dependencies:

```sh
cd deps
mkdir build
cd build
cmake ..
make
```

This will create a dependencies bundle inside the `build/destdir` directory.
You can also customize the bundle output path using the `-DDESTDIR=<some path>` option passed to `cmake`.

**Warning**: Once the dependency bundle is installed in a destdir, the destdir cannot be moved elsewhere.
(This is because wxWidgets hardcodes the installation path.)

FIXME The Cereal serialization library needs a tiny patch on some old OSX clang installations
https://github.com/USCiLab/cereal/issues/339#issuecomment-246166717


### Building PrusaSlicer

If dependencies are built without errors, you can proceed to build PrusaSlicer itself.
Go back to top level PrusaSlicer sources directory and use these commands:

```sh
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH="$PWD/../deps/build/destdir/usr/local"
```

The `CMAKE_PREFIX_PATH` is the path to the dependencies bundle but with `/usr/local` appended - if you set a custom path
using the `DESTDIR` option, you will need to change this accordingly. **Warning:** the `CMAKE_PREFIX_PATH` needs to be an absolute path.

The CMake command above prepares PrusaSlicer for building from the command line.
To start the build, use
```sh
make -jN
```

where `N` is the number of CPU cores, so, for example `make -j4` for a 4-core machine.

Alternatively, if you would like to use XCode GUI, modify the `cmake` command to include the `-GXcode` option:
```sh
cmake .. -GXcode -DCMAKE_PREFIX_PATH="$PWD/../deps/build/destdir/usr/local"
```

and then open the `PrusaSlicer.xcodeproj` file.
This should open up XCode where you can perform build using the GUI or perform other tasks.

### Note on Mac OS X SDKs

By default PrusaSlicer builds against whichever SDK is the default on the current system.

This can be customized. The `CMAKE_OSX_SYSROOT` option sets the path to the SDK directory location
and the `CMAKE_OSX_DEPLOYMENT_TARGET` option sets the target OS X system version (eg. `10.14` or similar).
Note you can set just one value and the other will be guessed automatically.
In case you set both, the two settings need to agree with each other. (Building with a lower deployment target
is currently unsupported because some of the dependencies don't support this, most notably wxWidgets.)

Please note that the `CMAKE_OSX_DEPLOYMENT_TARGET` and `CMAKE_OSX_SYSROOT` options need to be set the same
on both the dependencies bundle as well as PrusaSlicer itself.

Official macOS PrusaSlicer builds are currently (as of PrusaSlicer 2.5) built against SDK 10.12 to ensure compatibility with older Macs.

_Warning:_ XCode may be set such that it rejects SDKs bellow some version (silently, more or less).
This is set in the property list file

    /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Info.plist

To remove the limitation, simply delete the key `MinimumSDKVersion` from that file.

## Troubleshooting

### `CMath::CMath` target not found

At the moment (20.2.2024) PrusaSlicer cannot be built with CMake 3.28+. Use [CMake 3.27](https://github.com/Kitware/CMake/releases/tag/v3.27.9) instead.
I'm building with CMake 3.30
If you install the CMake application from [universal DMG](https://github.com/Kitware/CMake/releases/download/v3.27.9/cmake-3.27.9-macos-universal.dmg), you can invoke the CMake like this:

```sh
/Applications/CMake.app/Contents/bin/cmake
```

### Running `cmake -GXCode` fails with `No CMAKE_CXX_COMPILER could be found.` 

- If XCode command line tools wasn't already installed, run:
    ```sh
     sudo xcode-select --install
    ```
- If XCode command line tools are already installed, run:
    ```sh
    sudo xcode-select --reset
    ```

# TL; DR

Works on a fresh installation of MacOS Catalina 10.15.6

- Install [brew](https://brew.sh/) and add it to path
- Open Terminal

- Enter:

```sh
brew update
brew install cmake git gettext autoconf
brew upgrade
brew install opencascade OpenEXR OpenVDB CURL nlopt Cereal tbb wxWidgets icu4c
brew link icu4c --force
git clone https://github.com/TimTheBig/PrusaSlicer/
git checkout nonplanar
cd PrusaSlicer/deps
mkdir build
cd build
cmake .. -DDEP_WX_GTK3=ON -DDEP_DOWNLOAD_DIR=$(pwd)/../download
make -j
cd ../..
mkdir build
cd build
cmake .. -DSLIC3R_STATIC=1 -DSLIC3R_GTK=3 -DSLIC3R_PCH=OFF -DCMAKE_PREFIX_PATH=$(pwd)/../deps/build/destdir/usr/local
make install -j8
src/prusa-slicer
```
manually removing the forced `-licudata -licui18n -licuuc` and `-lzstd` in my generated Makefile

<!-- todo grep cmd to replace maches with "" -->
removed with:
```regx
(-licudata -licui18n -licuuc)|((-l)icudata|icui18n|icuuc)|-lzstd
```
and
-lTKSTEP -> -llibTKSTEP\
in all link.txt's

wight Depname.patch for:

in deps/build/dep_Boost-prefix/**: piecewise_construct -> piecewise_construct_t
in deps/build/destdir/usr/local/include/boost/functional.hpp:
"unary_function" -> "__unary_function"
"binary_function" -> "__binary_function"
in deps/build/destdir/usr/local/include/openvdb/version.h on line 59:
"#define OPENVDB_LIBRARY_MAJOR_VERSION_NUMBER 8" -> "#define OPENVDB_LIBRARY_MAJOR_VERSION_NUMBER 11"
in **/build.make:
"OpenEXR::lib-NOTFOUND" -> "/opt/homebrew/Cellar/openexr/3.2.4/lib/libOpenEXR-3_2.31.3.2.4.dylib"