// csaaq_includeinexternc.cpp                                         -*-C++-*-

#include <clang/Frontend/CompilerInstance.h>

#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/Token.h>

#include <csabase_analyser.h>
#include <csabase_clang.h>
#include <csabase_config.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_visitor.h>
#include <csaglb_comments.h>
#include <csaglb_includes.h>

#include <llvm/Support/Path.h>
#include <llvm/Support/Regex.h>

#include <unordered_map>
#include <unordered_set>

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("include-in-extern-c");

// ----------------------------------------------------------------------------

namespace
{

std::unordered_set<std::string> special{
    "bsl_stdhdrs_epilogue_recursive.h",
    "bsl_stdhdrs_epilogue.h",
    "bsl_stdhdrs_prologue.h",
};

struct data
    // Data attached to analyzer for this check.
{
    std::map<SourceLocation, std::string>           d_includes;
    std::map<SourceRange, const LinkageSpecDecl *>  d_linkages;
    std::unordered_map<std::string, SourceLocation> d_prop;
    std::unordered_map<std::string, bool>           d_system_headers;
};

struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    const LinkageSpecDecl *get_local_linkage(SourceLocation sl);

    void set_prop(SourceLocation sl, llvm::StringRef file);

    void operator()(SourceLocation        Loc,
                    const Token&          MacroNameTok,
                    const MacroDirective *MD);

    void operator()(const Token&          MacroNameTok,
                    const MacroDirective *MD,
                    SourceRange           Range);

    void operator()(const LinkageSpecDecl *decl);

    void operator()();
};

const LinkageSpecDecl *report::get_local_linkage(SourceLocation sl)
{
    const LinkageSpecDecl *result = 0;
    for (const auto& r : d.d_linkages) {
        if (!m.isBeforeInTranslationUnit(sl, r.first.getBegin()) &&
            !m.isBeforeInTranslationUnit(r.first.getEnd(), sl) &&
            m.getFileID(m.getExpansionLoc(sl)) ==
            m.getFileID(m.getExpansionLoc(r.first.getBegin()))) {
            result = r.second;
        }
    }
    return result;
}

void report::set_prop(SourceLocation sl, llvm::StringRef file)
{
    if (!d.d_prop[file.str()].isValid()) {
        for (SourceLocation isl = sl;
             isl.isValid();
             isl = m.getIncludeLoc(m.getFileID(isl))) {
            if (a.is_system_header(isl)) {
                return;
            }
        }
        for (SourceLocation isl = sl;
             isl.isValid();
             isl = m.getIncludeLoc(m.getFileID(isl))) {
            llvm::StringRef f = m.getFilename(isl);
            if (d.d_prop[f.str()].isValid()) {
                break;
            }
            d.d_prop[f.str()] = sl;
        }
    }
}

void report::operator()(const LinkageSpecDecl *decl)
{
    if (decl->hasBraces()) {
        d.d_linkages[decl->getSourceRange()] = decl;
    }
}

// TranslationUnitDone
void report::operator()()
{
    llvm::StringRef ns = a.config()->value("enterprise");
    llvm::Regex nsre(ns, llvm::Regex::IgnoreCase | llvm::Regex::Newline);
    llvm::SmallVector<llvm::StringRef, 7> matches;

    for (auto& c : a.attachment<CommentData>().d_comments) {
        llvm::StringRef f = c.first;
        if (!d.d_prop[f.str()].isValid()) {
            for (auto r : c.second) {
                llvm::StringRef s = a.get_source(r);
                if (nsre.match(s, &matches)) {
                    SourceLocation sl = r.getBegin();
                    sl = sl.getLocWithOffset(s.find(matches[0]));
                    set_prop(sl, f);
                }
            }
        }
    }

    for (;;) {
        auto n = d.d_prop.size();
        for (const auto& f : d.d_includes) {
            set_prop(f.first, f.second);
        }
        if (d.d_prop.size() == n) {
            break;
        }
    }

    for (const auto& f : a.attachment<IncludesData>().d_inclusions) {
        FullSourceLoc fsl = f.first;
        if (special.count(llvm::sys::path::filename(m.getFilename(fsl)).str())) {
            continue;
        }
        SourceLocation         sl  = fsl.getExpansionLoc();
        const LinkageSpecDecl *lsd = get_local_linkage(sl);
        StringRef              file = f.second.d_fe ? f.second.d_fe->getName()
                                       : a.get_source(f.second.d_file);
        if (!lsd ||
            lsd->getLanguage() != LinkageSpecDecl::lang_c ||
            !d.d_prop[file.str()].isValid() ||
            a.is_system_header(sl) ||
            a.is_system_header(lsd) ||
            a.is_system_header(d.d_prop[file.str()])) {
            continue;
        }
        auto report_1 = a.report(
            sl, check_name, "IC01",
            "'%0' header included within C linkage specification",
            true);

        report_1 << ns.str();

        a.report(lsd->getLocation(), check_name, "IC01",
                 "C linkage specification here",
                 true, DiagnosticIDs::Note);

        auto report_2 = a.report(
            d.d_prop[file.str()], check_name, "IC01",
            "'%0' evidence here",
            true, DiagnosticIDs::Note);

        report_2 << ns.str();
    }
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone  += report(analyser);
    visitor.onLinkageSpecDecl       += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &subscribe);

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
