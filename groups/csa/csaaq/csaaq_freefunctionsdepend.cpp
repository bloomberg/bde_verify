// csaaq_freefunctionsdepend.cpp                                      -*-C++-*-

#include <csabase_analyser.h>
#include <csabase_clang.h>
#include <csabase_config.h>
#include <csabase_debug.h>
#include <csabase_format.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_visitor.h>
#include <map>

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("free-functions-depend");

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

    bool depends(SourceLocation sl, QualType type);

    bool isFree(const FunctionDecl *decl);

    void operator()(const FunctionDecl *decl);

    void operator()(const FunctionTemplateDecl *decl);
};

bool report::isFree(const FunctionDecl *decl)
{
    const DeclContext *dc = decl->getDeclContext();
    while (dc->getDeclKind() == Decl::LinkageSpec) {
        dc = dc->getParent();
    }
    return dc->isFileContext();
}

bool report::depends(SourceLocation sl, QualType type)
{
    type = type.getDesugaredType(*a.context());

    QualType sub;
    while (!(sub = type->getPointeeType()).isNull()) {
        type = sub.getDesugaredType(*a.context());
    }
    if (type->isBuiltinType()) {
        return false;
    }
    if (type->getAs<TemplateTypeParmType>()) {
        return false;
    }
    if (auto dnt = type->getAs<DependentNameType>()) {
        if (auto nns = dnt->getQualifier()) {
            if (auto t = nns->getAsType()) {
                if (depends(sl, QualType(t, 0))) {
                    return true;
                }
            }
        }
        return false;
    }
    if (auto ft = type->getAs<FunctionProtoType>()) {
        unsigned n = ft->getNumParams();
        for (unsigned i = 0; i < n; ++i) {
            if (depends(sl, ft->getParamType(i))) {
                return true;
            }
        }
        return false;
    }
    //auto rd = type->getAsCXXRecordDecl();
    auto rd = type->getAsTagDecl();
    if (auto tspt = type->getAs<TemplateSpecializationType>()) {
        if (auto td = tspt->getTemplateName().getAsTemplateDecl()) {
            unsigned n = tspt->getNumArgs();
            for (unsigned i = 0; i < n; ++i) {
                auto &ta = tspt->getArg(i);
                if (ta.getKind() == ta.Type &&
                    depends(sl, ta.getAsType())) {
                    return true;
                }
            }
            if (auto ct = llvm::dyn_cast<ClassTemplateDecl>(td)) {
                rd = ct->getTemplatedDecl();
            }
        }
    }
    while (rd) {
        if (auto def = rd->getDefinition()) {
            if (m.getFileID(m.getExpansionLoc(def->getLocation())) ==
                m.getFileID(sl)) {
                return true;
            }
        }
        if (auto tpl = llvm::dyn_cast<ClassTemplateSpecializationDecl>(rd)) {
            auto &tl = tpl->getTemplateArgs();
            unsigned n = tl.size();
            for (unsigned i = 0; i < n; ++i) {
                auto &ta = tl.get(i);
                if (ta.getKind() == ta.Type && depends(sl, ta.getAsType())) {
                    return true;
                }
            }
            rd = tpl->getSpecializedTemplate()->getTemplatedDecl();
        }
        else {
            rd = 0;
        }
    }
    return false;
}

void report::operator()(const FunctionDecl *decl)
{
    if (decl->isExternC() ||
        a.is_test_driver() ||
        a.is_global_name(decl)) {
        return;
    }
    if (auto ns = llvm::dyn_cast<NamespaceDecl>(
            decl->getDeclContext()->getEnclosingNamespaceContext())) {
        if (ns->isAnonymousNamespace() ||
            a.is_standard_namespace(ns->getNameAsString())) {
            return;
        }
    }
    SourceLocation sl = m.getExpansionLoc(decl->getLocation());
    if (a.is_header(m.getFilename(sl).str()) &&
        !a.is_system_header(m.getFilename(sl)) &&
        isFree(decl) &&
        !depends(sl, decl->getTypeSourceInfo()->getType())) {
        a.report(sl, check_name, "AQS01",
                 "Free function must have a parameter using a type defined by "
                 "this file", true);
    }
}

void report::operator()(const FunctionTemplateDecl *decl)
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
