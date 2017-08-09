// csamisc_arrayargument.cpp                                          -*-C++-*-

#include <clang/AST/DeclTemplate.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_report.h>
#include <csabase_registercheck.h>
#include <csabase_visitor.h>

using namespace clang;
using namespace csabase;

static std::string const check_name("array-argument");

// ----------------------------------------------------------------------------

namespace
{

struct data
{
};

struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void operator()(const FunctionDecl *decl);

    void operator()(const FunctionTemplateDecl *decl);
};


void report::operator()(FunctionDecl const *decl)
{
    for (auto p : decl->parameters()) {
        QualType pt = p->getOriginalType();
        if (pt->isConstantArrayType() && pt.getAsString() != "va_list") {
            a.report(p, check_name, "AA01",
                     "Pointer parameter disguised as sized array");
        }
    }
}

void report::operator()(FunctionTemplateDecl const *decl)
{
    (*this)(decl->getTemplatedDecl());
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
// Hook up the callback functions.
{
    visitor.onFunctionDecl         += report(analyser);
    visitor.onFunctionTemplateDecl += report(analyser);
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
