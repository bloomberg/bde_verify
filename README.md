BDE Verify - A Static Checker for C++

Bde_verify is a static analysis tool that verifies that source code adheres
to the BDE coding standards.

The online documentation for bde_verify can be found at
<https://bloomberg.github.io/bde_verify/>

Build Instructions
==================

Supported Platforms:
- Linux x86
- SunOS sparc
- Windows
- MacOS (Darwin)

Prerequesites: 
- llvm/clang 9.0 (see below for instructions)
- gcc >= 5 (Linux and SunOS)
- Clang >= 9 (Darwin/MacOS)
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
    git clone -b master https://github.com/bloomberg/bde_verify $BVSRC

Installing LLVM/Clang Sources
=============================

If you are doing this on a **Bloomberg development machine**, follow the
instructions at
<http://tutti.prod.bloomberg.com/go-git-go-docs/configuration> to
configure `git` for access to `github` securely through an https proxy.

Note: this requires a great deal of free disk space.

    # We will place LLVM/Clang source code here
    export LCSRC=/path/to/llvm/source/directory
    mkdir -p $LCSRC

    # Clone the LLVM/Clang repositories
    export GL=https://github.com/llvm-mirror
    git clone -b release_90 $GL/llvm.git  $LCSRC
    git clone -b release_90 $GL/clang.git $LCSRC/tools/clang
    git clone -b release_90 $GL/clang-tools-extra.git $LCSRC/tools/clang/tools/extra

Building LLVM/Clang
===================

On Linux, SunOS, and MacOS
--------------------------

Set build and install directories.

    # We will build LLVM/Clang here
    export LCBLD=/path/to/llvm/build/directory
    mkdir -p $LCBLD

    # We will install LLVM/Clang here
    export LCINS=/path/to/llvm/install/directory
    mkdir -p $LCINS

(Linux and SunOS only) Set up compiler to use:

If `g++ -v` does not find a compiler or yields a gcc version of 4.8 or later,
set `GCCDIR` to a path that contains `bin/gcc` and `bin/g++`. Recommended
setting on all Bloomberg Linux and SunOS development machines is
`export GCCDIR=/opt/bb/lib/gcc-9`.

    # If GCCDIR is not set, default is '$(dirname $(dirname $(which g++)))'.
    # If set, $GCCDIR/bin/g++ must exist (so GCCDIR should not end in "/bin").
    export GCCDIR=/path/to/gcc/dir

Configure, build, and install

    cd $LCBLD

    # CMake configuration for LLVM/Clang in the Bloomberg environment can be
    # complicated due to various compiler installations, especially on SunOS.
    # Instead of specifying the full configuration here, we use a script
    # provided within the bde_verify source tree.
    $BVSRC/config_llvm.sh -DCMAKE_INSTALL_PREFIX=$LCINS $LCSRC

    # Build and install LLVM/Clang.
    cmake --build . --target install --config MinSizeRel -j 20

On Windows
----------

Install the NSIS packaging system:

    #  We refer to version 3.02.1 here, but newer versions should work.
    #
    #  Navigate to <https://sourceforge.net/projects/nsis/files/NSIS%203/3.02.1/>
    #  The installer is named <nsis-3.02.1-setup.exe>.  Download and run it.
    #
    #  Download the large-strings overlay <nsis-3.02.1-strlen_8192.zip>.
    #  Open that archive and extract its files on top of the installed ones.

Set build directory:

    # We will build LLVM/Clang here
    export LCBLD=/path/to/build/directory
    mkdir -p $LCBLD

Configure and build, and install llvm:

    cd $LCBLD
    cmake $LCSRC/llvm
    cmake --build . --target package --config MinSizeRel

    # The result is an executable named LLVM-5.{...}-win32.exe
    # Run it to install LLVM/Clang.  When asked, set the path for all users.

Building bde_verify 
===================

On Linux, SunOS, and MacOS
--------------------------

Note: The build is done in-place in the source directory. The installed
bde_verify script and executable will be found at
`$BVSRC/`*OS-Compiler*`/bin` (e.g., `$BVSRC/Linux-g++/bin`). The cmake
configuration is not set up for easily choosing your own installation
directory.

Set build directory:

    # We will build bde_verify here
    export BVBLD=/path/to/bde_verify/build/directory
    mkdir -p $BVBLD

Set up compiler to use (Linux and SunOS only):

If `g++ -v` does not find a compiler or yields a gcc version of before 4.8,
set `GCCDIR` to a path that contains `bin/gcc` and `bin/g++`. Recommended
setting on all Bloomberg Linux and SunOS development machines is
`export GCCDIR=/opt/bb/lib/gcc-9`.

    # If GCCDIR is not set, default is '$(dirname $(dirname $(which g++)))'.
    # If set, $GCCDIR/bin/g++ must exist (so GCCDIR should not end in "/bin").
    export GCCDIR=/path/to/gcc/dir

    # If you set GCCDIR, you must also set these:
    export CC=$GCCDIR/bin/g++
    export CXX=$GCCDIR/bin/g++

Configure, build, and install bde_verify:

    # LLVM_DIR should point to a directory containing 'LLVMConfig.cmake'
    # Alternatively, CMAKE_PREFIX_PATH=$LCINS works on Linux and Mac OS, but
    # not Sun OS.
    cd $BVBLD
    LLVM_DIR=$LCINS/lib64/cmake/llvm cmake $BVSRC
    cmake --build . --target install -j 20

Test the build if you like.  Note that the 'check' target is not buildable
through cmake yet; this command will rebuild all of `bde_verify` (slightly
differently than the cmake build) as well as building the 'check' target.

    cd ..
    make LLVMDIR=$LCINS -C $BVSRC -k check
   
On Windows
----------

Set build directory:

    # We will build bde_verify here
    export BVBLD=/path/to/bde_verify/build/directory

Configure, build, and install bde_verify

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
