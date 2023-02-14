// csaaq_runtimeinit.cpp                                              -*-C++-*-

#include <clang/Basic/PartialDiagnostic.h>
#include <csabase_analyser.h>
#include <csabase_clang.h>
#include <csabase_config.h>
#include <csabase_debug.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <csabase_visitor.h>
#include <set>

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("runtime-initialization");

// ----------------------------------------------------------------------------

namespace
{

struct BrieflyCPlusPlus11
    // Guard for turning on C++11 mode, so that EvaluateAsInitializer will do
    // its work even for arrays and records.
{
    LangOptions &victim;
    bool save;

    BrieflyCPlusPlus11(Analyser& a);
    ~BrieflyCPlusPlus11();
};

BrieflyCPlusPlus11::BrieflyCPlusPlus11(Analyser& a)
: victim(const_cast<LangOptions &>(a.context()->getLangOpts()))
, save(victim.CPlusPlus11)
{
    victim.CPlusPlus11 = true;
}

BrieflyCPlusPlus11::~BrieflyCPlusPlus11()
{
    victim.CPlusPlus11 = save;
}

struct data
    // Data attached to analyzer for this check.
{
    bool                      d_is_main = false;
    std::set<const VarDecl *> d_inits;
};

struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void operator()(const FunctionDecl *decl);
    void operator()(const VarDecl *decl);
    void operator()();
};

void report::operator()(const FunctionDecl *decl)
{
    if (decl->isMain()) {
        d.d_is_main = true;
    }
}

void report::operator()(const VarDecl *decl)
{
    BrieflyCPlusPlus11 guard(a);
    SmallVector<PartialDiagnosticAt, 7> n;
    APValue v;
    if (!a.is_test_driver(decl) &&
        decl->isFileVarDecl() &&
        decl->hasInit() &&
        !decl->getType()->isDependentType() &&
        !decl->getInit()->isValueDependent() &&
        !decl->checkForConstantInitialization(n) &&
        !decl->getInit()->EvaluateAsInitializer(v, *a.context(),
                                                decl, n, true)) {
        d.d_inits.insert(decl);
    }
}

void report::operator()()
{
    std::set<const VarDecl *, csabase::SortByLocation> vars(
        d.d_inits.begin(), d.d_inits.end(), csabase::SortByLocation(m));
    if (d.d_is_main) {
        for (auto decl : vars) {
            a.report(decl, check_name, "AQa02",
                     "Global variable '%0' of type %1 with runtime "
                     "initialization in translation unit with main()")
                << decl->getName() << decl->getType();
        }
    }
    else {
        for (auto decl : vars) {
            a.report(decl, check_name, "AQa01",
                     "Global variable '%0' of type %1 with runtime "
                     "initialization in translation unit without main()")
                << decl->getName() << decl->getType();
        }
    }
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
    visitor.onVarDecl += report(analyser);
    visitor.onFunctionDecl += report(analyser);
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
