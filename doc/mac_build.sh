#! /bin/bash

# Install Dependencies
echo "Installing dependencies..."
brew update
brew install cmake git gettext autoconf pkg-config
brew upgrade
brew install opencascade OpenEXR OpenVDB CURL nlopt Cereal tbb wxWidgets icu4c
brew link icu4c --force

# Check out the source code
git checkout nonplanar

# Build Dependencies
echo "Building dependencies..."
cd PrusaSlicer/deps || exit
mkdir build
cd build || exit
cmake .. -DDEP_WX_GTK3=ON -DDEP_DOWNLOAD_DIR=$(pwd)/../download
make -j
cd ../..
# Build PrusaSlicer
echo "Building PrusaSlicer..."
mkdir build
cd build || exit
cmake .. -DSLIC3R_STATIC=1 -DSLIC3R_GTK=3 -DSLIC3R_PCH=OFF -DCMAKE_PREFIX_PATH=$(pwd)/../deps/build/destdir/usr/local
make install -j8
brew cleanup -q
src/prusa-slicer
