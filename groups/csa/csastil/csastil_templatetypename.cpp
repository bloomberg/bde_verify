// csastil_templatetypename.cpp                                       -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#include <csabase_cast_ptr.h>
#include <llvm/Support/Regex.h>
#include <clang/Rewrite/Core/Rewriter.h>

using namespace clang;
using namespace bde_verify::csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("template-typename");

// ----------------------------------------------------------------------------

static llvm::Regex all_upper("^[[:upper:]](_?[[:upper:][:digit:]]+)*$");

static void
checkTemplateParameters(Analyser& analyser, TemplateParameterList const* parms)
{
    unsigned n = parms->size();
    for (unsigned i = 0; i < n; ++i) {
        const TemplateTypeParmDecl *parm =
            llvm::dyn_cast<TemplateTypeParmDecl>(parms->getParam(i));
        if (parm) {
            if (parm->wasDeclaredWithTypename()) {
                 analyser.report(parm, check_name, "TY01",
                     "Template parameter uses 'typename' instead of 'class'");
                 SourceRange r = parm->getSourceRange();
                 llvm::StringRef s = analyser.get_source(r);
                 size_t to = s.find("typename");
                 if (to != s.npos) {
                     analyser.rewriter().ReplaceText(
                        r.getBegin().getLocWithOffset(to), 8, "class");
                 }
            }
            if (parm->getName().size() == 1) {
                 analyser.report(parm, check_name, "TY02",
                     "Template parameter uses single-letter name");
            }
            if (!all_upper.match(parm->getName())) {
                 analyser.report(parm, check_name, "TY03",
                     "Template parameter is not in ALL_CAPS format");
            }
        }
    }
}

static void
check_t(Analyser& analyser, TemplateDecl const* decl)
{
    checkTemplateParameters(analyser, decl->getTemplateParameters());
}

static void
check_ctps(Analyser& analyser,
              ClassTemplatePartialSpecializationDecl const* decl)
{
    checkTemplateParameters(analyser, decl->getTemplateParameters());
}

static void
check_vtps(Analyser& analyser,
              VarTemplatePartialSpecializationDecl const* decl)
{
    checkTemplateParameters(analyser, decl->getTemplateParameters());
}

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &check_t);
static RegisterCheck c2(check_name, &check_ctps);
static RegisterCheck c3(check_name, &check_vtps);
