// csaglb_includes.h                                                  -*-C++-*-
#ifndef INCLUDED_CSAGLB_INCLUDES
#define INCLUDED_CSAGLB_INCLUDES

#include <llvm/ADT/StringRef.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Lex/Token.h>
#include <clang/Basic/FileManager.h>

#include <map>
#include <vector>
#include <optional>


// Maintain a "database" of included files, including their redundant include
// guards if present, and including as well those inclusions that are skipped
// because of their redundant include guards.
//
// An included file is simply a preprocessor directive of one of the forms
//    #include "file"
//    #include <file>
//    #include_next "file"
//    #include_next <file>
//
// A redundantly-guarded included file is one of the above surrounded by
//    #ifndef INCLUDED_guard_name
//    #include ...
//    #endif
//
// Additionally, a redundantly-guarded section may include
//    #define INCLUDED_guard_name
// before or after the inclusion.

namespace csabase {
struct IncludesData {
    struct Inclusion {
        Inclusion() : d_fe(nullptr) { }

        clang::SourceRange      d_fullRange;
            // The full range of the inclusion, including the include-guard
            // section if present.
        clang::SourceRange      d_guard;
            // The range of the include guard on the #ifndef line if present.
        clang::SourceRange      d_file;
            // The range of the file name, without bracketing <> or "".
        clang::SourceRange      d_fullFile;
            // The range of the file name, with bracketing <> or "".
        clang::SourceRange      d_definedGuard;
            // The range of the guard definition, if present, including the
            // #define.
        const clang::FileEntry *d_fe;
            // The file entry for the included file, if supplied by an
            // InclusionDirective callback.
    };

    typedef std::map<clang::FullSourceLoc,
                     Inclusion,
                     clang::FullSourceLoc::BeforeThanCompare>
               Inclusions;
    Inclusions d_inclusions;

    // Stack of active include guards, only ephemerally present.
    typedef std::vector<const clang::Token *> GuardStack;
    GuardStack d_guardStack;
};
}

#endif

// ----------------------------------------------------------------------------
// Copyright (C) 2017 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
