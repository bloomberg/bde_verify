// csatr_usingdeclarationinheader.cpp                                 -*-C++-*-

#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Lex/Token.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_visitor.h>
#include <llvm/Support/Regex.h>
#include <string>
#include <map>

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("using-declaration-in-header");

// ----------------------------------------------------------------------------

namespace {

struct data
    // Data attached to analyzer for this check.
{
    std::map<FileID, SourceLocation> d_uds;
    std::map<FileID, SourceLocation> d_ils;
};

struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void set_ud(SourceLocation& ud, SourceLocation sl);

    void operator()(UsingDecl const* decl);

    void set_il(SourceLocation& il, SourceLocation sl);

    void operator()(SourceLocation   HashLoc,
                    const Token&     IncludeTok,
                    StringRef        FileName,
                    bool             IsAngled,
                    CharSourceRange  FilenameRange,
                    const FileEntry *File,
                    StringRef        SearchPath,
                    StringRef        RelativePath,
                    const Module    *Imported);

    void operator()(const FileEntry&           ParentFile,
                    const Token&               FilenameTok,
                    SrcMgr::CharacteristicKind FileType);

    void operator()(SourceRange Range);

    void operator()();
};

void report::set_ud(SourceLocation& ud, SourceLocation sl)
{
    if (!ud.isValid() || m.isBeforeInTranslationUnit(sl, ud)) {
        ud = sl;
    }
}

void report::operator()(UsingDecl const* decl)
{
    if (decl->getLexicalDeclContext()->isFileContext()) {
        SourceLocation sl = decl->getLocation();
        if (!a.is_global_package() &&
            a.is_header(a.get_location(decl).file())) {
            a.report(sl, check_name, "TR16",
                            "Namespace level using declaration in header file")
                << decl->getSourceRange();
        }
        set_ud(d.d_uds[m.getFileID(sl)], sl);
    }
}

void report::set_il(SourceLocation& il, SourceLocation sl)
{
    if (!il.isValid() || m.isBeforeInTranslationUnit(il, sl)) {
        il = sl;
    }
}

// InclusionDirective
void report::operator()(SourceLocation   HashLoc,
                        const Token&     IncludeTok,
                        StringRef        FileName,
                        bool             IsAngled,
                        CharSourceRange  FilenameRange,
                        const FileEntry *File,
                        StringRef        SearchPath,
                        StringRef        RelativePath,
                        const Module    *Imported)
{
    SourceLocation sl = FilenameRange.getBegin();
    set_il(d.d_ils[m.getFileID(sl)], sl);
}

// FileSkipped
void report::operator()(const FileEntry&           ParentFile,
                        const Token&               FilenameTok,
                        SrcMgr::CharacteristicKind FileType)
{
    SourceLocation sl = FilenameTok.getLocation();
    set_il(d.d_ils[m.getFileID(sl)], sl);
}

// SourceRangeSkipped
void report::operator()(SourceRange Range)
{
    SourceLocation sl = Range.getBegin();
    llvm::StringRef s = a.get_source(Range);
    llvm::SmallVector<llvm::StringRef, 7> matches;
    static llvm::Regex r(" *ifn?def *INCLUDED_.*[[:space:]]+"
                        "# *include +([<\"][^\">]*[\">])");
    if (r.match(s, &matches) && s.find(matches[0]) == 0) {
        sl = sl.getLocWithOffset(s.find(matches[1]));
        set_il(d.d_ils[m.getFileID(sl)], sl);
    }
}

// TranslationUnitDone
void report::operator()()
{
    for (const auto& id : d.d_uds) {
        if (!a.is_header(m.getFilename(m.getLocForStartOfFile(id.first)))) {
            const auto& il = d.d_ils[id.first];
            if (il.isValid() &&
                m.isBeforeInTranslationUnit(id.second, il) &&
                !a.is_system_header(id.second) &&
                !a.is_system_header(il)) {
                a.report(id.second, check_name, "AQJ01",
                         "Using declaration precedes header inclusion");
                a.report(il, check_name, "AQJ01",
                         "Header included here",
                          false, DiagnosticIDs::Note);
            }
        }
    }
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    visitor.onUsingDecl += report(analyser);
    observer.onPPInclusionDirective += report(analyser,
                                                observer.e_InclusionDirective);
    observer.onPPFileSkipped        += report(analyser,
                                                observer.e_FileSkipped);
    observer.onPPSourceRangeSkipped += report(analyser,
                                                observer.e_SourceRangeSkipped);
    analyser.onTranslationUnitDone  += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck check(check_name, &subscribe);

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
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
