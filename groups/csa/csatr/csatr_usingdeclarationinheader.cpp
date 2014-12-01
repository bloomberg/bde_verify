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
    SourceLocation d_first_ud;
    SourceLocation d_last_il;
};

struct report
{
    report(Analyser& analyser,
            PPObserver::CallbackType type = PPObserver::e_None);
        // Create an object of this type, that will use the specified
        // 'analyser'.  Optionally specify a 'type' to identify the callback
        // that will be invoked, for preprocessor callbacks that have the same
        // signature.

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

    Analyser&                d_analyser;
    data&                    d_data;
    PPObserver::CallbackType d_type;

    SourceManager&           m;
};

report::report(Analyser& analyser, PPObserver::CallbackType type)
: d_analyser(analyser)
, d_data(analyser.attachment<data>())
, d_type(type)
, m(analyser.manager())
{
}

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
        if (!d_analyser.is_global_package() &&
            d_analyser.is_header(d_analyser.get_location(decl).file())) {
            d_analyser.report(sl, check_name, "TR16",
                            "Namespace level using declaration in header file")
                << decl->getSourceRange();
        }
        set_ud(d_data.d_uds[m.getFileID(sl)], sl);
        set_ud(d_data.d_first_ud, sl);
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
    set_il(d_data.d_ils[m.getFileID(sl)], sl);
    set_il(d_data.d_last_il, sl);
}

// FileSkipped
void report::operator()(const FileEntry&           ParentFile,
                        const Token&               FilenameTok,
                        SrcMgr::CharacteristicKind FileType)
{
    SourceLocation sl = FilenameTok.getLocation();
    set_il(d_data.d_ils[m.getFileID(sl)], sl);
    set_il(d_data.d_last_il, sl);
}

// SourceRangeSkipped
void report::operator()(SourceRange Range)
{
    SourceLocation sl = Range.getBegin();
    llvm::StringRef s = d_analyser.get_source(Range);
    llvm::SmallVector<llvm::StringRef, 7> matches;
    static llvm::Regex r(" *ifn?def *INCLUDED_.*[[:space:]]+"
                        "# *include +([<\"][^\">]*[\">])");
    if (r.match(s, &matches) && s.find(matches[0]) == 0) {
        sl = sl.getLocWithOffset(s.find(matches[1]));
        set_il(d_data.d_ils[m.getFileID(sl)], sl);
        set_il(d_data.d_last_il, sl);
    }
}

// TranslationUnitDone
void report::operator()()
{
    for (const auto& id : d_data.d_uds) {
        const auto& il = d_data.d_ils[id.first];
        if (il.isValid() && m.isBeforeInTranslationUnit(id.second, il)) {
            d_analyser.report(id.second, check_name, "AQJ01",
                              "Using declaration precedes header inclusion");
            d_analyser.report(il, check_name, "AQJ01",
                              "Header included here",
                              true, DiagnosticIDs::Note);
            if (d_data.d_first_ud.isValid() &&
                m.getFileID(d_data.d_first_ud) == id.first) {
                d_data.d_first_ud = SourceLocation();
            }
        }
    }

    if (d_data.d_first_ud.isValid() &&
        d_data.d_last_il.isValid() &&
        m.getFileID(d_data.d_first_ud) != m.getFileID(d_data.d_last_il)) {
        d_analyser.report(d_data.d_first_ud, check_name, "AQJ01",
                          "Using declaration precedes header inclusion",
                          true);
        d_analyser.report(d_data.d_last_il, check_name, "AQJ01",
                          "Header included here",
                          true, DiagnosticIDs::Note);
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
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------- END-OF-FILE ----------------------------------
