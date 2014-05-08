// csastil_unnamed_temporary.cpp                                      -*-C++-*-

#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <functional>

#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("unnamed-temporary");

// ----------------------------------------------------------------------------

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::ast_matchers::internal;
using namespace csabase;

namespace clang {
namespace ast_matchers {

const internal::VariadicDynCastAllOfMatcher<Stmt, ExprWithCleanups> cleanups;

}
}

namespace
{

struct report
    // Callback object for detecting top-level unnamed temporaries.
{
    Analyser& d_analyser;

    report(Analyser& analyser);
        // Initialize an object of this type.

    void operator()();
        // Callback for the end of the translation unit.

    void match_unnamed_temporary(const BoundNodes &nodes);
        // Callback when unnamed temporaries are found.
};

report::report(Analyser& analyser)
: d_analyser(analyser)
{
}

const internal::DynTypedMatcher &
unnamed_temporary_matcher()
    // Return an AST matcher which looks for expression statements which
    // construct temporary objects.
{
    static const internal::DynTypedMatcher matcher = decl(forEachDescendant(
        cleanups(
            hasParent(stmt(unless(expr()))),
            anyOf(
                has(functionalCastExpr()),
                has(bindTemporaryExpr())
            )
        ).bind("ut")
    ));
    return matcher;
}

void report::match_unnamed_temporary(const BoundNodes &nodes)
{
    const clang::Expr* e = nodes.getNodeAs<Expr>("ut");

    d_analyser.report(e, check_name, "UT01",
                      "Unnamed object will be immediately destructed");
}

void report::operator()()
{
    MatchFinder mf;
    OnMatch<report, &report::match_unnamed_temporary> m1(this);
    mf.addDynamicMatcher(unnamed_temporary_matcher(), &m1);
    mf.match(*d_analyser.context()->getTranslationUnitDecl(),
             *d_analyser.context());
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static csabase::RegisterCheck c1(check_name, &subscribe);
