// csamisc_donotuseendl.cpp                                           -*-C++-*-

#include <clang/AST/Expr.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <csabase_visitor.h>
#include <string>
#include <unordered_set>

using namespace clang;
using namespace clang::ast_matchers;
using namespace csabase;

static std::string const check_name("short-compare");

namespace
{

struct data
{
};

struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void operator()(const BinaryOperator *expr);
};

void report::operator()(const BinaryOperator *expr)
{
    if (expr->isComparisonOp()) {
        const auto *lhs = llvm::dyn_cast<ImplicitCastExpr>(expr->getLHS());
        const auto *rhs = llvm::dyn_cast<ImplicitCastExpr>(expr->getRHS());
        if (lhs && rhs) {
            const auto *lt = lhs->getType().getTypePtr()->getAs<BuiltinType>();
            const auto *rt = rhs->getType().getTypePtr()->getAs<BuiltinType>();
            if (lt && lt->getKind() == lt->Int &&
                rt && rt->getKind() == rt->Int) {
                const auto *ls = lhs->getSubExpr();
                const auto *rs = rhs->getSubExpr();
                const auto *lst =
                    ls->getType().getTypePtr()->getAs<BuiltinType>();
                const auto *rst =
                    rs->getType().getTypePtr()->getAs<BuiltinType>();
                if (lst && rst &&
                    ((lst->getKind() == lst->Short &&
                      rst->getKind() == rst->UShort) ||
                     (lst->getKind() == lst->UShort &&
                      rst->getKind() == rst->Short))) {
                    a.report(expr, check_name, "US01",
                             "Comparison between signed and unsigned short "
                             "may cause unexpected behavior");
                }
            }
        }
    }
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    visitor.onBinaryOperator += report(analyser);
}

}  // close anonymous namespace

// -----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &subscribe);

// ----------------------------------------------------------------------------
// Copyright (C) 2015 Bloomberg Finance L.P.
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
