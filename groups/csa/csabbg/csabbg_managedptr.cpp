// csabbg_managedptr.cpp                                              -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/DeclarationName.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchersInternal.h>
#include <clang/ASTMatchers/ASTMatchersMacros.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <csabase_visitor.h>
#include <set>
#include <string>
#include <utility>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::ast_matchers::internal;
using namespace csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("managed-pointer");

// ----------------------------------------------------------------------------

namespace clang {
namespace ast_matchers {

AST_MATCHER_P(CXXNewExpr, placementArgumentCountIs, unsigned, N) {
    return Node.getNumPlacementArgs() == N;
}

AST_MATCHER_P2(CXXNewExpr, hasPlacementArgument,
               unsigned, N,
               internal::Matcher<Expr>, InnerMatcher)
{
    return N < Node.getNumPlacementArgs() &&
           InnerMatcher.matches(
               *Node.getPlacementArg(N)->IgnoreParenImpCasts(),
               Finder, Builder);
}

}
}

namespace {

struct data
    // Data attached to analyzer for this check.
{
};

struct report : public Report<data>
    // Callback object invoked upon completion.
{
    INHERIT_REPORT_CTOR(report, Report, data);

    bool isSame(const Expr *e1, const Expr *e2);
        // Determine whether teh specified 'e1' and 'e2' represent the same
        // expression.  This is used to verify that in the two-argument managed
        // pointer constructor call, the same allocator is used and passed.

    void operator()();
        // Invoked to process reports.

    void operator()(const CXXConstructExpr *expr);
        // Invoked to process construct expressions.
};

bool report::isSame(const Expr *e1, const Expr *e2)
{
    static int d = 0;
    struct X { int *p; X(int *p) : p(p) { ++*p; } ~X() { --*p; } } x(&d);
    // This code is based on 'IdenticalExprChecker.cpp' from clang's analyzer.
    if (!e1 || !e2) return !e1 && !e2;
    e1 = e1->IgnoreParenImpCasts();
    e2 = e2->IgnoreParenImpCasts();
    if (e1->getStmtClass() != e2->getStmtClass()) return false;
    auto i1 = e1->child_begin();
    auto i2 = e2->child_begin();
    while (i1 != e1->child_end() && i2 != e2->child_end()) {
        if (!isSame(llvm::dyn_cast<Expr>(*i1++),
                    llvm::dyn_cast<Expr>(*i2++))) {
            return false;
        }
    }
    if (i1 != e1->child_end() || i2 != e2->child_end()) return false;

    switch (e1->getStmtClass()) {
      default:
        return false;
      case Stmt::ArraySubscriptExprClass:
      case Stmt::CXXDefaultArgExprClass:
      case Stmt::CallExprClass:
      case Stmt::ImplicitCastExprClass:
      case Stmt::ParenExprClass:
        return true;
      case Stmt::CStyleCastExprClass:
        return llvm::dyn_cast<CStyleCastExpr>(e1)->getTypeAsWritten() ==
               llvm::dyn_cast<CStyleCastExpr>(e2)->getTypeAsWritten();
      case Stmt::CompoundAssignOperatorClass:
      case Stmt::BinaryOperatorClass:
        return llvm::dyn_cast<BinaryOperator>(e1)->getOpcode() ==
               llvm::dyn_cast<BinaryOperator>(e2)->getOpcode();
      case Stmt::DeclRefExprClass:
        return llvm::dyn_cast<DeclRefExpr>(e1)->getDecl() ==
               llvm::dyn_cast<DeclRefExpr>(e2)->getDecl();
      case Stmt::MemberExprClass:
        return llvm::dyn_cast<MemberExpr>(e1)->getMemberDecl() ==
               llvm::dyn_cast<MemberExpr>(e2)->getMemberDecl();
      case Stmt::UnaryOperatorClass:
        return llvm::dyn_cast<UnaryOperator>(e1)->getOpcode() ==
               llvm::dyn_cast<UnaryOperator>(e2)->getOpcode();
    }
}

void report::operator()()
{
    auto        tu = a.context()->getTranslationUnitDecl();
    MatchFinder mf;

    OnMatch<> m0([&](const BoundNodes& nodes) {
        auto e   = nodes.getNodeAs<CXXConstructExpr>("e");
        auto p   = nodes.getNodeAs<Expr>("p");
        auto l   = nodes.getNodeAs<Expr>("l");
        auto ok1 = nodes.getNodeAs<CXXNewExpr>("ok1");
        auto ok2 = nodes.getNodeAs<CXXNewExpr>("ok2");
        auto ok3 = nodes.getNodeAs<CXXNewExpr>("ok3");
        auto n1  = nodes.getNodeAs<CXXNewExpr>("n1");
        auto n2  = nodes.getNodeAs<CXXNewExpr>("n2");
        auto n3  = nodes.getNodeAs<CXXNewExpr>("n3");

        if (ok1) {
            auto b = nodes.getNodeAs<BinaryOperator>("b");
            a.report(e, check_name, "MPOK1",
                     "ManagedPtr without deleter using default-assigned "
                     "allocator variable")
                << p->getSourceRange();
            a.report(b, check_name, "MPOK1",
                     "Assignment is here",
                     false, DiagnosticIDs::Note);
        }

        if (ok2) {
            auto v = nodes.getNodeAs<VarDecl>("v");
            a.report(e, check_name, "MPOK2",
                     "ManagedPtr without deleter using default-initialized "
                     "allocator variable")
                << p->getSourceRange();
            a.report(v, check_name, "MPOK2",
                     "Initialization is here",
                     false, DiagnosticIDs::Note);
        }

        if (ok3) {
            a.report(e, check_name, "MPOK3",
                "ManagedPtr without deleter using default allocator directly")
                << p->getSourceRange();
        }

        if (n1) {
            a.report(e, check_name, "MP01",
                     "ManagedPtr without deleter will use 'operator delete'")
                << n1->getSourceRange();
        }

        if (n2) {
            auto up = llvm::dyn_cast<UnaryOperator>(p);
            auto ul = llvm::dyn_cast<UnaryOperator>(l);
            if (up && up->getOpcode() == UO_Deref) {
                p = up->getSubExpr();
            }
            if (ul && ul->getOpcode() == UO_AddrOf) {
                l = ul->getSubExpr();
            }
            if (!isSame(p, l)) {
                a.report(e, check_name, "MP02",
                    "Allocator and deleter differ in MangedPtr construction")
                    << p->getSourceRange()
                    << l->getSourceRange();
            }
        }

        if (n3) {
            a.report(e, check_name, "MP03",
                     "Deleter provided for non-placement allocation in "
                     "MangedPtr construction")
                << n3->getSourceRange() << l->getSourceRange();
        }
    });

    // Apply 'test' to the first placement argument of the new expression, bind
    // "p" to the placement argument, and bind 'name' to the new expression.
    auto pa = [&](Matcher<Expr> test, const char *name) {
        return hasArgument(
            0,
            cxxNewExpr(hasPlacementArgument(0, expr(test).bind("p")))
                .bind(name));
    };

    // Apply 'test' to the dereferenced expression of the first placement
    // argument of the new expression, bind "p" to the placement argument, and
    // bind 'name' to the new expression.
    auto deref_pa = [&](Matcher<Expr> test, const char *name) {
        return pa(unaryOperator(hasOperatorName("*"),
                                hasUnaryOperand(ignoringParenImpCasts(test))),
                  name);
    };

    // Match a call to 'bslma::Default::allocator()'.
    auto da = [&] {
        return ignoringParenImpCasts(callExpr(callee(
            cxxMethodDecl(hasName("allocator"),
                          ofClass(hasName("BloombergLP::bslma::Default"))))));
    };

    // Match various styles of 'ManagedPtr' construyction.
    mf.addDynamicMatcher(
        decl(forEachDescendant(
            cxxConstructExpr(
                hasDeclaration(cxxConstructorDecl(
                    matchesName("::BloombergLP::bslma::ManagedPtr<"))),
                anyOf(
                    // No deleter, and allocation is through an allocator
                    // pointer that has been assigned the default allocator.
                    // This is likely OK.  Code looks like
                    //    bslma::Allocator *a;
                    //    a = bslma::Default::allocator();
                    //    ManagedPtr<int> mp(new (*a) int);
                    allOf(argumentCountIs(1),
                          deref_pa(
                              declRefExpr(to(varDecl(anything()).bind("v"))),
                              "ok1"),
                          hasAncestor(compoundStmt(hasAnySubstatement(
                              binaryOperator(hasOperatorName("="),
                                             hasLHS(declRefExpr(
                                                 to(equalsBoundNode("v")))),
                                             hasRHS(da()))
                                  .bind("b"))))),
                    // No deleter, and allocation is through an allocator
                    // pointer that has been initialized with the default
                    // allocator.  This is likely OK.  Code looks like
                    //    bslma::Allocator *a = bslma::Default::allocator();
                    //    ManagedPtr<int> mp(new (*a) int);
                    allOf(argumentCountIs(1),
                          deref_pa(declRefExpr(to(varDecl(hasInitializer(da()))
                                                      .bind("v"))),
                                   "ok2")),
                    // No deleter, and allocation is through the default
                    // allocator.  This is likely OK.  Code looks like
                    //    ManagedPtr<int>
                    //        mp(new (*bslma::Default::allocator()) int);
                    allOf(argumentCountIs(1), deref_pa(da(), "ok3")),
                    // No deleter, and allocation is through placement new, not
                    // matching the above.  Code looks like
                    //    bslma::TestAllocator ta;
                    //    ManagedPtr<int> mp(new (ta) int);
                    // or
                    //    void foo(bslma::Allocator *pa)
                    //    { ManagedPtr<int> mp(new (*pa) int); }
                    allOf(argumentCountIs(1), pa(anything(), "n1")),
                    // Deleter is present, and allocation is through placement
                    // new.  Handler will check for mismatch.  Code looks like
                    //    bslma::Allocator *a1, *a2;
                    //    ManagedPtr<int> mp(new (*a1) int, a2);
                    allOf(argumentCountIs(2),
                          pa(anything(), "n2"),
                          hasArgument(1, expr(anything()).bind("l"))),
                    // Deleter is present, and allocation is through
                    // non-placement new.  Code looks like
                    //    bslma::Allocator *a;
                    //    ManagedPtr<int> mp(new int, a);
                    allOf(argumentCountIs(2),
                          hasArgument(0,
                                      cxxNewExpr(placementArgumentCountIs(0))
                                          .bind("n3")),
                          hasArgument(1, expr(anything()).bind("l")))))
                .bind("e"))),
        &m0);
    mf.match(*tu, *a.context());
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &subscribe);

// ----------------------------------------------------------------------------
// Copyright (C) 2019 Bloomberg Finance L.P.
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
