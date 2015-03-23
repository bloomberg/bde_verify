// csabbg_deprecated.cpp                                              -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/DeclarationName.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/Specifiers.h>
#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <csabase_visitor.h>
#include <ctype.h>
#include <ext/alloc_traits.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Regex.h>
#include <stddef.h>
#include <stdlib.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace clang;
using namespace csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("deprecated");

// ----------------------------------------------------------------------------

namespace
{

struct data
    // Data attached to analyzer for this check.
{
    typedef std::vector<SourceRange> Ranges;
    typedef std::map<std::string, Ranges> Comments;
    Comments d_comments;  // Comment blocks per file.

    typedef std::map<std::string, SourceLocation> DeprecatedFiles;
    DeprecatedFiles d_dep_files;

    typedef std::map<SourceLocation, SourceLocation> DeprecatedComments;
    DeprecatedComments d_dep_comms;

    typedef std::map<const FunctionDecl*, SourceLocation> Deprecated;
    Deprecated d_deprecated;

    typedef std::vector<std::pair<const FunctionDecl*, SourceRange>> FunDecls;
    FunDecls d_fundecls;  // FunDecl, comment

    typedef std::map<Location, const FunctionDecl*> Calls;
    Calls d_calls;
};

struct report : Report<data>
    // Callback object invoked upon completion.
{
    using Report<data>::Report;

    void operator()();
        // Invoked to process reports.

    void operator()(const FunctionDecl *func);
    void operator()(const FunctionTemplateDecl *func);
    void operator()(const CallExpr *call);

    void operator()(SourceRange range);
        // The specified 'range', representing a comment, is either appended to
        // the previous comment or added separately to the comments list.

    SourceRange getContract(const FunctionDecl *func,
                            data::Ranges::iterator comments_begin,
                            data::Ranges::iterator comments_end);
        // Return the 'SourceRange' of the function contract of the specified
        // 'func' if it is present in the specified range of 'comments_begin'
        // up to 'comments_end', and return an invalid 'SourceRange' otherwise.
};

void report::operator()(SourceRange range)
{
    Location location(a.get_location(range.getBegin()));
    data::Ranges& c = d.d_comments[location.file()];
    if (c.size() == 0 || !areConsecutive(m, c.back(), range)) {
        c.push_back(range);
    } else {
        c.back().setEnd(range.getEnd());
    }
    if (!d.d_dep_files.count(location.file()) &&
        a.get_source(range).startswith("//@DEPRECATED:")) {
        d.d_dep_files[location.file()] = range.getBegin();
    }
    if (!d.d_dep_comms.count(c.back().getBegin()) &&
        a.get_source(range).startswith("// !DEPRECATED!:")) {
        d.d_dep_comms[c.back().getBegin()] = range.getBegin();
    }
}

void report::operator()(const CallExpr *call)
{
    if (auto func = call->getDirectCallee()) {
        if (auto f = func->getInstantiatedFromMemberFunction()) {
            func = f;
        }
        func = func->getCanonicalDecl();
        if (!a.is_test_driver() ||
            !a.is_component_header(func->getLocStart())) {
            d.d_calls[Location(m, call->getExprLoc())] =
                func->getCanonicalDecl();
        }
    }
}

void report::operator()()
{
    for (auto& p : d.d_fundecls) {
        Location location(a.get_location(p.first->getLocStart()));
        data::Ranges& c = d.d_comments[location.file()];
        p.second = getContract(p.first, c.begin(), c.end());
        auto j = d.d_dep_comms.find(p.second.getBegin());
        if (j != d.d_dep_comms.end()) {
            d.d_deprecated[p.first] = j->second;
        }
        else {
            auto i =
                d.d_dep_files.find(Location(m, p.first->getLocation()).file());
            if (i != d.d_dep_files.end()) {
                d.d_deprecated[p.first] = i->second;
            }
        }
    }

    for (auto& p : d.d_calls) {
        auto i = d.d_deprecated.find(p.second);
        if (i != d.d_deprecated.end()) {
            a.report(p.first.location(), check_name, "DP01",
                     "Call to deprecated function");
            a.report(i->second, check_name, "DP01",
                     "Deprecated here",
                     false, DiagnosticIDs::Note);
        }
    }
}

void report::operator()(const FunctionTemplateDecl* func)
{
    (*this)(func->getTemplatedDecl());
}

void report::operator()(const FunctionDecl* func)
{
    // Don't process compiler-defaulted methods, main, template instantiations,
    // or macro expansions
    if (   !func->isDefaulted()
        && !func->isMain()
        && !func->getLocation().isMacroID()
        && (   func->getTemplatedKind() == func->TK_NonTemplate
            || func->getTemplatedKind() == func->TK_FunctionTemplate)
            ) {
        d.d_fundecls.push_back(
            std::make_pair(func->getCanonicalDecl(), SourceRange()));
    }
}

SourceRange report::getContract(const FunctionDecl     *func,
                                data::Ranges::iterator  comments_begin,
                                data::Ranges::iterator  comments_end)
{
    SourceRange declarator = func->getSourceRange();
    declarator.setEnd(declarator.getEnd().getLocWithOffset(1));
    SourceRange contract;
    bool with_body = func->doesThisDeclarationHaveABody() && func->getBody();
    bool one_liner = with_body &&
                     m.getPresumedLineNumber(declarator.getBegin()) ==
                         m.getPresumedLineNumber(func->getBody()->getLocEnd());

    const CXXConstructorDecl *ctor = llvm::dyn_cast<CXXConstructorDecl>(func);

    if (ctor && with_body && ctor->getNumCtorInitializers() > 0) {
        // Constructor with body and initializers - look for a contract that
        // starts no earlier than the first initializer and has only whitespace
        // and a colon between itself and that initializer.
        SourceLocation initloc = (*ctor->init_begin())->getSourceLocation();
        if (initloc.isValid()) {
            data::Ranges::iterator it;
            for (it = comments_begin; it != comments_end; ++it) {
                if (m.isBeforeInTranslationUnit(initloc, it->getBegin())) {
                    break;
                }
                if (m.isBeforeInTranslationUnit(
                        it->getEnd(), declarator.getBegin())) {
                    continue;
                }
                llvm::StringRef s =
                    a.get_source(SourceRange(it->getEnd(), initloc), true);
                if (s.find_first_not_of(": \n") == llvm::StringRef::npos) {
                    contract = *it;
                    break;
                }
            }
        }
    }

    if (with_body && !contract.isValid()) {
        // Function with body - look for a comment that starts no earlier than
        // the function declarator and has only whitespace between itself and 
        // the open brace of the function.
        SourceLocation bodyloc = func->getBody()->getLocStart();
        data::Ranges::iterator it;
        for (it = comments_begin; it != comments_end; ++it) {
            if (m.isBeforeInTranslationUnit(bodyloc, it->getBegin())) {
                break;
            }
            if (m.isBeforeInTranslationUnit(
                    it->getEnd(), declarator.getBegin())) {
                continue;
            }
            llvm::StringRef s =
                a.get_source(SourceRange(it->getEnd(), bodyloc), true);
            if (s.find_first_not_of(" \n") == llvm::StringRef::npos) {
                contract = *it;
                break;
            }
        }
    }

    if (!with_body || (one_liner && !contract.isValid())) {
        // Function without body or one-liner - look for a comment following
        // the declaration separated from it by only whitespace and semicolon.
        SourceLocation endloc = declarator.getEnd();
        data::Ranges::iterator it;
        for (it = comments_begin; it != comments_end; ++it) {
            if (m.isBeforeInTranslationUnit(it->getEnd(), endloc)) {
                continue;
            }
            llvm::StringRef s =
                a.get_source(SourceRange(endloc, it->getBegin()), true);
            if (!with_body) {
                s = s.split(';').second;
            }
            if (s.find_first_not_of(" \n") == llvm::StringRef::npos &&
                s.count("\n") <= 1) {
                contract = *it;
            }
            break;
        }
    }
    return contract;
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
    visitor.onFunctionDecl += report(analyser);
    visitor.onFunctionTemplateDecl += report(analyser);
    visitor.onCallExpr += report(analyser);
    observer.onComment += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &subscribe);

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
