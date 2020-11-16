// csaaq_cppinexternc.cpp                                             -*-C++-*-

#include <clang/Lex/Token.h>

#include <csabase_analyser.h>
#include <csabase_clang.h>
#include <csabase_debug.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_visitor.h>
#include <csaglb_includes.h>

#include <llvm/Support/Path.h>
#include <llvm/Support/Regex.h>

#include <unordered_map>
#include <unordered_set>

using namespace csabase;
using namespace clang::tooling;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("cpp-in-extern-c");

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
    std::map<SourceLocation, std::string>          d_includes;
    std::map<SourceRange, const LinkageSpecDecl *> d_linkages;
    std::unordered_map<
        std::string,
        std::pair<SourceLocation, LinkageSpecDecl::LanguageIDs> >
                                                   d_types;
    std::unordered_set<SourceLocation>             d_done;
};

struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    const LinkageSpecDecl *get_linkage(SourceLocation sl);

    const LinkageSpecDecl *get_local_linkage(SourceLocation sl);

    void operator()(SourceLocation        Loc,
                    const Token&          MacroNameTok,
                    const MacroDirective *MD);

    void operator()(const Token&          MacroNameTok,
                    const MacroDirective *MD,
                    SourceRange           Range);

    void operator()(const Decl *decl);

    void set_lang(SourceLocation sl, LinkageSpecDecl::LanguageIDs lang);

    void operator()(const FunctionDecl *decl);

    void operator()(const VarDecl *decl);

    void operator()(const LinkageSpecDecl *decl);

    void operator()();
};

const LinkageSpecDecl *report::get_linkage(SourceLocation sl)
{
    const LinkageSpecDecl *result = 0;
    for (const auto& r : d.d_linkages) {
        if (!m.isBeforeInTranslationUnit(sl, r.first.getBegin()) &&
            !m.isBeforeInTranslationUnit(r.first.getEnd(), sl)) {
            result = r.second;
        }
    }
    return result;
}

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

void report::set_lang(SourceLocation sl, LinkageSpecDecl::LanguageIDs lang)
{
    SourceLocation osl = sl;
    while (sl.isValid() && !get_local_linkage(sl)) {
        llvm::StringRef name(llvm::sys::path::filename(m.getFilename(sl)));
        if (!special.count(name.str()) && !d.d_types[name.str()].second) {
            d.d_types[name.str()] = std::make_pair(osl, lang);
        }
        sl = m.getIncludeLoc(m.getFileID(sl));
    }
}

void report::operator()(const FunctionDecl *decl)
{
    SourceLocation sl = m.getExpansionLoc(decl->getLocation());
    if (decl->isGlobal() &&
        !decl->isCXXClassMember() &&
        !decl->isExternC() &&
        !get_local_linkage(sl)) {
        set_lang(sl, LinkageSpecDecl::lang_cxx);
    }
}

void report::operator()(const VarDecl *decl)
{
    SourceLocation sl = m.getExpansionLoc(decl->getLocation());
    if (decl->hasExternalStorage() &&
        !decl->isCXXClassMember() &&
        !decl->isExternC() &&
        !get_local_linkage(sl)) {
        set_lang(sl, LinkageSpecDecl::lang_cxx);
    }
}

void report::operator()(const LinkageSpecDecl *decl)
{
    if (decl->hasBraces()) {
        SourceRange r = decl->getSourceRange();
        r = SourceRange(m.getExpansionRange(r.getBegin()).getBegin(),
                        m.getExpansionRange(r.getEnd()).getEnd());
        d.d_linkages[r] = decl;
    }
}

void report::operator()(const Decl *decl)
{
    switch (decl->getKind()) {
      case Decl::ClassTemplate:
      case Decl::FunctionTemplate:
      case Decl::VarTemplate:
      case Decl::ClassTemplateSpecialization:
      case Decl::ClassTemplatePartialSpecialization:
      {
        SourceLocation sl = m.getExpansionLoc(decl->getLocation());
        set_lang(sl, LinkageSpecDecl::lang_cxx);
      }
      default:
        ;
    }
}

// TranslationUnitDone
void report::operator()()
{
    for (auto& f : a.attachment<IncludesData>().d_inclusions) {
        FullSourceLoc fsl = f.first;
        if (special.count(llvm::sys::path::filename(m.getFilename(fsl)).str())) {
            continue;
        }
        llvm::StringRef file = a.get_source(f.second.d_file);
        const LinkageSpecDecl *lsd = get_linkage(fsl);
        if (!lsd ||
            lsd->getLanguage() != LinkageSpecDecl::lang_c ||
            d.d_types[file.str()].second != LinkageSpecDecl::lang_cxx) {
            continue;
        }
        SourceLocation sl = fsl.getExpansionLoc();
        FileID lfid = m.getFileID(m.getExpansionLoc(lsd->getLocation()));
        while (sl.isValid() && m.getFileID(sl) != lfid) {
            sl = m.getIncludeLoc(m.getFileID(sl));
        }
        if (sl.isValid() && d.d_done.count(sl)) {
            continue;
        }
        bool sys = m.isInSystemHeader(d.d_types[file.str()].first);

        auto builder = d_analyser.report(
            f.second.d_file.getBegin(), check_name, "PC01",
            "C++ %0 included within C linkage specification");

        builder << (sys ? "system header" : "header");

        d_analyser.report(lsd->getLocation(), check_name, "PC01",
                          "C linkage specification here",
                          false, DiagnosticIDs::Note);
        if (sl.isValid() && sl != fsl) {
            d.d_done.insert(sl);
            d_analyser.report(sl, check_name, "PC01",
                              "Top level include within C linkage here",
                              false, DiagnosticIDs::Note);
        }
        if (!sys) {
            d_analyser.report(d.d_types[file.str()].first,
                              check_name, "PC01",
                              "Declaration with C++ linkage here",
                              false, DiagnosticIDs::Note);
        }
    }
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
    visitor.onDecl                 += report(analyser);
    visitor.onFunctionDecl         += report(analyser);
    visitor.onLinkageSpecDecl      += report(analyser);
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
