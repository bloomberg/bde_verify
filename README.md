# BDE Verify - A Static Checker for C++

Bde_verify is a static analysis tool that verifies that source code adheres
to the BDE coding standards.

The online documentation for bde_verify can be found at
<https://bloomberg.github.io/bde_verify/>

## Build Instructions

Supported Platforms:
- Linux x86
- SunOS sparc
- Windows
- MacOS (Darwin)

Prerequesites: 
- llvm/clang 14.0 (see below for instructions)
- gcc >= 7 (Linux and SunOS)
- Clang >= 9 (Darwin/MacOS)
- Visual Studio >= 2015 (Windows)
- nsis with large-string overlay (Windows, more details below)
- cmake >= 3.4.3
- gnu make
- aspell (package `libaspell-dev`)

Bde_verify incorporates llvm/clang-14.0 libraries.  These may already be
installed on the build machine.  If not, they can be installed (in the
Bloomberg environment) using dpkg, or built from source on Windows.

### Bloomberg Environment

#### Installing bde_verify sources

    # We will place bde_verify source code here
    export BVSRC=/path/to/bde_verify/source/directory
    mkdir -p ${BVSRC}
    git clone -b master bbgithub:bde/bde_verify ${BVSRC}

#### Installing refroot

    export DISTRIBUTION_REFROOT=/path/to/refroot
    refroot-install --arch=amd64 --refroot-path=${DISTRIBUTION_REFROOT} --config=${BVSRC}/debian/control


#### Configure bde-verify (bbcmake)
    cd $BVSRC
    mkdir _build; cd _build
    bbcmake -64 -G Ninja -DClang_DIR=${DISTRIBUTION_REFROOT}/opt/bb/lib/llvm-14.0/lib64/cmake/clang/  ../

Alternatively, with plain cmake:

    cd $BVSRC
    mkdir _build; cd _build
    export CXX=${DISTRIBUTION_REFROOT}/opt/bb/bin/g++
    export CC=${DISTRIBUTION_REFROOT}/opt/bb/bin/gcc
    cmake -G Ninja -DCMAKE_INSTALL_PREFIX=/opt/bb -DClang_DIR=${DISTRIBUTION_REFROOT}/opt/bb/lib/llvm-11.0/lib64/cmake/clang/ -DCMAKE_C_FLAGS=-m64 -DCMAKE_CXX_FLAGS=-m64 -DCMAKE_BUILD_TYPE=RELEASE ../

#### Build & install bde-verify

    cmake --build .
    DESTDIR=/path/to/install/dir cmake --install .

#### Test bde-verify

    cd ${BVSRC}
    DESTDIR=/path/to/install/dir/opt/bb/  make -f Makefile.test_only check


Installing LLVM/Clang Sources
=============================

If you are doing this on a **Bloomberg development machine**, follow the
instructions at
<http://tutti.prod.bloomberg.com/go-git-go-docs/configuration> to
configure `git` for access to `github` securely through an https proxy.

Note: this requires a great deal of free disk space.

    # We will place LLVM/Clang source code here
    export LCSRC=/path/to/llvm/source/directory
    mkdir -p ${LCSRC}

    # Clone the LLVM/Clang repositories
    git clone https://github.com/llvm/llvm-project.git ${LCSRC} 
    cd ${LCSRC}
    # Checkout llvm-11 release tag
    git checkout tags/llvmorg-11.0.1 -b llvm11

Building LLVM/Clang
===================

External references:

- [Getting started](https://clang.llvm.org/get_started.html)
- [Building LLVM with CMake](https://www.llvm.org/docs/CMake.html)

Below is the configuration line that enables clang and clang-tool-extra 
projects required by bde-verify:

    mkdir _build; cd _build
    # This is a configuration line for cmake that contain all relevant information (see description of defines in llvm howto).
    # In order to use different compiler, use CXX/CC variables to point to compilers:
    cmake -G Ninja -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" -DLLVM_ENABLE_BINDINGS=No -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_INSTALL_PREFIX=<install_path> -DLLVM_TARGETS_TO_BUILD=X86 -DLLVM_LIBDIR_SUFFIX=64 ../llvm/
    cmake --build .
    cmake --install .


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
