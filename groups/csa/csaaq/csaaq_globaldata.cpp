// csaaq_globaldata.cpp                                               -*-C++-*-

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_visitor.h>
#include <clang/AST/DeclTemplate.h>

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("global-data");

// ----------------------------------------------------------------------------

namespace
{

struct data
    // Data attached to analyzer for this check.
{
};

struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void operator()(const VarDecl *decl);
};

void report::operator()(const VarDecl *decl)
{
    StorageClass sc = decl->getStorageClass();
    if (!a.is_test_driver(decl) &&
        !llvm::dyn_cast<VarTemplateSpecializationDecl>(decl) &&
        decl->isFileVarDecl() &&
        decl->isExternallyVisible() &&
        sc != SC_Static &&
        !decl->isStaticDataMember()) {
        a.report(decl, check_name, "AQb01",
                 "Data variable with global visibilty");
    }
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    visitor.onVarDecl += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

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
