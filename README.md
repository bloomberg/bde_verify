BDE Verify - A Static Checker for C++

Bde_verify is a static analysis tool that verifies that source code adheres
to the BDE coding standards.

The online documentation for bde_verify can be found here:
    <http://bloomberg.github.io/bde_verify/bde_verify_build/html/>

Build Instructions
==================

Supported Platforms:
- Linux x86
- SunOS sparc
- Windows

Prerequesites: 
- llvm/clang 9.0 (see below for instructions)
- gcc >= 5 (Linux and SunOS)
- Visual Studio >= 2015 (Windows)
- nsis with large-string overlay (Windows, more details below)
- cmake >= 3.4.3
- gnu make
- aspell

Bde_verify incorporates llvm/clang-9.0 libraries.  These may already be
installed on the build machine.  If not, they can be installed (in the
Bloomberg environment) using dpkg, or built from source on Windows.

Installing bde_verify Sources
=============================

    # We will place bde_verify source code here
    export BVSRC=/path/to/bde_verify/source/directory
    mkdir -p $BVSRC
    git clone -b master https://bbgithub.dev.bloomberg.com/bde/bde-verify $BVSRC

Installing LLVM/Clang Sources
=============================
Note: this requires a great deal of free disk space.

    # We will place LLVM/Clang source code here
    export LCSRC=/path/to/source/directory
    mkdir -p $LCSRC

    # Clone the LLVM/Clang repositories
    export GL=http://github.com/llvm-mirror
    git clone -b release_90 $GL/llvm.git  $LCSRC/llvm
    git clone -b release_90 $GL/clang.git $LCSRC/llvm/tools/clang
    git clone -b release_90 $GL/clang-tools-extra.git \
                                       $LCSRC/llvm/tools/clang/tools/extra

Building LLVM/Clang on Linux and SunOS
======================================

    # We will build LLVM/Clang here
    export LCBLD=/path/to/build/directory
    mkdir -p $LCBLD

    # We will install LLVM/Clang here
    export LCINS=/path/to/install/directory
    mkdir -p $LCINS

    # Configure, build, and install
    cd $LCBLD
    export GCCDIR=/path/to/gcc/dir

    # CMake configuration for LLVM/Clang in the Bloomberg environment can be
    # complicated due to various compiler installations, especially on SunOS.
    # Instead of specifying the full configuration here, we use a script provided
    # within the bde_verify source tree.
    $BVSRC/config_llvm.sh -DCMAKE_INSTALL_PREFIX=$LCINS $LCSRC

    # Build and install LLVM/Clang.
    cmake --build . --target install --config MinSizeRel -- -j 20

Building LLVM/Clang on Windows
==============================

    # Install the NSIS packaging system:
    #  We refer to version 3.02.1 here, but newer versions should work.
    #
    #  Navigate to <https://sourceforge.net/projects/nsis/files/NSIS%203/3.02.1/>
    #  The installer is named <nsis-3.02.1-setup.exe>.  Download and run it.
    #
    #  Download the large-strings overlay <nsis-3.02.1-strlen_8192.zip>.
    #  Open that archive and extract its files on top of the installed ones.

    # We will build LLVM/Clang here
    export LCBLD=/path/to/build/directory
    mkdir -p $LCBLD

    # Configure and build
    cd $LCBLD
    cmake $LCSRC/llvm
    cmake --build . --target package --config MinSizeRel

    # The result is an executable named LLVM-5.{...}-win32.exe
    # Run it to install LLVM/Clang.  When asked, set the path for all users.

Building bde_verify on Linux and SunOS
======================================
Note: The build is done in-place in the source directory

    # We will install bde_verify here
    export BVINS=/path/to/bde_verify/install/directory
    mkdir -p $BVINS

    # Set up compiler to use
    export GCCDIR=/path/to/gcc/dir

    # Build and install bde_verify
    CXX=$GCCDIR/bin/g++ LLVMDIR=$LCINS DESTDIR=$BVINS make -C $BVSRC -j 20 install

    # Test the build if you like
    CXX=$GCCDIR/bin/g++ LLVMDIR=$LCINS DESTDIR=$BVINS make -C $BVSRC -k check
   
    # The installed bde_verify script will be found at
    # $BVINS/bin/bde_verify

Building bde_verify on Windows
==============================

    # We will build bde_verify here
    export BVBLD=/path/to/bde_verify/build/directory

    cd $BVBLD
    cmake $BVSRC
    cmake --build . --target package --config MinSizeRel

    # The result is an executable named bde_verify-{...}-win32.exe
    # Run it to install bde_verify.  When asked, set the path for all users.

    # Now build the Visual Studio bde_verify add-in

    cmake --build . --target bde_verify_vsix --config MinSizeRel
   
    # The result is $BVBLD/bde-verify-vs/BdeVerify.vsix
    # Running that file installs the extension into Visual Studio.
    # (It installs its own private copy of the bde_verify executable.)
