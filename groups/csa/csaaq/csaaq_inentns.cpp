// csaaq_inentns.cpp                                                  -*-C++-*-

#include <csabase_analyser.h>
#include <csabase_clang.h>
#include <csabase_config.h>
#include <csabase_debug.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_visitor.h>

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("in-enterprise-namespace");

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

    void operator()(const NamedDecl *decl);

    void operator()();
};

void report::operator()(const NamedDecl *decl)
{
    auto dn = decl->getDeclName();
    if (dn.getNameKind() == DeclarationName::CXXOperatorName) {
        switch (dn.getCXXOverloadedOperator()) {
          case OO_New:
          case OO_Array_New:
          case OO_Delete:
          case OO_Array_Delete:
            return;                                                   // RETURN
          default:
            break;
        }
    }
    if (const auto *fd = llvm::dyn_cast<FunctionDecl>(decl)) {
        if (fd->isMain() || fd->isExternC()) {
            return;
        }
    }
    if (const auto *vd = llvm::dyn_cast<VarDecl>(decl)) {
        if (vd->isExternC()) {
            return;
        }
    }
    if (!a.is_test_driver() &&
        decl->getLinkageInternal() == Linkage::ExternalLinkage &&
        !decl->isInAnonymousNamespace() &&
        !decl->isInStdNamespace() &&
        !decl->isCXXClassMember() &&
        !a.is_system_header(decl)) {
        const DeclContext *dc = llvm::dyn_cast<NamespaceDecl>(decl);
        if (!dc) {
            dc = decl->getDeclContext()->getEnclosingNamespaceContext();
        }
        if (dc && !dc->isTranslationUnit()) {
            for (;;) {
                auto pdc = dc->getParent()->getEnclosingNamespaceContext();
                if (pdc && !pdc->isTranslationUnit() && pdc != dc) {
                    dc = pdc;
                }
                else {
                    break;
                }
            }
        }
        const NamespaceDecl *nd = llvm::dyn_cast<NamespaceDecl>(dc);
        if (!nd) {
            auto report = a.report(decl, check_name, "AQQ01",
                     "Declaration in global, not ::%0 namespace", true);
            report << a.config()->toplevel_namespace();
        }
        else if (!nd->isAnonymousNamespace()) {
            std::string ns = nd->getNameAsString();
            if (!a.is_global_package(ns) &&
                !a.is_standard_namespace(ns) &&
                ns != a.config()->toplevel_namespace()) {
                auto builder = a.report(decl, check_name, "AQQ01",
                         "Declaration in %0, not ::%1 namespace", true);

                builder.value() << ns << a.config()->toplevel_namespace();
            }
        }
    }
}

// TranslationUnitDone
void report::operator()()
{
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
    visitor.onNamedDecl            += report(analyser);
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
