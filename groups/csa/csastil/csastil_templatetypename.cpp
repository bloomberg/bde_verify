// csastil_templatetypename.cpp                                       -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_util.h>
#include <csabase_registercheck.h>
#include <llvm/Support/Regex.h>
#include <clang/AST/ExprCXX.h>
#include <clang/Rewrite/Core/Rewriter.h>

using namespace clang;
using namespace clang::ast_matchers;
using namespace bde_verify::csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("template-typename");

// ----------------------------------------------------------------------------

namespace
{

struct data
{
    data();
};

data::data()
{
};

struct report
{
    report(Analyser& analyser);
    void match_has_template_parameters(const BoundNodes& nodes);
    void checkTemplateParameters(TemplateParameterList const* parms);
    void operator()();

    Analyser& d_analyser;
    data& d_data;
};

report::report(Analyser& analyser)
: d_analyser(analyser)
, d_data(analyser.attachment<data>())
{
}

const internal::DynTypedMatcher &
has_template_parameters_matcher()
    // Return an AST matcher which looks for declarations.
{
    static const internal::DynTypedMatcher matcher =
        decl(eachOf(
            forEachDescendant(decl().bind("decl")),
            forEachDescendant(lambdaExpr().bind("lambda"))
        ));
    return matcher;
}

void report::match_has_template_parameters(const BoundNodes& nodes)
{
    if (TemplateDecl const* decl = nodes.getNodeAs<TemplateDecl>("decl")) {
        checkTemplateParameters(decl->getTemplateParameters());
    }
    if (DeclaratorDecl const* decl = nodes.getNodeAs<DeclaratorDecl>("decl")) {
        unsigned n = decl->getNumTemplateParameterLists();
        for (unsigned i = 0; i < n; ++i) {
            checkTemplateParameters(decl->getTemplateParameterList(i));
        }
    }
    if (TagDecl const* decl = nodes.getNodeAs<TagDecl>("decl")) {
        unsigned n = decl->getNumTemplateParameterLists();
        for (unsigned i = 0; i < n; ++i) {
            checkTemplateParameters(decl->getTemplateParameterList(i));
        }
    }
    if (ClassTemplatePartialSpecializationDecl const* decl =
            nodes.getNodeAs<ClassTemplatePartialSpecializationDecl>("decl")) {
        checkTemplateParameters(decl->getTemplateParameters());
    }
    if (VarTemplatePartialSpecializationDecl const* decl =
            nodes.getNodeAs<VarTemplatePartialSpecializationDecl>("decl")) {
        checkTemplateParameters(decl->getTemplateParameters());
    }
    if (CXXRecordDecl const* decl = nodes.getNodeAs<CXXRecordDecl>("decl")) {
        checkTemplateParameters(decl->getGenericLambdaTemplateParameterList());
    }
    if (FriendDecl const* decl = nodes.getNodeAs<FriendDecl>("decl")) {
        unsigned n = decl->getFriendTypeNumTemplateParameterLists();
        for (unsigned i = 0; i < n; ++i) {
            checkTemplateParameters(
                decl->getFriendTypeTemplateParameterList(i));
        }
    }
    if (TemplateTemplateParmDecl const* decl =
            nodes.getNodeAs<TemplateTemplateParmDecl>("decl")) {
        unsigned n = decl->getNumExpansionTemplateParameters();
        for (unsigned i = 0; i < n; ++i) {
            checkTemplateParameters(decl->getExpansionTemplateParameters(i));
        }
    }
    if (LambdaExpr const* expr = nodes.getNodeAs<LambdaExpr>("lambda")) {
        checkTemplateParameters(expr->getTemplateParameterList());
    }
}

void report::operator()()
{
    MatchFinder mf;
    OnMatch<report, &report::match_has_template_parameters> m1(this);
    mf.addDynamicMatcher(has_template_parameters_matcher(), &m1);
    mf.match(*d_analyser.context()->getTranslationUnitDecl(),
             *d_analyser.context());
}

static llvm::Regex all_upper("^[[:upper:]](_?[[:upper:][:digit:]]+)*$");

void report::checkTemplateParameters(TemplateParameterList const* parms)
{
    unsigned n = parms ? parms->size() : 0;
    for (unsigned i = 0; i < n; ++i) {
        const TemplateTypeParmDecl *parm =
            llvm::dyn_cast<TemplateTypeParmDecl>(parms->getParam(i));
        if (parm) {
            if (parm->wasDeclaredWithTypename()) {
                 d_analyser.report(parm, check_name, "TY01",
                     "Template parameter uses 'typename' instead of 'class'");
                 SourceRange r = parm->getSourceRange();
                 llvm::StringRef s = d_analyser.get_source(r);
                 size_t to = s.find("typename");
                 if (to != s.npos) {
                     d_analyser.rewriter().ReplaceText(
                        r.getBegin().getLocWithOffset(to), 8, "class");
                 }
            }
            if (parm->getName().size() == 1) {
                 d_analyser.report(parm, check_name, "TY02",
                     "Template parameter uses single-letter name");
            }
            if (!all_upper.match(parm->getName())) {
                 d_analyser.report(parm, check_name, "TY03",
                     "Template parameter is not in ALL_CAPS format");
            }
        }
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver&)
{
    analyser.onTranslationUnitDone += report(analyser);
}

}

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &subscribe);
