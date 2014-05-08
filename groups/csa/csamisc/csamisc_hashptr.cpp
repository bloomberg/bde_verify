// csastil_hashptr.cpp                                                -*-C++-*-

#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <functional>

#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("hash-pointer");

// ----------------------------------------------------------------------------

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::ast_matchers::internal;
using namespace csabase;

namespace
{

struct report
    // Callback object for detecting calls to std::hash<const char *>().
{
    Analyser& d_analyser;

    report(Analyser& analyser);
        // Initialize an object of this type.

    void operator()();
        // Callback for the end of the translation unit.

    void match_hash_char_ptr(const BoundNodes &nodes);
        // Callback when matching calls are found.
};

report::report(Analyser& analyser)
: d_analyser(analyser)
{
}

const internal::DynTypedMatcher &
hash_char_ptr_matcher()
    // Return an AST matcher which looks for calls to std::hash<Type *>.
{
    static const internal::DynTypedMatcher matcher = decl(forEachDescendant(
        callExpr(callee(functionDecl(
            hasName("operator()"),
            parameterCountIs(1),
            hasParent(recordDecl(hasName("std::hash"))),
            hasParameter(0, hasType(pointerType(unless(anyOf(
                pointee(asString("void")),
                pointee(asString("const void")),
                pointee(asString("volatile void")),
                pointee(asString("const volatile void"))
            )))))
        ))).bind("hash")
    ));
    return matcher;
}

void report::match_hash_char_ptr(const BoundNodes &nodes)
{
    const clang::CallExpr *hash = nodes.getNodeAs<CallExpr>("hash");

    d_analyser.report(hash, check_name, "HC01",
                      "Hash will depend only on the pointer value, not the "
                      "contents, of the argument");
}

void report::operator()()
{
    MatchFinder mf;
    OnMatch<report, &report::match_hash_char_ptr> m1(this);
    mf.addDynamicMatcher(hash_char_ptr_matcher(), &m1);
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
