// csaxform_precon.cpp                                                -*-C++-*-

#include "csabase_analyser.h"
#include "csabase_debug.h"
#include "csabase_filenames.h"
#include "csabase_location.h"
#include "csabase_ppobserver.h"
#include "csabase_registercheck.h"
#include "csabase_report.h"
#include "csabase_util.h"
#include "csabase_visitor.h"

#include "csaglb_includes.h"

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Regex.h>

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Lex/Lexer.h>
#include <clang/Lex/MacroArgs.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>

#include <set>
#include <string>
#include <vector>

using namespace clang;
using namespace clang::ast_matchers;
using namespace csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("precondition-moves");

// ----------------------------------------------------------------------------

namespace
{

struct data
{
    std::vector<SourceRange>                                  d_asserts;
    std::vector<std::pair<const FunctionDecl *, SourceRange>> d_to_be_moved;
    std::map<Location, std::string>                           d_insertions;
};

// Callback object invoked upon completion.
struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void operator()();
        // Callback for end of compilation unit.

    void operator()(Token const &,
                    const MacroDefinition&,
                    SourceRange,
                    MacroArgs const *);
        // Callback for macro expansion.

    bool check_compatible_parameters(const FunctionDecl *from,
                                     const FunctionDecl *to);
        // Return whether the parameter names of the specified 'to' function
        // match those of the specified 'from' function.  This is asymmetric,
        // because 'from' is permitted to have names missing when they are
        // present in 'to', but not vice versa.

    void copy_assert(const FunctionDecl *to,
                     SourceRange         assertion,
                     llvm::StringRef     prefix);
        // Copy the specified 'assertion' code to the end of the declarator of
        // the specified function 'to', annotating the copy with the specified
        // prefix.

#if 0
    // This code implements pieces necessary for re-declaring methods
    // with the asserts outside class declaration. This functionality is not
    // implemented in this version of the check.

    std::string redecl_name(const FunctionDecl *f);
        // Return the "name" for redeclaring the specified function 'f', with
        // all necessary template headers included.  E.g., given
        //..
        //  template <class A, int B, class C> struct S { struct H {
        //      template <class D, int E, class F> struct T { struct I {
        //          template <class G, int H, class I> void foo(b);
        // }; }; }; };
        //..
        // the redeclaration of foo would be
        //..
        //  template <class A, int B, class C>
        //  template <class D, int E, class F>
        //  template <class G, int H, class I>
        //  void S<A, B, C>::H::T<D, E, F>::I::f(bool b);
        //..

    SourceLocation redecl_loc(const FunctionDecl *f);
        // Return the location for redeclaring the specified function 'f'.
        // This will return a valid source location provided that the function
        // is declared in the component header file.  The location will be at
        // file scope, after the last namespace instance that encloses the
        // function and after the last instance of a declaration of a member
        // of and enclosing context of the function.  That is, the location is
        // after the "interesting" contexts involving 'f'.
#endif

    bool do_template(const TemplateDecl *t,
                     std::string&        h,
                     std::string&        s,
                     std::string&        e);
        // Process the specified template declaration 't' for redeclarations.
        // The template header is prepended to the specified 'h', the
        // nested name specifier with template parameters is prepended to the
        // specified 's', and if a problem occurs, the specified 'e' is set to
        // a human-readable error string.  Return true on success and false on
        // failure.
};

bool report::check_compatible_parameters(const FunctionDecl *from,
                                         const FunctionDecl *to)
{
    unsigned n = from->getNumParams();
    if (n != to->getNumParams()) {
        return false;
    }
    for (unsigned i = 0; i < n; ++i) {
        auto fi = from->getParamDecl(i)->getIdentifier();
        auto ti = to->getParamDecl(i)->getIdentifier();
        if (fi && (!ti || fi->getName() != ti->getName())) {
            return false;
        }
    }
    return true;
}

void report::copy_assert(const FunctionDecl *to,
                         SourceRange         assertion,
                         llvm::StringRef     prefix)
{
    SourceRange fr = a.get_full_range(
             to->getTypeSourceInfo()->getTypeLoc().getSourceRange());
    Location fs(m, a.get_trim_line_range(fr.getBegin()).getBegin());
    SourceLocation ip = fr.getEnd();
    auto column = fs.column();
    Location ins(m, ip);
    /*
    if (a.is_component_header(to) && !to->doesThisDeclarationHaveABody()) {
        auto r = redecl_name(to);
        auto l = redecl_loc(to);
        if (l.isValid()) {
            ins =
                Location(m, a.get_line_range(l).getEnd().getLocWithOffset(1));
            if (!d.d_insertions.count(ins)) {
                d.d_insertions[ins] = ";" + r;
            }
            column = 1;
        }
    }
    */

    // Cut out only macro parameters from the full assertion macro.
    std::string macroParams = a.get_source(assertion).str();
    std::size_t pos = macroParams.find_first_of("(");

    std::string s = ("\n" + std::string(3 + column, ' ') + prefix +
                     macroParams.substr(pos))
                        .str();
    d.d_insertions[ins] += s;
    a.report(ip, check_name, "CA01",
             "Copying assertion to contract location")
        << assertion;
}

bool report::do_template(const TemplateDecl *t,
                         std::string&        h,
                         std::string&        s,
                         std::string&        e)
{
    if (auto p = t ? t->getTemplateParameters() : nullptr) {
        h = a.get_source(a.get_full_range(SourceRange(p->getTemplateLoc(),
                                                      p->getRAngleLoc())))
                .str() + "\n" + h;
        std::string sep = "";
        s += "<";
        for (auto n : *p) {
            const auto& y = n->getDeclName();
            if (y.isEmpty()) {
                e = "<missing parameter name>";
                return false;
            }
            s += sep + y.getAsString();
            sep = ", ";
        }
        s += ">";
    }
    return true;
}

#if 0
std::string report::redecl_name(const FunctionDecl *f)
{
    const auto& ni = f->getNameInfo();
    SourceRange nr = a.get_full_range(ni.getSourceRange());
    llvm::StringRef up_to_name =
        a.get_source(SourceRange(f->getOuterLocStart(), nr.getBegin()), true);
    std::string name = ni.getName().getAsString();
    llvm::StringRef after_name = a.get_source(
        SourceRange(
            nr.getEnd(),
            f->getTypeSourceInfo()->getTypeLoc().getEndLoc().getLocWithOffset(
                1)),
        true);
    if (f->isOutOfLine()) {
        return up_to_name.str() + name + after_name.str();
    }
    std::string h;
    std::string e;
    if (!do_template(f->getDescribedFunctionTemplate(), h, e, e)) {
        return e;
    }
    for (auto dc = f->getDeclContext(); dc; dc = dc->getParent()) {
        if (!dc->isLookupContext()) {
            continue;
        }
        if (auto n = llvm::dyn_cast<NamespaceDecl>(dc)) {
            if (!n->isAnonymousNamespace()) {
                name = n->getDeclName().getAsString() + "::" + name;
            }
        }
        else if (auto r = llvm::dyn_cast<CXXRecordDecl>(dc)) {
            const auto& y = r->getDeclName();
            if (y.isEmpty()) {
                return "<missing class name>";
            }
            std::string q = y.getAsString();
            if (!do_template(r->getDescribedClassTemplate(), h, q, e)) {
                return e;
            }
            name = q + "::" + name;
        }
    }
    return h + up_to_name.str() + name + after_name.str();
}

SourceLocation report::redecl_loc(const FunctionDecl *f)
{
    SourceLocation loc;
    for (const DeclContext *dc = f; dc; dc = dc->getLexicalParent()) {
        if (const Decl *d = llvm::dyn_cast<Decl>(dc)) {
            if (a.is_component(d->getEndLoc())) {
                loc = d->getEndLoc();
            }
        }
    }
    return loc;
}
#endif

void report::operator()()
{
    auto tu = a.context()->getTranslationUnitDecl();
    MatchFinder mf;
    const CompoundStmt *cc = 0;
    CompoundStmt::const_body_iterator bi;
    OnMatch<> m1([&](const BoundNodes &nodes) {
        auto e = nodes.getNodeAs<Expr>("e");
        auto f = nodes.getNodeAs<FunctionDecl>("f");
        auto o = nodes.getNodeAs<DoStmt>("o");
        auto c = nodes.getNodeAs<CompoundStmt>("c");

        SourceLocation sl = e->getExprLoc();
        if (cc != c) {
            cc = c;
            bi = c->body_begin();
        }
        if (!f->getTemplateInstantiationPattern() && a.is_component(sl)) {
            if (*bi++ != o) {
                cc = 0;
            }
            else {
                for (auto& r : d.d_asserts) {
                    if (m.isBeforeInTranslationUnit(r.getBegin(), sl) &&
                        m.isBeforeInTranslationUnit(sl, r.getEnd())) {
                        d.d_to_be_moved.emplace_back(f, r);
                        break;
                    }
                }
            }
        }
    });
    mf.addDynamicMatcher(
        decl(forEachDescendant(functionDecl(anyOf(
            cxxConstructorDecl(
                has(compoundStmt(forEach(doStmt(has(compoundStmt(has(ifStmt(
                    hasCondition(
                        expr(unless(anyOf(hasDescendant(declRefExpr(to(varDecl(
                                              hasAutomaticStorageDuration(),
                                              unless(parmVarDecl()))))),
                                          hasDescendant(memberExpr()))))
                            .bind("e")),
                    has(compoundStmt(has(callExpr(callee(namedDecl(
                        hasName("invokeHandler"))))))))))))
                                                .bind("o"))).bind("c"))),
            functionDecl(
                unless(cxxConstructorDecl()),
                has(compoundStmt(forEach(doStmt(has(compoundStmt(has(ifStmt(
                    hasCondition(
                        expr().bind("e")),
                    has(compoundStmt(has(callExpr(callee(namedDecl(
                        hasName("invokeHandler"))))))))))))
                                                .bind("o"))).bind("c")))
                )).bind("f"))),
        &m1);
    mf.match(*tu, *a.context());
    for (const auto &p : d.d_to_be_moved) {
        auto fd = p.first;
        auto cd = fd->getCanonicalDecl();
        if (check_compatible_parameters(p.first, cd)) {
            copy_assert(cd, p.second, "BSLS_CONTRACTS_PRE");
        }
#if 0
        // This code modifies the assertion at the implementation location.
        // For initial check functionality this replacement is not required.
        else {
            copy_assert(fd, p.second, "IMPL_CONTRACT_");
            a.report(cd, check_name, "CA03",
                     "Cannot copy contract here due to mismatched parameters")
                << fd;
        }
        a.InsertTextBefore(p.second.getBegin(), "TRANSFERRED_");
        a.report(p.second.getBegin(), check_name, "CA02",
                 "Marking assertion as copied to contract");
#endif
    }

    std::set<FileID> macroGenerated;
    for (const auto &p : d.d_insertions) {
        std::string s = p.second;
        if (p.first !=
            Location(m, a.get_line_range(p.first.location()).getEnd())) {
            s += "\n" + std::string(p.first.column() - 1, ' ');
        }
        if (s[0] == ';') {
            s = s.substr(1) + ';';
        }
        a.InsertTextBefore(p.first.location(), s);

        // Generating macro definition.  The macro will be inserted in the line
        // before first #include.
        FileID fileId = m.getFileID(p.first.location());
        if (0 == macroGenerated.count(fileId)) {
            macroGenerated.emplace(fileId);

            for (auto &id : a.attachment<IncludesData>().d_inclusions) {
                if (fileId == id.first.getFileID()) {
                    std::string macroDef = "#ifndef BSLS_CONTRACTS_PRE\n"
                                           "#define BSLS_CONTRACTS_PRE(X)\n"
                                           "#endif\n\n";
                    a.InsertTextBefore(id.second.d_fullRange.getBegin(),
                                       macroDef);
                    a.report(p.first.location(), check_name, "CA01",
                             "Need to generate the macro");
                    break;
                }
            }
        }
    }
}

void report::operator()(Token const& token,
                        const MacroDefinition&,
                        SourceRange r,
                        MacroArgs const *args)
{
    if (a.is_component(token.getLocation())) {
        auto s = Lexer::getSpelling(token, m, a.context()->getLangOpts());
        if (0 == strcmp(s.data(), "BSLS_ASSERT") ||
            0 == strcmp(s.data(), "BSLS_ASSERT_OPT") ||
            0 == strcmp(s.data(), "BSLS_ASSERT_SAFE")) {
            d.d_asserts.push_back(r);
        }
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    analyser.onTranslationUnitDone += report(analyser);
    observer.onPPMacroExpands      += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &subscribe);

// ----------------------------------------------------------------------------
// Copyright (C) 2020 Bloomberg Finance L.P.
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
