// csaglb_includes.cpp                                                -*-C++-*-

#include <csaglb_includes.h>

#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>

#include <llvm/Support/Path.h>
#include <clang/Lex/Preprocessor.h>
#include <utility>

namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang;
using namespace llvm;

// ----------------------------------------------------------------------------

namespace csabase {

}

namespace {

struct report : Report<IncludesData>
{
    INHERIT_REPORT_CTOR(report, Report, IncludesData);

    // Endif
    void operator()(SourceLocation, SourceLocation);

    // InclusionDirective
    void operator()(SourceLocation,
                    const Token&,
                    StringRef,
                    bool,
                    CharSourceRange,
                    const FileEntry            *,
                    StringRef,
                    StringRef,
                    const clang::Module        *,
                    SrcMgr::CharacteristicKind);
};

// Endif
void report::operator()(SourceLocation Loc, SourceLocation IfLoc)
{
#define mB  "[[:blank:]]"
#define mBS mB"*"
#define mBP mB"+"
#define mH  mBS "#" mBS
#define mE  "(" mBS "//[^\r\n]*" ")?" "[[:space:]]+"
#define mG  "INCLUDED_[_[:alnum:]]+"
#define mF  "[^>\"\r\n]+"

    static Regex guarded_include_initial("^" mH "ifndef" mBP "(" mG ")");
    static Regex guarded_include(
               "^"       mBS
               "ifndef"  mBP "(" mG ")"             mE       // 1 guard
        "(" mH "define"  mBP "(" mG ")"             mE ")?"  // 3 opt, 4 def
            mH "include" mBS "([\"<](" mF ")[>\"])" mE       // 6 inc, 7 file
        "(" mH "define"  mBP "(" mG ")"             mE ")?"  // 9 opt, 10 def
            mH "endif"   mBS
               "$");

    SmallVector<StringRef, 16> matches;
    if (guarded_include_initial.match(a.get_source_line(IfLoc), &matches) &&
        !(matches[1].drop_front(9).equals_lower(
            sys::path::stem(m.getFilename(Loc))))) {
        StringRef gi = a.get_source(SourceRange(IfLoc, Loc));
        if (guarded_include.match(gi, &matches)) {
            auto OffRange = [&](int i) {
                SourceLocation sl = IfLoc.getLocWithOffset(
                    matches[i].data() - matches[0].data());
                return SourceRange(
                    sl, sl.getLocWithOffset(matches[i].size() - 1));
            };
            SourceRange key = OffRange(7);
            FullSourceLoc fsl(key.getBegin(), m);
            IncludesData::Inclusion& inclusion = d.d_inclusions[fsl];
            inclusion.d_fullRange = SourceRange(IfLoc, Loc);
            inclusion.d_guard = OffRange(1);
            inclusion.d_file = key;
            inclusion.d_fullFile = OffRange(6);
            if (matches[3].size()) {
                inclusion.d_definedGuard = OffRange(4);
            }
            if (matches[9].size()) {
                inclusion.d_definedGuard = OffRange(10);
            }
            if (!inclusion.d_fe) {
                const DirectoryLookup *dl = 0;
                inclusion.d_fe = p.LookupFile(fsl,
                         a.get_source(inclusion.d_file),
                         a.get_source(inclusion.d_fullFile)[0] == '<',
                         0, 0, dl, 0, 0, 0, 0, 0);
            }
        }
    }
}

// InclusionDirective
void report::operator()(SourceLocation              HashLoc,
                        const Token&                IncludeTok,
                        StringRef                   FileName,
                        bool                        IsAngled,
                        CharSourceRange             FilenameRange,
                        const FileEntry            *File,
                        StringRef                   SearchPath,
                        StringRef                   RelativePath,
                        const clang::Module        *Imported,
                        SrcMgr::CharacteristicKind  FileType)
{
    SourceRange key(FilenameRange.getBegin().getLocWithOffset(1),
                    FilenameRange.getEnd().getLocWithOffset(-2));
    IncludesData::Inclusion& inclusion =
        d.d_inclusions[FullSourceLoc(key.getBegin(), m)];
    inclusion.d_fullFile = FilenameRange.getAsRange();
    inclusion.d_file = key;
    inclusion.d_fullRange = SourceRange(HashLoc, FilenameRange.getEnd());
    if (File) {
        inclusion.d_fe = FileEntryRef(FileName, *File);
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
    // Hook up the callback functions.
{
    observer.onPPEndif  += report(analyser, observer.e_Endif);

    observer.onPPInclusionDirective +=
        report(analyser, observer.e_InclusionDirective);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1("", &subscribe);

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
