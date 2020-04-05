#! /usr/bin/env bash
# config_llvm.sh                                             -*-Shell-script-*-

# Script for configuring an LLVM/Clang build via cmake

# The parameters differ somewhat among Linux, MacOS, and SunOS, and are
# complicated enough to warrant having a script rather than just
# documentation.

OS=$(uname -s)

if [ $# -lt 2 -o "${1:0:23}" != "-DCMAKE_INSTALL_PREFIX=" -o ! -d "${!#}" ]
then
    echo "usage: $(basename $0) -DCMAKE_INSTALL_PREFIX=dir ... llvm-source-dir"
    exit 1
fi

if [ "$OS" = "Darwin" ]; then
    CXX=$(which clang++)
    CC=$(which clang)
else
    if [ "$GCCDIR" = "" ]; then
        cxx=$CXX
        if [ "$cxx" = "" ]; then cxx=g++; fi

        g=$(which "$cxx" 2>/dev/null)
        if [ "$g" != "" ]; then
            GCCDIR=$(dirname $g)/..
        else
            echo "Set GCCDIR or CXX, or have g++ locatable on PATH"
            exit 1
        fi
    elif [ "$(basename $GCCDIR)" = bin ]; then
        echo "GCCDIR should not end in '/bin'"
        echo "try 'export GCCDIR=$(dirname $GCCDIR)'"
        exit 1
    fi

    if [ "$CXX" = "" ]; then CXX=$GCCDIR/bin/g++; fi
    if [ "$CC" = "" ]; then CC=$GCCDIR/bin/gcc; fi
fi

case $OS in
Linux)
    L1=$GCCDIR/lib64
    L2=/usr/lib64
    L="-L $L1 -L $L2 -Wl,-R,$L1,-R,$L2 -fno-use-linker-plugin"
    F="-D_GLIBCXX_USE_CXX11_ABI=0"
    targets="'X86'"
    ;;
SunOS)
    L1=$GCCDIR/lib/sparcv9
    L2=/usr/lib/sparcv9
    L="-L $L1 -L $L2 -Wl,-R,$L1,-R,$L2"
    F="-D_GLIBCXX_USE_CXX11_ABI=1"
    E=-DLIBXML2_LIBRARIES=/usr/lib/sparcv9/libxml2.so
    targets="'Sparc'"
    ;;
Darwin)
    if ! which aspell > /dev/null; then
        echo >&2 "Prerequisit 'aspell' is not installed."
        echo >&2 "Try 'brew install aspell'."
        exit 1
    fi
    W="-Wno-unused-function"
    targets="'X86'"
    ;;
*)
    echo "$(basename $0) is meant only for Linux, MacOS, and SunOS systems"
    exit 1
    ;;
esac

W="-Wno-unused-function"

cmake                                                                         \
    -DCMAKE_CXX_COMPILER="$CXX"                                               \
    -DCMAKE_C_COMPILER="$CC"                                                  \
    -DCMAKE_CXX_FLAGS="-m64 $L $W $F $CXXFLAGS"                               \
    -DCMAKE_C_FLAGS="-m64 $L $W $F $CFLAGS"                                   \
    -DCMAKE_EXE_LINKER_FLAGS="-m64 $L $W $F $LDFLAGS"                         \
    -DCMAKE_BUILD_TYPE=MinSizeRel                                             \
    -DLLVM_LIBDIR_SUFFIX=64                                                   \
    -DLLVM_TARGETS_TO_BUILD=$targets                                          \
    -DLLVM_ENABLE_TERMINFO=OFF                                                \
    $E "$@"
