// csamisc_hashptr.cpp                                                -*-C++-*-

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Type.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchersInternal.h>
#include <clang/ASTMatchers/ASTMatchersMacros.h>
#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/VariadicFunction.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <string>

namespace csabase { class PPObserver; }
namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::ast_matchers::internal;

// ----------------------------------------------------------------------------

static std::string const check_name("hash-pointer");

// ----------------------------------------------------------------------------

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

internal::DynTypedMatcher hash_char_ptr_matcher()
    // Return an AST matcher which looks for calls to std::hash<Type *>.
{
    return decl(forEachDescendant(
        callExpr(
            callee(functionDecl(
                hasName("operator()"),
                parameterCountIs(1),
                hasParent(recordDecl(hasName("std::hash"))),
                hasParameter(
                    0, hasType(pointerType(unless(anyOf(
                           pointee(asString("void")),
                           pointee(asString("const void")),
                           pointee(asString("volatile void")),
                           pointee(asString("const volatile void"))))))))))
            .bind("hash")));
}

void report::match_hash_char_ptr(const BoundNodes &nodes)
{
    const CallExpr *hash = nodes.getNodeAs<CallExpr>("hash");

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
